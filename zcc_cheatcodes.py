#!/usr/bin/env python3
"""
🔱 ZCC DEEP COMPILER CHEATCODES v1.0.0
═══════════════════════════════════════

The complete field manual for upgrading ZCC from self-hosting C89 compiler
to production-grade optimizing compiler. Each section is self-contained
with exact file locations, implementation patterns, traps, and verification.

                    ┌─────────────────────────┐
                    │  ZCC Architecture Map    │
                    ├─────────────────────────┤
                    │  part1.c  — Lexer        │  Tokens, keywords, literals
                    │  part2.c  — Parser/AST   │  Types, nodes, declarations
                    │  part3.c  — Sema/Decls   │  Global/local scope, type checking
                    │  part4.c  — Codegen      │  AST → x86-64 assembly
                    │  part5.c  — Main/Driver  │  CLI, file I/O, peephole hook
                    │  ir.c     — IR Backend   │  3-address IR emission
                    │  ir_bridge.h — Bridge    │  AST codegen ↔ IR temps
                    └─────────────────────────┘

    Concatenation: part1→part2→part3→ir.h→ir_emit_dispatch.h→ir_bridge.h→part4→part5→ir.c

    Golden Rule: stage2 == stage3 bitwise. Everything else is a hypothesis.
    C89 Rule:    All declarations at top of block. No exceptions.

════════════════════════════════════════════════════════════════════════════
"""

