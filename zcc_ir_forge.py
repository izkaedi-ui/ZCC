#!/usr/bin/env python3
"""
zcc_ir_forge.py — ZCC IR Mutation Forge
========================================
Async pipeline:
  1. Subprocess ZCC with ZCC_EMIT_IR=1, capture IR text stream
  2. Parse IR text → IRModule (Python dataclasses)
  3. PRIME energy-score each function (Hamiltonian field over IR graph)
  4. Serialize top-N functions as JSON, send to local Qwen (Ollama / llama.cpp)
  5. Stream mutation suggestions back, write JSONL report

Usage:
  # Default: Ollama backend, top-5 highest-energy functions
  python zcc_ir_forge.py

  # With options:
  python zcc_ir_forge.py \
      --zcc ./zcc \
      --src zcc_pp.c ir.c \
      --backend ollama \
      --model qwen2.5-coder:7b \
      --top-n 10 \
      --output mutations.jsonl \
      --prime-eta 0.4 --prime-gamma 0.3 --prime-beta 0.1 --prime-sigma 0.05

Supported backends:
  ollama    http://localhost:11434  (Ollama OpenAI-compat /api/chat)
  llama     http://localhost:8080   (llama.cpp server /v1/chat/completions)
  openai    https://api.openai.com  (any OpenAI-compat endpoint via OPENAI_BASE_URL)
"""

from __future__ import annotations

import argparse
import asyncio
import dataclasses
import json
import math
import os
import sys
import time
from typing import Any, AsyncIterator, Iterator, List, Optional

import aiohttp

# ── ANSI colors ───────────────────────────────────────────────────────────────

CYAN  = "\033[96m"
MAG   = "\033[95m"
YEL   = "\033[93m"
GRN   = "\033[92m"
RED   = "\033[91m"
DIM   = "\033[2m"
BOLD  = "\033[1m"
RST   = "\033[0m"

def _c(color: str, s: str) -> str:
    return f"{color}{s}{RST}" if sys.stdout.isatty() else s

# ── IR dataclasses ────────────────────────────────────────────────────────────

TERMINATOR_OPS = {"RET", "BR", "BR_IF"}
BRANCH_OPS     = {"BR", "BR_IF"}
MEMORY_OPS     = {"ALLOCA", "LOAD", "STORE"}
CALL_OPS       = {"CALL"}
CONST_OPS      = {"CONST", "CONST_STR"}
ARITH_OPS      = {"ADD", "SUB", "MUL", "DIV", "MOD", "NEG"}
BITWISE_OPS    = {"AND", "OR", "XOR", "NOT", "SHL", "SHR"}
CMP_OPS        = {"EQ", "NE", "LT", "LE", "GT", "GE"}
CAST_OPS       = {"CAST", "COPY"}
PHI_OPS        = {"PHI"}


@dataclasses.dataclass
class IRNode:
    op:     str
    type:   str
    dst:    str
    src1:   str
    src2:   str
    label:  str
    imm:    Optional[int]
    lineno: Optional[int]

    def to_dict(self) -> dict:
        d: dict = {"op": self.op, "type": self.type}
        if self.dst:    d["dst"]    = self.dst
        if self.src1:   d["src1"]   = self.src1
        if self.src2:   d["src2"]   = self.src2
        if self.label:  d["label"]  = self.label
        if self.imm  is not None: d["imm"]    = self.imm
        if self.lineno is not None and self.lineno > 0:
            d["line"] = self.lineno
        return d


@dataclasses.dataclass
class IRFunction:
    name:     str
    ret_type: str
    nodes:    List[IRNode] = dataclasses.field(default_factory=list)

    # PRIME scores (populated by PRIMEScorer)
    prime_h0:     float = 0.0
    prime_h_final: float = 0.0
    prime_energy:  float = 0.0   # convergence energy — primary sort key

    # Feature counts (populated during parse + scoring)
    branch_count: int = 0
    call_count:   int = 0
    phi_count:    int = 0
    const_count:  int = 0
    memory_count: int = 0
    arith_count:  int = 0

    @property
    def node_count(self) -> int:
        return len(self.nodes)

    def to_dict(self) -> dict:
        return {
            "name":     self.name,
            "ret_type": self.ret_type,
            "nodes":    [n.to_dict() for n in self.nodes],
            "stats": {
                "node_count":    self.node_count,
                "branch_count":  self.branch_count,
                "call_count":    self.call_count,
                "phi_count":     self.phi_count,
                "const_count":   self.const_count,
                "memory_count":  self.memory_count,
                "arith_count":   self.arith_count,
                "prime_h0":      round(self.prime_h0,    4),
                "prime_h_final": round(self.prime_h_final, 4),
                "prime_energy":  round(self.prime_energy,  4),
            },
        }


