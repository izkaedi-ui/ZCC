#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== Generating ZCC training data for ZKAEDI-MINI ==="

# Rebuild if needed
if [ ! -f zcc2 ]; then
    make selfhost 2>/dev/null
fi

# Concatenate source
cat part1.c part2.c part3.c ir.h ir_emit_dispatch.h ir_bridge.h part4.c part5.c ir.c ir_to_x86.c > zcc_pp.c

# 1. Capture IR pass statistics per function
echo "--- Capturing IR pass stats ---"
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o tmp_null 2>ir_pass_log.txt
echo "IR log: $(wc -l < ir_pass_log.txt) lines"

# 2. Generate AST vs IR assembly for diff pairs
echo "--- Generating AST assembly ---"
./zcc2 zcc_pp.c -o zcc_ast_out.s 2>/dev/null
echo "AST asm: $(wc -l < zcc_ast_out.s) lines"

echo "--- Generating IR assembly ---"
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc_ir_out.s 2>/dev/null
echo "IR asm: $(wc -l < zcc_ir_out.s 2>/dev/null || echo 0) lines"

# 3. Build the training JSONL
echo "--- Building training dataset ---"
python3 << 'PYEOF'
import json, re, os

dataset = []

# === SOURCE A: Bug corpus (12 entries) ===
if os.path.exists("zcc-compiler-bug-corpus/zcc-compiler-bug-corpus.json"):
    bugs = json.load(open("zcc-compiler-bug-corpus/zcc-compiler-bug-corpus.json"))
elif os.path.exists("zcc-compiler-bug-corpus.json"):
    bugs = json.load(open("zcc-compiler-bug-corpus.json"))
else:
    bugs = []

for bug in bugs:
    # Pattern identification
    dataset.append({
        "instruction": f"Identify the compiler codegen bug in this pattern:\n{bug['before_pattern']}",
        "input": "",
        "output": f"{bug['cwe']} ({bug['cwe_name']}): {bug['root_cause']}\nFix: {bug['after_pattern']}\nSeverity: {bug['severity']}, PRIME energy: {bug['prime_energy_score']}"
    })
    # Symptom diagnosis
    dataset.append({
        "instruction": f"A self-hosting C compiler crashes with this symptom: {bug['symptom']}\nWhat is the root cause?",
        "input": "",
        "output": f"Bug {bug['id']}: {bug['title']}\nRoot cause: {bug['root_cause']}\nDetection: {bug['detection_method']}\nFix: {bug['fix_summary']}"
    })
    # Regression test generation
    dataset.append({
        "instruction": f"Write a regression test for compiler bug {bug['id']}: {bug['title']}",
        "input": "",
        "output": f"Test specification: {bug['regression_test']}\nBug class: {', '.join(bug['tags'])}\nFix type: {bug['fix_type']}"
    })

print(f"Bug corpus: {len(dataset)} training pairs from {len(bugs)} bugs")

# === SOURCE B: IR pass log analysis ===
if os.path.exists("ir_pass_log.txt"):
    log = open("ir_pass_log.txt").read()
    
    # Extract per-function IR stats
    func_entries = re.findall(r'\[ZCC-IR\] fn=(\S+)\s+emitted.*?(\d+) blocks', log)
    
    # Extract global pass stats
    ir_opts = re.findall(r'\[IR-Opts\] Folded: (\d+) \| S-Reduce: (\d+) \| Copy-Prop: (\d+) \| Peephole: (\d+)', log)
    dce = re.findall(r'\[DCE->SSA\]\s+instructions removed: (\d+)', log)
    mem2reg = re.findall(r'\[Mem2Reg\]\s+single-block allocas promoted: (\d+)', log)
    rle = re.findall(r'\[RLE\]\s+redundant loads eliminated: (\d+)', log)
    escape = re.findall(r'\[EscapeAna\] allocations promoted to stack: (\d+)\s+\(of (\d+)', log)
    
    for fname, blocks in func_entries[:50]:
        dataset.append({
            "instruction": f"The ZCC compiler function '{fname}' has {blocks} basic blocks after optimization. What does this function likely do and what optimization opportunities remain?",
            "input": "",
            "output": f"Function '{fname}' with {blocks} blocks is part of ZCC, a self-hosting C compiler. Based on naming conventions: " +
                      ("lexer/tokenizer function" if any(x in fname for x in ["next_token", "read_", "skip_", "is_"]) else
                       "parser function" if any(x in fname for x in ["parse_", "expect", "peek"]) else
                       "codegen function" if any(x in fname for x in ["codegen_", "emit_", "gen_"]) else
                       "IR pass function" if any(x in fname for x in ["_pass", "mem2reg", "dce_", "licm_"]) else
                       "utility/infrastructure function") +
                      f". With {blocks} blocks, potential optimizations include: loop unrolling (if blocks > 10), tail call optimization, and further constant propagation."
        })
    
    # Pass statistics as training data
    if ir_opts:
        f, s, c, p = ir_opts[0]
        dataset.append({
            "instruction": "What optimization passes does ZCC's IR backend run and what are typical results?",
            "input": "",
            "output": f"ZCC runs 12 optimization passes in order: reachability, parameter escape marking, PGO instrumentation, constant folding ({f} folded), strength reduction ({s} reduced), copy propagation ({c} propagated), peephole ({p} simplified), redundant load elimination, DCE, escape analysis, Mem2Reg (scalar promotion), and PGO block reordering. The passes are chained in run_all_passes() in compiler_passes.c."
        })
    
    print(f"IR pass log: {len(func_entries)} functions found, {len(dataset)} total pairs so far")

