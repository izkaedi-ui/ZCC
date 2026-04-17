# ZCC Dead Code Elimination — Agent Prompt

## YOUR ROLE

You are implementing a **Dead Code Elimination (DCE)** optimization pass for ZCC, a self-hosting C compiler. You will write a standalone Python script that reads the compiler's IR dump, removes provably dead instructions, and outputs an optimized IR.

**SAFETY IS PARAMOUNT.** This IR represents a self-hosting compiler that has been mathematically verified (stage2 == stage3 bitwise identical). A bad optimization will silently corrupt the compiler's ability to compile itself. Every elimination must be provably safe.

---

## THE INPUT: zcc_ir.json

Location: `h:/__DOWNLOADS/selforglinux/zcc_ir.json`

This is a 17,000+ line JSON file produced by `make ir-self` — ZCC compiling its own source code with dual-mode IR emission. It contains the complete 3-address intermediate representation of every function in the compiler.

### IR Format Specification

The JSON structure is an array of **functions**. Each function contains an array of **instructions**. Each instruction has these fields:

```json
{
  "op": "IR_ADD",           // opcode string (see full list below)
  "type": "IR_TY_I64",     // result type string (see full list below)
  "dst": "%t5",            // destination temporary (or "" if none)
  "src1": "%t3",           // first operand (or "" if unused)
  "src2": "%t4",           // second operand (or "" if unused)
  "line": 142              // source line number (for diagnostics only)
}
```

### All Opcodes (ir_op_t)

**Side-effecting (NEVER eliminate):**
- `IR_RET` — function return. Always live.
- `IR_BR` — unconditional branch. Always live.
- `IR_BR_IF` — conditional branch. Always live.
- `IR_STORE` — memory write. Always live (conservative: we cannot prove memory is unobserved).
- `IR_CALL` — function call. Always live (may have side effects).
- `IR_ARG` — argument push before a call. Always live (consumed by the next CALL).
- `IR_LABEL` — branch target. Always live (removing it would break jumps).

