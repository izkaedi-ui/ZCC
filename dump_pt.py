#!/usr/bin/env python3
"""
ZKAEDI PRIME — Enhanced Checkpoint Inspector v2
=================================================
Production-grade .pt inspector with full error recovery,
adaptive diagnostics, and Hamiltonian phase analysis.

Drop next to your .pt files and run:
    python dump_pt.py                         # default: fused files + all seeds
    python dump_pt.py ZKAEDI_MASTER_FUSED.pt  # single file deep-dive
    python dump_pt.py --seeds                 # per-seed breakdown across all 3 optimizers
    python dump_pt.py --diff                  # weight diff between ULTIMA and MASTER
    python dump_pt.py --scan                  # auto-discover all .pt files in cwd
    python dump_pt.py --health                # quick health check across all checkpoints
    python dump_pt.py --export summary.json   # export structured report to JSON
"""

from __future__ import annotations

import sys
import os
import json
import time
import math
import hashlib
import traceback
import warnings
from pathlib import Path
from contextlib import contextmanager
from dataclasses import dataclass, field, asdict
from typing import Any

import torch

warnings.filterwarnings("ignore", category=FutureWarning, module="torch")

# ── ANSI ──────────────────────────────────────────────────────────────────────
_TTY = hasattr(sys.stdout, "isatty") and sys.stdout.isatty()
_NO_COLOR = os.environ.get("NO_COLOR")

def _c(code: str, text: str) -> str:
    if _TTY and not _NO_COLOR:
        return f"\033[{code}m{text}\033[0m"
    return str(text)

CYN  = lambda t: _c("96", t)
MAG  = lambda t: _c("95", t)
YLW  = lambda t: _c("93", t)
GRN  = lambda t: _c("92", t)
RED  = lambda t: _c("91", t)
WHT  = lambda t: _c("97;1", t)
DIM  = lambda t: _c("2", t)
BOLD = lambda t: _c("1", t)
BAR  = DIM("─" * 72)

# ── Error management ──────────────────────────────────────────────────────────

@dataclass
class InspectorError:
    path: str
    phase: str
    message: str
    suggestion: str = ""
    traceback_short: str = ""

    def display(self):
        print(RED(f"  ✗  [{self.phase.upper()}] {self.path}"))
        print(RED(f"     {self.message}"))
        if self.suggestion:
            print(YLW(f"     💡 {self.suggestion}"))
        if self.traceback_short:
            for line in self.traceback_short.strip().splitlines()[-3:]:
                print(DIM(f"        {line.strip()}"))


class ErrorCollector:
    def __init__(self):
        self.errors: list[InspectorError] = []

    def capture(self, path: str, phase: str, exc: Exception, suggestion: str = "") -> None:
        try:
            tb = traceback.format_exception(exc)
        except TypeError:
            # Fallback for older Python versions
            tb = traceback.format_exception(type(exc), exc, exc.__traceback__)
        self.errors.append(InspectorError(
            path=path, phase=phase, message=str(exc),
            suggestion=suggestion or self._auto_suggest(exc),
            traceback_short="".join(list(tb)[-3:]),
        ))

    def _auto_suggest(self, exc: Exception) -> str:
        msg = str(exc).lower()
        if "pickle" in msg or "unpickl" in msg:
            return "File may be corrupted or from an incompatible torch version. Try torch.load(..., weights_only=True)."
        if "cuda" in msg or "device" in msg:
            return "CUDA tensor on CPU-only machine. Already using map_location='cpu' — file may embed device refs."
        if "key" in msg and "not found" in msg:
            return "Checkpoint schema mismatch — inspect top-level keys first."
        if "memory" in msg or "alloc" in msg:
            return "Checkpoint too large for available RAM."
        return "Inspect manually or open an issue."

    @contextmanager
    def guard(self, path: str, phase: str, suggestion: str = ""):
        try:
            yield
        except Exception as exc:
            self.capture(path, phase, exc, suggestion)

    def has_errors(self) -> bool:
        return len(self.errors) > 0

    def summary(self):
        if not self.errors:
            print(GRN("\n  ✓  All operations completed without errors.\n"))
            return
        print(f"\n{BAR}")
        print(RED(f"  ⚠  ERROR SUMMARY — {len(self.errors)} issue(s) encountered"))
        print(BAR)
        for err in self.errors:
            err.display()
            print()


ERRORS = ErrorCollector()

# ── Structured report ─────────────────────────────────────────────────────────

