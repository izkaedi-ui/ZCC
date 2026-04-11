#!/usr/bin/env python3
"""Fuzz harness: compare ZCC IR backend vs GCC checksums."""
import subprocess, random, os, sys, argparse, tempfile

# Build template with %% style so we use % formatting, not .format()
HEADER = """\
#include <stdio.h>
unsigned int checksum = 0;
void update(int v) { checksum = checksum * 1000003 ^ (unsigned int)v; }
"""
FOOTER_TMPL = """\
int main(void) {
%s
    printf("CHECKSUM=%%u\\n", checksum);
    return 0;
}
"""

OPS_INT   = ['+', '-', '*', '&', '|', '^']
OPS_CMP   = ['<', '>', '==', '!=', '<=', '>=']
OPS_SHIFT = ['<<', '>>']

def rand_int(rng, lo=-50, hi=50):
    return rng.randint(lo, hi)

def gen_func(rng, idx):
    n     = rng.randint(2, 6)
    vars_ = ['v%d' % i for i in range(n)]
    lines = ['    int %s = %d;' % (v, rand_int(rng)) for v in vars_]
    for _ in range(rng.randint(4, 12)):
        kind = rng.choice(['arith', 'cmp', 'shift', 'neg', 'not'])
        dst  = rng.choice(vars_)
        a    = rng.choice(vars_)
        if kind == 'arith':
            b  = rng.choice(vars_)
            op = rng.choice(OPS_INT)
            lines.append('    %s = %s %s %s;' % (dst, a, op, b))
        elif kind == 'cmp':
            b  = rng.choice(vars_)
            op = rng.choice(OPS_CMP)
            lines.append('    %s = (%s %s %s);' % (dst, a, op, b))
        elif kind == 'shift':
            k  = rng.randint(0, 5)
            op = rng.choice(OPS_SHIFT)
            lines.append('    %s = %s %s %d;' % (dst, a, op, k))
        elif kind == 'neg':
            lines.append('    %s = -%s;' % (dst, a))
        elif kind == 'not':
            lines.append('    %s = ~%s;' % (dst, a))
    lines += ['    update(%s);' % v for v in vars_]
    body = '\n'.join(lines)
    return 'void f%d(void) {\n%s\n}' % (idx, body)

def build_src(funcs):
    calls = '\n'.join('    f%d();' % i for i in range(len(funcs)))
    return HEADER + '\n'.join(funcs) + '\n' + FOOTER_TMPL % calls

def compile_and_run(src, zcc_path, use_ir):
    fd_c, path_c = tempfile.mkstemp(suffix='.c')
    path_s = path_c.replace('.c', '.s')
    path_e = path_c.replace('.c', '')
    try:
        with os.fdopen(fd_c, 'w') as f:
            f.write(src)

        env = dict(os.environ)
        if use_ir:
            env['ZCC_IR_BACKEND'] = '1'

        # ZCC compile to asm
        r1 = subprocess.run([zcc_path, path_c, '-o', path_s],
                            capture_output=True, timeout=10, env=env)
        if r1.returncode != 0:
            return None, 'ZCC_ERR: ' + r1.stderr.decode(errors='replace')[:120]

        # Link with GCC
        wd = os.path.dirname(os.path.abspath(zcc_path))
        r2 = subprocess.run(
            ['gcc', '-O0', '-w', '-o', path_e, path_s],
            capture_output=True, timeout=10)
        if r2.returncode != 0:
            return None, 'LINK_ERR: ' + r2.stderr.decode(errors='replace')[:120]

        r3 = subprocess.run([path_e], capture_output=True, timeout=5)
        if r3.returncode != 0:
            return None, 'SIGFPE/CRASH rc=%d' % r3.returncode
        return r3.stdout.decode().strip(), None
    except subprocess.TimeoutExpired:
        return None, 'TIMEOUT'
    except Exception as e:
        return None, str(e)
    finally:
        for p in [path_c, path_s, path_e]:
            try: os.unlink(p)
            except: pass

def gcc_run(src):
    fd_c, path_c = tempfile.mkstemp(suffix='.c')
    path_e = path_c.replace('.c', '_gcc')
    try:
        with os.fdopen(fd_c, 'w') as f:
            f.write(src)
        r1 = subprocess.run(['gcc', '-O0', '-w', '-o', path_e, path_c],
                            capture_output=True, timeout=10)
        if r1.returncode != 0:
            return None, 'GCC_ERR'
        r2 = subprocess.run([path_e], capture_output=True, timeout=5)
        return r2.stdout.decode().strip(), None
    except Exception as e:
        return None, str(e)
    finally:
        for p in [path_c, path_e]:
            try: os.unlink(p)
            except: pass

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--seed',  type=int, default=42)
    ap.add_argument('--cases', type=int, default=50)
    args = ap.parse_args()

    wd       = os.path.dirname(os.path.abspath(__file__))
    zcc_path = os.path.join(wd, 'zcc')
    rng      = random.Random(args.seed)

    ok = mismatches = crashes = skipped = 0
    for i in range(args.cases):
        funcs = [gen_func(rng, j) for j in range(rng.randint(1, 4))]
        src   = build_src(funcs)

        gcc_out, gcc_err = gcc_run(src)
        if gcc_out is None:
            skipped += 1
            continue

        zcc_out, zcc_err = compile_and_run(src, zcc_path, use_ir=True)

        if zcc_out is None:
            crashes += 1
            print('  [%3d] CRASH   gcc=%-14s  err=%s' % (i, gcc_out, zcc_err))
        elif gcc_out != zcc_out:
            with open("fail_ast_" + str(i) + ".c", "w") as f:
                f.write(src)
            mismatches += 1
            print('  [%3d] MISMATCH gcc=%-14s  zcc=%s' % (i, gcc_out, zcc_out))
        else:
            ok += 1

    total = ok + mismatches + crashes
    print('\nResults  seed=%d  cases=%d' % (args.seed, args.cases))
    print('  OK:          %d/%d' % (ok, total))
    print('  Mismatches:  %d/%d' % (mismatches, total))
    print('  Crashes:     %d/%d' % (crashes, total))
    print('  Skipped:     %d'    % skipped)
    if mismatches == 0 and crashes == 0:
        print('  CLEAN')
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
