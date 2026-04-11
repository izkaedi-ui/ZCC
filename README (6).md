# 🔱 ZCC Self-Hosting Hardening Pipeline v1.0

## Overview

Automated verification, regression testing, and differential fuzzing for the ZCC self-hosting C compiler. Takes your compiler from "it works on my tests" to "it works on 175+ regression tests and 200 randomly generated programs."

## Components

| File | Lines | Purpose |
|------|-------|---------|
| `run_hardening.sh` | 253 | Master orchestrator — runs all 4 phases |
| `harness/bootstrap_verify.sh` | 313 | 3-stage bootstrap chain (stage1→stage2→stage3) |
| `harness/test_codegen.c` | 949 | **175 regression tests** targeting CG-001 through CG-010 |
| `harness/zcc_fuzz.py` | 640 | Differential fuzzer — random C programs, GCC vs ZCC |

## Quick Start

```bash
# Copy to your ZCC directory
cp -r harness/ /path/to/zcc/
cp run_hardening.sh /path/to/zcc/

# Run everything
cd /path/to/zcc
./run_hardening.sh .

# Or run individual phases:
bash harness/bootstrap_verify.sh . ./bootstrap_out
gcc -O0 -std=c99 -o test_ref harness/test_codegen.c -lm && ./test_ref
python3 harness/zcc_fuzz.py --zcc ./zcc --count 200 --output-dir ./fuzz_results
```

## WSL / PowerShell

```powershell
# From PowerShell, copy into your WSL working directory:
wsl cp -r /mnt/user-data/uploads/zcc_hardening/* /mnt/h/__DOWNLOADS/selforglinux/

# Run from WSL:
wsl bash -c "cd /mnt/h/__DOWNLOADS/selforglinux && bash run_hardening.sh ."
```

## Test Coverage

### Regression Tests (175 tests)

| Category | Tests | Bug Class |
|----------|-------|-----------|
| Sign Extension | 7 | CG-001: u32→u64, u8→u32, s8→s32 |
| Division | 8 | CG-002: unsigned div/mod, signed div, edge cases |
| Shift | 8 | CG-003: logical vs arithmetic right shift |
| Comparison | 9 | CG-005: signed vs unsigned branch |
| Stack Alignment | 2 | CG-006: 8-arg calls, nested calls |
| Struct Layout | 7 | CG-007: padding, sizeof, nested, member pointers |
| Label Collision | 8 | CG-010: identical control flow across functions |
| Arithmetic | 20 | Basic ops, overflow, 64-bit, bitwise, compound |
| Control Flow | 18 | if/while/for/do-while/switch/ternary/short-circuit |
| Pointers | 12 | deref, array, arithmetic, ptr-to-ptr, null, strings |
| Recursion | 10 | Fibonacci, factorial, Ackermann |
| Memory | 8 | malloc/free, memset, memcpy |
| Function Pointers | 5 | Callbacks, arrays of fptrs |
| Strings | 7 | Indexing, strlen, strcmp, char arithmetic |
| Globals | 2 | Cross-function state |
| Casting | 7 | Promotion chains, truncation, sizeof |
| Linked List | 4 | Struct + malloc + pointers combined |
| Binary Search | 4 | Algorithm correctness |
| Sorting | 6 | Insertion sort, pre-sorted, reversed |
| Hanoi | 4 | Deep recursion stress |
| Edge Cases | 7 | Comma, sizeof expr, cast chains, cond lvalue |

### Fuzzer

- Generates random valid C within ZCC's supported subset
- No system headers, no VLAs, no compound literals
- Biased toward sign extension, division, and shift edge cases
- Bounded loops (no infinite loops)
- Division/modulo protected against divide-by-zero
- Shift amounts masked to valid range

## Output

```
hardening_results/
  20260405_143200/
    bootstrap/
      stage2.s, stage3.s        # Assembly outputs
      verified_hash.txt          # SHA-256 of verified assembly
      stage2_vs_stage3.diff      # If bootstrap broken
    regression/
      ref_output.txt             # GCC reference output
      zcc_output.txt             # ZCC output
    fuzz/
      summary.json               # Machine-readable results
      crashes/                   # Crash-inducing programs
      mismatches/                # Output-divergent programs
    summary.json                 # Overall report
```

## Environment Variables

```bash
FUZZ_COUNT=500   # Number of fuzz programs (default: 200)
FUZZ_TIMEOUT=15  # Per-program timeout in seconds (default: 10)
```
