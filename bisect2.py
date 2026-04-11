#!/usr/bin/env python3
"""Binary search for broken IR function(s).
Usage:
  python3 bisect2.py list       — show all IR-compiled function names
  python3 bisect2.py first      — blacklist first half, test
  python3 bisect2.py second     — blacklist second half, test
  python3 bisect2.py restore    — restore original blacklist
  python3 bisect2.py set A,B,C  — set blacklist to specific functions + base
"""
import subprocess, sys, os, re

BUILD = "/mnt/h/__DOWNLOADS/selforglinux"
PART4 = os.path.join(BUILD, "part4.c")
BASE_BLACKLIST = ["main", "read_file", "init_compiler", "cc_alloc", "type_new"]

def get_ir_functions():
    r = subprocess.run(
        ["sh", "-c",
         "cd {} && ./zcc zcc.c -o /tmp/_bisect.s 2>/tmp/_bisect_stderr.txt;"
         " grep 'emitted from IR' /tmp/_bisect_stderr.txt"
         " | sed 's/.*fn=//;s/ .*//'".format(BUILD)],
        capture_output=True, text=True, timeout=120
    )
    fns = [f.strip() for f in r.stdout.strip().split('\n') if f.strip()]
    return fns

def set_blacklist(names):
    src = open(PART4).read()
    entries = ', '.join('"{}"'.format(n) for n in names)
    new_bl = ('static const char *blacklist[] = {{\n'
              '        {}, NULL}};'.format(entries))
    src = re.sub(
        r'static const char \*blacklist\[\] = \{[^}]+\};',
        new_bl, src)
    open(PART4, 'w').write(src)
    # Force rebuild zcc.c from parts
    subprocess.run(["sh", "-c",
                    "cd {} && rm -f zcc.c && make zcc.c zcc 2>&1 | tail -3".format(BUILD)],
                   timeout=60)

def test_stage3():
    """Build zcc2 from current source, test if zcc2 can compile tiny.c"""
    # Rebuild zcc2
    r = subprocess.run(
        ["sh", "-c",
         "cd {} && make clean && make zcc2 2>&1 | tail -5".format(BUILD)],
        capture_output=True, text=True, timeout=120
    )
    if r.returncode != 0:
        print("zcc2 build failed!")
        print(r.stdout[-300:])
        return False
    # Test: can zcc2 compile a trivial program?
    r2 = subprocess.run(
        ["sh", "-c",
         'cd {} && echo "int main() {{ return 42; }}" > /tmp/_test.c'
         ' && ./zcc2 /tmp/_test.c -o /tmp/_test.s 2>&1'.format(BUILD)],
        capture_output=True, text=True, timeout=30
    )
    return r2.returncode == 0

cmd = sys.argv[1] if len(sys.argv) > 1 else "list"

if cmd == "list":
    print("Collecting IR function list...")
    fns = get_ir_functions()
    print("Total IR functions: {}".format(len(fns)))
    for f in fns:
        print("  {}".format(f))

elif cmd == "first":
    fns = get_ir_functions()
    half = len(fns) // 2
    bl = list(set(BASE_BLACKLIST + fns[:half]))
    print("Blacklisting first {} of {} IR functions ({} total with base)".format(
        half, len(fns), len(bl)))
    set_blacklist(bl)
    ok = test_stage3()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    if ok:
        print("Bug is in the FIRST half (now blacklisted)")
    else:
        print("Bug is in the SECOND half (still active)")

elif cmd == "second":
    fns = get_ir_functions()
    half = len(fns) // 2
    bl = list(set(BASE_BLACKLIST + fns[half:]))
    print("Blacklisting second {} of {} IR functions ({} total with base)".format(
        len(fns) - half, len(fns), len(bl)))
    set_blacklist(bl)
    ok = test_stage3()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    if ok:
        print("Bug is in the SECOND half (now blacklisted)")
    else:
        print("Bug is in the FIRST half (still active)")

elif cmd == "restore":
    set_blacklist(BASE_BLACKLIST)
    print("Restored base blacklist: {}".format(BASE_BLACKLIST))

elif cmd == "set":
    extra = sys.argv[2].split(",") if len(sys.argv) > 2 else []
    bl = list(set(BASE_BLACKLIST + extra))
    set_blacklist(bl)
    print("Set blacklist: {}".format(bl))
    ok = test_stage3()
    print("Result: {}".format("PASS" if ok else "CRASH"))

else:
    print("Usage: bisect2.py [list|first|second|restore|set fn1,fn2,...]")