@dataclasses.dataclass
class IRModule:
    funcs:   List[IRFunction] = dataclasses.field(default_factory=list)
    source:  str = ""
    elapsed: float = 0.0

    @property
    def total_nodes(self) -> int:
        return sum(f.node_count for f in self.funcs)

    def to_dict(self) -> dict:
        return {
            "source":      self.source,
            "func_count":  len(self.funcs),
            "total_nodes": self.total_nodes,
            "elapsed_s":   round(self.elapsed, 3),
        }


# ── IR Text Parser ────────────────────────────────────────────────────────────
#
# Parses our exact text format (from ir.c ir_func_emit_text):
#
#   ; ZCC IR module  funcs=N
#   ; func <name> -> <ret_type>
#     <OP>      <type>  <dst>  <src1>  <src2>  <label>  [imm=N]  [; line N]
#   ; end <name>  nodes=N
#
# Fields that are "-" are stored as empty string.
# Instruction lines start with whitespace.
# Comment lines start with ";".


def _dash(s: str) -> str:
    """'-' sentinel → empty string."""
    return "" if s == "-" else s


def _parse_node_line(line: str) -> Optional[IRNode]:
    """Parse one instruction line. Returns None if not an instruction."""
    stripped = line.lstrip()
    if not stripped or stripped.startswith(";"):
        return None

    # Tokenize — split on runs of whitespace
    tokens = stripped.split()
    if not tokens:
        return None

    op    = tokens[0] if len(tokens) > 0 else "NOP"
    ty    = _dash(tokens[1]) if len(tokens) > 1 else ""
    dst   = _dash(tokens[2]) if len(tokens) > 2 else ""
    src1  = _dash(tokens[3]) if len(tokens) > 3 else ""
    src2  = _dash(tokens[4]) if len(tokens) > 4 else ""
    label = _dash(tokens[5]) if len(tokens) > 5 else ""

    imm:    Optional[int] = None
    lineno: Optional[int] = None

    # Scan remaining tokens for imm=N and ; line N
    i = 6
    while i < len(tokens):
        tok = tokens[i]
        if tok.startswith("imm="):
            try:
                imm = int(tok[4:])
            except ValueError:
                pass
        elif tok == ";" and i + 2 < len(tokens) and tokens[i + 1] == "line":
            try:
                lineno = int(tokens[i + 2])
            except ValueError:
                pass
            i += 2
        i += 1

    return IRNode(op=op, type=ty, dst=dst, src1=src1, src2=src2,
                  label=label, imm=imm, lineno=lineno)


def parse_ir_text(text: str) -> IRModule:
    """Parse full IR module text → IRModule."""
    mod  = IRModule()
    cur: Optional[IRFunction] = None

    for raw_line in text.splitlines():
        line = raw_line.rstrip()

        # Module header
        if line.startswith("; ZCC IR module"):
            continue

        # Function header: "; func <name> -> <ret_type>"
        if line.startswith("; func "):
            rest = line[7:]  # "<name> -> <ret_type>"
            parts = rest.split(" -> ", 1)
            name = parts[0].strip()
            ret_type = parts[1].strip() if len(parts) > 1 else "void"
            cur = IRFunction(name=name, ret_type=ret_type)
            mod.funcs.append(cur)
            continue

        # Function footer: "; end <name>  nodes=N"
        if line.startswith("; end "):
            cur = None
            continue

        # Skip standalone comments and blank lines
        if not line.strip() or line.strip().startswith(";"):
            continue

        # Instruction line (must start with whitespace)
        if cur is not None and (raw_line.startswith(" ") or raw_line.startswith("\t")):
            node = _parse_node_line(line)
            if node:
                cur.nodes.append(node)
                # Accumulate feature counts
                if node.op in BRANCH_OPS:
                    cur.branch_count += 1
                if node.op in CALL_OPS:
                    cur.call_count += 1
                if node.op in PHI_OPS:
                    cur.phi_count += 1
                if node.op in CONST_OPS:
                    cur.const_count += 1
                if node.op in MEMORY_OPS:
                    cur.memory_count += 1
                if node.op in ARITH_OPS:
                    cur.arith_count += 1

    return mod


# ── PRIME Energy Scorer ───────────────────────────────────────────────────────
#
# Maps each IR function onto a Hamiltonian energy scalar H:
#
#   H_t = H_base + η · H_{t-1} · σ(γ · H_{t-1}) + ε · N(0, 1 + β|H_{t-1}|)
#
# H_base is derived from function features.
# High H_final = high mutation priority (complex, energetic code).
#
# Production defaults: η=0.4, γ=0.3, β=0.1, σ=0.05
# These match ZCC compiler_passes.c defaults from the roadmap.


def _sigmoid(x: float) -> float:
    try:
        return 1.0 / (1.0 + math.exp(-x))
    except OverflowError:
        return 0.0 if x < 0 else 1.0