@dataclass
class CheckpointReport:
    path: str
    size_mb: float = 0.0
    sha256_short: str = ""
    top_level_keys: list[str] = field(default_factory=list)
    total_params: int = 0
    total_layers: int = 0
    fusion_method: str = ""
    fusion_sources: int = 0
    fusion_score: float | None = None
    hamiltonian: dict[str, Any] = field(default_factory=dict)
    lyapunov: float | None = None
    phase: str = ""
    training_meta: dict[str, Any] = field(default_factory=dict)
    errors: list[str] = field(default_factory=list)


REPORTS: list[CheckpointReport] = []

# ── Helpers ───────────────────────────────────────────────────────────────────

def safe_load(path: str) -> dict | None:
    if not Path(path).exists():
        ERRORS.errors.append(InspectorError(
            path=path, phase="load", message="File not found.",
            suggestion="Check path spelling or run --scan to discover .pt files.",
        ))
        return None
    with ERRORS.guard(path, "load"):
        ckpt = torch.load(path, map_location="cpu", weights_only=False)
        if not isinstance(ckpt, dict):
            if isinstance(ckpt, torch.Tensor):
                return {"__raw_tensor__": ckpt}
            print(YLW(f"  ⚠  {path}: loaded as {type(ckpt).__name__}, wrapping in dict"))
            return {"__raw__": ckpt}
        return ckpt
    return None


def sha256_short(path: str, nbytes: int = 1 << 20) -> str:
    try:
        h = hashlib.sha256()
        with open(path, "rb") as f:
            h.update(f.read(nbytes))
        return h.hexdigest()[:16]
    except Exception:
        return "?"


def tensor_stats(t: torch.Tensor) -> str:
    t = t.float()
    if t.numel() == 0:
        return "empty tensor"
    mn, mx = t.min().item(), t.max().item()
    mu = t.mean().item()
    sd = t.std().item() if t.numel() > 1 else 0.0
    nrm = t.norm().item()
    nans = torch.isnan(t).sum().item()
    infs = torch.isinf(t).sum().item()
    base = (f"min={mn:+.5f}  max={mx:+.5f}  "
            f"mean={mu:+.5f}  std={sd:.5f}  norm={nrm:.5f}")
    flags = []
    if nans > 0:   flags.append(RED(f"NaN×{nans}"))
    if infs > 0:   flags.append(RED(f"Inf×{infs}"))
    if sd < 1e-8 and t.numel() > 1: flags.append(YLW("DEAD"))
    if mx / (abs(mn) + 1e-12) > 1e4 and abs(mn) > 1e-8: flags.append(YLW("SKEW"))
    if flags:
        base += "  " + " ".join(flags)
    return base


def lyapunov_from_traj(eta: float, gamma: float, H_vals: list[float]) -> float:
    total, count = 0.0, 0
    eta_f, gamma_f = float(eta), float(gamma)
    for H in H_vals:
        H_f = max(min(float(H), 50.0), -50.0)
        sig = 1.0 / (1.0 + math.exp(-gamma_f * H_f))
        deriv = abs(eta_f * sig * (1.0 + gamma_f * H_f * (1.0 - sig)))
        if deriv > 1e-14:
            total += math.log(deriv)
            count += 1
    return total / count if count > 0 else float("nan")


def phase_label(lam: float) -> str:
    if math.isnan(lam):        return DIM("UNKNOWN")
    if lam >  0.50:            return MAG("⚡ LEGEND")
    if lam >  0.05:            return RED("🔥 CHAOTIC")
    if lam > -0.05:            return YLW("🌀 BIFURCATING")
    if lam > -0.25:            return CYN("〰️  WANDERING")
    return GRN("🎯 CONVERGING")


def phase_label_plain(lam: float) -> str:
    if math.isnan(lam):  return "UNKNOWN"
    if lam >  0.50:      return "LEGEND"
    if lam >  0.05:      return "CHAOTIC"
    if lam > -0.05:      return "BIFURCATING"
    if lam > -0.25:      return "WANDERING"
    return "CONVERGING"


def mbytes(path: str) -> float:
    try: return os.path.getsize(path) / 1e6
    except: return 0.0

def mbytes_str(path: str) -> str:
    mb = mbytes(path)
    return f"{mb:.1f} MB" if mb else "?"

def discover_pt_files(directory: str = ".") -> list[str]:
    exts = {".pt", ".pth", ".bin", ".ckpt"}
    return [str(f) for f in sorted(Path(directory).iterdir())
            if f.is_file() and f.suffix in exts]

# ── Section printers ──────────────────────────────────────────────────────────

