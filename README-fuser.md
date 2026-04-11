# Zkaedi Prime Fuser

Hamiltonian-driven backend orchestrator (TCC, Clang, GCC, Rust, Nuitka, Numba, PyO3, uv).

## What the fuser does exactly

1. **You call** `fuse(source, filename=..., mode=..., steps=..., safety_bias=..., perf_bias=...)` with either a file path or a string of code.

2. **If `mode == "auto"`** (default):
   - It builds an 8‑dimensional “energy” vector **H** (one entry per backend).
   - It runs an **Euler–Maruyama SDE** for `steps` iterations: drift (η·H·sigmoid), noise (σ), and a **forcing** term that pulls index 0 (TCC/uv) for small source size, index 3 (Rust) by `safety_bias`, and index 2 (GCC/Nuitka) by `perf_bias`.
   - It picks the backend whose **H** entry is **smallest** (lowest energy).
   - So: higher `safety_bias` tends to choose Rust; higher `perf_bias` tends to choose GCC/Nuitka; small source size tends to favor TCC/uv.

3. **If `mode` is not `"auto"`**, it ignores the SDE and uses a fixed backend: `script` → uv, `debug` → TCC, `release` → Nuitka, `safety` → Rust.

4. **It then runs that backend** on your source:
   - **uv_script**: `uv run <filename>` (run the script).
   - **tcc_fast**: compile C with TCC to an executable.
   - **rust_safe**: returns a message/path (no real build unless you add one).
   - **gcc_perf / nuitka_aot**: `python -m nuitka --standalone <filename>`.
   - **clang_diagnostics / pyo3_hybrid / etc.**: e.g. `ruff check --fix` or similar.

5. **It returns** a `FuseResult`: success/failure, output path, time, backend used, final energy, and a message. So in one call you get “which backend was chosen” and “what happened when we ran it.”

In short: **one API picks a toolchain (by an SDE + biases) and runs it on your code**, then reports the result.

## Run

```bash
pip install -r requirements-fuser.txt
python zkaedi_prime_fuser.py
```

## Self-host

Fuse the fuser with itself (same idea as a compiler compiling its own source):

```bash
python zkaedi_prime_fuser.py --self-host
# or: -s, and optional --steps 50 --mode auto --safety-bias 0.8 --perf-bias 0.7
```

## Experiments

Sweep steps, safety_bias, perf_bias, and SDE params; report which backend is chosen and energy.

```bash
python experiments_fuser.py              # includes a quick fuse step (may call uv)
python experiments_fuser.py --no-fuse   # backend + SDE sweeps only (instant)
python experiments_fuser.py --advanced  # + multi-seed stability, source-size sweep, H trajectory, boundary scan
python experiments_fuser.py --deep      # fine boundary (0.05 grid), convergence step, reverse slice, sensitivity
```

## Tests

```bash
pip install -r requirements-fuser.txt
python -m pytest tests/test_zkaedi_prime_fuser.py -v
```

## Tuning

| Where | Parameter | Default | Effect |
|-------|-----------|---------|--------|
| `ZkaediPrimeFuser(…)` | `eta` | 0.42 | SDE drift strength |
| | `gamma` | 0.33 | Sigmoid steepness |
| | `beta` | 0.11 | Noise scale with \|H\| |
| | `sigma` | 0.05 | Base noise level |
| `fuse(…)` | `steps` | 200 | Euler–Maruyama SDE steps |
| | `safety_bias` | 0.7 | Pull toward Rust/safety backends |
| | `perf_bias` | 0.8 | Pull toward GCC/Nuitka perf backends |

Example: faster auto choice with fewer steps and stronger safety bias:

```python
fuser = ZkaediPrimeFuser(eta=0.3, sigma=0.03)
res = fuser.fuse(code, filename="app.py", mode="auto", steps=50, safety_bias=0.9)
```