# === SOURCE C: ZCC architecture knowledge ===
arch_pairs = [
    {
        "instruction": "Explain ZCC's dual-emission codegen architecture.",
        "input": "",
        "output": "ZCC has two codegen paths: AST-direct (part4.c) walks the AST and emits x86-64 assembly directly, and the IR backend (compiler_passes.c) translates AST to 3-address SSA IR, runs optimization passes, then emits assembly. The IR backend operates in body_only mode: the AST owns the function prologue/epilogue while the IR owns the function body. This hybrid design is controlled by ir_whitelisted() in part4.c. The IR body emitter uses slot_base=-stack_size to place IR spill slots below the AST frame, avoiding slot collisions (CG-IR-008)."
    },
    {
        "instruction": "What is the self-host invariant in ZCC?",
        "input": "",
        "output": "The self-host invariant is: cmp zcc2.s zcc3.s must be byte-identical. zcc2 is built by compiling zcc.c with the GCC-bootstrapped zcc, then zcc2 compiles zcc.c again to produce zcc3.s. If zcc2.s == zcc3.s, the compiler is a fixed point - it produces identical output when compiling itself. Any codegen bug that affects any function in the compiler breaks this invariant."
    },
    {
        "instruction": "How does Mem2Reg work in ZCC's IR backend?",
        "input": "",
        "output": "ZCC's Mem2Reg (scalar_promotion_pass at line 1478 of compiler_passes.c) has two paths: single-block fast path for allocas where all loads/stores occur in one block (replaces memory ops with register copies), and multi-block path using iterated dominance frontiers for PHI placement followed by recursive SSA rename over the dominator tree. Parameter allocas are marked as escaping in run_all_passes() to prevent promotion. The multi-block path (multi_block_mem2reg_one at line 1681) computes def blocks, iterates the dominance frontier worklist, inserts PHI nodes at join points, then renames via mem2reg_rename_rec."
    },
    {
        "instruction": "What is the PHI edge copy problem and how does ZCC solve it?",
        "input": "",
        "output": "PHI edge copies resolve SSA PHI nodes to actual register moves on control flow edges. ZCC's ir_asm_emit_phi_edge_copy (line 4826) walks all consecutive OP_PHI instructions at a target block's head. For multi-PHI blocks (e.g., a loop carrying both i and sum), serial copy through a single register (rax) causes the lost-copy problem: PHI_A writes to a slot that PHI_B reads, corrupting the value. CG-IR-005 fixed this with parallel copy via push/pop - all PHI sources are pushed first, then popped into destinations in reverse order."
    },
    {
        "instruction": "Explain the CG-IR-012b bug - hollow accessor stubs.",
        "input": "",
        "output": "CG-IR-012b was the most destructive bug in ZCC's IR backend. compiler_passes.c contained 33 static stub functions (node_kind, node_lhs, node_rhs, node_cond, node_body, etc.) that all returned 0/NULL. These were placeholders from early development. Since they were static, the linker used them instead of the real implementations in zcc.c. The entire AST-to-IR bridge built structurally complete but semantically empty trees - every node had kind=0 and null children. The fix was replacing all 33 static stubs with extern declarations. PRIME energy score: 9.9 (highest in the corpus)."
    },
    {
        "instruction": "What is ZCC's IR instruction set?",
        "input": "",
        "output": "ZCC IR uses a register-machine model with unlimited virtual registers. Arithmetic: ADD, SUB, MUL, DIV, MOD, NEG. Bitwise: AND, OR, XOR, SHL, SHR, NOT. Comparison: EQ, NE, LT, LE, GT, GE. Memory: LOAD, STORE, ALLOCA, ADDR. Control: BR (unconditional branch), CONDBR (conditional branch with true/false targets), RET, CALL, PHI. Other: CONST (immediate), COPY (register move), CAST (type conversion), NOP. Each instruction has an opcode, destination register, source register array, and optional immediate value. PHI instructions carry an array of (block_id, register) pairs."
    },
    {
        "instruction": "How does ZCC handle the hybrid AST/IR stack frame?",
        "input": "",
        "output": "In body_only mode, the AST prologue allocates the stack frame (subq $N, %rsp) and saves callee-saved registers. The IR body emitter then extends the frame with subq $ir_extra, %rsp for its own spill slots. IR slots are addressed via slot_base = -stack_size, placing them below the AST frame. This prevents CG-IR-008 (slot collision where IR register 0 at -16(%rbp) overwrote the second parameter). CG-IR-011 forces all 5 callee-saved registers to be saved when IR is active, since the IR linear scan allocator may use different registers than the AST allocator predicted."
    }
]
dataset.extend(arch_pairs)
print(f"Architecture knowledge: {len(arch_pairs)} pairs, {len(dataset)} total")