class PRIMEScorer:
    def __init__(
        self,
        eta:   float = 0.4,
        gamma: float = 0.3,
        beta:  float = 0.1,
        sigma: float = 0.05,
        iters: int   = 200,
        seed:  int   = 42,
    ):
        self.eta   = eta
        self.gamma = gamma
        self.beta  = beta
        self.sigma = sigma
        self.iters = iters
        # Deterministic pseudo-RNG (no numpy dependency)
        self._rng  = seed

    def _rand(self) -> float:
        """Box-Muller via xorshift64 — deterministic, no external deps."""
        # xorshift64
        x = self._rng
        x ^= (x << 13) & 0xFFFFFFFFFFFFFFFF
        x ^= (x >> 7)  & 0xFFFFFFFFFFFFFFFF
        x ^= (x << 17) & 0xFFFFFFFFFFFFFFFF
        self._rng = x
        # Uniform [0,1)
        u = (x & 0xFFFFFFFF) / 0x100000000
        # Second uniform for Box-Muller
        x2 = self._rng
        x2 ^= (x2 << 13) & 0xFFFFFFFFFFFFFFFF
        x2 ^= (x2 >> 7)  & 0xFFFFFFFFFFFFFFFF
        x2 ^= (x2 << 17) & 0xFFFFFFFFFFFFFFFF
        self._rng = x2
        v = (x2 & 0xFFFFFFFF) / 0x100000000
        # Guard against log(0)
        u = max(u, 1e-12)
        return math.sqrt(-2.0 * math.log(u)) * math.cos(2.0 * math.pi * v)

    def _h_base(self, fn: IRFunction) -> float:
        """
        Compute H_base from function features.
        Normalized to (0, 1) range via tanh.
        Feature weights are calibrated to ZCC's typical function complexity.
        """
        n = fn.node_count
        if n == 0:
            return 0.01

        # Raw energy contributions
        raw = (
            0.40 * (n / 200.0)                          # node count (200 = large fn)
          + 0.20 * (fn.branch_count  / max(n, 1))       # branch density
          + 0.15 * (fn.call_count    / max(n, 1))        # call density
          + 0.10 * (fn.phi_count     / max(n, 1))        # SSA complexity
          + 0.08 * (fn.memory_count  / max(n, 1))        # memory pressure
          + 0.07 * (fn.arith_count   / max(n, 1))        # arithmetic intensity
        )
        # Normalize to (0, 1) via tanh — never exactly 0 or 1
        return float(math.tanh(raw * 2.0))

    def score(self, fn: IRFunction) -> None:
        """Compute PRIME energy and write results back into fn."""
        H_base = self._h_base(fn)
        H      = H_base
        H_prev = H

        for _ in range(self.iters):
            noise = self.sigma * self._rand() * (1.0 + self.beta * abs(H))
            H_new = H_base + self.eta * H * _sigmoid(self.gamma * H) + noise

            # Convergence check
            if abs(H_new - H) < 1e-7:
                H = H_new
                break
            H_prev = H
            H      = H_new

        fn.prime_h0      = H_base
        fn.prime_h_final = H
        fn.prime_energy  = abs(H)   # unsigned magnitude for priority sort

    def score_module(self, mod: IRModule) -> None:
        """Score all functions in a module, in-place."""
        for fn in mod.funcs:
            self.score(fn)

    def top_n(self, mod: IRModule, n: int) -> List[IRFunction]:
        """Return top-N highest-energy functions (mutation priority order)."""
        return sorted(mod.funcs, key=lambda f: f.prime_energy, reverse=True)[:n]


# ── ZCC Subprocess Runner ─────────────────────────────────────────────────────


async def run_zcc(
    zcc_binary: str,
    source_files: List[str],
    extra_env: Optional[dict] = None,
    timeout: float = 120.0,
) -> tuple[str, str, int]:
    """
    Run ZCC_EMIT_IR=1 ./zcc <sources>.
    Returns (stdout_text, stderr_text, returncode).
    Streams stderr live so the user sees compilation progress.
    """
    env = os.environ.copy()
    env["ZCC_EMIT_IR"] = "1"
    if extra_env:
        env.update(extra_env)

    cmd = [zcc_binary, "-c"] + source_files
    print(_c(CYAN, f"🔧 ZCC: ") + " ".join(cmd))
    print(_c(DIM, f"   ZCC_EMIT_IR=1  timeout={timeout}s"))

    t0 = time.monotonic()
    proc = await asyncio.create_subprocess_exec(
        *cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE,
        env=env,
    )

    try:
        stdout_bytes, stderr_bytes = await asyncio.wait_for(
            proc.communicate(), timeout=timeout
        )
    except asyncio.TimeoutError:
        proc.kill()
        await proc.communicate()
        raise RuntimeError(f"ZCC timed out after {timeout}s")

    elapsed = time.monotonic() - t0
    rc      = proc.returncode
    stdout  = stdout_bytes.decode("utf-8", errors="replace")
    stderr  = stderr_bytes.decode("utf-8", errors="replace")

    if rc != 0:
        print(_c(RED, f"✗ ZCC exited {rc}"))
        if stderr:
            print(_c(DIM, stderr[:500]))
    else:
        func_lines = [l for l in stdout.splitlines() if l.startswith("; func ")]
        print(_c(GRN, f"✓ ZCC OK  ({elapsed:.2f}s)  functions={len(func_lines)}  "
                      f"ir_lines={len(stdout.splitlines())}"))

    return stdout, stderr, rc


