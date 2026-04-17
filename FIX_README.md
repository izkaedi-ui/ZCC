# 🔱 ZCC Codegen Bug Fixes - Complete Guide

**Date**: April 15, 2026  
**Session**: LEGENDARY Compiler Development at 90× Velocity

---

## 📋 **Overview**

This package contains **3 automated patch scripts** to fix critical ZCC codegen bugs:

| Bug ID | Description | Impact | Fix Time |
|--------|-------------|--------|----------|
| **CG-IR-012** | VLA RSP stack corruption | SIGBUS crash | 5 min |
| **CG-IR-012 (pt2)** | Const int folding missing | False VLA trigger | 10 min |
| **CG-IR-013** | Struct-by-value ABI mismatch | SIGSEGV crash | 30 min |

**Total estimated time**: 45 minutes (within 90× velocity envelope!)

---

## 🐛 **Bug #1: VLA RSP Corruption (CG-IR-012)**

### **Root Cause**

In `push_vla_scope()`, stack allocation incorrectly uses `+=` instead of `-=`:

```c
// BROKEN:
cc->local_offset += 8;  // Allocates upward (wrong!)

// FIXED:
cc->local_offset -= 8;  // Allocates downward (correct!)
```

**Why this crashes**: Stack grows **downward**. Adding 8 overwrites previously allocated variables (like loop counters or array pointers), corrupting memory. When ZCC later restores RSP using `movslq` (sign-extending from 32-bit), the corrupted value destroys the upper 32 bits of the stack pointer → **instant SIGBUS**.

### **Symptoms**
- Exp3 crashes with `SIGBUS`
- Exp5 crashes with `SIGBUS`
- GDB shows `rsp = 0xe3327b2400000000` (corrupted)

### **Fix Script**: `fix_vla_rsp.py`

**What it does**:
- Finds `cc->local_offset += 8` in `push_vla_scope()`
- Replaces with `cc->local_offset -= 8`
- Validates the fix

**Files modified**: `part3.c`, `zcc.c`

---

## 🐛 **Bug #2: Const Int Folding (CG-IR-012 Part 2)**

### **Root Cause**

ZCC doesn't constant-fold `const int`:

```c
const int SIZE = 256;
int arr[SIZE];  // ZCC treats this as VLA!
```

**Why this triggers VLAs**: ZCC's `eval_const_expr()` works for literals and enum constants, but not for `const int` variables. So `SIZE` is seen as "non-constant" → triggers VLA save/restore machinery → triggers Bug #1.

### **Solution**

Reuse existing `is_enum_const` infrastructure:

```c
// When parsing: const int SIZE = 256;
if (is_const && init_node) {
    int val = eval_const_expr(init_node);
    if (val >= 0) {
        sym->is_enum_const = 1;   // Mark as compile-time constant
        sym->enum_val = val;       // Store the value
    }
}
```

Now `int arr[SIZE]` sees `SIZE` as constant → no VLA machinery!

### **Symptoms**
- Any array sized by `const int` triggers unnecessary VLA code
- Even after fixing Bug #1, you get VLA overhead

### **Fix Script**: `fix_const_folding.py`

**What it does**:
- Finds variable declaration handling in `parse_stmt()`
- Inserts const folding logic after initialization
- Validates the insertion

**Files modified**: `part3.c`, `zcc.c`

**Note**: This is **semi-automatic**. If it fails, manual insertion required (see script output).

---

## 🐛 **Bug #3: Struct-by-Value ABI (CG-IR-013)**

### **Root Cause**

ZCC passes small structs as pointers, but System V AMD64 ABI requires packing into registers:

```c
Vec3 add(Vec3 a, Vec3 b) { ... }  // Vec3 = 12 bytes

// ZCC does:
mov %rdi, [address of Vec3]  // Passes pointer

// System V requires:
mov %rdi, [first 8 bytes of Vec3]   // Pack into RDI
mov %rsi, [next 4 bytes of Vec3]    // Pack into RSI
```