def print_header(path: str):
    print(f"\n{BAR}")
    print(WHT(f"  📦  {path}") + DIM(f"  ({mbytes_str(path)})  sha256:{sha256_short(path)}"))
    print(BAR)


def dump_fusion_log(ckpt: dict, report: CheckpointReport | None = None):
    log = ckpt.get("log", {})
    if not log:
        print(DIM("  (no fusion log)"))
        return

    fused  = log.get("fused_from", [])
    w      = log.get("weights", [])
    method = log.get("method", "unknown")
    epoch  = log.get("epoch", log.get("fusion_epoch", "?"))
    score  = log.get("score", log.get("best_score", None))
    ts     = log.get("timestamp", log.get("fused_at", "?"))

    if report:
        report.fusion_method  = method
        report.fusion_sources = len(fused)
        report.fusion_score   = float(score) if score is not None else None

    print(CYN(f"\n  ── Fusion Metadata"))
    print(f"     Method      : {method}")
    print(f"     Epoch       : {epoch}")
    print(f"     Timestamp   : {ts}")
    if score is not None:
        print(f"     Score       : {YLW(f'{score:.6f}')}")
    print(f"     Sources     : {len(fused)} checkpoints")

    if len(w) == len(fused) and w:
        pairs    = sorted(zip(w, fused), reverse=True)
        non_zero = [(wt, nm) for wt, nm in pairs if wt > 0]
        zeroed   = sum(1 for wt, _ in pairs if wt == 0)

        total_w_sum = sum(wt for wt, _ in non_zero) or 1.0
        entropy = -sum((p := wt/total_w_sum) * math.log2(p)
                       for wt, _ in non_zero if wt > 1e-12)
        max_entropy = math.log2(len(non_zero)) if len(non_zero) > 1 else 1.0
        norm_entropy = entropy / max_entropy if max_entropy > 0 else 0.0
        ent_label = ("UNIFORM" if norm_entropy > 0.95
                     else "CONCENTRATED" if norm_entropy < 0.5 else "BALANCED")

        print(CYN(f"\n  ── Weight Distribution"))
        print(f"     Entropy     : {entropy:.3f} bits  "
              f"(normalized: {norm_entropy:.3f}  {ent_label})")
        print(f"     {'Rank':<5} {'Weight':>8}  {'Model'}")
        print(f"     {'─'*4:<5} {'─'*8:>8}  {'─'*38}")
        top_wt = non_zero[0][0] if non_zero else 1.0
        for rank, (wt, nm) in enumerate(non_zero[:15], 1):
            bar_len = int(wt / (top_wt or 1) * 40)
            bar = GRN("█" * bar_len) + DIM("░" * (40 - bar_len))
            print(f"     {rank:<5} {wt:>8.4f}  {nm:<38}  {bar}")
        if len(non_zero) > 15:
            print(DIM(f"     ... {len(non_zero)-15} more non-zero"))
        print(f"\n     Zeroed out  : {zeroed}")
        print(f"     Non-zero    : {len(non_zero)}")

        groups: dict[str, float] = {}
        for wt, nm in zip(w, fused):
            for g in ("AdamW", "SGD", "CATALYZE"):
                if g in nm:
                    groups[g] = groups.get(g, 0.0) + wt
                    break
            else:
                groups["other"] = groups.get("other", 0.0) + wt

        print(CYN(f"\n  ── Optimizer Group Totals"))
        total_gw = sum(groups.values()) or 1.0
        for g, total in sorted(groups.items(), key=lambda x: -x[1]):
            pct = 100 * total / total_gw
            bar = GRN("█" * int(pct / 2)) + DIM("░" * (50 - int(pct / 2)))
            print(f"     {g:<12}: {total:>7.4f}  ({pct:5.1f}%)  {bar}")
    else:
        if w and len(w) != len(fused):
            print(YLW(f"     ⚠  Weight array ({len(w)}) ≠ fused_from ({len(fused)}) — schema drift?"))
        for k, v in log.items():
            if k not in ("fused_from", "weights"):
                print(f"     {k}: {str(v)[:80]}")


