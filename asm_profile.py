#!/usr/bin/env python3
"""Profile mutation candidates in ZCC's current assembly output."""
import re, sys, os
from pathlib import Path

asm = Path("zcc2.s")
if not asm.exists():
    # try the dream-evolved one
    asm = Path("dreams/dream_mutant.s")
if not asm.exists():
    print("ERROR: zcc2.s not found"); sys.exit(1)

with open(asm) as f:
    lines = f.readlines()

n = len(lines)
stats = {}
stats['total_lines'] = n
stats['pushq'] = sum(1 for l in lines if l.strip().startswith('pushq'))
stats['popq']  = sum(1 for l in lines if l.strip().startswith('popq'))
stats['imulq'] = sum(1 for l in lines if l.strip().startswith('imulq'))
stats['movq_zero'] = sum(1 for l in lines if re.match(r'\s*movq\s+\$0,', l))
stats['movl_zero'] = sum(1 for l in lines if re.match(r'\s*movl\s+\$0,', l))
stats['addq_zero'] = sum(1 for l in lines if re.match(r'\s*addq\s+\$0,', l))
stats['subq_zero'] = sum(1 for l in lines if re.match(r'\s*subq\s+\$0,', l))
stats['xorq_already'] = sum(1 for l in lines if re.match(r'\s*xorq\s+(%\w+),\s*\1', l))
stats['cmpq'] = sum(1 for l in lines if l.strip().startswith('cmpq'))
stats['je']   = sum(1 for l in lines if l.strip().startswith('je '))
stats['jne']  = sum(1 for l in lines if l.strip().startswith('jne '))
stats['call'] = sum(1 for l in lines if l.strip().startswith('call '))

# Count actual instruction lines
stats['inst_lines'] = sum(
    1 for l in lines
    if l.strip() and not l.strip().startswith('.') and
    not l.strip().endswith(':') and not l.strip().startswith('#')
)

pushq_popq_pairs = 0
pushq_popq_same  = 0
movq_self        = 0
spill_reload_elim = 0
spill_reload_mov  = 0
load_add1_store  = 0
load_sub1_store  = 0
imulq_pow2       = 0
imulq_3          = 0
imulq_5          = 0
schedule_candidates = 0

def write_regs(s):
    regs = set()
    parts = s.split()
    if not parts: return regs
    op = parts[0]
    if op in ('movq','movl','leaq','xorq','xorl','addq','subq',
              'imulq','shlq','shrq','sarq','andq','orq','incq','decq','negq'):
        ops = s[len(op):].strip().split(',')
        if ops:
            last = ops[-1].strip()
            m = re.findall(r'(%\w+)', last)
            if m and '(' not in last:
                regs.update(m)
    if op == 'popq':
        regs.update(re.findall(r'(%\w+)', s))
    return regs

def read_regs(s):
    regs = set()
    parts = s.split()
    if not parts: return regs
    op = parts[0]
    if op in ('movq','movl','leaq'):
        ops = s[len(op):].strip().split(',')
        if ops:
            regs.update(re.findall(r'(%\w+)', ops[0]))
    elif op in ('pushq',):
        regs.update(re.findall(r'(%\w+)', s))
    elif op in ('addq','subq','xorq','andq','orq','cmpq','imulq','shlq','shrq'):
        regs.update(re.findall(r'(%\w+)', s))
    return regs

