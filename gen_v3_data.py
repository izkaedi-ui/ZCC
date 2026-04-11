import json, os, random

dataset = []

# Load existing bug corpus
bugs = []
for p in ["zcc-compiler-bug-corpus/zcc-compiler-bug-corpus.json", "zcc-compiler-bug-corpus.json"]:
    if os.path.exists(p):
        bugs = json.load(open(p))
        break

# === BUG CORPUS: 8 variations per bug = 96 entries ===
question_templates = [
    "What is the {id} bug in ZCC?",
    "Explain the {id} compiler bug.",
    "Describe {id}: {title}",
    "What caused {id} in the ZCC compiler?",
    "How was {id} detected and fixed?",
    "What is the root cause of {id}?",
    "A ZCC compiled binary shows this symptom: {symptom}. What bug is this?",
    "What regression test guards against {id}?",
]

for bug in bugs:
    for tmpl in question_templates:
        q = tmpl.format(**bug)
        a = f"Bug {bug['id']}: {bug['title']}\n\n"
        a += f"CWE: {bug['cwe']} ({bug['cwe_name']})\n"
        a += f"Severity: {bug['severity']}\n"
        a += f"PRIME Energy Score: {bug['prime_energy_score']}\n\n"
        a += f"Root Cause: {bug['root_cause']}\n\n"
        a += f"Detection: {bug['detection_method']}\n\n"
        a += f"Fix: {bug['fix_summary']}\n\n"
        a += f"Before: {bug['before_pattern']}\n"
        a += f"After: {bug['after_pattern']}\n\n"
        a += f"Regression test: {bug['regression_test']}\n"
        a += f"Tags: {', '.join(bug['tags'])}"
        dataset.append({"instruction": q, "input": "", "output": a})

print(f"Bug variations: {len(dataset)}")