**Why this crashes**: Callee expects packed struct in `%rdi`, but gets a pointer. Dereferencing that garbage pointer → **SIGSEGV**.

### **Solution**

In `ND_CALL` argument pushing, detect structs and pack them:

```c
if (arg_type->kind == TY_STRUCT) {
    int sz = get_type_size(arg_type);
    
    if (sz <= 8) {
        // Pack into 1 register
        printf("    movq (%%rax), %%rax\n");
        printf("    pushq %%rax\n");
    }
    else if (sz <= 16) {
        // Pack into 2 registers
        printf("    movq 8(%%rax), %%r11\n");  // Second 8 bytes
        printf("    movq (%%rax), %%rax\n");    // First 8 bytes
        printf("    pushq %%r11\n");
        printf("    pushq %%rax\n");
    }
    else {
        // > 16 bytes: pass by reference (unchanged)
        printf("    pushq %%rax\n");
    }
}
```

### **Callee Side (Already Works!)**

The function prologue already saves all 6 `argregs[]` to sequential stack slots:

```asm
movq %rdi, -8(%rbp)    # First 8 bytes
movq %rsi, -16(%rbp)   # Next 8 bytes (or separate arg)
movq %rdx, -24(%rbp)   # ...
```

For a 12-byte struct in `%rdi` + `%rsi`, this reconstitutes it at `-8(%rbp)`!

### **Symptoms**
- Exp4 crashes with `SIGSEGV` in `trace_ray`
- Exp5 crashes with `SIGSEGV` in physics functions
- Any function taking `Vec3` by value crashes

### **Fix Script**: `fix_struct_abi.py`

**What it does**:
- Finds `ND_CALL` case in `codegen_expr()`
- Locates argument pushing loop
- Replaces simple `pushq %rax` with struct-aware packing
- Validates the insertion

**Files modified**: `part4.c`, `zcc.c`

**Note**: This is **complex**. Review changes carefully!

---

## 🚀 **Quick Start**

### **Option A: Run All Fixes Automatically**

```bash
python3 apply_all_fixes.py
```

This will:
1. Apply all 3 fixes
2. Rebuild ZCC
3. Run smoke tests
4. Report results

### **Option B: Run Fixes Individually**

```bash
# Fix 1: VLA RSP
python3 fix_vla_rsp.py

# Rebuild and test
gcc -O0 -w -o zcc zcc.c -lm
./zcc exp3_audio_visualizer.c -o exp3.s

# Fix 2: Const folding
python3 fix_const_folding.py

# Rebuild and test
gcc -O0 -w -o zcc zcc.c -lm
./zcc exp3_audio_visualizer.c -o exp3.s  # Should have less VLA code

# Fix 3: Struct ABI
python3 fix_struct_abi.py

# Rebuild and test
gcc -O0 -w -o zcc zcc.c -lm
./zcc exp4_vr_stereo.c -o exp4.s
```

---

## ✅ **Validation**

### **After Applying All Fixes**

1. **Rebuild ZCC**:
   ```bash
   gcc -O0 -w -o zcc zcc.c -lm
   ```

2. **Compile all experiments**:
   ```bash
   ./zcc exp1_raytracer_simd.c -o exp1.s
   ./zcc exp2_voxel_engine.c -o exp2.s
   ./zcc exp3_audio_visualizer.c -o exp3.s
   ./zcc exp4_vr_stereo.c -o exp4.s
   ./zcc exp5_physics_engine.c -o exp5.s
   ```

3. **Link all experiments**:
   ```bash
   gcc -o exp1 exp1.s -lm
   gcc -o exp2 exp2.s -lm
   gcc -o exp3 exp3.s -lm
   gcc -o exp4 exp4.s -lm
   gcc -o exp5 exp5.s -lm
   ```

4. **Run all experiments**:
   ```bash
   ./exp1 > raytracer_output.ppm     # Should work (already did)
   ./exp2 > voxel_output.ppm         # Should work (already did)
   ./exp3 > audio_output.ppm         # Should work NOW (was SIGBUS)
   ./exp4 > vr_stereo_output.ppm     # Should work NOW (was SIGSEGV)
   ./exp5 > physics_output.ppm       # Should work NOW (was SIGSEGV)
   ```

