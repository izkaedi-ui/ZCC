#!/usr/bin/env python3
"""Binary search for the broken IR-compiled function.
Run with: python3 bisect_ir.py [step]
  step 1: collect all IR functions, blacklist first half, test
  step 2: after step 1 result, narrow further
"""
import subprocess, sys, os

BUILD = "/mnt/h/__DOWNLOADS/selforglinux"
PART4 = os.path.join(BUILD, "part4.c")

def get_ir_functions():
    """Get list of all functions that went through IR in stage 2."""
    r = subprocess.run(
        ["sh", "-c", f"cd {BUILD} && ./zcc zcc.c -o /dev/null 2>&1 | grep 'ZCC-IR.*fn=' | sed 's/.*fn=//;s/ .*//'"],
        capture_output=True, text=True
    )
    return [f.strip() for f in r.stdout.strip().split('\n') if f.strip()]

def get_blacklist():
    """Read current blacklist from part4.c."""
    src = open(PART4).read()
    import re
    m = re.search(r'static const char \*blacklist\[\] = \{([^}]+)\}', src)
    if not m: return []
    entries = re.findall(r'"([^"]+)"', m.group(1))
    return entries

def set_blacklist(names):
    """Replace the blacklist in part4.c."""
    src = open(PART4).read()
    import re
    # Build new blacklist string
    entries = ', '.join(f'"{n}"' for n in names)
    new_bl = f'static const char *blacklist[] = {{\n        {entries}, NULL}};'
    src = re.sub(
        r'static const char \*blacklist\[\] = \{[^}]+\};',
        new_bl,
        src
    )
    open(PART4, 'w').write(src)

def test_selfhost():
    """Run make clean && make selfhost, return True if success."""
    r = subprocess.run(
        ["sh", "-c", f"cd {BUILD} && make clean && make selfhost 2>&1"],
        capture_output=True, text=True, timeout=120
    )
    return "SELF-HOST VERIFIED" in r.stdout + r.stderr

def test_stage3():
    """Just test if zcc2 crashes compiling zcc.c."""
    subprocess.run(["sh", "-c", f"cd {BUILD} && make clean 2>&1"], capture_output=True)
    # Stage 1+2
    r1 = subprocess.run(
        ["sh", "-c", f"cd {BUILD} && make zcc2 2>&1"],
        capture_output=True, text=True, timeout=120
    )
    if r1.returncode != 0:
        print("Stage 1+2 failed!")
        print(r1.stdout[-500:])
        return False
    # Stage 3 only
    r2 = subprocess.run(
        ["sh", "-c", f"cd {BUILD} && ./zcc2 zcc.c -o zcc3.s 2>&1"],
        capture_output=True, text=True, timeout=60
    )
    return r2.returncode == 0

# Main
current_bl = get_blacklist()
print(f"Current blacklist ({len(current_bl)}): {current_bl}")

print("Collecting IR function list...")
ir_fns = get_ir_functions()
print(f"Total IR-compiled functions: {len(ir_fns)}")

if len(sys.argv) > 1 and sys.argv[1] == "list":
    for f in ir_fns:
        print(f"  {f}")
    sys.exit(0)

# Blacklist first half of IR functions + existing blacklist
half = len(ir_fns) // 2
first_half = ir_fns[:half]
second_half = ir_fns[half:]

print(f"\n=== TEST: blacklisting first {half} IR functions ===")
new_bl = list(set(current_bl + first_half))
set_blacklist(new_bl)
print(f"Blacklist size: {len(new_bl)}")

print("Testing stage 3 (zcc2 compiles zcc.c)...")
ok = test_stage3()
if ok:
    print(f"PASS — bug is in the first half ({half} functions)")
    print("First half functions:")
    for f in first_half[:20]:
        print(f"  {f}")
    if len(first_half) > 20:
        print(f"  ... and {len(first_half)-20} more")
else:
    print(f"FAIL — bug is in the second half ({len(second_half)} functions)")
    print("Second half functions:")
    for f in second_half[:20]:
        print(f"  {f}")
    if len(second_half) > 20:
        print(f"  ... and {len(second_half)-20} more")

# Restore original blacklist
set_blacklist(current_bl)
print(f"\nRestored original blacklist: {current_bl}")
print("Run again with narrowed set to continue bisecting.")