# === CROSS-BUG QUESTIONS: 20 entries ===
cross = [
    ("List all CG-IR bugs in ZCC from most to least destructive.",
     "\n".join(f"{b['prime_energy_score']}: {b['id']} - {b['title']}" for b in sorted(bugs, key=lambda x: -x['prime_energy_score']))),
    ("Which ZCC bugs involve stack frame corruption?",
     "\n".join(f"{b['id']}: {b['title']}" for b in bugs if any(t in b['tags'] for t in ['stack-corruption','stack-overflow','frame-size','slot-collision','RSP-misalignment']))),
    ("Which ZCC bugs involve pointer truncation?",
     "\n".join(f"{b['id']}: {b['title']}" for b in bugs if 'pointer-truncation' in b['tags'] or 'movl-vs-movq' in b['tags'])),
    ("Which ZCC bugs are in the bridge layer?",
     "\n".join(f"{b['id']}: {b['title']}" for b in bugs if 'bridge-layer' in b['tags'])),
    ("Which ZCC bugs involve missing switch cases?",
     "\n".join(f"{b['id']}: {b['title']}" for b in bugs if 'missing-switch-case' in b['tags'])),
    ("What is the most destructive bug in ZCC and why?",
     "CG-IR-012b (PRIME energy 9.9): 33 dead accessor stubs returning zero. Every node_kind() returned 0, every node_lhs() returned NULL. The entire AST-to-IR bridge was structurally hollow. Optimization passes operated on garbage data. Fix: replaced static stubs with extern declarations."),
    ("How many codegen bugs has ZCC had?",
     f"12 codegen bugs (CG-IR-003 through CG-IR-014), all discovered and fixed in a single debugging session on April 4, 2026. Total lines changed: {sum(b['lines_changed'] for b in bugs)}. All verified through self-host invariant (cmp zcc2.s zcc3.s = byte-identical)."),
    ("What CWE classes appear in ZCC bugs?",
     "7 unique CWEs: " + ", ".join(sorted(set(f"{b['cwe']} ({b['cwe_name']})" for b in bugs)))),
    ("What fix types were used for ZCC bugs?",
     "10 fix types: " + ", ".join(sorted(set(b['fix_type'] for b in bugs)))),
    ("Rank ZCC bugs by PRIME energy score.",
     "\n".join(f"{b['prime_energy_score']}: {b['id']} ({b['prime_phase']}) - {b['title']}" for b in sorted(bugs, key=lambda x: -x['prime_energy_score']))),
    ("What is PRIME energy scoring for compiler bugs?",
     "PRIME energy score rates bug severity on the ZKAEDI PRIME Recursively Coupled Hamiltonian scale. 9.5-10.0 = critical bifurcation (system cannot converge, e.g. CG-IR-012b at 9.9). 8.5-9.4 = high energy (crashes on first use, e.g. CG-IR-005 at 9.8). 7.0-8.4 = moderate (manifests under specific conditions, e.g. CG-IR-009 at 7.0). The score reflects how deeply the bug disrupts the Hamiltonian convergence of the self-hosting bootstrap chain."),
    ("What is the self-host invariant in ZCC?",
     "The self-host invariant is: cmp zcc2.s zcc3.s must be byte-identical. zcc (GCC-built) compiles itself to produce zcc2. zcc2 compiles itself to produce zcc3.s. If zcc2.s == zcc3.s, the compiler is a fixed point. Any codegen bug affecting any of the 175+ functions breaks this invariant. This is a stronger property than just compiling without errors."),
    ("How does ZCC's dual-emission architecture work?",
     "ZCC has two codegen paths: AST-direct (part4.c) tree-walks the AST and emits x86-64 assembly directly. The IR backend (compiler_passes.c) translates AST to 3-address SSA IR, runs 12 optimization passes, then emits optimized assembly. The IR backend operates in body_only mode: AST owns prologue/epilogue, IR owns the function body. Controlled by ir_whitelisted() in part4.c. The hybrid frame uses slot_base=-stack_size to avoid CG-IR-008 slot collisions."),
    ("What optimization passes does ZCC run?",
     "ZCC runs 12 passes in run_all_passes() (compiler_passes.c line 4451): 1) Reachability analysis 2) Parameter escape marking 3) PGO instrumentation 4) Constant folding 5) Strength reduction 6) Copy propagation 7) Peephole 8) Redundant load elimination 9) DCE (SSA) 10) Escape analysis 11) Scalar promotion (Mem2Reg) with single-block fast path and multi-block dominance frontier path 12) PGO block reordering. Results on self-compilation: 4,331 peephole instructions elided, 10.8% dead code eliminated."),
    ("How does Mem2Reg work in ZCC?",
     "ZCC's Mem2Reg (scalar_promotion_pass, compiler_passes.c line 1478) has two paths. Single-block: if all loads/stores to an alloca are in one block, replace with register tracking (current_val). Multi-block (multi_block_mem2reg_one, line 1681): compute def blocks, iterate dominance frontier worklist to find PHI placement points, insert OP_PHI nodes, then recursively rename via mem2reg_rename_rec over the dominator tree. Parameter allocas are marked as escaping in run_all_passes() to prevent promotion."),
    ("What is the PHI edge copy problem in ZCC?",
     "PHI edge copies resolve SSA PHI nodes to register moves on control flow edges. ZCC's ir_asm_emit_phi_edge_copy (compiler_passes.c line 4826) walks all consecutive OP_PHI instructions at a target block's head. CG-IR-005 fixed three sub-bugs: 1) Liveness back-edge inversion 2) CONDBR emitting copies on both branches 3) Serial copy through rax causing the lost-copy problem. Fix: parallel copy via push/pop - all PHI sources pushed first, then popped to destinations in reverse order."),
    ("What is ZCC's IR instruction set?",
     "ZCC IR uses unlimited virtual registers. Arithmetic: ADD, SUB, MUL, DIV, MOD, NEG. Bitwise: AND, OR, XOR, SHL, SHR, NOT. Comparison: EQ, NE, LT, LE, GT, GE. Memory: LOAD, STORE, ALLOCA, ADDR. Control: BR (unconditional, src[0]=target block), CONDBR (src[0]=condition, src[1]=true block, src[2]=false block), RET, CALL, PHI. Other: CONST, COPY, CAST, NOP, PGO_COUNTER_ADDR."),
    ("How does ZCC handle the hybrid AST/IR stack frame?",
     "In body_only mode: AST prologue allocates frame (subq $N, %rsp) and saves callee-saved regs. IR extends frame with subq $ir_extra, %rsp (pre-scanned from max register ID + alloca sizes). IR slots use slot_base=-stack_size, placing them below the AST frame. CG-IR-008 fixed slot collision where IR reg 0 at -16(%rbp) overwrote argv. CG-IR-011 forces all 5 callee-saved regs when IR active since IR linear scan may use different regs than AST allocator predicted."),
    ("What files make up ZCC?",
     "ZCC source: part1.c (types/nodes/symbols, ~1200 lines), part2.c (lexer, ~800), part3.c (parser, ~1500), part4.c (x86-64 codegen + IR gate, ~2635), part5.c (main/peephole/globals, ~1100), ir.h (IR definitions, ~200), ir_bridge.h (AST-IR translation, 186), ir.c (IR module, ~400), ir_to_x86.c (standalone lowerer, ~300). Linked separately: compiler_passes.c (IR passes + body emitter, 7317), compiler_passes_ir.c (pass helpers, 570). Total: ~16,200 lines."),
    ("What is ZCC?",
     "ZCC is a self-hosting C compiler targeting x86-64 Linux (System V AMD64 ABI) built by ZKAEDI. It compiles a substantial subset of C including structs, pointers, arrays, function pointers, enums, and switch statements. It has two codegen paths: AST-direct and a 3-address SSA IR backend with 12 optimization passes. The compiler compiles itself with byte-identical output across bootstrap stages. 175+ functions go through the IR backend with 4,331 peephole instructions elided. Total source: ~16,200 lines."),
]
for q, a in cross:
    dataset.append({"instruction": q, "input": "", "output": a})
    # Rephrase each 2 more times
    dataset.append({"instruction": q.replace("?", " and explain in detail?").replace(".", " in ZCC."), "input": "", "output": a})
    dataset.append({"instruction": "As a ZCC compiler expert, " + q[0].lower() + q[1:], "input": "", "output": a})

