# ZCC IR vs CWE Vulnerabilities: Baseline Tractability Analysis

This document serves as the ground-truth foundation for the proposed ML-driven IR security taint pipeline. Before engineering tensor schemas, we must assess whether our Intermediate Representation mathematically preserves enough semantic differentiation to distinguish a vulnerable code path from a secure one.

## 1. Tractability Breakdown by CWE Class

### CWE-476 (NULL Pointer Dereference)
* **Visiblity: HIGH**
* **Observed RAW IR Signature:** Verified empirically via Juliet `CWE476__char_04.c` compiled through ZCC (dumped using `ZCC_EMIT_IR=1` and `ZCC_IR_FLUSH`). The vulnerable variant exhibits a linear sequence where `load ptr %t7` flows immediately into `add ptr %t9` and then `load i8 %t10 %t9` without condition. The secure variant injects a structural `NULL` comparison topology directly into the IR:
  ```ir
  load        ptr     %t9  %stack_-8
  const       i32     %t10 -  -  -  imm=0
  cast        ptr     %t12 %t11
  ne          i32     %t13 %t9  %t12
  br_if       -       -    %t13 -   .L117
  ```
  This proves the branch preserves semantic type boundaries before lowering to x86 assembly.
* **ML Tractability:** Excellent. The presence or absence of the `%t13 -> br_if` check block immediately preceding a dereference node mapped to the same `%stack_-8` pointer alias is a tightly clustered, highly deterministic local pattern that classical ML or shallow transformers can easily detect.

### CWE-416 (Use After Free)
* **Visibility: SCALAR (MEDIUM intra-function, INVISIBLE cross-function)**
* **IR Signature:** `CALL` nodes invoking `free` followed by `IR_LOAD`/`IR_STORE` on the same alias.
* **ML Tractability:** Strictly limited to single-function scopes for v0.1. ZCC's IR is generated functionally; it has no LLVM-style Whole Program / Link-Time representations. Extracting alias chains that span call boundaries is natively impossible without a compiler re-architecture. Interprocedural CWE-416 is explicitly quarantined.

### CWE-121 / CWE-787 / CWE-125 (Stack Buffer Overflow / OOB Write / OOB Read)
* **Visibility: MEDIUM**
* **Observed RAW IR Signature:** Confirmed empirically against `CWE121_Stack_Based_Buffer_Overflow__CWE805_char_alloca_loop_03.c`. The raw IR reveals that both allocation size and loop bounds survive as correctly typed constants (`imm=50`, `imm=100`) rather than being flattened:
  ```ir
  ; Buffer Allocation:
  const       i32     %t0  -  -  -  imm=50
  call        i32     %t3  -  -  ALLOCA
  ...
  ; Loop Boundary:
  const       i32     %t30 -  -  -  imm=100
  lt          i32     %t32 %t29 %t31
  br_if       -       -    %t32 -  .L107
  ```
  Although the size bounds are strictly preserved in IR, they are anatomically disjoint (separated by 15+ instructions and independent temp registers). 
* **ML Tractability:** Requires targeted Data Flow Analysis via classical walkers. A token model cannot bind `imm=50` to `ALLOCA` lengths while mapping `imm=100` to the loop `lt` bound. The extractor must answer "what numeric expressions feed the ALLOCA length and what feed the nested comparison bound" to produce a simplified tuple feature `(alloca_length, logic_bound)`. A GNN escalation is only required when the variables are symbolic parameters (e.g. `n`), falling back to linear analysis for constants.

### CWE-20 (Improper Input Validation)
* **Visibility: INVISIBLE (SEMANTIC)**
* **IR Signature:** Missing logical barriers. No specific architectural IR pattern designates "unsanitized URL."
* **ML Tractability:** Zero from isolated IR tokens. Requires immense AST / Source-level text awareness relative to application boundaries (detecting HTTP parsing vs local parsing). Do not feed CWE-20 purely to an IR model.

---

## 2. ZCC Semantic Survival Matrix (Extractor Requirements)
Does ZCC's IR retain enough semantic structure to differentiate bad from good variants natively? This table establishes exactly what structural evidence survives the compile pipeline into the IR matrix, outlining hard requirements for the classical extractor routines.

| CWE Target | Lost During IR Lowering | Preserved in ZCC IR | Extractor Implication |
| :--- | :--- | :--- | :--- |
| **CWE-476 (NULL Deref)** | Exact pointer variable names | `load` flows branching into specific `ne` to `NULL` type evaluation nodes prior to dereference. | Hard requirement: Must walk Alias Def-Use chains up from the dereference node to discover a dominant check. |
| **CWE-190 (Int Overflow)** | C Source text operations | Signedness conversion paths (`IR_CAST` sequences), operand bit-width, explicit standard `carry/fault` wraps. | Hard requirement: Must isolate width-extensions `i32 -> u64` or bound tests directly preceding math evaluations. |
| **CWE-121 (Stack OOB)** | Immediate sequential proximity between definition and array loop operations. | Absolute immediate bounds inside the `ALLOCA` operands, relative bounds natively encoded inside branch compares. | Hard requirement: Classical Data Flow tracing to output a `[alloca_size, loop_bound_size]` scalar pair per localized array pattern. |
| **CWE-416 (Use-After-Free)** | Whole-program structural trace; cross-call parameters. | Intra-function immediate linear sequences (`call free` followed by `load/store` evaluating the exact stack pointer alias). | Hard requirement: Reject complex cross-file evaluations in v0.1. Tokenize immediate sequential paths using local alias tables. |

## 3. Top-Priority Target Vectors (Sprint v0.1 Scope)
The Sprint 1 Extractor topology focuses on explicitly isolated logic via per-CWE independent feature vectors.

| Priority | Target CWE | Extraction Path | v0.1 Estimated Features Needed |
| :---: | :--- | :--- | :--- |
| 1 | **CWE-476** (Null Deref) | `extract_476_null_check.py` | **4 Features** (`load-ptr`, `imm=0`, `ne`, `br_if` relative distance presence + alias lock). |
| 2 | **CWE-190** (Int Overflow) | `extract_190_overflow.py` | **3 Features** (cast sequence, signedness width, branch wrap status). |
| 3 | **CWE-121** (OOB Stack) | `extract_121_bounds.py` | **2 Features** (`alloca` constant bound, logic `cmp` bound derived via direct classical data-flow). |
| 4 | **CWE-416** (Use-After-Free) | `extract_415_double_free.py` | **2 Features** (`call free` node presence, subsequent alias `load/store` distance mapping). |

## 4. Extraction Engine Directives
We execute Option 1 ("Per-CWE feature vectors") for immediate v0.1 scaling:
1. One global `ir_parse.py` AST translator mapping the textual list back into a Python dependency tree.
2. Four localized `extract_*.py` routines isolating specifically designated feature arrays for classic classifiers.
3. Mechanical threshold tests checking `(vocalize > 180 / 200 pairs)` locally before ML parameter commitments.
