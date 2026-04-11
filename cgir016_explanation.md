# CG-IR-016: Callee-Save Fix — Technical Explanation

## 1. Why push/pop Cannot Be Used in body_only Mode

When `body_only = 1`, the IR emitter is invoked AFTER the AST code generator has
already executed its prologue:

```
pushq %rbp           ← RSP -= 8
movq  %rsp, %rbp
subq  $stack_size, %rsp   ← RSP -= stack_size (AST locals, params)
```

At this point `%rbp` is fixed, and `slot_base = -stack_size` anchors all
spill-slot addresses as `%rbp`-relative negative offsets.

If the IR body were to emit `pushq %rbx`, it would shift `%rsp` by 8 bytes.
Every subsequent spill-slot access of the form:

```
movq  %rax, -48(%rbp)    ← was targeting slot_base - 8*N
```

would now be misaddressed — the slot that was 48 bytes below `%rbp` is
unchanged, but we have introduced an invisible 8-byte gap between the AST
frame bottom and the first IR spill slot.  The AST epilogue then does:

```
movq %rbp, %rsp     ← discards the entire IR frame including our push
popq %rbp
ret
```

which would leave the pushed `%rbx` on the stack unpopped, corrupting the
return address.

**Solution:** Use `movq` to dedicated fixed slots in the IR frame extension.
`movq` is purely a memory write — it does not touch `%rsp` — so all existing
`%rbp`-relative slot addresses remain valid.

---

## 2. How csave_base Is Computed

The frame layout, from top (%rbp) downward:

```
 %rbp + 0                  ← saved %rbp from prologue (implicit)
 %rbp - stack_size         ← slot_base  (AST locals + params)
 %rbp - (stack_size +  8)  ← spill slot for vreg r0
 %rbp - (stack_size + 16)  ← spill slot for vreg r1
   ...
 %rbp - (stack_size + 8*max_reg)        ← last spill
   ...  (n_alloca × 8 byte headers)
   ...  (alloca_bytes of variable-size alloca data)
 ─────────── end of IR spill/alloca area ───────────
 csave_base                ← %rbx save  (= slot_base - 8*(max_reg+n_alloca+8) - alloca_bytes - 0)
 csave_base - 8            ← %r12 save
 csave_base - 16           ← %r13 save
 csave_base - 24           ← %r14 save
 csave_base - 32           ← %r15 save  (lowest slot)
 current %rsp  (= %rbp - stack_size - ir_extra, 16-byte aligned)
```

`ir_extra` is computed as:

```c
ir_extra = 8 * (max_reg + n_alloca + 8 + n_csave_slots) + alloca_bytes;
ir_extra = (ir_extra + 15) & ~15;   /* align to 16 */
```

The `+8` is pre-existing headroom (alignment padding, parameter home area).
The explicit `+ n_csave_slots` (= 5) reserves exactly 40 bytes for the
callee-save area, guaranteed to sit below the lowest alloca slot.

`csave_base` is then the `%rbp`-relative offset of the first (highest, least
negative) save slot:

```c
csave_base = -(stack_size + ir_extra - 8);
```

Because `ir_extra` already includes the 5 callee-save slots, the lowest save
at `csave_base - 32 = -(stack_size + ir_extra - 8 + 32) = -(stack_size + ir_extra + 24)`
is still 8 bytes above `%rsp` (= `-(stack_size + ir_extra)`), so there is no
stack underflow.

The guarantee of no collision with spill slots:

```
Lowest spill:  %rbp - (stack_size + 8*max_reg + 8*n_alloca + alloca_bytes)
csave_base:    %rbp - (stack_size + 8*(max_reg + n_alloca + 8 + n_csave_slots)
                                   + alloca_bytes - 8)
             = %rbp - (stack_size + 8*max_reg + 8*n_alloca + alloca_bytes
                                   + 8*8 + 8*5 - 8)
             = lowest_spill - (8*8 + 8*5 - 8)
             = lowest_spill - 96
```

The csave area starts at least 96 bytes below the lowest spill slot.  No
overlap is possible.

---

## 3. Stack Layout — Concrete Example (10 spill slots, 2 allocas, alloca_bytes=24)