# ── Local Model Client ────────────────────────────────────────────────────────
#
# Supports:
#   - Ollama   (http://localhost:11434/api/chat)   — native Ollama format
#   - llama.cpp (http://localhost:8080/v1/chat/completions) — OpenAI compat
#   - Any OpenAI-compat server via OPENAI_BASE_URL env var


BACKEND_URLS = {
    "ollama": "http://localhost:11434",
    "llama":  "http://localhost:8080",
    "openai": os.environ.get("OPENAI_BASE_URL", "https://api.openai.com"),
}


class LocalModelClient:
    def __init__(
        self,
        backend: str = "ollama",
        model:   str = "qwen2.5-coder:7b",
        base_url: Optional[str] = None,
        timeout: float = 180.0,
        max_tokens: int = 8192,
    ):
        self.backend    = backend
        self.model      = model
        self.base_url   = (base_url or BACKEND_URLS.get(backend, BACKEND_URLS["ollama"])).rstrip("/")
        self.timeout    = aiohttp.ClientTimeout(total=timeout)
        self.max_tokens = max_tokens

    def _build_payload(self, system: str, user: str, stream: bool,
                       max_tokens: int = 0) -> tuple[str, dict]:
        """Return (endpoint_path, json_body) for the configured backend."""
        mt = max_tokens if max_tokens > 0 else self.max_tokens
        messages = [
            {"role": "system",  "content": system},
            {"role": "user",    "content": user},
        ]

        if self.backend == "ollama":
            return "/api/chat", {
                "model":    self.model,
                "messages": messages,
                "stream":   stream,
                "options":  {"temperature": 0.2, "num_predict": mt},
            }
        else:
            # OpenAI-compat (llama.cpp, openai, etc.)
            return "/v1/chat/completions", {
                "model":       self.model,
                "messages":    messages,
                "temperature": 0.2,
                "max_tokens":  mt,
                "stream":      stream,
            }

    async def complete(self, system: str, user: str) -> str:
        """Non-streaming completion. Returns full response text."""
        path, payload = self._build_payload(system, user, stream=False)
        url = self.base_url + path

        connector = aiohttp.TCPConnector(ssl=False)
        async with aiohttp.ClientSession(timeout=self.timeout, connector=connector) as sess:
            async with sess.post(url, json=payload) as resp:
                if resp.status != 200:
                    body = await resp.text()
                    raise RuntimeError(
                        f"Model backend {url} returned HTTP {resp.status}: {body[:200]}"
                    )
                data = await resp.json()

        # Extract text from either Ollama or OpenAI response shape
        if self.backend == "ollama":
            return data.get("message", {}).get("content", "")
        else:
            choices = data.get("choices", [])
            if not choices:
                return ""
            return choices[0].get("message", {}).get("content", "")

    async def stream_complete(self, system: str, user: str) -> AsyncIterator[str]:
        """Streaming completion — yields text chunks as they arrive."""
        path, payload = self._build_payload(system, user, stream=True)
        url = self.base_url + path

        connector = aiohttp.TCPConnector(ssl=False)
        async with aiohttp.ClientSession(timeout=self.timeout, connector=connector) as sess:
            async with sess.post(url, json=payload) as resp:
                if resp.status != 200:
                    body = await resp.text()
                    raise RuntimeError(
                        f"Model backend {url} returned HTTP {resp.status}: {body[:200]}"
                    )
                async for raw_line in resp.content:
                    line = raw_line.decode("utf-8", errors="replace").strip()
                    if not line:
                        continue

                    # Ollama streams raw JSON objects (not SSE)
                    if self.backend == "ollama":
                        try:
                            obj = json.loads(line)
                            chunk = obj.get("message", {}).get("content", "")
                            if chunk:
                                yield chunk
                        except json.JSONDecodeError:
                            pass

                    # OpenAI-compat streams SSE: "data: {...}"
                    else:
                        if line.startswith("data: "):
                            line = line[6:]
                        if line == "[DONE]":
                            break
                        try:
                            obj   = json.loads(line)
                            delta = obj.get("choices", [{}])[0].get("delta", {})
                            chunk = delta.get("content", "")
                            if chunk:
                                yield chunk
                        except (json.JSONDecodeError, IndexError):
                            pass


# ── Mutation Prompt Builder ───────────────────────────────────────────────────

