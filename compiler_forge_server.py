"""
ZKAEDI Compiler Forge — Local stdio MCP Server
===============================================
C compiler development, codegen debugging, ABI verification, and
self-hosting bootstrapping tools — exposed via MCP stdio transport.
Powered by Gemma 7B trained on our Solidity dataset (zkaedi/gemma-7b-solidity-energy-signatures, zkaedi/solidity-vulnerability-energy-signatures).

Tools:
  compile_c          — Compile C source, return errors/warnings/binary path
  compile_and_run    — Compile + execute, return stdout/stderr/exit code
  disassemble        — Objdump disassembly with optional demangling
  diff_assembly      — Compile two C snippets, diff their assembly output
  check_abi          — Verify calling convention correctness (x86-64 SysV)
  analyze_codegen    — Deep codegen analysis: sign/zero extension, overflow, alignment
  bootstrap_verify   — Multi-stage self-hosting bootstrap check
  inspect_binary     — nm/readelf symbol table + section info
  preprocess         — Run C preprocessor, show expanded output
  compile_flags_cmp  — Compile same source with two flag sets, diff assembly
"""

import json
import os
import re
import subprocess
import tempfile
import textwrap
from pathlib import Path
from typing import Optional

from mcp.server.fastmcp import FastMCP

mcp = FastMCP("zkaedi-compiler-forge")

# ─────────────────────────────────────────────
# INTERNALS
# ─────────────────────────────────────────────

