import subprocess
result = subprocess.run(
    ['objdump', '-d', '--start-address=0x518580', '--stop-address=0x518c00', '/mnt/h/__DOWNLOADS/selforglinux/sqlite3_test'],
    capture_output=True, text=True
)
lines = result.stdout.split('\n')

# Look for pattern: load from pParse+offset, add 1, store to some address
# nMem at offset 56 = 0x38
# Pattern: mov offset(%rax), %rax where offset = 56, then add $1, %rax
for i, l in enumerate(lines):
    if '0x38' in l or '$0x38' in l:
        ctx = lines[max(0,i-2):i+5]
        print(f"Line {i}:")
        for x in ctx: print(f"  {x}")
        print()