def dump_model_weights(ckpt: dict, report: CheckpointReport | None = None):
    sd_key = next((k for k in ("model_state_dict", "state_dict", "model") if k in ckpt), None)
    if sd_key is None:
        print(DIM("\n  (no model state dict found)"))
        return

    sd = ckpt[sd_key]
    total_params = sum(v.numel() for v in sd.values() if isinstance(v, torch.Tensor))
    total_layers = sum(1 for v in sd.values() if isinstance(v, torch.Tensor))

    if report:
        report.total_params = total_params
        report.total_layers = total_layers

    health_issues: list[str] = []
    for name, val in sd.items():
        if not isinstance(val, torch.Tensor): continue
        vf = val.float()
        if torch.isnan(vf).any():            health_issues.append(f"NaN in {name}")
        if torch.isinf(vf).any():            health_issues.append(f"Inf in {name}")
        if vf.numel() > 1 and vf.std().item() < 1e-10:
            health_issues.append(f"DEAD (zero variance) in {name}")
        if vf.numel() > 1 and vf.abs().max().item() > 1e6:
            health_issues.append(f"EXPLODED (|max|>{vf.abs().max().item():.0f}) in {name}")

    print(CYN(f"\n  ── Model Weights ({sd_key})"))
    print(f"     Total parameters : {total_params:,}")
    print(f"     Total layers     : {total_layers}")

    if health_issues:
        print(RED(f"\n     ⚠  WEIGHT HEALTH ISSUES ({len(health_issues)}):"))
        for issue in health_issues[:10]: print(RED(f"        • {issue}"))
        if len(health_issues) > 10: print(RED(f"        ... and {len(health_issues)-10} more"))
        if report: report.errors.extend(health_issues[:5])
    else:
        print(GRN(f"     ✓  All layers healthy (no NaN/Inf/dead/exploded)"))

    print(f"\n     {'Layer':<45} {'Shape':<22} {'Stats'}")
    print(f"     {'─'*44:<45} {'─'*21:<22} {'─'*52}")
    for name, val in sd.items():
        if not isinstance(val, torch.Tensor): continue
        shape_str = str(tuple(val.shape))
        stats_str = tensor_stats(val) if val.numel() > 1 else f"scalar={val.item():+.6f}"
        print(f"     {name:<45} {shape_str:<22} {DIM(stats_str)}")


def dump_hamiltonian_state(ckpt: dict, report: CheckpointReport | None = None):
    hs = ckpt.get("hamiltonian_state", ckpt.get("hamilton_state", {}))
    if not hs:
        hs = {k: v for k, v in ckpt.items()
              if any(x in k.lower() for x in ("hamilt", "H_0", "eta", "gamma", "lyap", "energy"))}
    if not hs:
        print(DIM("\n  (no Hamiltonian state found)"))
        return

    print(CYN(f"\n  ── Hamiltonian State  ({len(hs)} keys)"))

    H_0 = eta = gamma = traj = None
    eta   = hs.get("eta",   hs.get("eta_final"))
    gamma = hs.get("gamma", hs.get("gamma_final"))
    traj  = hs.get("trajectory", hs.get("H_trajectory"))
    ham_report: dict[str, Any] = {}

    for k, v in hs.items():
        if isinstance(v, torch.Tensor):
            if v.numel() == 1:
                scalar = v.item()
                print(f"     {k:<30}: {YLW(f'{scalar:+.8f}')}")
                ham_report[k] = scalar
                if k in ("H_0", "H0", "h_zero"): H_0 = scalar
            else:
                print(f"     {k:<30}: shape={tuple(v.shape)}  {DIM(tensor_stats(v))}")
                ham_report[k] = f"tensor{tuple(v.shape)}"
                if k in ("H_0", "H0", "h_zero"): H_0 = v.mean().item()
        elif isinstance(v, (int, float)):
            print(f"     {k:<30}: {YLW(f'{float(v):+.8f}')}")
            ham_report[k] = float(v)
            if k == "eta":   eta   = float(v)
            if k == "gamma": gamma = float(v)
        elif isinstance(v, list) and v and isinstance(v[0], (int, float)):
            arr = v
            print(f"     {k:<30}: list[{len(arr)}]  "
                  f"min={min(arr):+.5f}  max={max(arr):+.5f}  "
                  f"mean={sum(arr)/len(arr):+.5f}")
            ham_report[k] = {"len": len(arr), "min": min(arr), "max": max(arr)}
            if "traj" in k.lower() or "h_val" in k.lower(): traj = arr
        else:
            print(f"     {k:<30}: {str(v)[:60]}")
            ham_report[k] = str(v)[:60]

    if report: report.hamiltonian = ham_report

    if traj and eta is not None and gamma is not None:
        if isinstance(traj, torch.Tensor): traj = traj.tolist()
        lam = lyapunov_from_traj(float(eta), float(gamma), traj)
        if report:
            report.lyapunov = lam if not math.isnan(lam) else None
            report.phase = phase_label_plain(lam)

        nan_count = sum(1 for h in traj if math.isnan(h))
        inf_count = sum(1 for h in traj if math.isinf(h))

        print(CYN(f"\n  ── PRIME Analysis (trajectory n={len(traj)})"))
        print(f"     η (eta)     : {float(eta):.6f}")
        print(f"     γ (gamma)   : {float(gamma):.6f}")
        print(f"     Lyapunov λ  : {YLW(f'{lam:+.6f}')}")
        print(f"     Phase       : {phase_label(lam)}")

        if nan_count or inf_count:
            print(RED(f"     ⚠  Trajectory: {nan_count} NaN, {inf_count} Inf"))

        if lam > 0.50:
            print(f"     ⚡  {MAG('LEGEND regime — transcendent attractor space')}")
        elif lam > 0.05:
            print(f"     ⚠️  {RED('Chaotic — reduce η or increase γ')}")
        elif lam > -0.05:
            print(f"     🌀  {YLW('Bifurcation edge — phase boundary exploration')}")
        elif lam < -0.3:
            print(f"     ✅  {GRN('Deep convergent — strong attractor')}")
    elif H_0 is not None and eta is not None and gamma is not None:
        print(DIM(f"\n  (no trajectory — H_0={H_0:+.5f}, η={eta:.4f}, γ={gamma:.4f})"))