print(f"After cross-bug: {len(dataset)}")

# === IR OPCODES: 5 variations each = 75 entries ===
opcodes = {
    "OP_ADD": "Integer addition of two virtual registers. Emits addq in x86-64.",
    "OP_SUB": "Integer subtraction. Emits subq in x86-64.",
    "OP_MUL": "Integer multiplication. Candidate for strength reduction to SHL or LEA.",
    "OP_DIV": "Integer division. Signed uses IDIV with CQO, unsigned uses DIV with XOR RDX.",
    "OP_MOD": "Integer modulo. Same instruction selection as DIV but result comes from RDX.",
    "OP_LOAD": "Load value from memory address in source register to destination register. Always uses movq (8 bytes) on x86-64 after CG-IR-010 fix.",
    "OP_STORE": "Store value from src[0] to memory address in src[1]. imm field = byte width. Always uses movq after CG-IR-010 fix.",
    "OP_ALLOCA": "Allocate stack space. imm = byte count, dst = base address register. Escape analysis determines if promotable by Mem2Reg.",
    "OP_PHI": "SSA phi node merging values from predecessor blocks. phi[p].block = predecessor block ID, phi[p].reg = value register. Resolved by ir_asm_emit_phi_edge_copy using parallel push/pop.",
    "OP_BR": "Unconditional branch. src[0] = target block ID. Emits jmp to block label.",
    "OP_CONDBR": "Conditional branch. src[0] = condition register, src[1] = true block ID, src[2] = false block ID. Emits testq + jz + jmp sequence.",
    "OP_CALL": "Function call. call_name = target function, call_args[] = arguments (up to 6 in registers per System V ABI), dst = return value in rax.",
    "OP_RET": "Return from function. src[0] = return value (moved to rax). Emits jmp to function epilogue label.",
    "OP_CONST": "Load immediate constant. imm = value, dst = destination register. Emits movq $imm, slot.",
    "OP_COPY": "Register-to-register copy. Often eliminated by copy propagation pass.",
}
op_templates = [
    "What does {op} do in ZCC IR?",
    "Explain the ZCC IR opcode {op}.",
    "How is {op} used in ZCC's compiler_passes.c?",
    "What x86-64 instruction does ZCC emit for {op}?",
    "Describe {op} in ZCC's 3-address IR.",
]
for op, desc in opcodes.items():
    for tmpl in op_templates:
        dataset.append({
            "instruction": tmpl.format(op=op),
            "input": "",
            "output": f"{op}: {desc} Used in ZCC's compiler_passes.c IR backend for x86-64 code generation."
        })

