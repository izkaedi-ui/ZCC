#!/usr/bin/env python3
"""Switch ir_whitelisted to whitelist mode and test individual functions.
Usage:
  python3 whitelist.py none        — disable IR for ALL functions (verify baseline)
  python3 whitelist.py one is_digit — enable IR for ONLY is_digit
  python3 whitelist.py few fn1,fn2  — enable IR for specific functions
  python3 whitelist.py restore      — restore original blacklist mode
"""
import subprocess, sys, os, re

BUILD = "/mnt/h/__DOWNLOADS/selforglinux"
PART4 = os.path.join(BUILD, "part4.c")

def set_whitelist_mode(whitelist_fns):
    """Replace ir_whitelisted with whitelist mode."""
    src = open(PART4).read()
    if whitelist_fns is None:
        # All disabled
        new_func = '''static int ir_whitelisted(const char *name) {
    (void)name;
    return 0;
}'''
    else:
        entries = ', '.join('"{}"'.format(n) for n in whitelist_fns)
        new_func = '''static int ir_whitelisted(const char *name) {{
    static const char *whitelist[] = {{{}, NULL}};
    int i;
    if (!name) return 0;
    for (i = 0; whitelist[i]; i++)
        if (strcmp(name, whitelist[i]) == 0) return 1;
    return 0;
}}'''.format(entries)

    # Replace the entire ir_whitelisted function
    src = re.sub(
        r'static int ir_whitelisted\(const char \*name\) \{.*?\n\}',
        new_func, src, flags=re.DOTALL)
    open(PART4, 'w').write(src)
    subprocess.run(["sh", "-c",
        "cd {} && rm -f zcc.c && make zcc.c zcc 2>/dev/null".format(BUILD)],
        capture_output=True, timeout=60)

def set_blacklist_mode():
    """Restore original blacklist mode."""
    src = open(PART4).read()
    new_func = '''static int ir_whitelisted(const char *name) {
    if (!name) return 0;
    static const char *blacklist[] = {
        "main", "read_file", "init_compiler", "cc_alloc", "type_new", NULL};
    int i;
    for (i = 0; blacklist[i]; i++) {
        if (strcmp(name, blacklist[i]) == 0) {
            fprintf(stderr, "[ZCC-BLACKLIST] HIT (skipping IR): %s\\n", name);
            return 0;
        }
    }
    return 1;
}'''
    src = re.sub(
        r'static int ir_whitelisted\(const char \*name\) \{.*?\n\}',
        new_func, src, flags=re.DOTALL)
    open(PART4, 'w').write(src)
    subprocess.run(["sh", "-c",
        "cd {} && rm -f zcc.c && make zcc.c zcc 2>/dev/null".format(BUILD)],
        capture_output=True, timeout=60)

def test():
    r = subprocess.run(["sh", "-c",
        "cd {} && make clean 2>/dev/null && make zcc2 2>/dev/null".format(BUILD)],
        capture_output=True, text=True, timeout=120)
    if r.returncode != 0:
        return "BUILD_FAIL"
    r2 = subprocess.run(["sh", "-c",
        'echo "int main() {{ return 42; }}" > /tmp/_t.c && '
        "cd {} && timeout 10 ./zcc2 /tmp/_t.c -o /tmp/_t.s 2>/dev/null".format(BUILD)],
        capture_output=True, text=True, timeout=30)
    if r2.returncode == 0:
        return "PASS"
    elif r2.returncode == 124:
        return "TIMEOUT"
    else:
        return "CRASH"

cmd = sys.argv[1] if len(sys.argv) > 1 else "help"

if cmd == "none":
    print("Disabling IR for ALL functions...")
    set_whitelist_mode(None)
    result = test()
    print("Result: {}".format(result))

elif cmd == "one":
    fn = sys.argv[2] if len(sys.argv) > 2 else "is_digit"
    print("Enabling IR for ONLY: {}".format(fn))
    set_whitelist_mode([fn])
    result = test()
    print("Result: {}".format(result))

elif cmd == "few":
    fns = sys.argv[2].split(",") if len(sys.argv) > 2 else []
    print("Enabling IR for: {}".format(fns))
    set_whitelist_mode(fns)
    result = test()
    print("Result: {}".format(result))

elif cmd == "restore":
    print("Restoring blacklist mode...")
    set_blacklist_mode()
    print("Done.")

else:
    print("Usage:")
    print("  whitelist.py none          — all AST (verify baseline)")
    print("  whitelist.py one is_digit  — one IR function")
    print("  whitelist.py few a,b,c     — specific IR functions")
    print("  whitelist.py restore       — back to blacklist mode")