def dump_training_metadata(ckpt: dict, report: CheckpointReport | None = None):
    keys_of_interest = [
        "epoch", "step", "global_step", "loss", "val_loss", "best_loss",
        "accuracy", "val_accuracy", "lr", "learning_rate", "optimizer",
        "seed", "score", "f1", "precision", "recall", "train_time",
        "gradient_norm", "grad_norm", "wall_time",
    ]
    found: dict[str, Any] = {}
    for k in keys_of_interest:
        if k in ckpt: found[k] = ckpt[k]
    for top_k, top_v in ckpt.items():
        if isinstance(top_v, dict):
            for k in keys_of_interest:
                if k in top_v and k not in found:
                    found[f"{top_k}.{k}"] = top_v[k]

    if not found:
        print(DIM("\n  (no training metadata)"))
        return

    if report:
        report.training_meta = {
            k: (float(v) if isinstance(v, (int, float)) else str(v)[:60])
            for k, v in found.items()
        }

    print(CYN(f"\n  ── Training Metadata"))
    for k, v in found.items():
        if isinstance(v, float): print(f"     {k:<25}: {YLW(f'{v:.8f}')}")
        elif isinstance(v, dict): print(f"     {k:<25}: dict({len(v)} keys)")
        else: print(f"     {k:<25}: {v}")


def dump_top_level_keys(ckpt: dict, report: CheckpointReport | None = None):
    if report: report.top_level_keys = list(ckpt.keys())
    print(CYN(f"\n  ── All Top-Level Keys ({len(ckpt)})"))
    for k, v in ckpt.items():
        if isinstance(v, torch.Tensor):
            print(f"     {k:<30}  Tensor{str(tuple(v.shape))}")
        elif isinstance(v, dict):
            print(f"     {k:<30}  dict({len(v)} keys)")
        elif isinstance(v, list):
            print(f"     {k:<30}  list[{len(v)}]")
        else:
            print(f"     {k:<30}  {str(v)[:60].replace(chr(10),' ')}")


def dump_optimizer_state(ckpt: dict):
    opt = ckpt.get("optimizer_state_dict", ckpt.get("optimizer", None))
    if not isinstance(opt, dict): return
    param_groups = opt.get("param_groups", [])
    state = opt.get("state", {})
    if not param_groups and not state: return
    print(CYN(f"\n  ── Optimizer State"))
    print(f"     Param groups : {len(param_groups)}")
    print(f"     State entries: {len(state)}")
    for i, pg in enumerate(param_groups):
        print(f"     Group {i}: lr={pg.get('lr','?')}  "
              f"weight_decay={pg.get('weight_decay','?')}  "
              f"betas={pg.get('betas','?')}")
    if state:
        steps = [s.get("step", 0) for s in state.values()
                 if isinstance(s, dict) and "step" in s]
        if steps: print(f"     Step range   : {min(steps)} – {max(steps)}")


# ── Full single-file dump ─────────────────────────────────────────────────────

