# ZCC commands reference

Run all of these from the repo root. Use a real shell (e.g. WSL/Kali); paths below are for Linux.

---

## Build and self-host

```bash
# Full bootstrap + self-host check (builds zcc, zcc_pp.c, zcc2; runs zcc2 → zcc3.s)
./run_selfhost.sh
```

If stage 2 crashes, `zcc3.s` is empty and the script exits with FAIL. The compare step runs only when `zcc3.s` exists:

```bash
# Only after a successful run (zcc3.s exists):
cmp zcc2.s zcc3.s
```

Manual build (same as the script does):

```bash
cat part1.c part2.c part3.c part4.c part5.c > zcc.c
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c compiler_passes.c -lm

sed '/^_Static_assert/d' zcc.c > zcc_pp.c
sed -e '/#include "zcc_ast_bridge.h"/r zcc_ast_bridge_zcc.h' -e '/#include "zcc_ast_bridge.h"/d' zcc_pp.c > zcc_pp.c.tmp && mv zcc_pp.c.tmp zcc_pp.c

./zcc zcc_pp.c -o zcc2.s
gcc -O0 -w -o zcc2 zcc2.s compiler_passes.c -lm
./zcc2 zcc_pp.c -o zcc3.s
```

---

## Compile a single file

Use an existing source file (e.g. `hello.c` or `audit/t1_int.c`), not the literal name `file.c`:

```bash
./zcc hello.c -o hello.s
gcc -o hello hello.s -lm
./hello
```

Or with the audit test:

```bash
./zcc audit/t1_int.c -o t1.s
gcc -o t1 t1.s -lm
./t1
```

---

## Debug (when zcc2 segfaults)

```bash
./run_selfhost.sh 2> debug.log
tail -20 debug.log
awk '/ZCC_LAYOUT/{n++} n==2,/Segmentation/' debug.log | tail -30
```

To get a backtrace, install and use GDB:

```bash
sudo apt install gdb
gdb -batch -nx -ex "run" -ex "bt" -ex "p/x \$rdi" -ex "quit" --args ./zcc2 zcc_pp.c -o zcc3.s
```

---

## Stub / reduce (Phase 5.5)

```bash
python3 scripts/stub_functions.py zcc_pp.c --list
python3 scripts/stub_functions.py zcc_pp.c --keep main,codegen_program,codegen_func --out stubbed.c
python3 scripts/stub_functions.py zcc_pp.c --stub-from 2000 --stub-to 4000 --out halved.c
```

**Stub coverage** — which `--keep` subsets still fail (bug dependency / minimal repro). Use when reducing a crash to a minimal set of functions.

```bash
python3 scripts/stub_functions.py zcc.c --coverage --run-cmd "./zcc %s -o /tmp/out.s" --max-size 2
python3 scripts/stub_functions.py zcc.c --coverage --out-json coverage.json [--no-progress]
```

---

## Minimal runtime (zcc_rt)

Link small programs without libc for putchar/malloc/memcpy/memset/_exit (see `docs/ZCC_ABI.md`). Use when testing ZCC-built code in isolation.

```bash
gcc -c zcc_rt.c -o zcc_rt.o
./zcc hello.c -o hello.s
gcc hello.s zcc_rt.o -o hello
```

---

## Differential fuzzer (ZCC vs gcc -O0)

Use to find codegen bugs (ZCC exit code ≠ gcc -O0 on the same source).

```bash
python3 scripts/zcc_fuzz.py --count 50 [--seed N] [--keep-failing] [--div] [--repro] [--verbose]
python3 scripts/zcc_fuzz.py --one path/to/repro.c [--use-rt]
```

---

## Bug bounties (ZKAEDI PRIME)

Use the mempool pipeline in dry-run to log opportunities, then export report snippets for Immunefi / Code4rena / Sherlock. See **docs/BUG_BOUNTY.md** for workflow and report template.

```bash
# 1. Run pipeline with DRY_RUN=1, OPPORTUNITIES_CSV=./bounty_candidates.csv
# 2. Export last N rows as markdown (paste into bounty form)
cd zkaedi_prime
python scripts/bounty_export.py bounty_candidates.csv --last 10 --format md
# JSON
python scripts/bounty_export.py opportunities.csv --last 5 --format json
```

---

## Clean rebuild

```bash
rm -f zcc2.s zcc2 zcc3.s zcc_pp.c
./run_selfhost.sh
```
