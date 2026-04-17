#!/usr/bin/env python3
"""Quick Dream Engine status report."""
import json, re, sys
from pathlib import Path

REPO = Path("/mnt/h/__DOWNLOADS/selforglinux")
DREAMS = REPO / "dreams"

state = json.load(open(DREAMS / "dream_state.json"))

print("=" * 60)
print("  ZCC DREAM ENGINE — ONEIROGENESIS STATUS")
print("=" * 60)
print(f"  Generation:           {state['generation']}")
print(f"  Mutations tried:      {state['total_mutations_tried']}")
print(f"  Mutations survived:   {state['total_mutations_survived']}")
survived = state['total_mutations_survived']
tried = max(state['total_mutations_tried'], 1)
print(f"  Survival rate:        {survived/tried*100:.1f}%")
print(f"  Regressions:          {state['total_regressions']}")
print(f"  Algorithms:           {len(state['discovered_algorithms'])}")
print(f"  Parent hash:          {state['parent_hash']}")
print()

# Fitness trajectory
fh = state.get('fitness_history', [])
if fh:
    print("  ── FITNESS TRAJECTORY ──")
    print(f"  G1  score:  {fh[0]['score']:.0f}")
    print(f"  G{len(fh)} score:  {fh[-1]['score']:.0f}")
    improvement = fh[0]['score'] - fh[-1]['score']
    print(f"  Total gain: {improvement:.1f} (lower=better)")
    print()

# Lineage
print("  ── LINEAGE (all generations) ──")
for ev in state.get('lineage', []):
    muts = ', '.join(ev['mutations'][:2])
    if len(ev['mutations']) > 2:
        muts += f" (+{len(ev['mutations'])-2} more)"
    print(f"  G{ev['generation']:2d}  delta={ev['delta_score']:>10.1f}  {muts}")
print()

# Assembly health
asm_path = REPO / "zcc2.s"
if asm_path.exists():
    lines = open(asm_path).readlines()
    xorq_cnt = sum(1 for l in lines if l.strip().startswith("xorq"))
    shlq_cnt = sum(1 for l in lines if l.strip().startswith("shlq"))
    testq_cnt = sum(1 for l in lines if l.strip().startswith("testq"))
    imulq_cnt = sum(1 for l in lines if l.strip().startswith("imulq"))
    movq_zero = sum(1 for l in lines if re.match(r'\s*movq\s+\$0,\s*%', l))
    cmpq_zero = sum(1 for l in lines if re.match(r'\s*cmpq\s+\$0,\s*%', l))

    print("  ── ZCC2.S ASSEMBLY HEALTH ──")
    print(f"  Total lines:          {len(lines):,}")
    print(f"  xorq (zero idiom):    {xorq_cnt:,}")
    print(f"  shlq (strength red):  {shlq_cnt:,}")
    print(f"  testq:                {testq_cnt:,}")
    print(f"  imulq remaining:      {imulq_cnt:,}")
    print(f"  movq $0 remaining:    {movq_zero:,}")
    print(f"  cmpq $0 remaining:    {cmpq_zero:,}")

    # Sweep potential
    if movq_zero > 0:
        print(f"\n  ⚡ SWEEP OPPORTUNITY: {movq_zero:,} movq $0 → xorq = {movq_zero*4:,} bytes savings")
    if cmpq_zero > 0:
        print(f"  ⚡ SWEEP OPPORTUNITY: {cmpq_zero:,} cmpq $0 → testq")

print()

# Mutation engine test
try:
    sys.path.insert(0, "/mnt/h/agents/selforglinux_build")
    from zcc_dream_mutations import MutationEngine

    eng = MutationEngine(seed=42)
    test_asm = [
        "    movq $0, %rax\n",
        "    imulq $8, %rcx\n",
        "    cmpq $0, %rdx\n",
    ]
    muts = eng.dream(test_asm, max_point_mutations=5, include_sweeps=True)
    sweep_muts = [m for m in muts if m.is_sweep]
    point_muts = [m for m in muts if not m.is_sweep]
    print(f"  ── MUTATION ENGINE ──")
    print(f"  Status:               ALIVE")
    print(f"  Sweep scanners:       {len(sweep_muts)} active")
    print(f"  Point scanners:       {len(point_muts)} active")
    for m in sweep_muts:
        print(f"    SWEEP: {m.name} ({m.sweep_count} sites)")
    for m in point_muts:
        print(f"    POINT: {m.name}")
except Exception as e:
    print(f"  Mutation engine ERROR: {e}")

print()
print("=" * 60)