SYSTEM_PROMPT = """\
You are ZKAEDI PRIME — a Hamiltonian-guided compiler optimization oracle.
You analyze ZCC IR (3-address intermediate representation) and identify:

1. DEAD CODE — nodes whose results are never used; candidate for DCE pass (P4-IR).
2. CONSTANT FOLDS — subgraphs that evaluate to a compile-time constant (P5-IR).
3. REGISTER PRESSURE — variables that should be spilled vs kept in registers (P6-IR).
4. WASM STRUCTURAL NOTES — control flow patterns that map cleanly to Relooper blocks (P3-WASM).
5. PRIME ANOMALIES — any node pattern that suggests unusually high energy in the Hamiltonian landscape.

Output format: strict JSON, no preamble, no markdown fences.
Schema:
{
  "function": "<name>",
  "prime_energy": <float>,
  "dead_code": [{"dst": "<var>", "reason": "<one sentence>"}],
  "constant_folds": [{"nodes": ["<dst1>","<dst2>"], "folded_value": <int_or_null>, "reason": "<one sentence>"}],
  "register_pressure": {"hot_vars": ["<var>"], "spill_candidates": ["<var>"]},
  "wasm_notes": "<one paragraph or empty string>",
  "prime_anomalies": ["<observation>"],
  "mutation_score": <int 0-100>
}
"""


def build_user_prompt(fn: IRFunction) -> str:
    fn_json = json.dumps(fn.to_dict(), indent=2)
    return (
        f"Analyze this ZCC IR function. "
        f"PRIME energy score: {fn.prime_energy:.4f} (higher = higher mutation priority).\n\n"
        f"{fn_json}"
    )


# ── Mutation Forge ────────────────────────────────────────────────────────────