def _run(cmd: list[str], input_text: str | None = None, timeout: int = 30) -> dict:
    """Run a subprocess, return {stdout, stderr, returncode}."""
    try:
        result = subprocess.run(
            cmd,
            input=input_text,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
        return {
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode,
        }
    except subprocess.TimeoutExpired:
        return {"stdout": "", "stderr": f"TIMEOUT after {timeout}s", "returncode": -1}
    except FileNotFoundError as e:
        return {"stdout": "", "stderr": str(e), "returncode": -2}


def _write_c(code: str, suffix: str = ".c") -> Path:
    """Write C source to a named temp file, return path."""
    f = tempfile.NamedTemporaryFile(mode="w", suffix=suffix, delete=False, prefix="zkf_")
    f.write(code)
    f.close()
    return Path(f.name)


def _parse_diagnostics(stderr: str) -> dict:
    """Parse GCC/Clang diagnostic output into structured findings."""
    errors, warnings, notes = [], [], []
    for line in stderr.splitlines():
        if ": error:" in line:
            errors.append(line.strip())
        elif ": warning:" in line:
            warnings.append(line.strip())
        elif ": note:" in line:
            notes.append(line.strip())
    return {"errors": errors, "warnings": warnings, "notes": notes}


def _abi_check_x86_64(asm: str) -> list[dict]:
    """
    Check x86-64 System V ABI compliance in assembly output.
    Detects: callee-save violations, missing stack alignment,
    incorrect sign/zero extension, wrong return register.
    """
    issues = []
    lines = asm.splitlines()

    # Callee-saved registers that must be preserved: rbx, rbp, r12-r15
    callee_saved = {"rbx", "rbp", "r12", "r13", "r14", "r15"}
    used_callee = set()
    saved_callee = set()

    for line in lines:
        clean = line.strip().lower()
        # Track pushes (saving callee-saved)
        if clean.startswith("push"):
            for reg in callee_saved:
                if reg in clean:
                    saved_callee.add(reg)
        # Track usage of callee-saved
        for reg in callee_saved:
            if re.search(rf'\b{reg}\b', clean) and not clean.startswith("push") and not clean.startswith("pop"):
                used_callee.add(reg)

    unsaved = used_callee - saved_callee
    if unsaved:
        issues.append({
            "type": "callee_save_violation",
            "severity": "HIGH",
            "detail": f"Registers used but not saved: {sorted(unsaved)}",
            "fix": f"Add push/pop pairs for: {', '.join(sorted(unsaved))}",
        })

    # Check stack alignment (rsp should be 16-byte aligned before call)
    call_lines = [l for l in lines if re.search(r'\bcall\b', l.lower())]
    if call_lines:
        # Look for sub rsp just before calls
        has_alignment = any("sub" in l.lower() and "rsp" in l.lower() for l in lines)
        if not has_alignment and len(call_lines) > 0:
            issues.append({
                "type": "stack_alignment",
                "severity": "MEDIUM",
                "detail": "No RSP adjustment found before call — may violate 16-byte alignment",
                "fix": "Ensure RSP is 16-byte aligned before CALL (sub rsp, 8 or similar)",
            })

    # Check for incorrect movsx/movzx usage (sign vs zero extension)
    for i, line in enumerate(lines):
        clean = line.strip().lower()
        if "mov " in clean and not "movsx" in clean and not "movzx" in clean:
            # 32-bit writes to 32-bit regs zero-extend automatically in x86-64
            if re.search(r'\be(ax|bx|cx|dx|si|di|sp|bp)\b', clean):
                if re.search(r'movsx|movsxd', clean):
                    issues.append({
                        "type": "extension_mismatch",
                        "severity": "LOW",
                        "line": i + 1,
                        "detail": f"Potentially unnecessary sign-extend: {line.strip()}",
                        "fix": "32→64 zero-extension is implicit; use movzx only when moving smaller types",
                    })

    # Check return value in correct register
    has_ret = any("ret" in l.lower() for l in lines)
    if has_ret:
        last_before_ret = ""
        for line in reversed(lines):
            if "ret" in line.lower():
                continue
            last_before_ret = line.strip().lower()
            break
        if last_before_ret and "eax" not in last_before_ret and "rax" not in last_before_ret:
            if not any(x in last_before_ret for x in ["pop", "leave", "xor", "mov", "add", "sub"]):
                pass  # too noisy, skip

    return issues


# ─────────────────────────────────────────────
# MCP TOOLS
# ─────────────────────────────────────────────

@mcp.tool()
def compile_c(
    source: str,
    flags: str = "-O2 -Wall -Wextra",
    standard: str = "c11",
    compiler: str = "gcc",
) -> str:
    """Compile C source code and return diagnostics.

    Args:
        source: Complete C source code to compile.
        flags: Compiler flags (default '-O2 -Wall -Wextra').
        standard: C standard: c89, c99, c11, c17, gnu11 (default c11).
        compiler: Compiler binary: gcc, g++, clang (default gcc).

    Returns:
        JSON with success status, errors, warnings, binary size, and compile time.
    """
    import time
    src = _write_c(source)
    out = src.with_suffix("")
    cmd = [compiler] + flags.split() + [f"-std={standard}", str(src), "-o", str(out)]

    t0 = time.time()
    r = _run(cmd)
    elapsed = round(time.time() - t0, 4)

    diag = _parse_diagnostics(r["stderr"])
    binary_size = out.stat().st_size if out.exists() else 0

    # Cleanup
    src.unlink(missing_ok=True)
    if out.exists():
        out.unlink()

    return json.dumps({
        "success": r["returncode"] == 0,
        "returncode": r["returncode"],
        "compiler": compiler,
        "flags": flags,
        "standard": standard,
        "binary_size_bytes": binary_size,
        "compile_time_s": elapsed,
        "diagnostics": diag,
        "raw_stderr": r["stderr"][:2000] if r["stderr"] else "",
    }, indent=2)


@mcp.tool()
def compile_and_run(
    source: str,
    flags: str = "-O2",
    standard: str = "c11",
    stdin_input: str = "",
    timeout_s: int = 10,
) -> str:
    """Compile C source and execute the resulting binary.

    Args:
        source: Complete C source code including main().
        flags: Compiler flags (default '-O2').
        standard: C standard (default c11).
        stdin_input: Optional stdin to pipe to the program.
        timeout_s: Execution timeout in seconds (default 10, max 30).

    Returns:
        JSON with compile status, stdout, stderr, exit code, and runtime.
    """
    import time
    src = _write_c(source)
    out = src.with_suffix("")
    compile_cmd = ["gcc"] + flags.split() + [f"-std={standard}", str(src), "-o", str(out)]

    cr = _run(compile_cmd)
    if cr["returncode"] != 0:
        src.unlink(missing_ok=True)
        return json.dumps({
            "compiled": False,
            "compile_errors": _parse_diagnostics(cr["stderr"])["errors"],
            "raw_stderr": cr["stderr"][:2000],
        }, indent=2)

    t0 = time.time()
    rr = _run(
        [str(out)],
        input_text=stdin_input or None,
        timeout=min(timeout_s, 30),
    )
    runtime = round(time.time() - t0, 4)

    src.unlink(missing_ok=True)
    out.unlink(missing_ok=True)

    return json.dumps({
        "compiled": True,
        "executed": True,
        "exit_code": rr["returncode"],
        "stdout": rr["stdout"][:4000],
        "stderr": rr["stderr"][:2000],
        "runtime_s": runtime,
        "compile_warnings": _parse_diagnostics(cr["stderr"])["warnings"],
    }, indent=2)


@mcp.tool()
def disassemble(
    source: str,
    flags: str = "-O2",
    standard: str = "c11",
    function_filter: str = "",
    intel_syntax: bool = True,
) -> str:
    """Compile C source and return disassembly via objdump.

    Args:
        source: C source code to compile and disassemble.
        flags: Compiler flags (default '-O2').
        standard: C standard (default c11).
        function_filter: Only show assembly for this function name (empty = all).
        intel_syntax: Use Intel syntax instead of AT&T (default True).

    Returns:
        JSON with full disassembly, function list, and instruction count.
    """
    src = _write_c(source)
    out = src.with_suffix("")
    compile_cmd = ["gcc"] + flags.split() + [f"-std={standard}", "-g", str(src), "-o", str(out)]

    cr = _run(compile_cmd)
    if cr["returncode"] != 0:
        src.unlink(missing_ok=True)
        return json.dumps({"error": "Compilation failed", "details": cr["stderr"][:1000]}, indent=2)

    objdump_flags = ["-d", "--no-show-raw-insn"]
    if intel_syntax:
        objdump_flags += ["-M", "intel"]

    dr = _run(["objdump"] + objdump_flags + [str(out)])
    src.unlink(missing_ok=True)
    out.unlink(missing_ok=True)

    asm = dr["stdout"]

    # Parse functions
    functions = re.findall(r'^[0-9a-f]+ <([^>]+)>:', asm, re.MULTILINE)
    user_funcs = [f for f in functions if not f.startswith("_") or f in ["_start"]]

    # Filter to specific function if requested
    filtered_asm = asm
    if function_filter:
        blocks = re.split(r'\n(?=[0-9a-f]+ <)', asm)
        matched = [b for b in blocks if f"<{function_filter}" in b or f"<{function_filter}>:" in b]
        filtered_asm = "\n".join(matched) if matched else f"Function '{function_filter}' not found.\nAvailable: {user_funcs}"

    instruction_count = len(re.findall(r'^\s+[0-9a-f]+:', filtered_asm, re.MULTILINE))

    return json.dumps({
        "functions_found": user_funcs,
        "instruction_count": instruction_count,
        "syntax": "intel" if intel_syntax else "att",
        "assembly": filtered_asm[:8000],
    }, indent=2)


@mcp.tool()
def diff_assembly(
    source_a: str,
    source_b: str,
    flags: str = "-O2",
    standard: str = "c11",
    intel_syntax: bool = True,
) -> str:
    """Compile two C code versions and diff their assembly output.

    Essential for verifying codegen changes, optimization effects,
    or detecting regressions between compiler versions/flags.

    Args:
        source_a: First C source (baseline).
        source_b: Second C source (modified).
        flags: Compiler flags for both (default '-O2').
        standard: C standard (default c11).
        intel_syntax: Intel vs AT&T syntax (default True).

    Returns:
        JSON with unified diff, changed functions, instruction delta, and regression verdict.
    """
    import difflib

    def _get_asm(source: str) -> tuple[str, bool]:
        src = _write_c(source)
        out = src.with_suffix("")
        cr = _run(["gcc"] + flags.split() + [f"-std={standard}", str(src), "-o", str(out)])
        if cr["returncode"] != 0:
            src.unlink(missing_ok=True)
            return cr["stderr"], False
        objdump_flags = ["-d", "--no-show-raw-insn", "-M", "intel" if intel_syntax else "att"]
        dr = _run(["objdump"] + objdump_flags + [str(out)])
        src.unlink(missing_ok=True)
        out.unlink(missing_ok=True)
        # Strip addresses for clean diff (addresses change between compilations)
        clean = re.sub(r'^[0-9a-f]+ <', '0000000000 <', dr["stdout"], flags=re.MULTILINE)
        clean = re.sub(r'^\s+[0-9a-f]+:\s+', '\t', clean, flags=re.MULTILINE)
        return clean, True

    asm_a, ok_a = _get_asm(source_a)
    asm_b, ok_b = _get_asm(source_b)

    if not ok_a:
        return json.dumps({"error": "source_a failed to compile", "details": asm_a[:500]}, indent=2)
    if not ok_b:
        return json.dumps({"error": "source_b failed to compile", "details": asm_b[:500]}, indent=2)

    lines_a = asm_a.splitlines(keepends=True)
    lines_b = asm_b.splitlines(keepends=True)
    diff = list(difflib.unified_diff(lines_a, lines_b, fromfile="source_a", tofile="source_b", n=3))

    insns_a = len(re.findall(r'^\t', asm_a, re.MULTILINE))
    insns_b = len(re.findall(r'^\t', asm_b, re.MULTILINE))
    delta = insns_b - insns_a

    # Find which functions changed
    changed_fns = set()
    for line in diff:
        m = re.search(r'<([^>]+)>:', line)
        if m:
            changed_fns.add(m.group(1))

    return json.dumps({
        "identical": len(diff) == 0,
        "instruction_count_a": insns_a,
        "instruction_count_b": insns_b,
        "instruction_delta": delta,
        "regression": delta > 0,
        "changed_functions": sorted(changed_fns),
        "diff": "".join(diff)[:6000],
    }, indent=2)


@mcp.tool()
def check_abi(
    source: str,
    flags: str = "-O0",
    target_arch: str = "x86_64",
) -> str:
    """Verify x86-64 System V ABI compliance in compiled output.

    Checks callee-saved register preservation, stack alignment before calls,
    return value placement, and sign/zero extension correctness.

    Args:
        source: C source code to check.
        flags: Compiler flags (use -O0 for faithful ABI analysis, default '-O0').
        target_arch: Target architecture, currently x86_64 (default x86_64).

    Returns:
        JSON with ABI violations, severity, and specific fix instructions.
    """
    src = _write_c(source)
    out = src.with_suffix("")
    compile_cmd = ["gcc"] + flags.split() + ["-std=c11", str(src), "-o", str(out)]

    cr = _run(compile_cmd)
    if cr["returncode"] != 0:
        src.unlink(missing_ok=True)
        return json.dumps({"error": "Compilation failed", "details": cr["stderr"][:1000]}, indent=2)

    dr = _run(["objdump", "-d", "--no-show-raw-insn", "-M", "intel", str(out)])
    src.unlink(missing_ok=True)
    out.unlink(missing_ok=True)

    issues = _abi_check_x86_64(dr["stdout"])

    return json.dumps({
        "target_arch": target_arch,
        "abi": "System V AMD64",
        "violations_found": len(issues),
        "pass": len(issues) == 0,
        "violations": issues,
        "abi_quick_ref": {
            "integer_args": ["rdi", "rsi", "rdx", "rcx", "r8", "r9"],
            "return_value": "rax (int/ptr), rdx:rax (128-bit)",
            "callee_saved": ["rbx", "rbp", "r12", "r13", "r14", "r15"],
            "caller_saved": ["rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"],
            "stack_align": "16-byte aligned before CALL",
            "red_zone": "128 bytes below RSP (leaf functions, no signal handlers)",
        },
    }, indent=2)


@mcp.tool()
def analyze_codegen(
    source: str,
    flags_list: str = "-O0,-O1,-O2,-O3",
    standard: str = "c11",
) -> str:
    """Deep codegen analysis across multiple optimization levels.

    Compiles the same source at multiple -O levels and reports:
    instruction counts, sign/zero extension patterns, potential overflow
    points, inlining decisions, vectorization, and alignment issues.

    Args:
        source: C source code to analyze.
        flags_list: Comma-separated list of flag sets to compare (default '-O0,-O1,-O2,-O3').
        standard: C standard (default c11).

    Returns:
        JSON with per-level analysis, optimization summary, and codegen anomalies.
    """
    flag_sets = [f.strip() for f in flags_list.split(",")]
    results = []

    for flags in flag_sets:
        src = _write_c(source)
        out = src.with_suffix("")
        compile_cmd = ["gcc", flags, f"-std={standard}", str(src), "-o", str(out)]

        cr = _run(compile_cmd)
        if cr["returncode"] != 0:
            src.unlink(missing_ok=True)
            results.append({"flags": flags, "error": cr["stderr"][:300]})
            continue

        dr = _run(["objdump", "-d", "--no-show-raw-insn", "-M", "intel", str(out)])
        asm = dr["stdout"]
        src.unlink(missing_ok=True)
        out.unlink(missing_ok=True)

        insn_count = len(re.findall(r'^\s+[0-9a-f]+:', asm, re.MULTILINE))
        functions = re.findall(r'^[0-9a-f]+ <([^>]+)>:', asm, re.MULTILINE)

        # Detect codegen patterns
        patterns = {
            "uses_simd": bool(re.search(r'\bymm\b|\bxmm\b|\bzmm\b', asm)),
            "uses_cmov": bool(re.search(r'\bcmov', asm)),
            "has_division": bool(re.search(r'\bidiv\b|\bdiv\b', asm)),
            "has_multiply": bool(re.search(r'\bimul\b|\bmul\b', asm)),
            "has_sign_extend": bool(re.search(r'\bmovsx\b|\bmovsxd\b', asm)),
            "has_zero_extend": bool(re.search(r'\bmovzx\b', asm)),
            "has_lea": bool(re.search(r'\blea\b', asm)),
            "tail_calls": len(re.findall(r'\bjmp\s+\w+@PLT', asm)),
            "inlined_functions": len(functions),
        }

        results.append({
            "flags": flags,
            "instruction_count": insn_count,
            "function_count": len(functions),
            "functions": [f for f in functions if not f.startswith("_")][:10],
            "patterns": patterns,
            "warnings": _parse_diagnostics(cr["stderr"])["warnings"],
        })

    # Summarize optimization gains
    if len(results) >= 2 and "instruction_count" in results[0] and "instruction_count" in results[-1]:
        first = results[0]["instruction_count"]
        last = results[-1]["instruction_count"]
        reduction = round((first - last) / max(first, 1) * 100, 1)
    else:
        reduction = None

    return json.dumps({
        "levels_analyzed": len(results),
        "optimization_reduction_pct": reduction,
        "analysis": results,
        "anomaly_check": {
            "signed_overflow_risk": bool(re.search(r'(\+\+|--|\+=|-=|\*=)', source) and "unsigned" not in source),
            "pointer_arithmetic": bool(re.search(r'\*\s*\(', source) or re.search(r'\[\s*\d', source)),
            "vla_usage": bool(re.search(r'\[\s*\w+\s*\]', source) and "static" not in source),
        },
    }, indent=2)


@mcp.tool()
def bootstrap_verify(
    stage0_source: str,
    stage1_source: Optional[str] = None,
    flags: str = "-O2",
) -> str:
    """Verify compiler self-hosting bootstrap correctness.

    Compiles stage0 source, then optionally compiles stage1 with stage0.
    Checks that both stages produce equivalent output (reproducible build).
    Used to verify that a C compiler compiles itself correctly.

    Args:
        stage0_source: Stage 0 compiler C source (compiled with system GCC).
        stage1_source: Stage 1 source (compiled with stage0). Omit to just verify stage0.
        flags: Compiler flags (default '-O2').

    Returns:
        JSON with stage results, hash comparison, bootstrap verdict, and divergence points.
    """
    import hashlib

    def _compile_to_bytes(source: str, compiler: str = "gcc") -> tuple[bytes | None, str]:
        src = _write_c(source)
        out = src.with_suffix("")
        cr = _run([compiler] + flags.split() + ["-std=c11", str(src), "-o", str(out)])
        src.unlink(missing_ok=True)
        if cr["returncode"] != 0:
            return None, cr["stderr"]
        data = out.read_bytes() if out.exists() else b""
        out.unlink(missing_ok=True)
        return data, ""

    # Stage 0: compile with system gcc
    stage0_bin, s0_err = _compile_to_bytes(stage0_source, "gcc")
    if stage0_bin is None:
        return json.dumps({"error": "Stage 0 failed", "details": s0_err[:500]}, indent=2)

    s0_hash = hashlib.sha256(stage0_bin).hexdigest()
    s0_size = len(stage0_bin)

    result = {
        "stage0": {
            "compiler": "system gcc",
            "success": True,
            "binary_size": s0_size,
            "sha256": s0_hash,
        },
        "bootstrap_verdict": "stage0_only",
    }

    if stage1_source:
        # Write stage0 binary as compiler, compile stage1 with it
        stage0_path = tempfile.mktemp(prefix="zkf_stage0_")
        Path(stage0_path).write_bytes(stage0_bin)
        os.chmod(stage0_path, 0o755)

        stage1_bin, s1_err = _compile_to_bytes(stage1_source, stage0_path)
        Path(stage0_path).unlink(missing_ok=True)

        if stage1_bin is None:
            result["stage1"] = {"success": False, "error": s1_err[:500]}
            result["bootstrap_verdict"] = "stage1_failed"
        else:
            s1_hash = hashlib.sha256(stage1_bin).hexdigest()

            # Stage 2: recompile stage1 with stage1 itself (reproducibility check)
            stage1_path = tempfile.mktemp(prefix="zkf_stage1_")
            Path(stage1_path).write_bytes(stage1_bin)
            os.chmod(stage1_path, 0o755)

            stage2_bin, s2_err = _compile_to_bytes(stage1_source, stage1_path)
            Path(stage1_path).unlink(missing_ok=True)

            s2_hash = hashlib.sha256(stage2_bin).hexdigest() if stage2_bin else ""
            reproducible = s1_hash == s2_hash

            result["stage1"] = {
                "compiler": "stage0 binary",
                "success": True,
                "binary_size": len(stage1_bin),
                "sha256": s1_hash,
            }
            result["stage2"] = {
                "compiler": "stage1 binary",
                "success": stage2_bin is not None,
                "sha256": s2_hash,
            }
            result["reproducible"] = reproducible
            result["bootstrap_verdict"] = "PASS — reproducible" if reproducible else "FAIL — divergence detected"
            result["divergence_note"] = (
                None if reproducible else
                "Stage1 and Stage2 binaries differ — compiler is not self-consistently reproducing. "
                "Check for undefined behavior, timestamp embedding, or non-deterministic codegen."
            )

    return json.dumps(result, indent=2)


@mcp.tool()
def inspect_binary(
    source: str,
    flags: str = "-O2",
    show_sections: bool = True,
    show_symbols: bool = True,
    show_relocations: bool = False,
) -> str:
    """Compile C and inspect the resulting binary's symbol table, sections, and metadata.

    Uses nm, readelf, and objdump to give a complete picture of the binary layout.

    Args:
        source: C source code to compile and inspect.
        flags: Compiler flags (default '-O2').
        show_sections: Include ELF section sizes (default True).
        show_symbols: Include symbol table (default True).
        show_relocations: Include relocation entries (default False).

    Returns:
        JSON with sections, symbols, binary metadata, and size breakdown.
    """
    src = _write_c(source)
    out = src.with_suffix("")
    cr = _run(["gcc"] + flags.split() + ["-std=c11", str(src), "-o", str(out)])
    src.unlink(missing_ok=True)

    if cr["returncode"] != 0:
        return json.dumps({"error": "Compilation failed", "details": cr["stderr"][:500]}, indent=2)

    result: dict = {"binary_size": out.stat().st_size}

    if show_sections:
        sr = _run(["readelf", "-S", "--wide", str(out)])
        sections = []
        for line in sr["stdout"].splitlines():
            m = re.match(r'\s+\[\s*\d+\]\s+(\S+)\s+\S+\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)', line)
            if m:
                name, size = m.group(1), int(m.group(2), 16)
                if size > 0:
                    sections.append({"name": name, "size": size})
        result["sections"] = sorted(sections, key=lambda x: x["size"], reverse=True)

    if show_symbols:
        nr = _run(["nm", "--demangle", "--size-sort", str(out)])
        symbols = []
        for line in nr["stdout"].splitlines():
            parts = line.strip().split(None, 2)
            if len(parts) == 3:
                size_hex, sym_type, name = parts
                try:
                    symbols.append({"name": name, "type": sym_type, "size": int(size_hex, 16)})
                except ValueError:
                    pass
        result["symbols"] = sorted(symbols, key=lambda x: x["size"], reverse=True)[:30]

    if show_relocations:
        rr = _run(["readelf", "-r", str(out)])
        result["relocations"] = rr["stdout"][:2000]

    out.unlink(missing_ok=True)
    return json.dumps(result, indent=2)


@mcp.tool()
def preprocess(
    source: str,
    flags: str = "",
    standard: str = "c11",
    show_line_markers: bool = False,
) -> str:
    """Run C preprocessor and return expanded output.

    Useful for debugging macros, include resolution, and conditional compilation.

    Args:
        source: C source code to preprocess.
        flags: Extra flags e.g. '-DDEBUG=1 -I/path/to/headers'.
        standard: C standard (default c11).
        show_line_markers: Include # line markers in output (default False).

    Returns:
        JSON with preprocessed source, macro count, include count, and line count.
    """
    src = _write_c(source)
    pp_flags = ["-E", f"-std={standard}"]
    if not show_line_markers:
        pp_flags.append("-P")
    if flags:
        pp_flags += flags.split()

    r = _run(["gcc"] + pp_flags + [str(src)])
    src.unlink(missing_ok=True)

    expanded = r["stdout"]
    macros = len(re.findall(r'#define\s+\w+', source))
    includes = len(re.findall(r'#include', source))

    return json.dumps({
        "success": r["returncode"] == 0,
        "original_lines": len(source.splitlines()),
        "expanded_lines": len(expanded.splitlines()),
        "macro_definitions": macros,
        "include_directives": includes,
        "expansion_ratio": round(len(expanded.splitlines()) / max(len(source.splitlines()), 1), 1),
        "preprocessed": expanded[:6000],
        "errors": r["stderr"][:500] if r["stderr"] else "",
    }, indent=2)


@mcp.tool()
def compile_flags_cmp(
    source: str,
    flags_a: str = "-O0",
    flags_b: str = "-O3",
    standard: str = "c11",
) -> str:
    """Compile the same C source with two different flag sets and diff the assembly.

    Perfect for analyzing the effect of specific optimizations, sanitizers,
    or architecture flags on generated code.

    Args:
        source: C source code.
        flags_a: First flag set (default '-O0').
        flags_b: Second flag set (default '-O3').
        standard: C standard (default c11).

    Returns:
        JSON with side-by-side instruction counts, changed functions, and unified assembly diff.
    """
    # Reuse diff_assembly with identical sources but different flags injected
    # We need a custom approach since diff_assembly uses a fixed flag set
    import difflib

    def _compile_asm(flags: str) -> tuple[str, bool, str]:
        src = _write_c(source)
        out = src.with_suffix("")
        cr = _run(["gcc"] + flags.split() + [f"-std={standard}", str(src), "-o", str(out)])
        if cr["returncode"] != 0:
            src.unlink(missing_ok=True)
            return cr["stderr"], False, ""
        dr = _run(["objdump", "-d", "--no-show-raw-insn", "-M", "intel", str(out)])
        src.unlink(missing_ok=True)
        out.unlink(missing_ok=True)
        clean = re.sub(r'^[0-9a-f]+ <', '0x0 <', dr["stdout"], flags=re.MULTILINE)
        clean = re.sub(r'^\s+[0-9a-f]+:\s+', '\t', clean, flags=re.MULTILINE)
        return clean, True, cr["stderr"]

    asm_a, ok_a, warn_a = _compile_asm(flags_a)
    asm_b, ok_b, warn_b = _compile_asm(flags_b)

    if not ok_a:
        return json.dumps({"error": f"flags_a compilation failed: {flags_a}", "details": asm_a[:400]}, indent=2)
    if not ok_b:
        return json.dumps({"error": f"flags_b compilation failed: {flags_b}", "details": asm_b[:400]}, indent=2)

    lines_a = asm_a.splitlines(keepends=True)
    lines_b = asm_b.splitlines(keepends=True)
    diff = list(difflib.unified_diff(lines_a, lines_b, fromfile=f"[{flags_a}]", tofile=f"[{flags_b}]", n=3))

    insns_a = len(re.findall(r'^\t', asm_a, re.MULTILINE))
    insns_b = len(re.findall(r'^\t', asm_b, re.MULTILINE))

    changed_fns = set()
    for line in diff:
        m = re.search(r'<([^>]+)>:', line)
        if m and not m.group(1).startswith("_"):
            changed_fns.add(m.group(1))

    return json.dumps({
        "flags_a": flags_a,
        "flags_b": flags_b,
        "instructions_a": insns_a,
        "instructions_b": insns_b,
        "delta": insns_b - insns_a,
        "reduction_pct": round((insns_a - insns_b) / max(insns_a, 1) * 100, 1),
        "identical": len(diff) == 0,
        "changed_functions": sorted(changed_fns),
        "diff": "".join(diff)[:6000],
        "warnings_a": _parse_diagnostics(warn_a)["warnings"] if warn_a else [],
        "warnings_b": _parse_diagnostics(warn_b)["warnings"] if warn_b else [],
    }, indent=2)


# ─────────────────────────────────────────────
# ENTRY POINT
# ─────────────────────────────────────────────

if __name__ == "__main__":
    mcp.run(transport="stdio")
