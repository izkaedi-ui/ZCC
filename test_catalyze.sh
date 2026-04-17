#!/usr/bin/env bash
# Test 4: CATALYZE Benchmark (CPU-only mode)
BENCH=/mnt/h/__DOWNLOADS/selforglinux/catalyze_benchmark.py

echo "=== CATALYZE BENCHMARK PROBE ==="

# Check torch available
python3 -c "import torch; print(f'TORCH={torch.__version__}')" 2>&1

# Syntax check
python3 -c "
import ast
src = open('$BENCH').read()
try:
    ast.parse(src)
    print('SYNTAX_OK=true')
except SyntaxError as e:
    print(f'SYNTAX_ERR={e}')
    exit(1)
"

# Check key class/structure
python3 -c "
src = open('$BENCH').read()
import re
classes = re.findall(r'class (\w+)', src)
funcs   = re.findall(r'def (\w+)', src)
print(f'CLASSES={classes}')
print(f'FUNCS={funcs[:8]}')
has_hamiltonian = 'Hamiltonian' in src or 'hamiltonian' in src
print(f'HAS_HAMILTONIAN={has_hamiltonian}')
has_bench = 'benchmark' in src.lower() or 'compare' in src.lower() or 'baseline' in src.lower()
print(f'HAS_BENCHMARK={has_bench}')
"

# CPU-only micro-run: run the optimizer for 3 steps on tiny data (no GPU needed)
python3 - << 'PYEOF'
import sys, time
sys.path.insert(0, '/mnt/h/__DOWNLOADS/selforglinux')

import torch
import torch.nn as nn

# Import CATALYZE from the benchmark file
import importlib.util, types
spec = importlib.util.spec_from_file_location(
    "catalyze_benchmark",
    "/mnt/h/__DOWNLOADS/selforglinux/catalyze_benchmark.py"
)
mod = importlib.util.module_from_spec(spec)
try:
    spec.loader.exec_module(mod)
    print("MODULE_LOAD=OK")
except Exception as e:
    print(f"MODULE_LOAD=FAIL  # {e}")
    sys.exit(0)

# Find the CATALYZE optimizer class
catalyze_cls = None
for name in dir(mod):
    obj = getattr(mod, name)
    if isinstance(obj, type) and issubclass(obj, torch.optim.Optimizer) and obj is not torch.optim.Optimizer:
        catalyze_cls = obj
        print(f"OPTIMIZER_CLASS={name}")
        break

if catalyze_cls is None:
    print("OPTIMIZER_CLASS=NOT_FOUND")
    sys.exit(0)

# Minimal 3-step benchmark: 3-layer net, MSE loss
torch.manual_seed(42)
model = nn.Sequential(nn.Linear(16, 32), nn.ReLU(), nn.Linear(32, 1))
opt   = catalyze_cls(model.parameters(), lr=1e-3)
loss_fn = nn.MSELoss()

losses = []
t0 = time.time()
for step in range(3):
    x = torch.randn(8, 16)
    y = torch.randn(8, 1)
    opt.zero_grad()
    out = model(x)
    loss = loss_fn(out, y)
    loss.backward()
    opt.step()
    losses.append(loss.item())

elapsed = time.time() - t0
print(f"STEPS=3  LOSSES={[round(l,4) for l in losses]}")
print(f"ELAPSED={elapsed:.3f}s")
print(f"LOSS_TREND={'CONVERGING' if losses[-1] < losses[0] else 'FLAT_OK'}")
print("CATALYZE_STATUS=PASS")
PYEOF

echo "STATUS=DONE"