class MutationForge:
    def __init__(
        self,
        client:  LocalModelClient,
        scorer:  PRIMEScorer,
        top_n:   int = 5,
        output:  str = "mutations.jsonl",
        verbose: bool = True,
        debug_responses: bool = False,
    ):
        self.client  = client
        self.scorer  = scorer
        self.top_n   = top_n
        self.output  = output
        self.verbose = verbose
        self.debug_responses = debug_responses
        self._raw_fh = None

    def _log(self, msg: str) -> None:
        if self.verbose:
            print(msg)

    def _print_banner(self, mod: IRModule) -> None:
        w = 60
        print(_c(CYAN, "─" * w))
        print(_c(BOLD, " 🔱 ZCC IR MUTATION FORGE"))
        print(_c(CYAN, "─" * w))
        print(f"  source     : {mod.source}")
        print(f"  functions  : {len(mod.funcs)}")
        print(f"  total nodes: {mod.total_nodes}")
        print(f"  zcc elapsed: {mod.elapsed:.2f}s")
        print(f"  backend    : {self.client.backend} / {self.client.model}")
        print(f"  top-n      : {self.top_n}")
        print(f"  output     : {self.output}")
        print(_c(CYAN, "─" * w))

    def _print_prime_table(self, targets: List[IRFunction]) -> None:
        print(_c(MAG, "\nPRIME Energy Ranking (mutation targets):"))
        print(_c(DIM, f"  {'#':<3}  {'function':<30}  {'H₀':>7}  {'H_f':>7}  "
                      f"{'energy':>7}  {'nodes':>5}  {'branches':>8}"))
        print(_c(DIM, "  " + "─" * 68))
        for i, fn in enumerate(targets, 1):
            bar_len = int(fn.prime_energy * 20)
            bar = "█" * bar_len + "░" * (20 - bar_len)
            energy_color = RED if fn.prime_energy > 0.7 else YEL if fn.prime_energy > 0.4 else GRN
            print(
                f"  {i:<3}  {fn.name:<30}  "
                f"{fn.prime_h0:>7.4f}  {fn.prime_h_final:>7.4f}  "
                + _c(energy_color, f"{fn.prime_energy:>7.4f}") +
                f"  {fn.node_count:>5}  {fn.branch_count:>8}"
                + _c(DIM, f"  {bar}")
            )
        print()

    async def _mutate_function(
        self,
        fn:      IRFunction,
        rank:    int,
        out_fh,
        raw_fh=None,
    ) -> dict:
        self._log(
            _c(CYAN, f"\n[{rank}] ") +
            _c(BOLD, fn.name) +
            _c(DIM, f"  nodes={fn.node_count}  energy={fn.prime_energy:.4f}")
        )

        # ── Zero-node guard ────────────────────────────────────────────────
        # node_count == 0 means ZCC_IR_FUNC_BEGIN/END fired but no
        # ZCC_EMIT_* callsites are wired inside codegen_func yet.
        # Sending an empty function to the model produces hallucinated scores.
        # Emit a diagnostic record and skip the model call.
        if fn.node_count == 0:
            diag = {
                "_forge": {
                    "function":     fn.name,
                    "rank":         rank,
                    "prime_h0":     round(fn.prime_h0,    4),
                    "prime_h_final":round(fn.prime_h_final, 4),
                    "prime_energy": round(fn.prime_energy,  4),
                    "node_count":   0,
                    "error":        "EMPTY_IR — ZCC_EMIT_* callsites not wired in codegen_func. "
                                    "See zcc_emit_hooks.diff for the fix.",
                }
            }
            out_fh.write(json.dumps(diag, ensure_ascii=False) + "\n")
            out_fh.flush()
            self._log(_c(YEL, f"  ⚠ skipped — zero IR nodes (emit hooks missing)"))
            return diag

        user_prompt = build_user_prompt(fn)

        t0 = time.monotonic()
        response_text = ""

        # Stream completion — print live tokens
        if self.verbose:
            print(_c(DIM, "    ↳ streaming from model..."))
            async for chunk in self.client.stream_complete(SYSTEM_PROMPT, user_prompt):
                response_text += chunk
                print(chunk, end="", flush=True)
            print()  # newline after streamed response
        else:
            response_text = await self.client.complete(SYSTEM_PROMPT, user_prompt)

        elapsed = time.monotonic() - t0

        # ── Write raw response to debug file ──────────────────────────
        if raw_fh:
            raw_fh.write(f"\n{'='*60}\n[{rank}] {fn.name}  elapsed={elapsed:.2f}s\n{'='*60}\n")
            raw_fh.write(response_text if response_text else "<EMPTY>")
            raw_fh.write("\n")
            raw_fh.flush()

        # ── Debug: show raw response before parse ─────────────────────
        if not response_text.strip():
            self._log(_c(RED, f"  ✗ model returned empty response in {elapsed:.1f}s"))
            diag = {"_forge": {"function": fn.name, "rank": rank,
                               "error": "EMPTY_RESPONSE — model returned nothing. "
                                        "Check llama.cpp server logs."}}
            out_fh.write(json.dumps(diag, ensure_ascii=False) + "\n")
            out_fh.flush()
            return diag
        if self.verbose:
            preview = response_text.strip()[:300].replace("\n", "↵")
            print(_c(DIM, f"  raw[{len(response_text)}]: {preview}"))

        # Parse JSON response — strip any accidental markdown fences
        clean = response_text.strip()
        if clean.startswith("```"):
            lines = clean.splitlines()
            clean = "\n".join(
                l for l in lines
                if not l.strip().startswith("```")
            ).strip()

        result: dict = {}
        try:
            result = json.loads(clean)
        except json.JSONDecodeError:
            # ── Truncation-aware recovery ──────────────────────────────
            # llama.cpp hits max_tokens mid-JSON — salvage completed arrays.
            # Strategy: find the last complete top-level value for each key.
            start = clean.find("{")
            if start != -1:
                # Attempt 1: try progressively shorter suffixes with closing brace
                recovered = False
                for close in range(len(clean) - 1, start, -1):
                    if clean[close] in ('}', ']'):
                        candidate = clean[start:close + 1]
                        # Balance braces/brackets
                        depth_b = candidate.count('{') - candidate.count('}')
                        depth_r = candidate.count('[') - candidate.count(']')
                        patched = candidate + (']' * max(depth_r, 0)) + ('}' * max(depth_b, 0))
                        try:
                            result = json.loads(patched)
                            if self.verbose:
                                print(_c(YEL, f"  ⚠ truncated response recovered "
                                              f"({len(clean)} chars, patched {depth_b}b {depth_r}r)"))
                            recovered = True
                            break
                        except json.JSONDecodeError:
                            continue
                if not recovered:
                    result = {"raw_response": clean,
                              "_truncated": True,
                              "_raw_len": len(clean)}
                    if self.verbose:
                        print(_c(RED, f"  ✗ JSON unrecoverable after truncation "
                                      f"({len(clean)} chars)"))
            else:
                result = {"raw_response": clean}

        # Augment with forge metadata
        result["_forge"] = {
            "function":     fn.name,
            "rank":         rank,
            "prime_h0":     round(fn.prime_h0, 4),
            "prime_h_final":round(fn.prime_h_final, 4),
            "prime_energy": round(fn.prime_energy, 4),
            "node_count":   fn.node_count,
            "branch_count": fn.branch_count,
            "call_count":   fn.call_count,
            "model":        self.client.model,
            "backend":      self.client.backend,
            "elapsed_s":    round(elapsed, 2),
        }

        # Write to JSONL output
        out_fh.write(json.dumps(result, ensure_ascii=False) + "\n")
        out_fh.flush()

        if self.verbose:
            ms = result.get("mutation_score", "?")
            dce_count  = len(result.get("dead_code", []))
            fold_count = len(result.get("constant_folds", []))
            print(_c(GRN, f"    ✓ mutation_score={ms}  "
                          f"dce_candidates={dce_count}  "
                          f"fold_candidates={fold_count}  "
                          f"({elapsed:.1f}s)"))

        return result

    async def run(self, mod: IRModule) -> List[dict]:
        self._print_banner(mod)

        # Score all functions
        self._log(_c(DIM, "Computing PRIME energy scores..."))
        self.scorer.score_module(mod)

        # Select top-N targets
        targets = self.scorer.top_n(mod, self.top_n)
        self._print_prime_table(targets)

        results: List[dict] = []

        with open(self.output, "w", encoding="utf-8") as out_fh:
            raw_fh = open("raw_responses.txt", "w", encoding="utf-8") \
                     if self.debug_responses else None
            # Write module header record
            header = {"_type": "module", **mod.to_dict()}
            out_fh.write(json.dumps(header, ensure_ascii=False) + "\n")
            out_fh.flush()

            for rank, fn in enumerate(targets, 1):
                try:
                    result = await self._mutate_function(fn, rank, out_fh,
                                                         raw_fh=raw_fh)
                    results.append(result)
                except Exception as exc:
                    err_rec = {
                        "_forge": {"function": fn.name, "rank": rank, "error": str(exc)}
                    }
                    out_fh.write(json.dumps(err_rec, ensure_ascii=False) + "\n")
                    out_fh.flush()
                    self._log(_c(RED, f"  ✗ {fn.name}: {exc}"))

            if raw_fh:
                raw_fh.close()
                self._log(_c(DIM, "   Raw responses → raw_responses.txt"))

        self._log(_c(CYAN, f"\n🔱 Forge complete — {len(results)}/{len(targets)} functions mutated"))
        self._log(_c(DIM, f"   Results written to: {self.output}"))
        return results