# === SOURCE D: IR opcode training ===
opcodes = {
    "OP_ADD": "Integer addition of two virtual registers",
    "OP_SUB": "Integer subtraction",
    "OP_MUL": "Integer multiplication (candidate for strength reduction to SHL/LEA)",
    "OP_DIV": "Integer division (signed uses IDIV with CQO, unsigned uses DIV with XOR RDX)",
    "OP_LOAD": "Load value from memory address in source register to destination register",
    "OP_STORE": "Store value from src[0] to memory address in src[1], imm field = byte width",
    "OP_ALLOCA": "Allocate stack space, imm = byte count, dst = base address register",
    "OP_PHI": "SSA phi node merging values from predecessor blocks, resolved by parallel edge copy",
    "OP_BR": "Unconditional branch, src[0] = target block ID",
    "OP_CONDBR": "Conditional branch, src[0] = condition, src[1] = true block, src[2] = false block",
    "OP_CALL": "Function call, call_name = target, call_args[] = arguments, dst = return value",
    "OP_RET": "Return from function, src[0] = return value (if any)",
    "OP_CONST": "Load immediate constant, imm = value, dst = destination register",
    "OP_COPY": "Register-to-register copy (often eliminated by copy propagation)",
    "OP_CAST": "Type conversion between IR types (sign extend, zero extend, truncate)",
}
for op, desc in opcodes.items():
    dataset.append({
        "instruction": f"What does the ZCC IR opcode {op} do?",
        "input": "",
        "output": f"{op}: {desc}. Used in compiler_passes.c IR backend for x86-64 code generation."
    })
print(f"Opcode training: {len(opcodes)} pairs, {len(dataset)} total")

# === SOURCE E: Test suite knowledge ===
tests = [
    ("return_42", "Basic return value", "Verifies IR can emit a function that returns a constant"),
    ("for_loop_sum", "For-loop with accumulator", "Guards against CG-IR-014 (ZND_ASSIGN missing from expr handler). Sum 0..9 must equal 45."),
    ("while_factorial", "While-loop factorial", "Tests Mem2Reg on loop-carried multiplication. 5! must equal 120."),
    ("pointer_deref", "Pointer dereference", "Guards against CG-IR-010 (movl for pointer load). Dereferenced value must match original."),
    ("recursive_fib", "Recursive fibonacci", "Tests callee-saved register preservation (CG-IR-011) across recursive calls. fib(10)=55."),
    ("alloc_pattern", "Malloc + zero-fill loop", "Guards against CG-IR-005 (PHI lost-copy). The original crash pattern from cc_alloc."),
    ("nested_loops", "Nested for-loops", "Tests multi-PHI edge copy with two loop levels. 5*3=15 iterations."),
    ("eight_locals", "8 local variables", "Stress-tests register allocation spilling. Sum of 1..8 must equal 36."),
]
for name, title, desc in tests:
    dataset.append({
        "instruction": f"What does the ZCC regression test '{name}' verify?",
        "input": "",
        "output": f"Test '{name}' ({title}): {desc}"
    })
print(f"Test suite: {len(tests)} pairs, {len(dataset)} total")

# Write JSONL
out_path = "zcc_mini_training.jsonl"
with open(out_path, "w") as f:
    for entry in dataset:
        f.write(json.dumps(entry) + "\n")

print(f"\n=== COMPLETE ===")
print(f"Total training pairs: {len(dataset)}")
print(f"Output: {out_path} ({os.path.getsize(out_path)} bytes)")

# Also write as JSON for HuggingFace upload
with open("zcc_mini_training.json", "w") as f:
    json.dump(dataset, f, indent=2)
print(f"Also wrote: zcc_mini_training.json")
PYEOF

echo "=== DONE ==="