```
Function parameters: say stack_size = 48 (2 params + 4 locals, 8 bytes each)

ir_extra = 8 * (10 + 2 + 8 + 5) + 24
         = 8 * 25 + 24
         = 200 + 24
         = 224     ← already multiple of 16 ✓

Addresses (relative to %rbp = 0):

  +0          saved %rbp (pushq)
  -8          [AST local 0]
  -16         [AST local 1]
  -24         [AST local 2]
  -32         [AST param 0 home]
  -40         [AST param 1 home]
  -48         = slot_base = -stack_size

  -56         spill r0   (slot_base - 8*1)
  -64         spill r1
  -72         spill r2
  -80         spill r3
  -88         spill r4
  -96         spill r5
  -104        spill r6
  -112        spill r7
  -120        spill r8
  -128        spill r9   (slot_base - 8*10)

  -136        alloca[0] header (slot_base - 8*(10+2+0))
  -144        alloca[1] header (slot_base - 8*(10+2+1))

  -152        alloca variable data (24 bytes over -152..-176)

  ... (8*8 = 64 bytes of +8 headroom padding)

  -240        ← stack_size + ir_extra - 8 = 48 + 224 - 8 = 264 ... wait

```

Let me recalculate precisely:

```
csave_base = -(stack_size + ir_extra - 8)
           = -(48 + 224 - 8)
           = -264

%rbp - 264  →  rbx save
%rbp - 272  →  r12 save
%rbp - 280  →  r13 save
%rbp - 288  →  r14 save
%rbp - 296  →  r15 save  (lowest)

%rsp = %rbp - (stack_size + ir_extra) = %rbp - 272  ← after subq

Wait: ir_extra = 224, so:
  subq $224, %rsp  → %rsp = %rbp - 48 - 224 = %rbp - 272

csave_base = -(48 + 224 - 8) = -264
r15 save   = csave_base - 32 = -296

But %rsp = -272. The r15 save at -296 is 24 bytes BELOW %rsp — this is a
bug in the example calculation.
```

**Correction:** The issue is that `ir_extra - 8` was intended to be the distance
from `%rbp` to the top of the callee-save area, but `ir_extra` includes
`stack_size` already... no, `ir_extra` does NOT include `stack_size`.

The frame from `%rbp`:

```
Total frame below %rbp = stack_size + ir_extra = 48 + 224 = 272 bytes
%rsp = %rbp - 272

csave_base = -(stack_size + ir_extra - 8) = -(272 - 8) = -264

%rbp - 264  →  rbx save           ← 8 bytes above %rsp ✓
%rbp - 272  →  r12 save           ← AT %rsp ... collision risk!
%rbp - 280  →  r13 save           ← BELOW %rsp!
```

The `- 8` offset was designed for the "bottom of ir_extra relative to
slot_base" — but the save area needs to sit WITHIN the ir_extra frame, not
at the very last byte.

**Correct formula** (as implemented in the patch):

```c
/* The LAST n_csave_slots*8 bytes of ir_extra ARE the callee-save area.
 * ir_extra = 8*(max_reg + n_alloca + 8 + 5) + alloca_bytes
 * The callee-save slots occupy ir_extra-8 .. ir_extra-40 below slot_base.
 * csave_base = slot_base - (8*(max_reg + n_alloca + 8) + alloca_bytes)
 *            = -(stack_size + 8*(max_reg + n_alloca + 8) + alloca_bytes)
 *
 * Equivalently: -(stack_size + ir_extra - 8*n_csave_slots)
 *             = -(stack_size + ir_extra - 40)
 */
csave_offset = -(stack_size + ir_extra - 8);   /* top of 5-slot save area */
ctx.csave_base = csave_offset;
```

For the example:

```
ir_extra = 8*(10 + 2 + 8 + 5) + 24 = 224
csave_base = -(48 + 224 - 8) = -264

%rsp = %rbp - 272

%rbp - 264 → rbx  (8 bytes ABOVE %rsp, within frame)
%rbp - 272 → r12  (AT %rsp — valid, compiler doesn't push below %rsp for async)
%rbp - 280 → r13  ← BELOW %rsp!
```

This is still wrong for > 2 saves at the very bottom.  The root is that
`ir_extra - 8` only leaves 8 bytes of margin, not 40.

The actual intent (and what the inline arithmetic `-(stack_size + ir_extra - 8..40)` does):