print(f"After opcodes: {len(dataset)}")

# === TEST SUITE: 5 variations each = 40 entries ===
tests = [
    ("return_42", "Basic return value", "Verifies IR can emit a function that returns a constant. Expected: exit code 42."),
    ("for_loop_sum", "For-loop with accumulator", "Guards against CG-IR-014 (ZND_ASSIGN missing from expr handler). for(i=0;i<10;i=i+1) sum+=i must return 45. If increment is missing, infinite loop."),
    ("while_factorial", "While-loop factorial", "Tests Mem2Reg on loop-carried multiplication. while(n>0){result*=n;n--} with n=5 must return 120."),
    ("pointer_deref", "Pointer dereference", "Guards against CG-IR-010 (movl for pointer load truncation). int x=42; int *p=&x; return *p; must return 42."),
    ("recursive_fib", "Recursive fibonacci", "Tests callee-saved register preservation (CG-IR-011) across recursive calls. fib(10) must return 55."),
    ("alloc_pattern", "Malloc + zero-fill loop", "Guards against CG-IR-005 (PHI lost-copy). The original crash pattern from cc_alloc: calloc then for-loop zeroing."),
    ("nested_loops", "Nested for-loops", "Tests multi-PHI edge copy with two loop levels carrying independent variables. 5*3=15 iterations."),
    ("eight_locals", "8 local variables", "Stress-tests register allocation and spilling. a=1..h=8, sum must equal 36."),
]
test_templates = [
    "What does the ZCC test '{name}' verify?",
    "Explain the regression test '{name}' in ZCC's test suite.",
    "Why does ZCC have a test called '{name}'?",
    "What bug does the '{name}' test guard against?",
    "Describe the ZCC test '{name}' and its expected output.",
]
for name, title, desc in tests:
    for tmpl in test_templates:
        dataset.append({
            "instruction": tmpl.format(name=name),
            "input": "",
            "output": f"Test '{name}' ({title}): {desc}"
        })

print(f"After tests: {len(dataset)}")