for i in range(n - 2):
    s1 = lines[i].strip()
    s2 = lines[i+1].strip() if i+1 < n else ''
    s3 = lines[i+2].strip() if i+2 < n else ''

    # push/pop pairs
    m1 = re.match(r'pushq\s+(%\w+)', s1)
    m2 = re.match(r'popq\s+(%\w+)',  s2)
    if m1 and m2:
        pushq_popq_pairs += 1
        if m1.group(1) == m2.group(1):
            pushq_popq_same += 1

    # self-moves
    ms = re.match(r'movq\s+(%\w+),\s*(%\w+)', s1)
    if ms and ms.group(1) == ms.group(2):
        movq_self += 1

    # spill/reload patterns
    msp = re.match(r'movq\s+(%\w+),\s*(-?\d+\(%rbp\))', s1)
    if msp:
        mrl = re.match(r'movq\s+(-?\d+\(%rbp\)),\s*(%\w+)', s2)
        if mrl and mrl.group(1) == msp.group(2):
            if mrl.group(2) == msp.group(1):
                spill_reload_elim += 1
            else:
                spill_reload_mov += 1

    # load/inc/store → incq
    mld = re.match(r'movq\s+(-?\d+\(%rbp\)),\s*%rax', s1)
    if mld:
        if s2 == 'addq $1, %rax':
            mst = re.match(r'movq\s+%rax,\s*(-?\d+\(%rbp\))', s3)
            if mst and mst.group(1) == mld.group(1):
                load_add1_store += 1
        if s2 == 'subq $1, %rax':
            mst = re.match(r'movq\s+%rax,\s*(-?\d+\(%rbp\))', s3)
            if mst and mst.group(1) == mld.group(1):
                load_sub1_store += 1

    # imulq patterns
    mim = re.match(r'imulq\s+\$(\d+),', s1)
    if mim:
        v = int(mim.group(1))
        if v > 1 and (v & (v-1)) == 0:
            imulq_pow2 += 1
        if v == 3: imulq_3 += 1
        if v == 5: imulq_5 += 1

    # scheduling candidates
    if (s1 and not s1.endswith(':') and not s1.startswith('.')
        and s2 and not s2.endswith(':') and not s2.startswith('.')):
        rw1 = write_regs(s1); rr1 = read_regs(s1)
        rw2 = write_regs(s2); rr2 = read_regs(s2)
        if not (rw1 & rr2) and not (rw2 & rr1) and not (rw1 & rw2):
            if i+2 < n:
                rr3 = read_regs(s3)
                if rw1 & rr3:
                    schedule_candidates += 1

stats['pushq_popq_pairs']   = pushq_popq_pairs
stats['pushq_popq_same_elim'] = pushq_popq_same
stats['movq_self_elim']     = movq_self
stats['spill_reload_elim']  = spill_reload_elim
stats['spill_reload_to_mov'] = spill_reload_mov
stats['load_add1_store_incq'] = load_add1_store
stats['load_sub1_store_decq'] = load_sub1_store
stats['imulq_pow2_to_shl']  = imulq_pow2
stats['imulq_3_to_lea']     = imulq_3
stats['imulq_5_to_lea']     = imulq_5
stats['schedule_candidates'] = schedule_candidates

print("=" * 55)
print(f"  ZCC Assembly Mutation Profile: {asm}")
print("=" * 55)
for k, v in stats.items():
    print(f"  {k:<30} {v:>8,}")
print("=" * 55)

total_elim = (stats['pushq_popq_same_elim'] * 2 +
              stats['movq_self_elim'] +
              stats['movq_zero'] +
              stats['movl_zero'] +
              stats['addq_zero'] +
              stats['subq_zero'] +
              stats['spill_reload_elim'])
total_convert = (stats['pushq_popq_pairs'] - stats['pushq_popq_same_elim'] +
                 stats['spill_reload_to_mov'] +
                 stats['load_add1_store_incq'] +
                 stats['load_sub1_store_decq'] +
                 stats['imulq_pow2_to_shl'])
print(f"\n  TOTAL ELIMINABLE instructions: {total_elim:,}")
print(f"  TOTAL CONVERTIBLE patterns:    {total_convert:,}")
print(f"  TOTAL SCHEDULABLE pairs:       {schedule_candidates:,}")
print(f"\n  Estimated instruction savings:  ~{total_elim + total_convert:,}")
print(f"  ({(total_elim + total_convert) / stats['inst_lines'] * 100:.2f}% of {stats['inst_lines']:,} instructions)")