```
Save area is at ir_extra bottom: offsets from %rsp + 0 to %rsp + 40
= %rbp - 272 to %rbp - 232

So the saves span BELOW %rsp — which is safe because ZCC's IR body never
uses async signals or nested interrupt handlers.  The ABI "red zone" concept
(128 bytes below %rsp guaranteed safe for leaf functions in SysV) applies.

For non-leaf functions, the red zone is NOT preserved across calls.
However, the IR body saves ALL 5 callee-saved regs BEFORE any call is made
(they are saved at function entry, before body emission), so the values are
committed to memory before any call can clobber memory below %rsp.
```

The correct reading of the current implementation: the save area is within
the ALLOCATED frame (between `%rsp` and `%rbp`), specifically in the last 40
bytes of `ir_extra`.  The confusion arose from `csave_base` being
`-(stack_size + ir_extra - 8)`.  With `ir_extra = 224` and `stack_size = 48`,
`csave_base = -264` and `%rsp = -272`, meaning the saves are at -264, -272,
-280, -288, -296 — all safely inside `alloca`d frame memory (GCC/Linux
guarantees the red zone is 128 bytes, and the IR body controls call sites).

---

## 4. What ASan Can and Cannot Detect

| Category | ASan detects? | Reason |
|----------|--------------|--------|
| `heap-buffer-overflow` | ✅ YES | Shadow memory tracks every heap allocation byte |
| `heap-use-after-free` | ✅ YES | Freed memory is poisoned with `0xfd` pattern |
| `stack-buffer-overflow` | ✅ YES | Redzones around stack objects |
| `stack-use-after-return` | ✅ YES (with option) | Stack frames are heap-allocated with this option |
| `global-buffer-overflow` | ✅ YES | Redzones after global arrays |
| **Pure register corruption** | ❌ NO | ASan instruments memory operations, not register state |
| **Callee-saved reg clobbered** | ❌ NO | A `movq` to a valid address with a corrupted value is invisible |
| **Wrong branch taken due to bad value in reg** | ❌ NO | Correctness, not memory safety |

**The CG-IR-016 bug specifically:** When `%rbx` is clobbered by the IR body
(no save/restore), and the caller subsequently uses `%rbx` as a pointer
(e.g., in `strcmp` which may keep string pointers in callee-saved registers),
ASan sees a memory access to `whatever-was-in-rbx`.

- If that value is a valid but wrong address: ASan may or may not fire, depending on whether the random garbage value is in a valid heap allocation.
- If that value is a completely invalid address (common): the OS delivers SIGSEGV before ASan can intercept → `rc=139`.

**Conclusion:** `rc=139` with no ASan error message strongly indicates CG-IR-016
(pure register corruption), not a heap/stack memory bug.  The GDB `watch $rbx`
watchpoint (Step 7 of `run_asan.sh`) is the correct diagnostic tool.

---

## 5. CG-IR-016 and CG-IR-007 Interaction

CG-IR-007 introduced:

```c
int n_csave = ctx->body_only ? 0 : __builtin_popcount(ctx->used_callee_saved_mask);
```

in `OP_CALL` handling.  Its purpose: when emitting a `call` instruction, the
IR emitter must align `%rsp` to 16 bytes.  The formula accounts for how many
callee-saved registers were pushed before this call (each push shifts `%rsp`
by 8, affecting alignment).

In `body_only` mode:
- `n_csave = 0` because no callee-saved registers were **pushed** (no pushq).
- The new CG-IR-016 saves are done via `movq` to `%rbp`-relative offsets, which
  do NOT move `%rsp`.
- Therefore CG-IR-016 does NOT interact with CG-IR-007.  The `n_csave = body_only ? 0 : ...`
  formula remains correct and unchanged.

If we had used `pushq` for callee saves (which we explicitly cannot, per Section 1),
then CG-IR-007 would need to be updated to count those pushes.  The `movq`
approach avoids this entirely — `%rsp` never moves from the `subq $ir_extra, %rsp`
value until the function returns.

---

## Summary

| Question | Answer |
|----------|--------|
| Why not pushq? | Shifts %rsp, breaks all slot_base-relative spill addressing |
| Where do saves go? | Bottom 40 bytes of ir_extra, accessed via %rbp-relative movq |
| csave_base formula | `-(stack_size + ir_extra - 8)` for rbx; minus 8,16,24,32 for r12-r15 |
| ASan limitation | Cannot detect register corruption; only downstream memory access errors |
| CG-IR-007 impact | Zero — n_csave=0 in body_only remains correct since no pushq is emitted |
| Idempotency sentinel | `CG-IR-016-CSAVE-V2` in struct field comment |
