"""
benchmark_results.py — ZKAEDI PRIME v5.0 Performance Benchmarks

Runs:
  1. Mode A on 256×256, 512×512, 1024×1024 for 10k and 50k iterations
  2. Mode B on the same grids for 10k steps
  3. Persistent (SYNC_EVERY=100) vs step-by-step (sync each CHUNK_SIZE) comparison

Prints an aligned table and saves benchmark_results.json.

Target machine: GIGABYTE AERO X16 | RTX 5070 (GB206 Blackwell) | 672 GB/s GDDR7
"""

from __future__ import annotations

import json
import time
from pathlib import Path
from typing import Dict, List

import torch

from zkaedi_prime_persistent import (
    CHUNK_SIZE,
    SYNC_EVERY,
    WILSON_FISHER_ETA,
    benchmark_fused_vs_separate,
    solve_prime_fused,
)

GRID_SIZES   = [(256, 256), (512, 512), (1024, 1024)]
ITER_CONFIGS = [10_000, 50_000]
MODE_B_STEPS = 10_000
SEED         = 42


def _make_field(rows: int, cols: int, seed: int = 0) -> torch.Tensor:
    gen = torch.Generator()
    gen.manual_seed(seed)
    return torch.randn(rows, cols, generator=gen)


def _tflops(rows: int, cols: int, iters: int, seconds: float) -> float:
    """Rough TFLOPS: ~10 fp32 ops per element per step."""
    return rows * cols * iters * 10 / max(seconds, 1e-9) / 1e12


def run_all() -> List[Dict]:
    if not torch.cuda.is_available():
        print("⚠  No CUDA device — benchmark requires RTX 5070")
        return []

    gpu_name = torch.cuda.get_device_name(0)
    print(f"\n🔱 ZKAEDI PRIME v5.0 — Benchmark Suite")
    print(f"   GPU       : {gpu_name}")
    print(f"   WF η      : {WILSON_FISHER_ETA:.6f}")
    print(f"   CHUNK_SIZE: {CHUNK_SIZE}  SYNC_EVERY: {SYNC_EVERY}\n")

    results: List[Dict] = []

    col_w = [12, 6, 8, 10, 12, 9]
    hdr = (
        f"{'Grid':>{col_w[0]}}  {'Mode':>{col_w[1]}}  {'Iters':>{col_w[2]}}  "
        f"{'Time(s)':>{col_w[3]}}  {'Iters/s':>{col_w[4]}}  {'TFLOPS':>{col_w[5]}}"
    )
    sep = "─" * len(hdr)
    print(sep)
    print(hdr)
    print(sep)

    for rows, cols in GRID_SIZES:
        H0 = _make_field(rows, cols, seed=SEED)

        for n_iter in ITER_CONFIGS:
            torch.cuda.synchronize()
            t0 = time.perf_counter()
            _, _, actual, converged = solve_prime_fused(
                H0, mode=0, max_iter=n_iter, tol=1e-8,
                eta=WILSON_FISHER_ETA, seed=SEED,
            )
            torch.cuda.synchronize()
            elapsed = time.perf_counter() - t0

            ips    = actual / max(elapsed, 1e-9)
            tflops = _tflops(rows, cols, actual, elapsed)

            row = {"grid": f"{rows}x{cols}", "mode": "A",
                   "n_iter": n_iter, "actual_iters": actual, "converged": converged,
                   "time_s": round(elapsed, 4), "iters_per_sec": round(ips, 1),
                   "tflops": round(tflops, 6)}
            results.append(row)
            print(
                f"{f'{rows}x{cols}':>{col_w[0]}}  {'A':>{col_w[1]}}  {n_iter:>{col_w[2]}}  "
                f"{elapsed:>{col_w[3]}.3f}  {ips:>{col_w[4]}.1f}  {tflops:>{col_w[5]}.5f}"
            )

        # Mode B — fixed budget (chaos has no convergence criterion)
        torch.cuda.synchronize()
        t0 = time.perf_counter()
        _, _, actual_b, _ = solve_prime_fused(
            H0, mode=1, max_iter=MODE_B_STEPS, tol=1e-20,
            kappa=0.4, sigma_fhn=0.001, eps_fhn=0.04, dt=0.1, seed=SEED,
        )
        torch.cuda.synchronize()
        elapsed_b = time.perf_counter() - t0

        ips_b    = actual_b / max(elapsed_b, 1e-9)
        tflops_b = _tflops(rows, cols, actual_b, elapsed_b)
        row_b = {"grid": f"{rows}x{cols}", "mode": "B",
                 "n_iter": MODE_B_STEPS, "actual_iters": actual_b, "converged": False,
                 "time_s": round(elapsed_b, 4), "iters_per_sec": round(ips_b, 1),
                 "tflops": round(tflops_b, 6)}
        results.append(row_b)
        print(
            f"{f'{rows}x{cols}':>{col_w[0]}}  {'B':>{col_w[1]}}  {MODE_B_STEPS:>{col_w[2]}}  "
            f"{elapsed_b:>{col_w[3]}.3f}  {ips_b:>{col_w[4]}.1f}  {tflops_b:>{col_w[5]}.5f}"
        )

    # ── Fused vs step-by-step ─────────────────────────────────────────────────
    print(f"\n{sep}")
    print("Persistent (SYNC=100) vs Step-by-step (sync each CHUNK_SIZE)")
    print(sep)
    fhdr = (
        f"{'Grid':>12}  {'T_pers(s)':>11}  {'T_step(s)':>11}  "
        f"{'Speedup':>9}  {'MaxDiff':>10}  {'EqFP16':>7}"
    )
    print(fhdr)
    print(sep)

    fused_results: List[Dict] = []
    for rows, cols in GRID_SIZES:
        comp = benchmark_fused_vs_separate(rows=rows, cols=cols, n_iter=10_000, seed=SEED)
        comp["grid"] = f"{rows}x{cols}"
        fused_results.append(comp)
        print(
            f"{comp['grid']:>12}  {comp['t_fast_s']:>11.4f}  {comp['t_slow_s']:>11.4f}  "
            f"{comp['speedup_fused']:>9.3f}  {comp['numerical_max_diff']:>10.6f}  "
            f"{'✓' if comp['numerically_equivalent'] else '✗':>7}"
        )

    print(sep)
    all_eq = all(r["numerically_equivalent"] for r in fused_results)
    print(f"\n  Numerical equivalence (|diff|<0.01): {'✓ ALL PASS' if all_eq else '✗ FAILURES'}")

    # ── Save JSON ─────────────────────────────────────────────────────────────
    output = {
        "gpu": gpu_name,
        "wilson_fisher_eta": WILSON_FISHER_ETA,
        "chunk_size": CHUNK_SIZE,
        "sync_every": SYNC_EVERY,
        "grid_benchmarks":  results,
        "fused_comparison": fused_results,
    }
    out_path = Path("./benchmark_results.json")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(output, f, indent=2)

    print(f"\n  Saved → {out_path}\n")
    return results


if __name__ == "__main__":
    run_all()
