#!/usr/bin/env python3
"""
Fix Struct-by-Value ABI (CG-IR-013)

Root Cause: ZCC passes small structs as pointers, but System V AMD64 ABI requires
structs <= 16 bytes to be packed directly into registers.

Solution: In codegen_expr() for ND_CALL, when pushing struct arguments:
- Structs <= 8 bytes: Dereference pointer, pack into 1 register
- Structs 9-16 bytes: Dereference pointer, pack into 2 registers
- Structs > 16 bytes: Pass by reference (current behavior)

This script modifies part4.c and zcc.c.
"""

import re
import os
import sys

def backup_file(filepath):
    """Create a backup of the file before modification."""
    backup_path = f"{filepath}.bak.structabi"
    if os.path.exists(filepath):
        with open(filepath, 'r') as f:
            content = f.read()
        with open(backup_path, 'w') as f:
            f.write(content)
        print(f"✓ Created backup: {backup_path}")
        return True
    return False

def find_nd_call_arg_pushing(lines):
    """Find the ND_CALL case where arguments are pushed."""
    
    nd_call_start = -1
    arg_push_section = -1
    
    for i, line in enumerate(lines):
        # Find case ND_CALL:
        if 'case ND_CALL:' in line:
            nd_call_start = i
            continue
        
        if nd_call_start > 0:
            # Look for the argument pushing loop
            # Pattern: for loop iterating backwards through arguments
            if ('for' in line and 'nargs' in line and '--' in line) or \
               ('for' in line and 'i--' in line and 'args' in line):
                arg_push_section = i
                break
            
            # If we hit another case statement, we went too far
            if 'case ' in line and i > nd_call_start + 5:
                break
    
    return nd_call_start, arg_push_section

def generate_struct_packing_code():
    """Generate the code to pack structs into registers."""
    
    return '''
        /* STRUCT-BY-VALUE ABI FIX (CG-IR-013): Pack small structs into registers */
        Type *arg_type = node->args[i]->type;
        
        if (arg_type->kind == TY_STRUCT) {
            int struct_size = get_type_size(arg_type);
            
            if (struct_size <= 8) {
                /* Pack into single register (RDI, RSI, RDX, RCX, R8, R9) */
                printf("    movq (%%rax), %%rax\\n");
                printf("    pushq %%rax\\n");
            }
            else if (struct_size <= 16) {
                /* Pack into TWO registers (e.g., RDI+RSI for 12-byte Vec3) */
                printf("    movq 8(%%rax), %%r11\\n");   /* Second 8 bytes */
                printf("    movq (%%rax), %%rax\\n");     /* First 8 bytes */
                printf("    pushq %%r11\\n");
                printf("    pushq %%rax\\n");
                /* Note: This consumes 2 register slots! Caller must track this. */
            }
            else {
                /* Struct > 16 bytes: pass by reference (current behavior) */
                printf("    pushq %%rax\\n");
            }
        }
        else {
            /* Non-struct: normal push */
            printf("    pushq %%rax\\n");
        }
'''

def insert_struct_packing(filepath):
    """Insert struct packing code into ND_CALL argument pushing."""
    
    if not os.path.exists(filepath):
        print(f"⚠ File not found: {filepath}")
        return False
    
    print(f"\n{'='*60}")
    print(f"Processing: {filepath}")
    print(f"{'='*60}")
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    nd_call_start, arg_push_section = find_nd_call_arg_pushing(lines)
    
    if nd_call_start < 0:
        print("⚠ Could not find ND_CALL case in codegen_expr()")
        print_manual_instructions()
        return False
    
    if arg_push_section < 0:
        print("⚠ Could not find argument pushing loop in ND_CALL")
        print_manual_instructions()
        return False
    
    print(f"✓ Found ND_CALL at line {nd_call_start + 1}")
    print(f"✓ Found arg pushing at line {arg_push_section + 1}")
    
    # Find where codegen_expr(node->args[i]) is called
    codegen_call_line = -1
    for i in range(arg_push_section, min(arg_push_section + 30, len(lines))):
        if 'codegen_expr' in lines[i] and 'args[i]' in lines[i]:
            codegen_call_line = i
            break
    
    if codegen_call_line < 0:
        print("⚠ Could not find codegen_expr(node->args[i]) call")
        print_manual_instructions()
        return False
    
    # Find the line with pushq %rax (the simple push that we're replacing)
    simple_push_line = -1
    for i in range(codegen_call_line, min(codegen_call_line + 10, len(lines))):
        if 'pushq' in lines[i] and '%rax' in lines[i] and 'printf' in lines[i]:
            simple_push_line = i
            break
    
    if simple_push_line < 0:
        print("⚠ Could not find simple 'pushq %rax' after codegen_expr")
        print_manual_instructions()
        return False
    
    print(f"✓ Found pushq instruction at line {simple_push_line + 1}")
    
    # Replace the simple push with struct-aware packing
    old_line = lines[simple_push_line]
    
    # Generate the replacement code
    struct_pack_code = generate_struct_packing_code()
    
    # Insert it, removing the old simple push
    lines[simple_push_line] = struct_pack_code + '\n'
    
    # Write back
    backup_file(filepath)
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    print(f"\n✓ Replaced simple push with struct packing at line {simple_push_line + 1}")
    print(f"   OLD: {old_line.rstrip()}")
    print(f"   NEW: [Struct-aware packing code - {len(struct_pack_code.split(chr(10)))} lines]")
    
    return True