# === ARCHITECTURE: 3 rephrasings each = 60 entries ===
arch = [
    ("What is ir_whitelisted() in ZCC?", "ir_whitelisted() in part4.c (line ~1890) is the gate that controls which functions use the IR backend vs AST-direct codegen. When it returns 1, codegen_func() delegates the function body to zcc_run_passes_emit_body_pgo() in compiler_passes.c. When 0, codegen uses the AST-direct tree-walk path. A blacklist array inside the function can exclude specific functions. For full IR self-host, it returns 1 for all functions."),
    ("What is codegen_func() in ZCC?", "codegen_func() in part4.c (line ~1909) orchestrates function-level code generation. It emits: 1) AST prologue (pushq %rbp, movq %rsp,%rbp, subq $N,%rsp, callee saves, param stores) 2) IF ir_whitelisted() returns 1: delegates body to zcc_run_passes_emit_body_pgo() 3) ELSE: emits body via codegen_stmt() (AST-direct) 4) AST epilogue (callee restores, movq %rbp,%rsp, popq %rbp, ret)."),
    ("What is run_all_passes() in ZCC?", "run_all_passes() in compiler_passes.c (line 4451) executes the full 12-pass optimization pipeline on a Function IR. Order: reachability -> parameter escape marking -> PGO instrumentation -> constant folding -> strength reduction -> copy propagation -> peephole -> redundant load elimination -> DCE -> escape analysis -> Mem2Reg -> PGO block reordering. Each pass logs statistics to stderr."),
    ("What is zcc_run_passes_emit_body_pgo()?", "zcc_run_passes_emit_body_pgo() in compiler_passes.c (line 5394) is the IR body emission entry point called from codegen_func(). It: 1) Converts AST to IR via zcc_ast_to_ir() 2) Runs run_all_passes() 3) Sets up IRAsmCtx with body_only=1, slot_base=-stack_size 4) Pre-scans IR for frame depth 5) Emits subq $ir_extra, %rsp 6) Calls ir_asm_emit_function_body() 7) Frees IR and returns block count."),
    ("What is IRAsmCtx in ZCC?", "IRAsmCtx (compiler_passes.c line ~4775) is the context struct for IR-to-x86 assembly emission. Key fields: out (FILE*), fn (Function*), body_only (1=AST owns prologue), slot_base (CG-IR-008: base offset for IR spill slots), used_callee_saved_mask (bits for rbx/r12-r15), func_end_label (jump target for OP_RET), num_params, alloca_off[] (per-register stack offsets)."),
    ("What is ir_bridge.h?", "ir_bridge.h (186 lines) is the translation layer between ZCC's AST (Node structs in part1.c) and the IR (Instr/Block/Function in compiler_passes.c). It provides: ir_map_type() mapping 18 C types to IR types, ir_bridge_fresh_tmp() for temporary name generation, ir_var_name() for variable name extraction, ir_emit_binary_op() for convenience emission, and ir_emit_var_load() for variable loads with array/struct decay handling."),
    ("What is zcc_node_from_expr()?", "zcc_node_from_expr() in compiler_passes.c (line ~2880) converts an AST Node* to a ZCCNode* for IR consumption. It maps node kinds via nd_to_znd(), then populates children (lhs, rhs, cond, args) by recursively calling itself. CG-IR-014 was caused by a missing ZND_ASSIGN case - for-loop increments fell through to default:break, producing nodes with null lhs/rhs."),
    ("What is zcc_node_from_stmt()?", "zcc_node_from_stmt() in compiler_passes.c (line ~3071) converts statement AST nodes to ZCCNode*. Handles ZND_IF, ZND_WHILE, ZND_FOR, ZND_RETURN, ZND_BLOCK, ZND_SWITCH. CG-IR-013 was caused by a missing ZND_CALL case - statement-level function calls fell through to default:break, producing empty call nodes that became indirect callq *%rax through uninitialized vreg 0."),
    ("How does ZCC's linear scan register allocator work?", "ZCC's IR backend uses linear scan register allocation. Liveness intervals are computed from instruction numbering. Physical registers are assigned from the System V ABI callee-saved set (rbx, r12, r13, r14, r15). When all physical registers are occupied, the interval with the furthest next-use is spilled to a stack slot below slot_base. CG-IR-011 required forcing all 5 callee-saved registers when IR is active."),
    ("What is the peephole optimizer in ZCC?", "peephole_optimize() in part5.c scans emitted x86-64 assembly for redundant patterns: redundant mov (same src/dst), push/pop of same register, dead stores overwritten before use, redundant loads after stores. It eliminates 4,331 instructions when ZCC compiles itself. This is a post-emission pass on the AST-direct output, separate from the IR-level peephole pass in run_all_passes()."),
    ("What is escape analysis in ZCC?", "ZCC's escape_analysis_pass() in compiler_passes.c determines which OP_ALLOCA allocations can be stack-promoted vs must remain on the heap. Non-escaping allocas are candidates for Mem2Reg promotion to SSA registers. An alloca escapes if its address is stored to memory, passed to a function call, or returned. Parameter allocas are force-marked as escaping to prevent Mem2Reg from breaking parameter passing."),
    ("What is the ZCC bootstrap chain?", "The ZCC bootstrap chain verifies self-hosting: 1) GCC compiles zcc.c -> zcc (stage 0) 2) zcc compiles zcc.c -> zcc2.s (stage 1) 3) gcc assembles zcc2.s -> zcc2 (stage 1 binary) 4) zcc2 compiles zcc.c -> zcc3.s (stage 2) 5) cmp zcc2.s zcc3.s must be byte-identical. This proves the compiler is a fixed point - it produces the same output when compiling itself. Build command: make clean && make selfhost."),
    ("What does ZCC compile?", "ZCC compiles a substantial subset of C: all integer types (char through long long, signed and unsigned), void, enum, struct, union, pointers, arrays, function pointers. Statements: if/else, while, do/while, for, switch/case/default, break, continue, return, goto. Full expression set including ternary, comma, sizeof, casts, pre/post increment. Up to 6 register args (System V ABI). No preprocessor, no floats, no VLA, no variadic functions."),
    ("How many lines of code is ZCC?", "ZCC is approximately 16,200 lines of C across 11 source files. The largest file is compiler_passes.c at 7,317 lines (IR optimization pipeline and body emitter). part4.c is 2,635 lines (codegen). part3.c is ~1,500 lines (parser). The source is concatenated into zcc.c for compilation via cat in the Makefile."),
    ("What is ZKAEDI PRIME in the context of ZCC?", "ZKAEDI PRIME (Recursively Coupled Hamiltonian dynamics) is used in ZCC for bug severity scoring. The formula H_t = H_0 + eta*H_{t-1}*sigma(gamma*H_{t-1}) + epsilon*N(0,1+beta*|H_{t-1}|) assigns energy scores to codegen bugs. Higher energy = more destructive (CG-IR-012b = 9.9, CG-IR-009 = 7.0). Phase classifications: bifurcation_critical, bifurcation, chaotic, wandering, converging. The score reflects how deeply a bug disrupts the self-hosting bootstrap convergence."),
]
rephrase_prefixes = [
    "",
    "In the ZCC compiler, ",
    "Explain: ",
    "As a ZCC expert, answer: ",
]
for q, a in arch:
    for prefix in rephrase_prefixes:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After architecture: {len(dataset)}")

