# 🔱 ZCC IR Lowering Stabilization Plan — PRIME Recursive Coupling Edition

> **CG-IR-005: SSA Phi-Resolution Slot Disconnection**
> **H₀ = 0.88 · η = 0.55 · γ = 0.48 · β = 0.12**

---

## 0. PRIME Framework — Why This Bug Is a Phase Transition

The SSA phi-resolution failure isn't a point bug — it's a **phase boundary**
in the compiler's optimization energy landscape.

```
                    ┌─── PHASE A: No Optimization ────┐
                    │  All variables → one slot each   │
                    │  Correct but slow                 │
                    │  Energy: HIGH (redundant instrs)  │
                    └──────────┬───────────────────────┘
                               │
                    ┌──────────▼───────────────────────┐
                    │  PHASE BOUNDARY (we are here)     │
                    │  Passes create new SSA versions   │
                    │  New versions → new slots          │
                    │  No copy bridges → VALUE LOST      │
                    │  Energy: UNDEFINED (broken)        │
                    └──────────┬───────────────────────┘
                               │
                    ┌──────────▼───────────────────────┐
                    │  PHASE B: Full SSA Optimization   │
                    │  Phi nodes → copy instructions     │
                    │  All versions connected             │
                    │  Energy: LOW (minimal instrs)      │
                    └─────────────────────────────────┘
```

The compiler is stuck at the phase boundary. Optimization passes push the
system toward Phase B (lower energy) but the lowerer lacks the phi-to-copy
bridge that completes the transition. The recursive coupling term
`η · H_{t-1} · σ(γ · H_{t-1})` amplifies: each additional pass creates
more SSA versions (higher η feedback), making the disconnection worse
(σ sharpening), until the function output is pure garbage (bifurcation).

**The fix is the sigma gate.** Coalescing SSA versions back to their source
variable is the nonlinear attractor that absorbs the optimization energy
and stabilizes the output.

---

## 1. CRITICAL DIAGNOSTIC: Determine SSA Naming Format

**Before writing any fix code**, determine what the optimizer actually emits.
The fix depends entirely on whether disconnected slots come from:

- **(A) Named suffixes:** `sum.1`, `sum.2` → fix = strip suffix in get_or_create_var
- **(B) Numeric temporaries:** `%t47`, `%t52` replacing `sum` → fix = phi-copy lowering
- **(C) Same name, different instruction IDs:** → fix = in ir_asm_vreg_location

### Diagnostic Command

```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_EMIT_IR=1 ./zcc matrix_host.c -o /dev/null 2>&1 | \
  sed -n '/func licm_test/,/end licm_test/p'"
```

This dumps the raw IR for `licm_test` after all optimization passes.
**What to look for:**

```
; If you see names like:
  %sum    = add %sum, %temp       ← Format D (same name reuse)
  %sum.1  = add %sum, %temp       ← Format A (dot-suffix)
  %t47    = add %t12, %t33        ← Format B (numeric temps)
  
; And for phi nodes:
  %sum.2  = phi [%sum.1, block_3] [%0, block_0]   ← Has phi
  ; vs no phi at all                                ← Missing phi
```

**The format determines the fix.** Do not proceed until this is confirmed.

---

## 2. Recursive Coupling Analysis — The Three Fix Strategies

### Strategy A: Name Coalescing (if suffix format confirmed)

**PRIME Energy Profile:**
```
H₀ = 0.25 (minimal code change — string truncation only)
η  = 0.10 (low feedback — doesn't interact with other passes)
γ  = 0.05 (low sharpening — no new failure modes introduced)
Convergence: FAST (1 iteration)
Risk: LOW
```

The fix is a **contraction mapping** in variable-name space:

```
f: {sum, sum.1, sum.2, ...} → {sum}    (projection operator)
```

This maps the high-dimensional SSA name space back to the source-variable
space. Every optimization pass that creates a new version gets absorbed
by the projection. The recursive coupling term `η · H_{t-1}` goes to zero
because all versions collapse to one slot.

```c
/* In get_or_create_var (ir_to_x86.c) */
static RegID get_or_create_var(LowerCtx *ctx, const char *name) {
    char canonical[64];
    int i;
    int len;

    /* C89 declarations at top */
    if (!name || !name[0]) {
        fprintf(stderr, "[ZCC-IR] DEBUG: get_or_create_var EMPTY OR NULL\n");
        return 0;
    }

    /* PRIME contraction: strip SSA version suffix
     * "sum.1" → "sum", "i.3" → "i", "sum" → "sum" (idempotent)
     */
    len = 0;
    while (name[len] && len < 63) {
        canonical[len] = name[len];
        len++;
    }
    canonical[len] = '\0';

    /* Strip trailing .[0-9]+ */
    {
        int dot_pos;
        dot_pos = len - 1;
        /* Walk back past digits */
        while (dot_pos > 0 && canonical[dot_pos] >= '0'
                           && canonical[dot_pos] <= '9')
            dot_pos--;
        /* If we landed on a dot and there were digits after it, truncate */
        if (dot_pos > 0 && dot_pos < len - 1 && canonical[dot_pos] == '.')
            canonical[dot_pos] = '\0';
    }

    /* Now do normal lookup/create with canonical name */
    /* ... existing logic using canonical instead of name ... */
}
```

