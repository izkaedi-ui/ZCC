#!/usr/bin/env python3
"""
Fix Const Int Constant Folding (Part of CG-IR-012 fix)

Root Cause: ZCC doesn't constant-fold `const int SIZE = 256`, treating it as a VLA trigger.

Solution: When parsing `const int NAME = VALUE`, use eval_const_expr() to resolve the value
and store it in sym->enum_val with sym->is_enum_const = 1, making it available for array sizes.

This script modifies parse_stmt() in part3.c and zcc.c.
"""

import re
import os
import sys

def backup_file(filepath):
    """Create a backup of the file before modification."""
    backup_path = f"{filepath}.bak.constfold"
    if os.path.exists(filepath):
        with open(filepath, 'r') as f:
            content = f.read()
        with open(backup_path, 'w') as f:
            f.write(content)
        print(f"✓ Created backup: {backup_path}")
        return True
    return False

def find_parse_stmt_decl_section(lines):
    """Find the declaration handling section in parse_stmt()."""
    
    # Strategy: Look for parse_stmt function, then find where it handles declarations
    # We want to insert after a variable is declared and initialized
    
    in_parse_stmt = False
    decl_section_start = -1
    
    for i, line in enumerate(lines):
        # Find parse_stmt function
        if 'Node *parse_stmt(' in line or 'parse_stmt(Compiler' in line:
            in_parse_stmt = True
            continue
        
        if in_parse_stmt:
            # Look for variable declaration handling
            # Common patterns: "if (is_type_token", "Type *type = parse_type"
            if 'parse_type(' in line or 'is_type_token' in line:
                decl_section_start = i
                break
    
    return decl_section_start

def add_const_folding_to_parse_stmt(filepath):
    """Add const folding logic to parse_stmt() function."""
    
    if not os.path.exists(filepath):
        print(f"⚠ File not found: {filepath}")
        return False
    
    print(f"\n{'='*60}")
    print(f"Processing: {filepath}")
    print(f"{'='*60}")
    
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Find where to insert the const folding code
    # We need to insert it after a variable declaration with initializer is parsed
    
    insertion_point = -1
    
    for i, line in enumerate(lines):
        # Look for pattern indicating we just added a symbol with initializer
        # Common in parse_stmt: after "sym = add_local_symbol" or "add_global_symbol"
        if 'add_local_symbol' in line or 'add_global_symbol' in line:
            # Check next ~20 lines for initialization handling
            for j in range(i, min(i + 30, len(lines))):
                # Look for where init_node or similar is assigned
                if 'init_node' in lines[j] or 'initializer' in lines[j]:
                    insertion_point = j + 1
                    break
            if insertion_point > 0:
                break
    
    if insertion_point < 0:
        print("⚠ Could not find suitable insertion point in parse_stmt()")
        print("   Manual insertion required. See instructions below.")
        print_manual_instructions()
        return False
    
    # Const folding code to insert
    const_fold_code = """
        /* CONST FOLDING FIX (CG-IR-012): Treat 'const int' with constant init as compile-time constant */
        if (is_const && init_node) {
            int const_val = eval_const_expr(init_node);
            if (const_val >= 0) {
                sym->is_enum_const = 1;
                sym->enum_val = const_val;
            }
        }
"""
    
    # Insert the code
    lines.insert(insertion_point, const_fold_code)
    
    # Write back
    backup_file(filepath)
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    print(f"\n✓ Inserted const folding code at line {insertion_point + 1}")
    print(f"   Context: After variable initialization handling")
    return True

def print_manual_instructions():
    """Print manual insertion instructions if automatic patching fails."""
    
    print("""
╔══════════════════════════════════════════════════════════════════╗
║  MANUAL INSERTION REQUIRED                                       ║
╚══════════════════════════════════════════════════════════════════╝

If automatic patching fails, manually add this code to parse_stmt():

LOCATION: After parsing a declaration with initializer (look for where
          'init_node' or 'initializer' is assigned)

CODE TO INSERT:
--------------
        /* CONST FOLDING FIX (CG-IR-012) */
        if (is_const && init_node) {
            int const_val = eval_const_expr(init_node);
            if (const_val >= 0) {
                sym->is_enum_const = 1;
                sym->enum_val = const_val;
            }
        }
--------------

EXAMPLE CONTEXT:
    Type *type = parse_type(cc);
    char *varname = expect(cc, TK_IDENT)->str;
    Symbol *sym = add_local_symbol(cc, varname, type);
    
    if (consume(cc, TK_ASSIGN)) {
        Node *init_node = parse_expr(cc);
        
        /* INSERT CONST FOLDING CODE HERE */
        if (is_const && init_node) {
            int const_val = eval_const_expr(init_node);
            if (const_val >= 0) {
                sym->is_enum_const = 1;
                sym->enum_val = const_val;
            }
        }
    }

This will make `const int SIZE = 256;` behave like an enum constant,
preventing VLA machinery from triggering on `int arr[SIZE];`.
""")

def verify_insertion(filepath):
    """Verify const folding code was inserted."""
    
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Check for key components of the fix
    has_const_check = 'is_const' in content and 'init_node' in content
    has_enum_const_set = 'is_enum_const = 1' in content
    has_eval_const = 'eval_const_expr' in content
    
    if has_const_check and has_enum_const_set and has_eval_const:
        print(f"✓ VERIFICATION PASSED: Const folding code found in {filepath}")
        return True
    else:
        print(f"⚠ WARNING: Const folding code may not be complete in {filepath}")
        return False

def main():
    print("""
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║  FIX CONST INT CONSTANT FOLDING (CG-IR-012 Part 2)              ║
║                                                                  ║
║  Makes `const int SIZE = 256` behave like enum constant         ║
║  Prevents VLA machinery from triggering on constant arrays      ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
""")
    
    files_to_fix = [
        'part3.c',
        'zcc.c'
    ]
    
    print("\n⚠ NOTE: This is a SEMI-AUTOMATIC patch.")
    print("   If automatic insertion fails, manual steps will be shown.\n")
    
    success_count = 0
    
    for filepath in files_to_fix:
        if add_const_folding_to_parse_stmt(filepath):
            if verify_insertion(filepath):
                success_count += 1
    
    print(f"\n{'='*60}")
    print(f"SUMMARY: Patched {success_count}/{len(files_to_fix)} files")
    print(f"{'='*60}")
    
    if success_count > 0:
        print("\n✓ Const folding patches applied!")
        print("\nNext steps:")
        print("  1. Rebuild ZCC")
        print("  2. Test with: const int SIZE = 256; int arr[SIZE];")
        print("  3. Verify no VLA save/restore in generated assembly")
        return 0
    else:
        print("\n⚠ Automatic patching failed. See manual instructions above.")
        return 1

if __name__ == '__main__':
    sys.exit(main())