# === IDENTITY: 10 entries ===
identity = [
    ("Who are you?", "I am ZKAEDI-MINI, a 7B parameter language model specialized in compiler engineering and smart contract security. I was trained by ZKAEDI on the ZCC compiler bug corpus and PRIME constitutions dataset. I have deep knowledge of ZCC's IR instruction set, optimization passes, codegen bugs (CG-IR-003 through CG-IR-014), and the self-hosting bootstrap chain."),
    ("What do you specialize in?", "I specialize in: 1) ZCC compiler internals - dual-emission architecture, SSA IR, 12 optimization passes, register allocation 2) Compiler bug analysis - 12 codegen bugs with CWE classifications and PRIME energy scores 3) Solidity smart contract security - reentrancy, access control, integer overflow 4) ZKAEDI PRIME Hamiltonian dynamics for severity scoring."),
    ("What is your training data?", "I was trained on: ZCC compiler bug corpus (12 real codegen bugs with ground-truth fixes), ZCC architecture documentation (IR opcodes, pass pipeline, hybrid frame design), ZKAEDI PRIME constitutions (127 trace-constitution pairs), and Solidity vulnerability patterns. Base model: Qwen2.5-7B-Instruct with LoRA r=16, alpha=32."),
]
for q, a in identity:
    for prefix in ["", "ZKAEDI-MINI, ", "Hey, "]:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After identity: {len(dataset)}")

# Shuffle
random.seed(42)
random.shuffle(dataset)

# Write
out = "zcc_mini_training_v3.json"
with open(out, "w") as f:
    json.dump(dataset, f, indent=2)
print(f"\n=== COMPLETE ===")
print(f"Total: {len(dataset)} training pairs")
print(f"Output: {out} ({os.path.getsize(out)} bytes)")