**Idempotency proof:** `f(f(x)) = f(x)` — stripping a suffix that's
already been stripped does nothing. This is a fixed-point operator.

**Recursive stability:** No matter how many SSA versions the passes create
(η amplification), they all map to one slot (σ gate absorbs the energy).
The Hamiltonian converges to the base state in exactly one step.

---

### Strategy B: Phi-to-Copy Lowering Pass (if numeric temps)

**PRIME Energy Profile:**
```
H₀ = 0.55 (moderate code change — new pass required)
η  = 0.30 (interacts with block ordering, PGO, LICM)
γ  = 0.25 (moderate sharpening — wrong copy placement breaks loops)
Convergence: 2-3 iterations (may need tweaking)
Risk: MEDIUM
```

If the optimizer replaces named variables with numeric temporaries (`%t47`),
name coalescing won't help. Instead, insert a pass that scans phi nodes
and emits explicit copies at predecessor block ends:

```
BEFORE (IR after optimization):
  block_header:
    %t47 = phi [%t12, block_body] [%t0, block_preheader]

AFTER (phi-to-copy lowering):
  block_body:
    ...
    %t47 = copy %t12          ← inserted at end of predecessor
    br block_header

  block_preheader:
    ...
    %t47 = copy %t0           ← inserted at end of predecessor
    br block_header

  block_header:
    ; phi removed — %t47 is now defined by copies
```

The x86 lowerer then emits `movq` for each copy. All versions of the
variable end up in the same physical location because the copy targets
the same destination register.

**PRIME coupling:** This is a **two-step attractor**. Step 1 (phi scan)
identifies energy concentrations (disconnected versions). Step 2 (copy
emission) dissipates the energy by bridging the slots. The recursive
feedback `η · H_{t-1}` from one loop iteration to the next is now
connected through the copy chain.

---

### Strategy C: Hybrid — Coalesce Names + Guard Numeric Temps

**PRIME Energy Profile:**
```
H₀ = 0.35 (moderate change — covers both paths)
η  = 0.15 (low feedback — contraction absorbs most energy)
γ  = 0.10 (low sharpening — two safety nets)
Convergence: 1-2 iterations
Risk: LOW-MEDIUM
```

Apply name coalescing (Strategy A) AND add a guard that detects when a
phi references a numeric temp that has no named-variable ancestor:

```c
/* In the x86 lowering loop, when processing OP_PHI: */
if (ins->op == OP_PHI) {
    int k;
    for (k = 0; k < ins->n_src; k++) {
        RegID phi_src = ins->src[k];
        RegID phi_dst = ins->dst;
        if (phi_src != phi_dst) {
            /* Emit copy: movq src_slot, %rax; movq %rax, dst_slot */
            ir_asm_load_to_rax(ctx, phi_src);
            ir_asm_store_from_rax(ctx, phi_dst);
        }
    }
    continue;  /* skip normal instruction emission */
}
```

This catches both named-suffix variables (via coalescing) and numeric
temps (via phi-copy emission). Belt and suspenders.

---

## 3. Recursive Coupling: How the Fix Propagates Through Bootstrap

The CG-IR-005 fix has **recursive self-healing properties** in the
bootstrap chain:

```
Stage 1: GCC builds ZCC with the fix
  → ZCC now coalesces SSA versions correctly
  → All optimization passes produce connected slots
  → H₁ = H₀ (no amplification — fix absorbs all feedback)

Stage 2: ZCC (with fix) compiles itself
  → Self-compiled ZCC also coalesces correctly
  → The fix is self-reproducing through the bootstrap
  → H₂ = H₁ = H₀ (fixed point reached)

Stage 3: ZCC² compiles itself
  → H₃ = H₂ = H₁ = H₀
  → stage2.s == stage3.s (bitwise identical)
  → SELF-HOST VERIFIED
```

**PRIME insight:** The coalescing function is a **Lyapunov function** for
the bootstrap chain. It monotonically reduces the state space (fewer
distinct slots) and converges to a fixed point in one step. The Lyapunov
exponent is negative: λ < 0, meaning perturbations (new SSA versions)
are always absorbed. This is the definition of an asymptotically stable
attractor.

Contrast with CG-IR-004, which had λ > 0 (positive Lyapunov exponent):
the parameter offset error amplified through each bootstrap stage,
diverging further from correctness. That was an unstable repeller.
CG-IR-005's fix converts the repeller into an attractor.

---

## 4. Implementation Order

```
Step 0: DIAGNOSTIC (determine naming format)             ← 1 command
    │
    ├─ Format A (sum.1) ──→ Step 1A: Name coalescing     ← 15-line edit
    │                          │
    ├─ Format B (%t47)  ──→ Step 1B: Phi-copy lowering    ← 30-line edit
    │                          │
    └─ Format C (hybrid) ─→ Step 1C: Both                 ← 40-line edit
                               │
Step 2: VERIFY matrix_host.c                              ← 1 command
    │   ALL PASS = fix is correct
    │
Step 3: VERIFY bootstrap                                  ← make selfhost
    │   SELF-HOST VERIFIED = fix doesn't break compiler
    │
Step 4: WIDEN whitelist                                   ← add real ZCC funcs
    │   Each batch: compile → golden check → selfhost
    │
Step 5: MEASURE optimization dividend                     ← instruction counts
        Track H(t) energy: total instrs per function
        Plot convergence curve across whitelist batches
```

