import re, glob
for file in glob.glob('fuzz_run6/mismatches/mismatch_seed*.c'):
    with open(file, 'r') as f:
        src = f.read()
    # Replace printf("CHECKSUM=%ld\n", checksum);
    # with per-function printf
    src = src.replace('checksum += (long)f1(1, 2, 3);', 'long r1 = f1(1,2,3); printf("f1=%ld\\n", r1); checksum += r1;')
    src = src.replace('checksum += (long)f2(1, 2, 3);', 'long r2 = f2(1,2,3); printf("f2=%ld\\n", r2); checksum += r2;')
    src = src.replace('checksum += (long)f3(1, 2, 3);', 'long r3 = f3(1,2,3); printf("f3=%ld\\n", r3); checksum += r3;')
    src = src.replace('checksum += (long)f4(1, 2, 3);', 'long r4 = f4(1,2,3); printf("f4=%ld\\n", r4); checksum += r4;')
    with open(file.replace('.c', '_debug.c'), 'w') as f:
        f.write(src)