CHEATCODES = """

╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 1: Large Array Initializers → .rodata Emission              ║
║  Difficulty: ★★★☆☆   Impact: ★★★★★   Files: part3.c, part4.c         ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  ZCC chokes on `static const long long W1[2944] = {12345LL, ...};`
  because the parser converts each initializer element into an ND_ASSIGN
  AST node. 12,000 array elements = 12,000 AST nodes = token arena overflow.

  Current workaround: Emit weights as raw GNU assembler (.quad directives)
  in a separate .s file. Works but requires a two-file build.

THE FIX — Three surgical changes:

─── Step 1: Lexer (part1.c) ───
  No changes needed. The lexer already tokenizes `{`, `}`, `,`, and
  integer literals correctly. The bottleneck is the parser.

─── Step 2: Parser (part3.c) — decl_global() ───

  Currently, global arrays with initializers do something like:

      for each element:
          create ND_ASSIGN(array[i], value)
          add to init_list

  Replace with a DIRECT DATA EMISSION path:

  ```c
  /* Inside decl_global() or wherever global var init is handled */

  /* Detect: type is array AND has initializer list AND all elements are constants */
  if (var->ty->kind == TY_ARRAY && is_all_constant_init(init_list)) {
      /* Don't create AST nodes. Store raw values in a data buffer. */
      var->init_data = alloc_init_buffer(var->ty->array_len * type_size(var->ty->base));
      int offset;
      offset = 0;
      for (/* each initializer */) {
          long long val;
          val = eval_const_expr(init_node);
          memcpy(var->init_data + offset, &val, type_size(var->ty->base));
          offset = offset + type_size(var->ty->base);
      }
      var->has_init_data = 1;
      /* Skip AST init list entirely */
      return;
  }
  ```

  THE STRUCT ADDITION (part2.c or wherever Symbol/Var is defined):
  ```c
  /* Add to your variable/symbol struct: */
  char *init_data;       /* raw bytes for constant initializers */
  int   init_data_size;  /* byte count */
  int   has_init_data;   /* flag: 1 = emit as .rodata, 0 = normal codegen */
  ```

─── Step 3: Codegen (part4.c) — emit_global_data() ───

  When emitting assembly for global variables, check for init_data:

  ```c
  void emit_global_var(Var *var) {
      if (var->has_init_data) {
          /* Emit directly to .rodata (read-only) or .data */
          fprintf(out, "    .section .rodata\\n");
          fprintf(out, "    .p2align 3\\n");  /* 8-byte align for long long */
          fprintf(out, "    .globl %s\\n", var->name);
          fprintf(out, "%s:\\n", var->name);

          int i;
          int elem_size;
          elem_size = type_size(var->ty->base);

          for (i = 0; i < var->init_data_size; i += elem_size) {
              long long val;
              memcpy(&val, var->init_data + i, elem_size);

              if (elem_size == 8)
                  fprintf(out, "    .quad %lld\\n", val);
              else if (elem_size == 4)
                  fprintf(out, "    .long %d\\n", (int)val);
              else if (elem_size == 2)
                  fprintf(out, "    .short %d\\n", (short)val);
              else
                  fprintf(out, "    .byte %d\\n", (char)val);
          }
          fprintf(out, "    .text\\n");
          return;
      }

      /* ... existing codegen for non-constant globals ... */
  }
  ```

─── Traps ───

  ⚠️ TRAP 1: String initializers.
     `char s[] = "hello"` is also a constant initializer but uses
     ND_STR, not ND_NUM. Handle separately — emit as .asciz.

  ⚠️ TRAP 2: Nested struct initializers.
     `struct { int x; int y; } p = {1, 2}` has two levels.
     Start with flat arrays only. Structs can come later.

  ⚠️ TRAP 3: Designated initializers.
     `int a[100] = {[50] = 1}` — ZCC doesn't support these anyway.
     But if you add them later, the init_data buffer needs sparse fill.

  ⚠️ TRAP 4: C89 declaration ordering.
     All variables inside the new parser path MUST be declared at
     block top. This code will be self-hosted.

─── Verification ───

  Test file:
  ```c
  static const long long W[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  int main() {
      long long sum; int i;
      sum = 0;
      for (i = 0; i < 8; i++) sum = sum + W[i];
      printf("%lld\\n", sum);  /* expect 36 */
      return 0;
  }
  ```

  Verify: gcc output == zcc output == 36
  Then scale to 12,352 elements (the full weight array).
  Then: make clean && make selfhost (stage2 == stage3).


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 2: FPU / SSE Pipeline (float & double)                      ║
║  Difficulty: ★★★★★   Impact: ★★★★★   Files: part1-4.c                 ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  ZCC has zero floating-point support. No float, no double, no XMM
  registers. All neural inference runs through Q16.16 fixed-point.

THE FIX — Four layers:

─── Layer 1: Lexer (part1.c) — Add float/double tokens ───

  Add keywords:
  ```c
  /* In the keyword table (wherever "int", "long", etc. are registered): */
  {"float",    TK_FLOAT},
  {"double",   TK_DOUBLE},
  ```

  Add float literal parsing:
  ```c
  /* In the number literal scanner, after integer parsing: */
  /* Detect: digits followed by '.' or 'e'/'E' */
  if (/* current char == '.' || current char == 'e' || current char == 'E' */) {
      /* Parse as double */
      double fval;
      fval = strtod(start, &end);
      tok->kind = TK_FLIT;
      tok->fval = fval;
      /* Check for 'f' suffix → float, otherwise double */
  }
  ```

  STRUCT ADDITION (Token):
  ```c
  double fval;  /* add to Token struct for float literals */
  ```

─── Layer 2: Type System (part2.c) — TY_FLOAT, TY_DOUBLE ───

  ```c
  /* Add to type kinds enum: */
  TY_FLOAT,   /* 4 bytes, XMM register */
  TY_DOUBLE,  /* 8 bytes, XMM register */

  /* Type creation helpers: */
  Type *ty_float(void)  { return new_type(TY_FLOAT,  4, 4); }
  Type *ty_double(void) { return new_type(TY_DOUBLE, 8, 8); }

  /* Type checking helpers: */
  int is_float_type(Type *ty) {
      return ty->kind == TY_FLOAT || ty->kind == TY_DOUBLE;
  }
  ```

  IMPLICIT CONVERSION RULES (C standard):
  ```
  int    + float  → float   (int widened to float)
  int    + double → double  (int widened to double)
  float  + double → double  (float widened to double)
  ```

  You need a `usual_arithmetic_conversion()` function that returns
  the common type for binary operations.

─── Layer 3: Codegen (part4.c) — XMM Register Emission ───

  This is the hard part. You need a parallel register world.

  SYSTEM V AMD64 FLOAT CALLING CONVENTION:
  ```
  Arguments:  %xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7
  Return:     %xmm0
  Callee-saved: NONE (all XMM are caller-saved!)
  ```

  KEY: Integer args still go in rdi/rsi/rcx/rdx/r8/r9.
  Float args go in xmm0-xmm7. They use SEPARATE COUNTERS.

  ```c
  /* Mixed call example: void foo(int a, float b, int c, double d) */
  /* a → %edi (int slot 0)   */
  /* b → %xmm0 (float slot 0) */
  /* c → %esi (int slot 1)   */
  /* d → %xmm1 (float slot 1) */
  ```

  INSTRUCTION TABLE:
  ```
  Operation       float (32-bit)    double (64-bit)
  ─────────────   ──────────────    ───────────────
  Load            movss             movsd
  Store           movss             movsd
  Add             addss             addsd
  Subtract        subss             subsd
  Multiply        mulss             mulsd
  Divide          divss             divsd
  Compare         ucomiss           ucomisd
  Int→Float       cvtsi2ss          cvtsi2sd
  Float→Int       cvttss2si         cvttsd2si
  Float→Double    cvtss2sd          —
  Double→Float    —                 cvtsd2ss
  Negate          xorps (flip sign) xorpd (flip sign)
  ```

  NEGATE TRICK (no hardware fneg for SSE):
  ```asm
  /* Negate float in %xmm0: XOR with sign-bit mask */
  .section .rodata
  .p2align 4
  float_sign_mask: .long 0x80000000, 0, 0, 0
  /* ... */
  xorps float_sign_mask(%rip), %xmm0
  ```

  FLOAT LITERAL EMISSION:
  ```asm
  /* Can't movss $3.14, %xmm0 — no immediate float loads in x86-64! */
  /* Must load from .rodata: */
  .section .rodata
  .p2align 2
  .LC0: .long 0x4048F5C3   /* IEEE 754 encoding of 3.14f */
  /* ... */
  movss .LC0(%rip), %xmm0
  ```

  ```c
  /* In codegen, for float literal node: */
  void emit_float_literal(float val) {
      /* Emit to .rodata section with unique label */
      int label_id;
      unsigned int bits;
      label_id = next_label();
      memcpy(&bits, &val, 4);
      emit_rodata_float(label_id, bits);
      fprintf(out, "    movss .LC%d(%%rip), %%xmm0\\n", label_id);
  }
  ```

─── Layer 4: Float Comparison Codegen ───

  Float comparisons use `ucomiss`/`ucomisd` which set FLAGS differently:

  ```asm
  ucomiss %xmm1, %xmm0    /* compare xmm0 vs xmm1 */
  /* CF=1 if xmm0 < xmm1 (or NaN) */
  /* ZF=1 if xmm0 == xmm1 */
  /* PF=1 if either is NaN (UNORDERED) */

  /* Ordered comparisons (NaN → false): */
  ja   label    /* xmm0 > xmm1 (unsigned above — NOT jg!) */
  jae  label    /* xmm0 >= xmm1 */
  jb   label    /* xmm0 < xmm1 */
  jbe  label    /* xmm0 <= xmm1 */
  je   label    /* xmm0 == xmm1 (and neither is NaN) */
  jne  label    /* xmm0 != xmm1 (or either is NaN) */
  jp   label    /* either is NaN (parity flag) */
  ```

  ⚠️ CRITICAL: Use UNSIGNED branch mnemonics (ja/jb) not signed (jg/jl)!
  This is CG-005 all over again but for floats.

─── Traps ───

  ⚠️ TRAP 1: XMM registers are 128-bit but you only use the low 32/64.
     movss loads 32 bits and zeros the upper 96.
     movsd loads 64 bits and zeros the upper 64.
     Do NOT use movaps/movapd for scalar ops — alignment requirements differ.

  ⚠️ TRAP 2: printf format specifiers.
     C promotes float to double for variadic calls.
     printf("%f", some_float) expects double in xmm0.
     You MUST emit cvtss2sd before the call if the arg is float.

  ⚠️ TRAP 3: Stack alignment for XMM spills.
     XMM spills to stack require 16-byte alignment.
     movaps crashes on misaligned addresses.
     Use movups (unaligned) for safety, or guarantee alignment.

  ⚠️ TRAP 4: Self-hosting.
     ZCC's own source code doesn't use float/double.
     So FPU support won't break self-hosting — but you can't TEST
     it via self-hosting either. You need a separate float test suite.

─── Verification ───

  ```c
  double mul(double a, double b) { return a * b; }
  int main() {
      double x; x = mul(3.14, 2.0);
      printf("%.2f\\n", x);  /* expect 6.28 */
      return 0;
  }
  ```
  Compare: gcc -O0 -S output vs zcc output, diff the function bodies.


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 3: Graph-Coloring Register Allocation                       ║
║  Difficulty: ★★★★☆   Impact: ★★★★☆   Files: part4.c (+ new regalloc)  ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  ZCC spills every local variable to the stack. `sum` in the inference
  inner loop lives at -8(%rbp), loaded and stored on every iteration.
  This adds 2 memory ops per variable access (~4-7 cycles each).

THE FIX — Linear Scan (simpler than Chaitin, nearly as good):

─── Available Registers ───

  System V callee-saved (survive across calls, ideal for loop vars):
    %rbx, %r12, %r13, %r14, %r15          (5 registers)

  Caller-saved (free to use between calls):
    %r10, %r11                              (2 registers)
    %rax, %rcx, %rdx, %rsi, %rdi           (5, but used for args/returns)

  For loop-carried variables (like `sum`, `i`), target the callee-saved
  set. They survive function calls so you don't need save/restore in loops.

─── Implementation Strategy (Linear Scan) ───

  Phase 1: Compute Live Ranges
  ```
  For each variable v in the function:
    v.start = first instruction index where v is defined
    v.end   = last instruction index where v is used
  ```

  Phase 2: Sort by start point, ascending.

  Phase 3: Linear scan allocation:
  ```python
  free_regs = ['%rbx', '%r12', '%r13', '%r14', '%r15']
  active = []  # (end_point, register, variable) sorted by end

  for var in sorted_by_start:
      # Expire old intervals
      expire_old(active, var.start, free_regs)

      if free_regs:
          reg = free_regs.pop()
          var.assigned_reg = reg
          active.append((var.end, reg, var))
          active.sort()  # keep sorted by end
      else:
          # Spill: pick the var with furthest end point
          spill_candidate = active[-1]
          if spill_candidate.end > var.end:
              # Spill the longer-lived var, give its reg to current
              var.assigned_reg = spill_candidate.reg
              spill_candidate.var.spilled = True
              active.pop()
              active.append((var.end, var.assigned_reg, var))
              active.sort()
          else:
              var.spilled = True  # current var gets spilled
  ```

  Phase 4: Modify codegen to use assigned registers.
  ```c
  /* In codegen_expr for variable access: */
  if (var->assigned_reg) {
      fprintf(out, "    movq %s, %%rax\\n", var->assigned_reg);
  } else {
      fprintf(out, "    movq %d(%%rbp), %%rax\\n", var->stack_offset);
  }
  ```

─── Where to Hook It ───

  Insert between AST construction and codegen emission:
  ```
  parse_function() → build AST → LIVENESS ANALYSIS → REGALLOC → codegen_func()
  ```

  The regalloc pass annotates each variable's Symbol with either
  `assigned_reg` (a string like "%r12") or leaves it NULL (spilled).

  STRUCT ADDITION (Symbol/Var):
  ```c
  char *assigned_reg;  /* NULL = stack, else register name */
  int   live_start;    /* first use instruction index */
  int   live_end;      /* last use instruction index */
  ```

─── Traps ───

  ⚠️ TRAP 1: Callee-saved register protocol.
     If you assign %r12 to a variable, the function prologue MUST
     push %r12 and the epilogue MUST pop it. Otherwise the caller's
     %r12 is corrupted. Add push/pop for every callee-saved reg
     that the allocator actually uses.

  ⚠️ TRAP 2: Address-taken variables CANNOT be register-allocated.
     If any code does `&var`, that variable MUST live on the stack.
     Scan the AST for ND_ADDR nodes and mark those vars as non-allocable.

  ⚠️ TRAP 3: Variables modified through pointers.
     `int *p = &x; *p = 5;` modifies x through p.
     Conservative: if x's address is ever taken, never register-allocate x.

  ⚠️ TRAP 4: Function calls clobber caller-saved registers.
     If a variable is live across a CALL node and assigned to a
     caller-saved register (%r10/%r11), it MUST be spilled before
     the call and reloaded after. Callee-saved regs (%rbx, %r12-r15)
     don't have this problem.

  ⚠️ TRAP 5: Self-hosting loop.
     Regalloc changes assembly output. If any reg assignment is wrong,
     stage2 != stage3 and you've broken the bootstrap. Test on external
     programs FIRST, then attempt self-host.

─── Verification ───

  The Q16.16 inference inner loop:
  ```c
  for (i = 0; i < L1_DIM; i++) {
      long long sum;
      sum = B1[i] * SCALE;
      for (j = 0; j < INPUT_DIM; j++) {
          sum = sum + (input[j] * W1[i * INPUT_DIM + j]);
      }
      h1[i] = sum / SCALE;
  }
  ```

  After regalloc, `sum` should live in %r12 (or similar),
  `i` in %r13, `j` in %r14. Zero stack traffic in the hot loop.

  Verify: gcc -O2 output vs zcc output — count memory ops in the loop.
  Target: ≤ 2 memory loads per iteration (W1[idx] and input[j]).


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 4: Control Flow Graph (CFG) Construction                    ║
║  Difficulty: ★★★☆☆   Impact: ★★★★★   Files: new cfg.c / cfg.h         ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  All current optimization passes operate on flat instruction lists.
  No knowledge of basic blocks, dominators, or loop structure.
  This limits every pass: DCE can't see cross-block liveness,
  constant propagation can't merge at join points, LICM can't
  identify loop-invariant code.

THE FIX — Basic Block Construction:

─── Data Structures ───

  ```c
  typedef struct BasicBlock {
      int id;
      int start_idx;      /* first instruction index in flat list */
      int end_idx;        /* last instruction index (inclusive) */
      int *successors;    /* array of BB ids */
      int num_succ;
      int *predecessors;  /* array of BB ids */
      int num_pred;
      int is_loop_header; /* set by loop detection */
      int loop_depth;     /* 0 = not in loop */
  } BasicBlock;

  typedef struct CFG {
      BasicBlock *blocks;
      int num_blocks;
      int entry_block;    /* always 0 */
      int *exit_blocks;   /* blocks ending in RET */
      int num_exits;
  } CFG;
  ```

─── Block Boundary Detection ───

  A new basic block starts at:
    1. The first instruction of a function
    2. Any LABEL instruction
    3. The instruction AFTER a BR, BR_IF, or RET

  A basic block ends at:
    1. A BR, BR_IF, or RET instruction
    2. The instruction BEFORE a LABEL (fall-through)

  ```python
  def build_cfg(instructions):
      leaders = {0}  # first instruction is always a leader

      for i, instr in enumerate(instructions):
          if instr.op == 'LABEL':
              leaders.add(i)
          elif instr.op in ('BR', 'BR_IF', 'RET'):
              if i + 1 < len(instructions):
                  leaders.add(i + 1)

      leaders = sorted(leaders)

      blocks = []
      for k, start in enumerate(leaders):
          end = leaders[k+1] - 1 if k+1 < len(leaders) else len(instructions) - 1
          blocks.append(BasicBlock(id=k, start=start, end=end))

      # Build edges
      label_to_block = {}
      for b in blocks:
          first = instructions[b.start]
          if first.op == 'LABEL':
              label_to_block[first.target] = b.id

      for b in blocks:
          last = instructions[b.end]
          if last.op == 'BR':
              b.successors = [label_to_block[last.target]]
          elif last.op == 'BR_IF':
              # True branch + fall-through
              b.successors = [label_to_block[last.target], b.id + 1]
          elif last.op == 'RET':
              b.successors = []
          else:
              # Fall-through to next block
              if b.id + 1 < len(blocks):
                  b.successors = [b.id + 1]

      return blocks
  ```

─── Loop Detection (Natural Loops) ───

  After CFG construction, find back edges using DFS:
  ```
  A back edge is an edge B→A where A dominates B.
  The natural loop for back edge B→A is: {A} ∪ {all nodes that
  can reach B without going through A}.
  ```

  Simpler approach (sufficient for ZCC):
  ```python
  def find_loops(blocks):
      # A LABEL that appears as a BR/BR_IF target from a LATER block
      # is a loop header. The source of that back edge is the loop tail.
      for b in blocks:
          for succ in b.successors:
              if succ <= b.id:  # back edge: jumps to earlier block
                  blocks[succ].is_loop_header = True
                  # Mark all blocks between header and tail as loop body
                  for mid in range(succ, b.id + 1):
                      blocks[mid].loop_depth += 1
  ```

─── Why This Unlocks Everything ───

  With a CFG, you can implement:
    • Loop-Invariant Code Motion (CHEATCODE 5)
    • Global Value Numbering (redundancy elimination across blocks)
    • Dominator tree → SSA form → all modern optimizations
    • Dead block elimination (unreachable code after return)
    • Loop unrolling (know iteration count from back edge structure)


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 5: Loop-Invariant Code Motion (LICM)                        ║
║  Difficulty: ★★★★☆   Impact: ★★★★☆   Files: new licm.py (IR level)    ║
║  Requires: CHEATCODE 4 (CFG)                                           ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  In the inference inner loop:
  ```c
  for (j = 0; j < INPUT_DIM; j++) {
      sum += input[j] * W1[i * INPUT_DIM + j];
  }
  ```
  The expression `i * INPUT_DIM` is recomputed every iteration of j,
  but `i` doesn't change inside the j-loop. It should be hoisted.

THE FIX:

─── Definition ───

  An instruction is LOOP-INVARIANT if:
    1. All its operands are defined outside the loop, OR
    2. All its operands are themselves loop-invariant

  An instruction can be HOISTED if:
    1. It is loop-invariant
    2. It has no side effects (not STORE, CALL, BR, etc.)
    3. Its destination is not redefined inside the loop
    4. It dominates all loop exits (ensures it would have executed anyway)

─── Algorithm ───

  ```python
  def licm(cfg, instructions):
      hoisted = 0
      for loop_header in cfg.loop_headers():
          loop_blocks = cfg.get_loop_body(loop_header)
          loop_instrs = set()
          for b in loop_blocks:
              for idx in range(b.start, b.end + 1):
                  loop_instrs.add(idx)

          # Find all defs inside the loop
          loop_defs = set()
          for idx in loop_instrs:
              dst = instructions[idx].dst
              if dst:
                  loop_defs.add(dst)

          # Iterative: mark loop-invariant instructions
          changed = True
          invariant = set()
          while changed:
              changed = False
              for idx in loop_instrs:
                  if idx in invariant:
                      continue
                  instr = instructions[idx]
                  if instr.op in SIDE_EFFECT_OPS:
                      continue
                  # Check: all operands are either constants,
                  # defined outside loop, or already marked invariant
                  all_invariant = True
                  for operand in get_operands(instr):
                      if operand in loop_defs and operand_def_idx not in invariant:
                          all_invariant = False
                          break
                  if all_invariant:
                      invariant.add(idx)
                      changed = True

          # Hoist: move invariant instructions to preheader
          preheader_idx = loop_header.start - 1  # insert before loop
          for idx in sorted(invariant):
              # Move instruction to preheader
              instructions.insert(preheader_idx, instructions.pop(idx))
              hoisted += 1

      return hoisted
  ```

─── The Preheader ───

  LICM needs a "preheader" block — a block that executes exactly once
  before the loop starts, with a single edge into the loop header.

  If no preheader exists, CREATE ONE:
  ```
  Before:  ... → [loop_header] → [loop_body] → [loop_header]

  After:   ... → [preheader] → [loop_header] → [loop_body] → [loop_header]
  ```

  Hoisted instructions go into the preheader.

─── Traps ───

  ⚠️ TRAP 1: Don't hoist loads.
     `LOAD %t0, addr` might read different values on each iteration
     if `addr` points to memory modified inside the loop.
     Only hoist loads from provably loop-invariant addresses
     (e.g., global constants, function arguments that aren't modified).

  ⚠️ TRAP 2: Don't hoist past exception points.
     `DIV %t0, %t1, %t2` could divide by zero. If it's inside a
     conditional branch in the loop, hoisting it makes it execute
     unconditionally — potentially crashing on inputs that the
     original code would have skipped.

  ⚠️ TRAP 3: Instruction index invalidation.
     Moving instructions changes all indices. Either:
     a) Collect all hoists first, then apply in reverse order, or
     b) Use a linked-list representation instead of array indices.


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 6: Peephole Optimizer In C (Replace Python)                 ║
║  Difficulty: ★★☆☆☆   Impact: ★★★☆☆   Files: part5.c                   ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  The peephole optimizer is a Python script (zcc_peephole.py) that
  reads and rewrites the .s file. This adds Python startup overhead
  and creates a dependency on Python being installed.

THE FIX — Port the critical patterns to C inside part5.c:

  ```c
  /* Key peephole patterns to port (most impactful first): */

  /* 1. Push immediately followed by pop to same register */
  /*    pushq %rax; popq %rax  →  (delete both)           */

  /* 2. Push followed by pop to different register */
  /*    pushq %rax; popq %rcx  →  movq %rax, %rcx         */

  /* 3. Redundant moves */
  /*    movq %rax, %rcx; movq %rcx, %rax  →  movq %rax, %rcx */

  /* 4. Load followed by identical store */
  /*    movq -8(%rbp), %rax; movq %rax, -8(%rbp)  →  (delete store) */

  /* 5. Add/sub zero */
  /*    addq $0, %rax  →  (delete)                          */
  /*    subq $0, %rax  →  (delete)                          */

  /* 6. Multiply by 1 */
  /*    imulq $1, %rax, %rax  →  (delete)                   */
  ```

  Implementation: read the .s file line by line into a buffer,
  apply pattern matching on consecutive pairs/triples, write back.
  All string comparison, no regex needed.

  ```c
  /* In part5.c, after assembly file is closed: */
  void peephole_optimize(char *filename) {
      /* Read all lines into array */
      char lines[MAX_LINES][MAX_LINE_LEN];
      int nlines;
      int i;
      int eliminated;

      nlines = read_asm_file(filename, lines, MAX_LINES);
      eliminated = 0;

      /* Pattern: pushq %REG; popq %REG → delete both */
      for (i = 0; i < nlines - 1; i++) {
          if (strncmp(lines[i], "    pushq ", 10) == 0 &&
              strncmp(lines[i+1], "    popq ", 9) == 0) {
              char *push_reg;
              char *pop_reg;
              push_reg = lines[i] + 10;
              pop_reg = lines[i+1] + 9;
              if (strcmp(push_reg, pop_reg) == 0) {
                  /* Same register — delete both */
                  lines[i][0] = 0;
                  lines[i+1][0] = 0;
                  eliminated = eliminated + 2;
              }
              /* Different register — convert to movq */
              /* ... */
          }
      }

      write_asm_file(filename, lines, nlines);
  }
  ```


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 7: Preprocessor (#include, #define, #ifdef)                 ║
║  Difficulty: ★★★☆☆   Impact: ★★★★☆   Files: new preproc.c             ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  ZCC can't parse #include <stdio.h> or any system headers.
  Self-hosting requires manual sed preprocessing to strip includes.
  External code (cJSON, SQLite) can't compile without includes.

THE FIX — Minimal preprocessor (3 directives):

  ```
  #define NAME value          — text substitution
  #include "file.h"          — local includes only (not system)
  #ifdef NAME / #ifndef / #endif / #else — conditional compilation
  ```

  DO NOT attempt to handle:
    - System includes (#include <...>) — too many GCC extensions
    - Macro functions (#define FOO(x) ...) — combinatorial explosion
    - Token pasting (##) or stringification (#)
    - #pragma, #line, #error

  Implementation: a single-pass text filter that runs BEFORE the lexer.
  Read source → expand #defines → inline #includes → strip #ifdef blocks
  → feed result to existing lexer.

  This is enough to self-host without sed hacks, and to compile
  most embedded C code that uses simple macros and local includes.

─── Traps ───

  ⚠️ TRAP: #include recursion.
     Track included files to prevent infinite loops.
     #include guard pattern (#ifndef FOO_H / #define FOO_H / #endif)
     is the standard protection — your #ifdef support handles it.

  ⚠️ TRAP: #define inside #ifdef.
     Defines must be conditional. If inside a false #ifdef block,
     the define should NOT be registered. Track ifdef nesting depth.


╔══════════════════════════════════════════════════════════════════════════╗
║  CHEATCODE 8: IR → LLVM IR Cross-Compilation                          ║
║  Difficulty: ★★★★☆   Impact: ★★★★★   Files: new llvm_emit.py          ║
╚══════════════════════════════════════════════════════════════════════════╝

THE PROBLEM:
  ZCC only targets x86-64. The IR is already in 3-address SSA-style
  form — it's structurally close to LLVM IR. Translating it would
  give you ARM64, RISC-V, WebAssembly, and all LLVM optimization
  passes for free.

THE FIX — Direct translation from zcc_ir.json → .ll file:

  ZCC IR → LLVM IR mapping:
  ```
  ZCC IR          LLVM IR
  ──────          ───────
  CONST 42        %t0 = add i64 0, 42
  ADD %t1 %t2     %t3 = add i64 %t1, %t2
  SUB %t1 %t2     %t3 = sub i64 %t1, %t2
  MUL             mul
  DIV (signed)    sdiv
  DIV (unsigned)  udiv
  MOD (signed)    srem
  MOD (unsigned)  urem
  SHL             shl
  SHR (signed)    ashr
  SHR (unsigned)  lshr
  AND             and
  OR              or
  XOR             xor
  EQ              icmp eq
  NE              icmp ne
  LT (signed)     icmp slt
  LT (unsigned)   icmp ult
  LE              icmp sle / ule
  GT              icmp sgt / ugt
  GE              icmp sge / uge
  LOAD            load i64, i64* %addr
  STORE           store i64 %val, i64* %addr
  ALLOCA          %ptr = alloca i64
  BR              br label %target
  BR_IF           br i1 %cond, label %true, label %false
  RET             ret i64 %val
  CALL            %r = call i64 @func(i64 %a, i64 %b)
  LABEL           target:
  CAST            trunc/zext/sext/bitcast
  COPY            (just reuse the source name)
  ```

  After generating the .ll file:
  ```bash
  llc -O2 -march=aarch64 output.ll -o output_arm64.s     # ARM64
  llc -O2 -march=riscv64 output.ll -o output_riscv.s      # RISC-V
  llc -O2 output.ll -o output_x86.s                        # x86-64 with LLVM opts
  ```

  This gives ZCC a path to multi-architecture support without
  writing a single additional backend.


══════════════════════════════════════════════════════════════════════════
  PRIORITY ORDER (maximum impact per effort):
══════════════════════════════════════════════════════════════════════════

  1. Large Array Initializers (CHEATCODE 1)  — kills the 2-file build
  2. C Peephole in part5.c (CHEATCODE 6)    — removes Python dependency
  3. CFG Construction (CHEATCODE 4)          — unlocks everything below
  4. LICM (CHEATCODE 5)                      — hoists i*INPUT_DIM
  5. Register Allocation (CHEATCODE 3)       — eliminates stack traffic
  6. Preprocessor (CHEATCODE 7)              — compiles external code
  7. FPU/SSE (CHEATCODE 2)                   — kills Q16.16 requirement
  8. LLVM IR Backend (CHEATCODE 8)           — multi-arch for free

══════════════════════════════════════════════════════════════════════════
"""


def main():
    print(CHEATCODES)


if __name__ == '__main__':
    main()
"""
]
"""
