"""
Zkaedi Prime Fuser — experiments.
Run: python experiments_fuser.py
Varies steps, safety_bias, perf_bias (and optionally SDE params) and reports backend chosen + energy.
"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import numpy as np
from zkaedi_prime_fuser import ZkaediPrimeFuser

# Suppress print from fuser during experiments
def _quiet_fuse(fuser, source, **kwargs):
    import io
    import contextlib
    with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
        return fuser.fuse(source, **kwargs)

def run_backend_choice_experiments():
    """Vary biases and steps; report which backend is chosen (no subprocess)."""
    src_size = 400  # small script
    print("Backend choice experiments (source_size=400, mode=auto)")
    print("-" * 70)
    results = []
    np.random.seed(42)
    for safety_bias in [0.3, 0.7, 0.95]:
        for perf_bias in [0.2, 0.5, 0.9]:
            for steps in [20, 100, 200]:
                f = ZkaediPrimeFuser()
                backend = f._choose_backend(src_size, safety_bias=safety_bias, perf_bias=perf_bias, steps=steps)
                energy = float(np.min(f.H))
                results.append((safety_bias, perf_bias, steps, backend, energy))
    # Dedupe display: show one row per (safety_bias, perf_bias) with steps=200
    print(f"{'safety_bias':>12} {'perf_bias':>10} {'steps':>6} -> backend           energy")
    for (sb, pb, steps, backend, energy) in results:
        if steps == 200:
            print(f"{sb:>12.2f} {pb:>10.2f} {steps:>6} -> {backend:<18} {energy:>8.3f}")
    print()
    return results

def run_steps_sweep():
    """Same biases, varying steps — see if backend stabilizes."""
    print("Steps sweep (safety_bias=0.7, perf_bias=0.8)")
    print("-" * 70)
    np.random.seed(123)
    f = ZkaediPrimeFuser()
    print(f"{'steps':>8} -> backend           energy")
    for steps in [5, 10, 20, 50, 100, 200, 500]:
        backend = f._choose_backend(400, safety_bias=0.7, perf_bias=0.8, steps=steps)
        energy = float(np.min(f.H))
        print(f"{steps:>8} -> {backend:<18} {energy:>8.3f}")
    print()

def run_sde_param_sweep():
    """Vary eta, sigma; fixed biases and steps."""
    print("SDE param sweep (eta, sigma) — safety=0.7, perf=0.8, steps=100")
    print("-" * 70)
    print(f"{'eta':>6} {'sigma':>6} -> backend           energy")
    for eta in [0.2, 0.42, 0.6]:
        for sigma in [0.02, 0.05, 0.12]:
            np.random.seed(1)
            f = ZkaediPrimeFuser(eta=eta, sigma=sigma)
            backend = f._choose_backend(400, safety_bias=0.7, perf_bias=0.8, steps=100)
            energy = float(np.min(f.H))
            print(f"{eta:>6.2f} {sigma:>6.2f} -> {backend:<18} {energy:>8.3f}")
    print()

def run_quick_fuse_experiments():
    """Run fuse() for safety/script; for auto only report chosen backend (no Nuitka run)."""
    print("Quick fuse experiments (tiny snippet, modes safety | script | auto with steps=10)")
    print("-" * 70)
    code = "print(1)"
    tmp = Path("_experiment_tmp.py")
    tmp.write_text(code)
    try:
        f = ZkaediPrimeFuser()
        for mode in ["safety", "script"]:
            res = _quiet_fuse(f, code, filename=str(tmp), mode=mode, steps=10)
            print(f"  mode={mode:<8} -> backend={res.backend_used:<14} success={res.success} time={res.time_taken:.3f}s")
        # Auto: report backend choice only (avoid running Nuitka for minutes)
        backend = f._choose_backend(len(code), steps=10)
        print(f"  mode=auto    -> backend={backend:<14} (choice only, not run)")
    finally:
        if tmp.exists():
            tmp.unlink(missing_ok=True)
    print()


# ------------------------- Advanced experiments -------------------------

def run_multi_seed_stability(quiet=False):
    """Same config, many seeds — report backend distribution (stability). Returns Counter."""
    from collections import Counter
    backends = []
    for seed in range(50):
        np.random.seed(seed)
        f = ZkaediPrimeFuser()
        backend = f._choose_backend(400, safety_bias=0.7, perf_bias=0.5, steps=100)
        backends.append(backend)
    counts = Counter(backends)
    if not quiet:
        print("Multi-seed stability (safety=0.7, perf=0.5, steps=100, seeds=50)")
        print("-" * 70)
        print("  backend           count")
        for b, c in counts.most_common():
            print(f"  {b:<18} {c:>5}")
        print()
    return counts

def run_source_size_sweep():
    """Vary source size — TCC/uv favored for small, others for large."""
    print("Source size sweep (safety=0.7, perf=0.8, steps=100)")
    print("-" * 70)
    np.random.seed(1)
    f = ZkaediPrimeFuser()
    print(f"{'source_size':>12} -> backend           energy")
    for size in [50, 200, 500, 1000, 2000, 5000, 15000]:
        backend = f._choose_backend(size, safety_bias=0.7, perf_bias=0.8, steps=100)
        energy = float(np.min(f.H))
        print(f"{size:>12} -> {backend:<18} {energy:>8.3f}")
    print()

def run_H_trajectory():
    """Record H and chosen backend at sample steps (convergence)."""
    print("H trajectory (safety=0.7, perf=0.8, source_size=400, steps=200)")
    print("-" * 70)
    np.random.seed(5)
    f = ZkaediPrimeFuser()
    sample_at = [0, 25, 50, 100, 150, 200]
    H_history = []
    # Re-run evolution and sample H at each point
    H = f.H.copy()
    for t in range(201):
        if t in sample_at:
            idx = int(np.argmin(H))
            backend = f.backends[idx % len(f.backends)]
            energy = float(np.min(H))
            H_history.append((t, backend, energy))
        if t >= 200:
            break
        sigmoid = 1 / (1 + np.exp(-f.gamma * H))
        drift = f.eta * H * sigmoid
        noise = np.random.normal(0, 1 + f.beta * np.abs(H)) * f.sigma
        F = np.zeros_like(H)
        F[0] -= 0.3   # source_size 400 < 500
        F[3] -= 0.7 * 1.2
        F[2] -= 0.8 * 1.8
        H = H + drift + noise + F
        H = H / (np.max(np.abs(H)) + 1e-8) + 0.01
    print(f"  {'step':>6} -> backend           energy")
    for t, backend, energy in H_history:
        print(f"  {t:>6} -> {backend:<18} {energy:>8.3f}")
    print()

def run_boundary_scan(quiet=False):
    """Fine grid around (safety, perf) to find backend flip boundary. Returns 5x5 grid of 'R'/'G'."""
    np.random.seed(0)
    safety_vals = [0.5, 0.6, 0.7, 0.8, 0.9]
    perf_vals = [0.3, 0.4, 0.5, 0.6, 0.7]
    grid = []
    for sb in safety_vals:
        row = []
        for pb in perf_vals:
            f = ZkaediPrimeFuser()
            backend = f._choose_backend(400, safety_bias=sb, perf_bias=pb, steps=100)
            short = "R" if "rust" in backend else "G" if "gcc" in backend else "?"
            row.append(short)
        grid.append(row)
    if not quiet:
        print("Boundary scan (steps=100) — find (safety, perf) where backend flips")
        print("-" * 70)
        print("       ", end="")
        for p in perf_vals:
            print(f" p={p:.1f}    ", end="")
        print()
        for i, sb in enumerate(safety_vals):
            row = f" s={sb:.1f} "
            for short in grid[i]:
                row += f" {short:<8}"
            print(row)
        print("  (R=rust_safe, G=gcc_perf)")
        print()
    return grid

def run_slice(perf_fixed=0.3, safety_vals=None, steps=100):
    """Fixed perf_bias, sweep safety_bias — print (p, s) -> backend."""
    if safety_vals is None:
        safety_vals = [0.5, 0.6, 0.7, 0.8, 0.9]
    print("Slice: p={} (safety sweep), steps={}".format(perf_fixed, steps))
    print("-" * 70)
    np.random.seed(0)
    f = ZkaediPrimeFuser()
    print("  p={}  s     -> backend".format(perf_fixed))
    for s in safety_vals:
        backend = f._choose_backend(400, safety_bias=s, perf_bias=perf_fixed, steps=steps)
        print("  p={:.1f}  s={:.1f} -> {}".format(perf_fixed, s, backend))
    print()

# ------------------------- Deep experiments -------------------------

def run_fine_boundary(step=0.05):
    """Finer grid (default 0.05) to locate boundary more precisely."""
    safety_vals = [round(0.5 + i * step, 2) for i in range(int((0.9 - 0.5) / step) + 1)]
    perf_vals = [round(0.3 + i * step, 2) for i in range(int((0.7 - 0.3) / step) + 1)]
    print("Fine boundary (step={})".format(step))
    print("-" * 70)
    np.random.seed(0)
    grid = []
    for sb in safety_vals:
        row = []
        for pb in perf_vals:
            f = ZkaediPrimeFuser()
            backend = f._choose_backend(400, safety_bias=sb, perf_bias=pb, steps=100)
            row.append("R" if "rust" in backend else "G")
        grid.append(row)
    print("      " + "".join(f" p={p:.2f}" for p in perf_vals))
    for i, sb in enumerate(safety_vals):
        print(" s={:.2f} ".format(sb) + "".join(f" {grid[i][j]:^6}" for j in range(len(perf_vals))))
    print("  (R=rust_safe, G=gcc_perf)")
    print()

def run_convergence_step(safety=0.7, perf=0.5, steps=200, window=20):
    """Report first step at which chosen backend stabilizes (same for last `window` steps)."""
    print("Convergence step (s={}, p={}, steps={}, stable window={})".format(safety, perf, steps, window))
    print("-" * 70)
    np.random.seed(0)
    f = ZkaediPrimeFuser()
    H = f.H.copy()
    backend_at = []
    for t in range(steps):
        idx = int(np.argmin(H))
        backend_at.append(f.backends[idx % len(f.backends)])
        if t >= steps - 1:
            break
        sigmoid = 1 / (1 + np.exp(-f.gamma * H))
        drift = f.eta * H * sigmoid
        noise = np.random.normal(0, 1 + f.beta * np.abs(H)) * f.sigma
        F = np.zeros_like(H)
        F[0] -= 0.3
        F[3] -= safety * 1.2
        F[2] -= perf * 1.8
        H = H + drift + noise + F
        H = H / (np.max(np.abs(H)) + 1e-8) + 0.01
    final_backend = backend_at[-1]
    stable_from = None
    for t in range(steps - window):
        if all(backend_at[t + k] == final_backend for k in range(window)):
            stable_from = t
            break
    print("  final backend: {}".format(final_backend))
    print("  stable from step: {} (backend unchanged for last {} steps from there)".format(stable_from if stable_from is not None else "N/A", window))
    if stable_from is not None and stable_from > 0:
        print("  backend at step 0: {}".format(backend_at[0]))
    print()

def run_reverse_slice(safety_fixed=0.7, perf_vals=None, steps=100):
    """Fixed safety_bias, sweep perf_bias (reverse of slice)."""
    if perf_vals is None:
        perf_vals = [0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7]
    print("Reverse slice: s={} (perf sweep), steps={}".format(safety_fixed, steps))
    print("-" * 70)
    np.random.seed(0)
    f = ZkaediPrimeFuser()
    print("  s={}  p     -> backend".format(safety_fixed))
    for p in perf_vals:
        backend = f._choose_backend(400, safety_bias=safety_fixed, perf_bias=p, steps=steps)
        print("  s={:.2f}  p={:.2f} -> {}".format(safety_fixed, p, backend))
    print()

def run_sensitivity(safety=0.7, perf=0.5, epsilon=0.05, steps=100):
    """At (s, p), probe ±epsilon; report backend flips."""
    print("Sensitivity at (s={}, p={}), epsilon={}".format(safety, perf, epsilon))
    print("-" * 70)
    np.random.seed(0)
    f = ZkaediPrimeFuser()
    center = f._choose_backend(400, safety_bias=safety, perf_bias=perf, steps=steps)
    probes = [
        (safety + epsilon, perf, "s+"),
        (safety - epsilon, perf, "s-"),
        (safety, perf + epsilon, "p+"),
        (safety, perf - epsilon, "p-"),
    ]
    print("  center (s={}, p={}) -> {}".format(safety, perf, center))
    for s, p, label in probes:
        if 0 <= s <= 1 and 0 <= p <= 1:
            b = f._choose_backend(400, safety_bias=s, perf_bias=p, steps=steps)
            flip = " (flip)" if b != center else ""
            print("  {} (s={:.2f}, p={:.2f}) -> {}{}".format(label, s, p, b, flip))
    print()

if __name__ == "__main__":
    import argparse
    p = argparse.ArgumentParser(description="Zkaedi Prime Fuser experiments")
    p.add_argument("--no-fuse", action="store_true", help="Skip quick fuse step")
    p.add_argument("--advanced", "-a", action="store_true", help="Run advanced: multi-seed, source-size, H trajectory, boundary scan")
    p.add_argument("--runs", "-r", type=int, default=1, metavar="N", help="Repeat advanced (multi-seed + boundary) N times and print aggregate (default 1)")
    p.add_argument("--slice", type=float, default=None, metavar="P", help="Run slice: fixed perf_bias P, sweep safety 0.5..0.9 (e.g. --slice 0.3)")
    p.add_argument("--deep", "-d", action="store_true", help="Run deep: fine boundary, convergence step, reverse slice, sensitivity")
    args = p.parse_args()
    runs = max(1, args.runs)
    print("=== Zkaedi Prime Fuser — experiments ===\n")
    if args.slice is not None:
        run_slice(perf_fixed=args.slice)
        print("Done.")
        raise SystemExit(0)
    if args.deep:
        run_fine_boundary(step=0.05)
        run_convergence_step(safety=0.7, perf=0.5, steps=200, window=20)
        run_reverse_slice(safety_fixed=0.7)
        run_sensitivity(safety=0.7, perf=0.5, epsilon=0.05)
        print("Done.")
        raise SystemExit(0)
    run_backend_choice_experiments()
    run_steps_sweep()
    run_sde_param_sweep()
    if args.advanced:
        multi_seed_counts_list = []
        boundary_grids = []
        for run in range(runs):
            multi_seed_counts_list.append(run_multi_seed_stability(quiet=(runs > 1 and run > 0)))
            boundary_grids.append(run_boundary_scan(quiet=(runs > 1 and run > 0)))
        run_source_size_sweep()
        run_H_trajectory()
        if runs > 1:
            # Aggregate over runs
            from collections import Counter
            print("Summary over {} runs (multi-seed + boundary)".format(runs))
            print("-" * 70)
            tot = Counter()
            for c in multi_seed_counts_list:
                tot.update(c)
            print("  Multi-seed totals ({} x 50 seeds):".format(runs))
            for b, count in tot.most_common():
                print(f"    {b:<18} {count:>5}")
            print("  Boundary R count (R=rust_safe in how many of {} runs):".format(runs))
            safety_vals = [0.5, 0.6, 0.7, 0.8, 0.9]
            perf_vals = [0.3, 0.4, 0.5, 0.6, 0.7]
            print("       ", end="")
            for p in perf_vals:
                print(f" p={p:.1f}  ", end="")
            print()
            for i, sb in enumerate(safety_vals):
                row = f" s={sb:.1f} "
                for j in range(len(perf_vals)):
                    r_count = sum(1 for g in boundary_grids if g[i][j] == "R")
                    row += f" {r_count}/{runs}   "
                print(row)
            print()
    if not args.no_fuse:
        run_quick_fuse_experiments()
    else:
        print("(Skipped quick fuse; use without --no-fuse to run fuse experiments.)\n")
    print("Done.")