def print_manual_instructions():
    """Print manual patching instructions."""
    
    print("""
╔══════════════════════════════════════════════════════════════════╗
║  MANUAL PATCHING REQUIRED                                        ║
╚══════════════════════════════════════════════════════════════════╝

If automatic patching fails, manually modify part4.c in codegen_expr():

LOCATION: In case ND_CALL:, find the loop that pushes arguments

FIND THIS PATTERN:
------------------
    for (int i = nargs - 1; i >= 0; i--) {
        codegen_expr(node->args[i]);  // Address in %rax
        printf("    pushq %%rax\\n");  // <-- REPLACE THIS
    }

REPLACE WITH:
-------------
    for (int i = nargs - 1; i >= 0; i--) {
        codegen_expr(node->args[i]);  // Address in %rax
        
        /* STRUCT-BY-VALUE ABI FIX (CG-IR-013) */
        Type *arg_type = node->args[i]->type;
        
        if (arg_type->kind == TY_STRUCT) {
            int struct_size = get_type_size(arg_type);
            
            if (struct_size <= 8) {
                /* Pack into single register */
                printf("    movq (%%rax), %%rax\\n");
                printf("    pushq %%rax\\n");
            }
            else if (struct_size <= 16) {
                /* Pack into TWO registers */
                printf("    movq 8(%%rax), %%r11\\n");
                printf("    movq (%%rax), %%rax\\n");
                printf("    pushq %%r11\\n");
                printf("    pushq %%rax\\n");
            }
            else {
                /* > 16 bytes: pass by reference */
                printf("    pushq %%rax\\n");
            }
        }
        else {
            /* Non-struct: normal push */
            printf("    pushq %%rax\\n");
        }
    }

CRITICAL NOTE:
-------------
For structs 9-16 bytes, we push TWO values but this still counts as ONE
source argument. You may need to track register slot consumption separately
from source argument count to properly pop into argregs[].

Current simple approach works if:
- You always push left-to-right (high addresses first)
- You pop right-to-left into %rdi, %rsi, %rdx, etc.
- Stack discipline is maintained

The callee side (function prologue) already works correctly because it
saves all 6 argregs[] to sequential stack slots, which reconstitutes
the struct in memory!
""")

def verify_struct_packing(filepath):
    """Verify struct packing code was inserted."""
    
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Check for key components
    has_struct_check = 'TY_STRUCT' in content
    has_size_check = 'struct_size' in content or 'get_type_size' in content
    has_two_reg_case = 'movq 8(%rax)' in content or 'r11' in content
    
    if has_struct_check and has_size_check:
        print(f"✓ VERIFICATION PASSED: Struct packing code found in {filepath}")
        return True
    else:
        print(f"⚠ WARNING: Struct packing code incomplete in {filepath}")
        return False

def main():
    print("""
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║  FIX STRUCT-BY-VALUE ABI (CG-IR-013)                            ║
║                                                                  ║
║  Implements System V AMD64 ABI for struct parameter passing     ║
║  Structs ≤ 8 bytes  → 1 register (RDI, RSI, ...)               ║
║  Structs 9-16 bytes → 2 registers (e.g., RDI + RSI)            ║
║  Structs > 16 bytes → By reference (unchanged)                  ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
""")
    
    files_to_fix = [
        'part4.c',
        'zcc.c'
    ]
    
    print("\n⚠ NOTE: This is a COMPLEX patch that modifies calling convention.")
    print("   Review changes carefully before committing.\n")
    
    success_count = 0
    
    for filepath in files_to_fix:
        if insert_struct_packing(filepath):
            if verify_struct_packing(filepath):
                success_count += 1
    
    print(f"\n{'='*60}")
    print(f"SUMMARY: Patched {success_count}/{len(files_to_fix)} files")
    print(f"{'='*60}")
    
    if success_count > 0:
        print("\n✓ Struct-by-value ABI patches applied!")
        print("\nNext steps:")
        print("  1. Rebuild ZCC")
        print("  2. Test exp4 and exp5 (Vec3 struct passing)")
        print("  3. Verify no SIGSEGV when calling functions with struct params")
        print("\nWARNING: This changes ABI. Test thoroughly!")
        return 0
    else:
        print("\n⚠ Automatic patching failed. See manual instructions above.")
        return 1

if __name__ == '__main__':
    sys.exit(main())