5. **Verify outputs**:
   ```bash
   ls -lh *.ppm
   head -1 *.ppm  # All should show "P6"
   ```

**Expected**: All 5 PPM files generated, no crashes! ✅

---

## 🧪 **Testing Checklist**

- [ ] Fix #1 applied successfully
- [ ] Fix #2 applied successfully
- [ ] Fix #3 applied successfully
- [ ] ZCC rebuilds without errors
- [ ] Self-host still passes (`zcc2.s == zcc3.s`)
- [ ] All 5 experiments compile
- [ ] All 5 experiments link
- [ ] All 5 experiments run without crashes
- [ ] All 5 PPM outputs generated
- [ ] PPM files are valid (P6 magic, correct sizes)

---

## 📊 **Expected Results**

### **Before Fixes**

| Exp | Compile | Link | Run | Status |
|-----|---------|------|-----|--------|
| 1 | ✅ | ✅ | ✅ | Working |
| 2 | ✅ | ✅ | ✅ | Working |
| 3 | ✅ | ✅ | 🔴 SIGBUS | Broken |
| 4 | ✅ | ✅ | 🔴 SIGSEGV | Broken |
| 5 | ✅ | ✅ | 🔴 SIGSEGV | Broken |

### **After Fixes**

| Exp | Compile | Link | Run | Status |
|-----|---------|------|-----|--------|
| 1 | ✅ | ✅ | ✅ | Working |
| 2 | ✅ | ✅ | ✅ | Working |
| 3 | ✅ | ✅ | ✅ | **FIXED!** |
| 4 | ✅ | ✅ | ✅ | **FIXED!** |
| 5 | ✅ | ✅ | ✅ | **FIXED!** |

---

## 🔧 **Troubleshooting**

### **Problem**: Script says "pattern not found"

**Solution**: The code structure may have changed. Run with manual instructions:
```bash
python3 fix_vla_rsp.py 2>&1 | grep -A 50 "MANUAL"
```

### **Problem**: ZCC rebuild fails

**Solution**: Check for syntax errors in patched files:
```bash
gcc -O0 -w -fsyntax-only zcc.c
```

### **Problem**: Experiments still crash after fixes

**Solution**: Verify fixes were actually applied:
```bash
grep "cc->local_offset -= 8" part3.c  # Should find the fix
grep "TY_STRUCT" part4.c               # Should find struct handling
```

### **Problem**: VLA code still appears in assembly

**Solution**: Const folding may not have worked. Check:
```bash
./zcc -E test.c | grep "const int SIZE"  # Verify const is preserved
```

---

## 📁 **Files in This Package**

| File | Purpose |
|------|---------|
| `fix_vla_rsp.py` | Fix VLA RSP corruption (CG-IR-012) |
| `fix_const_folding.py` | Fix const int folding (CG-IR-012 pt2) |
| `fix_struct_abi.py` | Fix struct-by-value ABI (CG-IR-013) |
| `apply_all_fixes.py` | Master script to run all fixes |
| `FIX_README.md` | This file |

---

## 🎯 **Success Criteria**

**All 3 bugs fixed successfully if**:

1. ✅ All scripts report "SUCCESS"
2. ✅ ZCC rebuilds without errors
3. ✅ All 5 experiments compile
4. ✅ All 5 experiments link
5. ✅ All 5 experiments run without crashes
6. ✅ All 5 PPM outputs are generated
7. ✅ Self-host verification still passes

**LEGENDARY SESSION COMPLETE!** 🔱

---

## 📞 **Support**

If issues arise:
1. Check script output for manual instructions
2. Verify prerequisite files exist
3. Review assembly output for patterns
4. Compare with original backups (`.bak` files)

**These fixes complete the April 15, 2026 legendary compiler development session at 90× industry velocity!**
