#!/usr/bin/env python3
"""Dream Engine — full assembly scan for available mutations."""
import sys
sys.path.insert(0, '/mnt/h/agents/selforglinux_build')
from zcc_dream_mutations import MutationEngine

lines = open('/mnt/h/agents/selforglinux_build/zcc2.s').readlines()
print(f'ZCC2.S: {len(lines):,} lines')

eng = MutationEngine(seed=42)
muts = eng.dream(lines, max_point_mutations=10, include_sweeps=True)

sweep = [m for m in muts if m.is_sweep]
point = [m for m in muts if not m.is_sweep]

print(f'\nSWEEP mutations available: {len(sweep)}')
for m in sweep:
    print(f'  {m.name}: {m.sweep_count:,} sites, energy_delta={m.energy_delta:.0f}')

print(f'\nPOINT mutations available: {len(point)}')
for m in point[:10]:
    print(f'  {m.name}: {m.description[:80]}')

total_energy = sum(m.energy_delta for m in muts)
print(f'\nTotal energy delta available: {total_energy:.0f}')