def dump_full(path: str):
    if not Path(path).exists():
        print(YLW(f"  (skipping {path} — not found)"))
        return
    report = CheckpointReport(path=path, size_mb=mbytes(path),
                               sha256_short=sha256_short(path))
    print_header(path)
    with ERRORS.guard(path, "analyse"):
        ckpt = safe_load(path)
        if ckpt is None:
            report.errors.append("Failed to load")
            REPORTS.append(report)
            return
        dump_top_level_keys(ckpt, report)
        dump_fusion_log(ckpt, report)
        dump_hamiltonian_state(ckpt, report)
        dump_training_metadata(ckpt, report)
        dump_optimizer_state(ckpt)
        dump_model_weights(ckpt, report)
    REPORTS.append(report)
    print()


# ── Per-seed comparison ───────────────────────────────────────────────────────

def dump_seeds():
    print(f"\n{BAR}")
    print(WHT("  📊  PER-SEED COMPARISON"))
    print(BAR)

    for opt, prefix in [("SGD","A_SGD"),("AdamW","A_AdamW"),("CATALYZE","A_CATALYZE")]:
        print(CYN(f"\n  ── {opt}  (seeds 0–19)"))
        print(f"     {'Seed':<7} {'Loss/Score':>12}  {'Epoch':>6}  "
              f"{'λ':>10}  {'Phase':<22}  {'H_0':>10}  {'Health'}")
        print(f"     {'─'*6:<7} {'─'*12:>12}  {'─'*6:>6}  "
              f"{'─'*10:>10}  {'─'*22:<22}  {'─'*10:>10}  {'─'*8}")
        found, best_score, best_seed = 0, None, None
        for seed in range(20):
            fname = f"{prefix}_seed{seed}.pt"
            if not Path(fname).exists(): continue
            found += 1
            with ERRORS.guard(fname, "seeds"):
                ckpt = safe_load(fname)
                if ckpt is None: continue
                score = (ckpt.get("score") or ckpt.get("best_loss") or
                         ckpt.get("val_loss") or ckpt.get("loss"))
                epoch = ckpt.get("epoch", "?")
                hs    = ckpt.get("hamiltonian_state", ckpt.get("hamilton_state", {}))
                H_0 = eta = gamma = traj = None
                if hs:
                    for k in ("H_0","H0","h_zero"):
                        if k in hs:
                            H_0_t = hs[k]
                            H_0 = (H_0_t.mean().item() if isinstance(H_0_t, torch.Tensor)
                                   else float(H_0_t))
                    eta   = hs.get("eta",   hs.get("eta_final"))
                    gamma = hs.get("gamma", hs.get("gamma_final"))
                    traj  = hs.get("trajectory", hs.get("H_trajectory"))
                    if isinstance(traj, torch.Tensor): traj = traj.tolist()
                lam_str = phase_str = H0_str = "?"
                if traj and eta is not None and gamma is not None:
                    lam = lyapunov_from_traj(float(eta), float(gamma), traj)
                    lam_str, phase_str = f"{lam:+.5f}", phase_label(lam)
                if H_0 is not None: H0_str = f"{H_0:+.6f}"
                health = GRN("✓")
                sd_key = next((k for k in ("model_state_dict","state_dict","model")
                               if k in ckpt), None)
                if sd_key:
                    for v in ckpt[sd_key].values():
                        if isinstance(v, torch.Tensor):
                            if torch.isnan(v).any() or torch.isinf(v).any():
                                health = RED("✗ NaN/Inf"); break
                score_str = f"{float(score):.6f}" if score is not None else "?"
                if score is not None:
                    if best_score is None or float(score) < best_score:
                        best_score, best_seed = float(score), seed
                print(f"     {seed:<7} {score_str:>12}  {str(epoch):>6}  "
                      f"{lam_str:>10}  {phase_str:<22}  {H0_str:>10}  {health}")
        if not found:
            print(DIM(f"     (no files found for {prefix}_seed*.pt)"))
        elif best_seed is not None:
            print(GRN(f"\n     🏆 Best seed: {best_seed} (score: {best_score:.6f})"))


# ── Weight diff ───────────────────────────────────────────────────────────────