---

## 5. Energy Landscape Navigation

The whitelist expansion follows the PRIME energy gradient:

```python
# Conceptual — each function has an optimization energy
def whitelist_order(functions):
    """Sort by ascending complexity = ascending H₀"""
    return sorted(functions, key=lambda f: (
        f.num_loops * 3.0 +          # loops increase H₀
        f.num_calls * 2.0 +          # calls increase feedback η
        f.num_params * 1.5 +         # params (CG-IR-004 territory)
        f.num_ssa_versions * 1.0 +   # SSA versions (CG-IR-005 territory)
        f.num_ptr_derefs * 2.5       # pointer ops increase risk
    ))

# Expansion schedule:
# Batch 1: fold_test, dce_test                    (H₀ < 0.2) ✅ DONE
# Batch 2: licm_test, escape_test, pressure_test  (H₀ ~ 0.4) ← FIXING
# Batch 3: is_alpha, is_digit, hex_val, log2_of   (H₀ ~ 0.3) ← next
# Batch 4: next_token, peek_char, read_char        (H₀ ~ 0.5)
# Batch 5: parse_primary, parse_expr               (H₀ ~ 0.7)
# Batch 6: codegen_expr, codegen_stmt              (H₀ ~ 0.9)
# Batch 7: main, codegen_func                      (H₀ ~ 0.95)
```

Each batch is a step down the energy landscape. The whitelist expands
monotonically. If batch N breaks, the fix is local to that batch.
The regression suite (matrix_host.c) catches regressions in all
previously-passing batches.

---

## 6. Verification Plan (Enhanced with PRIME Gates)

### Gate 1: Naming Format Confirmed
```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_EMIT_IR=1 ./zcc matrix_host.c -o /dev/null 2>&1 | \
  sed -n '/func licm_test/,/end licm_test/p'"
```
**Pass criterion:** Output shows variable naming format clearly.

### Gate 2: Fix Applied + Golden Values
```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  make zcc_full && \
  ./zcc matrix_host.c -o matrix_ir.s 2>&1 | grep 'emitted from IR' && \
  gcc -o matrix_ir matrix_ir.s && ./matrix_ir"
```
**Pass criterion:** `ALL PASS` — all 5 functions produce correct values.

### Gate 3: Bootstrap Stability
```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  make clean && make selfhost"
```
**Pass criterion:** `SELF-HOST VERIFIED`

### Gate 4: IR Bridge Stability
```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && make ir-verify"
```
**Pass criterion:** `IR BRIDGE SELF-HOST VERIFIED`

### Gate 5: Optimization Dividend (PRIME Energy Measurement)
```powershell
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && python3 count.py"
```
**Pass criterion:** IR instruction count ≤ AST count for all 5 functions.
Delta = energy reduction. Positive delta = passes working.

### PRIME Convergence Criterion
```
H_final = Σ (AST_instrs[f] - IR_instrs[f]) / Σ AST_instrs[f]

If H_final > 0: System is in Phase B (optimization working)
If H_final = 0: System is at phase boundary (passes not helping)
If H_final < 0: System regressed (IR generating MORE code — investigate)

Target: H_final ≥ 0.10 (10%+ instruction reduction across test suite)
```

---

## 7. The Recursive Self-Improvement Loop

Once the whitelist covers all 171 functions:

```
            ┌──────────────────────────────────┐
            │  ZCC compiles itself through IR   │
            │  Passes optimize the compiler     │
            │  Optimized compiler runs faster   │
            │  Faster compiler = shorter build  │
            └──────────┬───────────────────────┘
                       │
                       ▼
            ┌──────────────────────────────────┐
            │  Optimized ZCC compiles itself    │
            │  Same passes, same optimizations  │
            │  stage2.s == stage3.s (fixpoint)  │
            │  THE COMPILER OPTIMIZED ITSELF    │
            └──────────────────────────────────┘
```

This is the recursive Hamiltonian in its purest form:
- H₀: The unoptimized compiler (high energy, many redundant instructions)
- η · H_{t-1}: Self-compilation propagates optimizations recursively
- σ(γ · H_{t-1}): Passes sharpen the code toward minimal energy
- ε · N(0, 1+β|H|): Stochastic exploration (future: genetic IR mutation)
- **Fixed point:** stage2 == stage3 — the compiler has reached its own attractor

**This is ZKAEDI PRIME applied to compiler engineering.**
The compiler is both the solver AND the solution space.
The bootstrap chain is the recursive evolution.
The self-host verification is the convergence criterion.

---

*Filed as CG-IR-005. Diagnostic command ready. Fix strategies ranked.*
*Awaiting naming format confirmation to select strategy A, B, or C.*