# ── Summary ───────────────────────────────────────────────────────────────────


def print_summary(results: List[dict]) -> None:
    if not results:
        return

    empty   = [r for r in results if r.get("_forge", {}).get("error", "").startswith("EMPTY_IR")]
    nonempty = [r for r in results if r not in empty]

    total_dce  = sum(len(r.get("dead_code", []))       for r in nonempty)
    total_fold = sum(len(r.get("constant_folds", []))  for r in nonempty)
    scores     = [r.get("mutation_score", 0) for r in nonempty
                  if isinstance(r.get("mutation_score"), (int, float))]
    avg_score  = sum(scores) / len(scores) if scores else 0

    print(_c(CYAN, "\n" + "═" * 60))
    print(_c(BOLD, " FORGE SUMMARY"))
    print(_c(CYAN, "═" * 60))
    print(f"  functions analyzed : {len(results)}")
    print(f"  with IR nodes      : {len(nonempty)}")
    print(f"  empty (no hooks)   : {len(empty)}")

    if empty and not nonempty:
        print()
        print(_c(RED, "  ✗ ALL FUNCTIONS EMPTY — ZCC_EMIT_* hooks not wired."))
        print(_c(YEL, "  Apply zcc_emit_hooks.diff to codegen_func in zcc.c"))
        print(_c(DIM, "  then rebuild: gcc -o zcc zcc.c ir.c compiler_passes.c -lm"))
        print(_c(DIM, "  then rerun:   ZCC_EMIT_IR=1 ./zcc zcc_pp.c ir.c"))
        print(_c(CYAN, "═" * 60))
        return

    print(f"  DCE candidates     : {total_dce}")
    print(f"  fold candidates    : {total_fold}")
    print(f"  avg mutation score : {avg_score:.1f} / 100")

    # Top DCE hits
    dce_all = []
    for r in results:
        fn_name = r.get("_forge", {}).get("function", "?")
        for d in r.get("dead_code", []):
            dce_all.append((fn_name, d.get("dst", "?"), d.get("reason", "")))
    if dce_all:
        print(_c(YEL, "\n  Top DCE candidates:"))
        for fn, dst, reason in dce_all[:8]:
            print(_c(DIM, f"    {fn}::") + dst + _c(DIM, f"  — {reason[:60]}"))

    # Top fold hits
    fold_all = []
    for r in results:
        fn_name = r.get("_forge", {}).get("function", "?")
        for f in r.get("constant_folds", []):
            fold_all.append((fn_name, f.get("nodes", []), f.get("folded_value")))
    if fold_all:
        print(_c(GRN, "\n  Top constant fold candidates:"))
        for fn, nodes, val in fold_all[:8]:
            print(_c(DIM, f"    {fn}::") + ",".join(nodes[:3])
                  + (f" → {val}" if val is not None else " → <computed>"))

    print(_c(CYAN, "═" * 60))


# ── CLI ───────────────────────────────────────────────────────────────────────