def dump_diff(path_a: str = "ZKAEDI_ULTIMA_FUSED.pt",
              path_b: str = "ZKAEDI_MASTER_FUSED.pt"):
    print(f"\n{BAR}")
    print(WHT(f"  🔬  DIFF:  {path_a}  vs  {path_b}"))
    print(BAR)
    missing = [p for p in (path_a, path_b) if not Path(p).exists()]
    if missing:
        print(RED(f"  Missing: {', '.join(missing)}"))
        print(YLW("  💡 Run --scan to see available checkpoints.")); return
    with ERRORS.guard(f"{path_a} vs {path_b}", "diff"):
        a, b = safe_load(path_a), safe_load(path_b)
        if a is None or b is None: return
        sd_key = next((k for k in ("model_state_dict","state_dict","model")
                       if k in a and k in b), None)
        if sd_key is None:
            print(DIM("  (no matching state_dict key)")); return
        sd_a, sd_b = a[sd_key], b[sd_key]
        common = set(sd_a) & set(sd_b)
        only_a = set(sd_a) - set(sd_b)
        only_b = set(sd_b) - set(sd_a)
        print(f"\n     Shared layers   : {len(common)}")
        print(f"     Only in {Path(path_a).name}: {len(only_a)}")
        print(f"     Only in {Path(path_b).name}: {len(only_b)}")
        for label, only in [(Path(path_a).name, only_a),(Path(path_b).name, only_b)]:
            if only:
                print(YLW(f"\n     Layers only in {label}:"))
                for k in sorted(only)[:5]: print(f"       • {k}")
                if len(only) > 5: print(DIM(f"       ... and {len(only)-5} more"))
        print(CYN(f"\n     {'Layer':<45} {'L2':>10}  {'max|Δ|':>10}  {'mean|Δ|':>10}  {'cosine':>8}"))
        print(f"     {'─'*44:<45} {'─'*10:>10}  {'─'*10:>10}  {'─'*10:>10}  {'─'*8:>8}")
        diffs, total_l2_sq = [], 0.0
        for k in sorted(common):
            va, vb = sd_a[k].float(), sd_b[k].float()
            if va.shape != vb.shape:
                print(f"     {k:<45} {RED('shape mismatch')}"); continue
            d = (va - vb).abs()
            l2 = (va - vb).norm().item()
            total_l2_sq += l2 ** 2
            cos = torch.nn.functional.cosine_similarity(
                va.flatten().unsqueeze(0), vb.flatten().unsqueeze(0)
            ).item() if va.numel() > 0 else float("nan")
            diffs.append((l2, k, cos))
            flag = (RED(" ◄ HIGH DRIFT") if l2 > 0.5 else
                    YLW(" ◄ drift") if l2 > 0.1 else "")
            if not math.isnan(cos) and cos < 0.9:
                flag += RED(" ◄ DIVERGED")
            print(f"     {k:<45} {l2:>10.6f}  {d.max().item():>10.6f}  "
                  f"{d.mean().item():>10.8f}  {cos:>8.4f}{flag}")
        if diffs:
            print(f"\n     Global L2 : {YLW(f'{math.sqrt(total_l2_sq):.6f}')}")
            diffs.sort(reverse=True)
            print(CYN(f"\n  ── Top 5 Most Diverged"))
            for l2, name, cos in diffs[:5]:
                print(f"     {l2:.6f}  cos={cos:.4f}  {name}")
            print(CYN(f"\n  ── Top 5 Most Similar"))
            for l2, name, cos in sorted(diffs)[:5]:
                print(f"     {l2:.6f}  cos={cos:.4f}  {name}")


# ── Health check ──────────────────────────────────────────────────────────────

def dump_health():
    files = discover_pt_files()
    if not files:
        print(YLW("  No .pt/.pth/.bin/.ckpt files found.")); return
    print(f"\n{BAR}")
    print(WHT(f"  🏥  HEALTH CHECK — {len(files)} checkpoint(s)"))
    print(BAR)
    print(f"\n     {'File':<40} {'Size':>8}  {'Loadable':>8}  "
          f"{'Params':>12}  {'NaN':>5}  {'Inf':>5}  {'Dead':>5}  {'Status'}")
    print(f"     {'─'*39:<40} {'─'*8:>8}  {'─'*8:>8}  "
          f"{'─'*12:>12}  {'─'*5:>5}  {'─'*5:>5}  {'─'*5:>5}  {'─'*10}")
    for path in files:
        size = f"{mbytes(path):.1f}M"
        with ERRORS.guard(path, "health"):
            ckpt = safe_load(path)
            if ckpt is None:
                print(f"     {Path(path).name:<40} {size:>8}  {RED('FAIL'):>8}"); continue
            sd_key = next((k for k in ("model_state_dict","state_dict","model")
                           if k in ckpt), None)
            if sd_key is None:
                print(f"     {Path(path).name:<40} {size:>8}  {GRN('OK'):>8}  "
                      f"{'N/A':>12}  {'–':>5}  {'–':>5}  {'–':>5}  {DIM('no weights')}"); continue
            sd = ckpt[sd_key]
            params = sum(v.numel() for v in sd.values() if isinstance(v, torch.Tensor))
            nans = infs = dead = 0
            for v in sd.values():
                if not isinstance(v, torch.Tensor): continue
                vf = v.float()
                nans += torch.isnan(vf).sum().item()
                infs += torch.isinf(vf).sum().item()
                if vf.numel() > 1 and vf.std().item() < 1e-10: dead += 1
            status = (RED("CORRUPT") if nans or infs
                      else YLW("DEGRADED") if dead > 0 else GRN("HEALTHY"))
            print(f"     {Path(path).name:<40} {size:>8}  {GRN('OK'):>8}  "
                  f"{params:>12,}  {nans:>5}  {infs:>5}  {dead:>5}  {status}")


