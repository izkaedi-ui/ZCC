import os, sys, subprocess, argparse, glob

def compile_and_run(src_file, zcc_path):
    s_file = src_file.replace('.c', '.s')
    bin_file = src_file.replace('.c', '_zcc_bin')
    
    r1 = subprocess.run([zcc_path, src_file, '-o', s_file], capture_output=True, timeout=5)
    if r1.returncode != 0: return None, None, "ZCC_ERR: " + r1.stderr.decode()[:100]
    
    r2 = subprocess.run(["gcc", "-O0", "-w", "-o", bin_file, s_file], capture_output=True, timeout=5)
    if r2.returncode != 0: return None, None, "LINK_ERR: " + r2.stderr.decode()[:100]
    
    r3 = subprocess.run(["./" + bin_file], capture_output=True, timeout=5)
    return r3.stdout.decode().strip(), r3.returncode, None

def gcc_run(src_file):
    bin_file = src_file.replace('.c', '_gcc_bin')
    r1 = subprocess.run(["gcc", "-O0", "-w", "-o", bin_file, src_file], capture_output=True, timeout=5)
    if r1.returncode != 0: return None, None, "GCC_ERR"
    r2 = subprocess.run(["./" + bin_file], capture_output=True, timeout=5)
    return r2.stdout.decode().strip(), r2.returncode, None

ap = argparse.ArgumentParser()
ap.add_argument('--seeds', required=True)
ap.add_argument('--zcc', required=True)
args = ap.parse_args()

files = glob.glob(f"{args.seeds}/*.c")
ok = 0
for f in sorted(files):
    gcc_out, gcc_rc, _ = gcc_run(f)
    zcc_out, zcc_rc, err = compile_and_run(f, args.zcc)
    if zcc_out == gcc_out and zcc_rc == gcc_rc and zcc_out is not None:
        ok += 1
    else:
        print(f"FAIL {f}: GCC:[{gcc_out}|rc={gcc_rc}] ZCC:[{zcc_out}|rc={zcc_rc}] ERR:[{err}]")

print(f"PASS COUNT: {ok}/{len(files)}")
if ok != len(files):
    sys.exit(1)