**Eliminable (dead if result is never used):**
- `IR_ADD`, `IR_SUB`, `IR_MUL`, `IR_DIV`, `IR_MOD` — arithmetic
- `IR_AND`, `IR_OR`, `IR_XOR`, `IR_SHL`, `IR_SHR` — bitwise
- `IR_EQ`, `IR_NE`, `IR_LT`, `IR_LE`, `IR_GT`, `IR_GE` — comparisons
- `IR_NEG`, `IR_NOT` — unary
- `IR_CAST`, `IR_COPY` — type conversion / register copy
- `IR_CONST`, `IR_CONST_STR` — constant materialization
- `IR_LOAD` — memory read (safe to eliminate IF result unused AND address has no side-effect, which is always true for our IR since loads don't fault on valid addresses in this compiler)
- `IR_ALLOCA` — stack allocation (safe to eliminate if the allocated address is never stored to or loaded from)
- `IR_PHI` — SSA phi node (eliminable if result unused)
- `IR_NOP` — no-op (always eliminable)

### All Types (ir_type_t)

```
IR_TY_VOID, IR_TY_I8, IR_TY_I16, IR_TY_I32, IR_TY_I64,
IR_TY_U8, IR_TY_U16, IR_TY_U32, IR_TY_U64,
IR_TY_PTR, IR_TY_F32, IR_TY_F64
```

---

## THE ALGORITHM: Conservative Mark-Sweep DCE

Implement a **reverse-walk mark-sweep** algorithm. This is the standard safe DCE:

### Phase 1: Mark

1. Initialize a set `live` (empty).
2. For each function in the IR:
   a. Walk instructions in **reverse order** (last to first).
   b. **Seed:** Mark every side-effecting instruction as live:
      - `IR_RET`, `IR_BR`, `IR_BR_IF`, `IR_STORE`, `IR_CALL`, `IR_ARG`, `IR_LABEL`
   c. **Propagate:** When an instruction is marked live, also mark as live every instruction whose `dst` is used as `src1` or `src2` (or `cond` for `BR_IF`) by this live instruction.
   d. **Repeat** the propagation until no new instructions are marked (fixed-point iteration).

### Phase 2: Sweep

3. Remove every instruction NOT in the `live` set.
4. `IR_NOP` instructions are always removed.

### Phase 3: ALLOCA cleanup

5. After sweep, check each remaining `IR_ALLOCA`. If the allocated variable (`dst`) is never referenced by any surviving `STORE` or `LOAD` instruction in the function, eliminate the `ALLOCA` too.

### CRITICAL SAFETY RULES

- **NEVER eliminate `IR_CALL`.** Even if the return value is unused, the function may have side effects (printf, free, fwrite, etc.). There are ZERO exceptions.
- **NEVER eliminate `IR_ARG`.** ARG instructions are consumed by the immediately following CALL. Removing an ARG corrupts the calling convention.
- **NEVER eliminate `IR_STORE`.** We cannot prove that memory writes are unobserved without alias analysis, which we do not have.
- **NEVER eliminate `IR_RET`, `IR_BR`, `IR_BR_IF`, `IR_LABEL`.** These are structural — removing them changes control flow.
- **NEVER reorder instructions.** DCE only removes. It never moves, reorders, or merges.
- **NEVER modify instruction fields.** If an instruction survives, it must be byte-identical to the original.
- **When in doubt, keep it.** A false positive (keeping dead code) is harmless. A false negative (removing live code) is catastrophic.

---

## THE OUTPUT

### Primary: zcc_ir_optimized.json

Same JSON format as the input, with dead instructions removed. Write to:
`h:/__DOWNLOADS/selforglinux/zcc_ir_optimized.json`

### Secondary: dce_report.json

Statistics file. Write to:
`h:/__DOWNLOADS/selforglinux/dce_report.json`

```json
{
  "input_file": "zcc_ir.json",
  "output_file": "zcc_ir_optimized.json",
  "total_functions": 87,
  "total_instructions_before": 17342,
  "total_instructions_after": 15201,
  "total_eliminated": 2141,
  "elimination_rate_percent": 12.35,
  "per_function": [
    {
      "name": "codegen_expr",
      "before": 842,
      "after": 731,
      "eliminated": 111,
      "eliminated_by_opcode": {
        "IR_CONST": 45,
        "IR_LOAD": 32,
        "IR_ADD": 18,
        "IR_COPY": 16
      }
    }
  ],
  "safety_audit": {
    "calls_preserved": true,
    "stores_preserved": true,
    "branches_preserved": true,
    "returns_preserved": true,
    "labels_preserved": true,
    "args_preserved": true
  }
}
```

### Tertiary: stdout summary

Print a human-readable summary to stdout:

```
🔱 ZCC DCE Pass — Dead Code Elimination Report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Input:  zcc_ir.json (17,342 instructions across 87 functions)
Output: zcc_ir_optimized.json (15,201 instructions)
Eliminated: 2,141 instructions (12.35%)

Top 5 functions by elimination:
  1. codegen_expr        — 111 eliminated (13.2%)
  2. codegen_stmt        —  87 eliminated (11.8%)
  3. parse_unary         —  64 eliminated (15.1%)
  ...

Safety audit: ALL SIDE-EFFECTING INSTRUCTIONS PRESERVED ✅

Breakdown by opcode:
  IR_CONST:     580 eliminated
  IR_LOAD:      412 eliminated
  IR_ADD:       234 eliminated
  ...
```

---

## IMPLEMENTATION REQUIREMENTS

1. **Language:** Python 3. No external dependencies beyond the standard library.
2. **File name:** `zcc_dce.py`
3. **Location:** `h:/__DOWNLOADS/selforglinux/zcc_dce.py`
4. **Invocation:** `python zcc_dce.py` (reads zcc_ir.json from same directory, writes outputs to same directory)
5. **Also accept:** `python zcc_dce.py <input.json> <output.json>` for custom paths.
6. **Error handling:** If zcc_ir.json is missing or malformed, print a clear error and exit 1.
7. **Idempotent:** Running DCE on already-optimized IR should produce identical output (no further eliminations except possibly IR_NOP).

---

## VERIFICATION PROCEDURE

After the script runs, verify correctness:

```bash
# 1. The optimized IR must be valid JSON
python -m json.tool zcc_ir_optimized.json > /dev/null

# 2. Safety audit must pass (all side-effecting ops preserved)
python -c "
import json
orig = json.load(open('zcc_ir.json'))
opt = json.load(open('zcc_ir_optimized.json'))

# Count side-effecting ops in original vs optimized
side_ops = {'IR_RET','IR_BR','IR_BR_IF','IR_STORE','IR_CALL','IR_ARG','IR_LABEL'}
def count_side(ir):
    n = 0
    for func in ir:
        for inst in func.get('instructions', func.get('body', [])):
            if inst['op'] in side_ops:
                n += 1
    return n

orig_side = count_side(orig)
opt_side = count_side(opt)
assert orig_side == opt_side, f'SAFETY VIOLATION: {orig_side} vs {opt_side} side-effecting ops'
print(f'Safety verified: {opt_side} side-effecting instructions preserved')
"

# 3. Function count must be identical (DCE never removes entire functions)
python -c "
import json
orig = json.load(open('zcc_ir.json'))
opt = json.load(open('zcc_ir_optimized.json'))
assert len(orig) == len(opt), f'Function count mismatch: {len(orig)} vs {len(opt)}'
print(f'Function count verified: {len(opt)} functions')
"
```

---

## WHAT TO DO FIRST

1. **Read zcc_ir.json** and examine the actual JSON structure. The format described above is the specification, but the actual keys may use slightly different names (e.g., `"body"` vs `"instructions"`, `"operands"` vs `"src1"/"src2"`). **Adapt to whatever you find in the actual file.** Do NOT assume — inspect first.

2. **Print a summary** of what you find: number of functions, total instructions, distribution of opcodes. This confirms you've parsed it correctly before running any elimination.

3. **Then implement the DCE.** Start with the mark-sweep. Test on small functions first. Verify side-effecting instruction counts match before and after.

4. **Generate all three outputs:** optimized JSON, report JSON, stdout summary.

---

## WHAT NOT TO DO

- Do NOT modify any .c files in the repository. This is a read-only analysis pass.
- Do NOT modify zcc_ir.json. It is the ground-truth artifact.
- Do NOT attempt to recompile ZCC or run make targets. This script is offline analysis only.
- Do NOT implement constant folding, copy propagation, or any other optimization. DCE only.
- Do NOT rename, reorder, or renumber IR temporaries. The surviving instructions must be byte-identical to their originals.
- Do NOT remove functions, even if they appear unused. Inter-procedural DCE is out of scope.
- Do NOT use any heuristics or ML. This is classical compiler optimization with a proof obligation.