# ── Scan ──────────────────────────────────────────────────────────────────────

def dump_scan():
    files = discover_pt_files()
    print(f"\n{BAR}")
    print(WHT(f"  🔍  SCAN — {len(files)} checkpoint file(s)"))
    print(BAR)
    if not files:
        print(YLW("  No .pt/.pth/.bin/.ckpt files found.")); return
    total = 0.0
    for f in files:
        sz = mbytes(f); total += sz
        print(f"     {Path(f).name:<50}  {sz:>8.1f} MB  sha256:{sha256_short(f)}")
    print(f"\n     Total: {total:.1f} MB across {len(files)} file(s)")


# ── Export ────────────────────────────────────────────────────────────────────

def _remove_empty(obj: Any) -> Any:
    if isinstance(obj, dict):
        return {k: _remove_empty(v) for k, v in obj.items() if v not in (None, "", [], {})}
    elif isinstance(obj, list):
        return [_remove_empty(v) for v in obj if v not in (None, "", [], {})]
    return obj

def export_report(output_path: str):
    data: dict[str, Any] = {
        "generated_by": "ZKAEDI PRIME Checkpoint Inspector v2",
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "checkpoints": [asdict(r) for r in REPORTS],
        "errors": [{"path": e.path, "phase": e.phase, "message": e.message,
                    "suggestion": e.suggestion} for e in ERRORS.errors],
    }
    
    # Filter out nulls and empties
    clean_data = _remove_empty(data)
    
    with open(output_path, "w") as f:
        json.dump(clean_data, f, indent=2, default=str)
    print(GRN(f"\n  ✓  Report exported to {output_path}"))


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    t0   = time.monotonic()
    args = sys.argv[1:]

    cuda_info = (f"CUDA ✓ {torch.cuda.get_device_name(0)}"
                 if torch.cuda.is_available() else "CUDA ✗")
    print(WHT("\n  ⚡ ZKAEDI PRIME — Checkpoint Inspector v2"))
    print(DIM(f"     torch {torch.__version__}  •  {cuda_info}  •  "
              f"CPU threads {torch.get_num_threads()}"))

    export_path = None
    if "--export" in args:
        idx = args.index("--export")
        if idx + 1 < len(args):
            export_path = args[idx + 1]
            args = args[:idx] + args[idx+2:]
        else:
            print(RED("  --export requires a filename")); sys.exit(1)

    if   "--health" in args: dump_health()
    elif "--scan"   in args: dump_scan()
    elif "--seeds"  in args: dump_seeds()
    elif "--diff"   in args:
        rest = [a for a in args if a != "--diff"]
        dump_diff(rest[0], rest[1]) if len(rest) >= 2 else dump_diff()
    elif args:
        for path in args: dump_full(path)
    else:
        for t in ["ZKAEDI_MASTER_FUSED.pt","ZKAEDI_ULTIMA_FUSED.pt",
                  "ZKAEDI_OMEGA_FUSED.pt","B_CATALYZE_seed0.pt","B_CATALYZE_seed1.pt"]:
            dump_full(t)

    ERRORS.summary()
    if export_path: export_report(export_path)

    print(f"{BAR}")
    print(WHT("  💡  USAGE"))
    print(BAR)
    print("     python dump_pt.py                             # fused files (default)")
    print("     python dump_pt.py A_CATALYZE_seed5.pt         # single file deep-dive")
    print("     python dump_pt.py --seeds                     # all 60 seeds (3×20)")
    print("     python dump_pt.py --diff                      # ULTIMA vs MASTER diff")
    print("     python dump_pt.py --diff a.pt b.pt            # custom diff pair")
    print("     python dump_pt.py --scan                      # discover all .pt files")
    print("     python dump_pt.py --health                    # NaN/Inf/dead check")
    print("     python dump_pt.py --export report.json        # JSON export (combinable)")
    print(f"\n     Completed in {time.monotonic()-t0:.2f}s")
    print(f"{BAR}\n")


if __name__ == "__main__":
    main()