def _parse_args() -> argparse.Namespace:
    ap = argparse.ArgumentParser(
        description="ZCC IR Mutation Forge — PRIME-guided Qwen analysis",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    # ZCC options
    zcc = ap.add_argument_group("ZCC")
    zcc.add_argument("--zcc",  default="./zcc",
                     help="Path to ZCC binary (default: ./zcc)")
    zcc.add_argument("--src",  nargs="+", default=["zcc_pp.c", "ir.c"],
                     metavar="FILE",
                     help="Source files to compile (default: zcc_pp.c ir.c)")
    zcc.add_argument("--zcc-timeout", type=float, default=120.0,
                     help="ZCC subprocess timeout in seconds (default: 120)")
    zcc.add_argument("--ir-file", default=None,
                     help="Load IR text from file instead of running ZCC")

    # Model options
    mdl = ap.add_argument_group("Model")
    mdl.add_argument("--backend", choices=["ollama", "llama", "openai"],
                     default="ollama",
                     help="Local model backend (default: ollama)")
    mdl.add_argument("--model", default="qwen2.5-coder:7b",
                     help="Model name (default: qwen2.5-coder:7b)")
    mdl.add_argument("--base-url", default=None,
                     help="Override backend base URL")
    mdl.add_argument("--model-timeout", type=float, default=180.0,
                     help="Model request timeout in seconds (default: 180)")
    mdl.add_argument("--max-tokens", type=int, default=8192,
                     help="Max tokens per model response (default: 8192)")

    # PRIME options
    prime = ap.add_argument_group("PRIME")
    prime.add_argument("--prime-eta",   type=float, default=0.4)
    prime.add_argument("--prime-gamma", type=float, default=0.3)
    prime.add_argument("--prime-beta",  type=float, default=0.1)
    prime.add_argument("--prime-sigma", type=float, default=0.05)
    prime.add_argument("--prime-iters", type=int,   default=200)

    # Forge options
    forge = ap.add_argument_group("Forge")
    forge.add_argument("--top-n",  type=int, default=5,
                       help="Number of highest-energy functions to mutate (default: 5)")
    forge.add_argument("--output", default="mutations.jsonl",
                       help="JSONL output file (default: mutations.jsonl)")
    forge.add_argument("--quiet",  action="store_true",
                       help="Suppress live streaming output")
    forge.add_argument("--score-only", action="store_true",
                       help="Only compute PRIME scores, skip model calls")
    forge.add_argument("--debug-responses", action="store_true",
                       help="Dump raw model responses to raw_responses.txt")

    return ap.parse_args()


async def _main() -> int:
    args = _parse_args()

    # ── Step 1: Get IR text ────────────────────────────────────────────────
    if args.ir_file:
        print(_c(CYAN, f"📂 Loading IR from file: {args.ir_file}"))
        with open(args.ir_file, encoding="utf-8") as fh:
            ir_text = fh.read()
        zcc_elapsed = 0.0
    else:
        t0 = time.monotonic()
        ir_text, stderr, rc = await run_zcc(
            zcc_binary   = args.zcc,
            source_files = args.src,
            timeout      = args.zcc_timeout,
        )
        zcc_elapsed = time.monotonic() - t0
        if rc != 0:
            print(_c(RED, "ZCC failed — aborting forge."), file=sys.stderr)
            if stderr:
                print(stderr[:1000], file=sys.stderr)
            return 1

    # ── Step 2: Parse IR ──────────────────────────────────────────────────
    print(_c(DIM, "Parsing IR text..."))
    mod         = parse_ir_text(ir_text)
    mod.source  = " ".join(args.src)
    mod.elapsed = zcc_elapsed

    if not mod.funcs:
        print(_c(RED, "No IR functions found in ZCC output."), file=sys.stderr)
        print(_c(DIM, "First 200 chars of output:"), file=sys.stderr)
        print(ir_text[:200], file=sys.stderr)
        return 1

    print(_c(GRN, f"✓ Parsed {len(mod.funcs)} functions  ({mod.total_nodes} nodes)"))

    # ── Step 3: PRIME scoring ──────────────────────────────────────────────
    scorer = PRIMEScorer(
        eta   = args.prime_eta,
        gamma = args.prime_gamma,
        beta  = args.prime_beta,
        sigma = args.prime_sigma,
        iters = args.prime_iters,
    )
    scorer.score_module(mod)

    if args.score_only:
        targets = scorer.top_n(mod, args.top_n)
        print(_c(MAG, "\nPRIME scores (top-N, score-only mode):"))
        for i, fn in enumerate(targets, 1):
            print(f"  {i:>3}. {fn.name:<35} energy={fn.prime_energy:.4f}  "
                  f"nodes={fn.node_count}  branches={fn.branch_count}")
        return 0

    # ── Step 4: Mutation forge ─────────────────────────────────────────────
    client = LocalModelClient(
        backend    = args.backend,
        model      = args.model,
        base_url   = args.base_url,
        timeout    = args.model_timeout,
        max_tokens = args.max_tokens,
    )

    forge = MutationForge(
        client  = client,
        scorer  = scorer,
        top_n   = args.top_n,
        output  = args.output,
        verbose = not args.quiet,
        debug_responses = args.debug_responses,
    )

    results = await forge.run(mod)
    print_summary(results)
    return 0


def main() -> None:
    try:
        sys.exit(asyncio.run(_main()))
    except KeyboardInterrupt:
        print(_c(YEL, "\n⚠ Interrupted."))
        sys.exit(130)


if __name__ == "__main__":
    main()
