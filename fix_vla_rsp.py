#!/usr/bin/env python3
"""
Fix VLA RSP Corruption Bug (CG-IR-012)

Root Cause: push_vla_scope() incorrectly uses += instead of -= for stack allocation.
Stack grows downward, so local_offset should DECREASE, not increase.

This script fixes both part3.c and zcc.c (monolithic version).
"""

import re
import os
import sys
from pathlib import Path

def backup_file(filepath):
    """Create a backup of the file before modification."""
    backup_path = f"{filepath}.bak"
    if os.path.exists(filepath):
        with open(filepath, 'r') as f:
            content = f.read()
        with open(backup_path, 'w') as f:
            f.write(content)
        print(f"✓ Created backup: {backup_path}")
        return True
    return False

def fix_vla_rsp_in_file(filepath):
    """Fix the VLA RSP bug in the specified file."""
    
    if not os.path.exists(filepath):
        print(f"⚠ File not found: {filepath}")
        return False
    
    print(f"\n{'='*60}")
    print(f"Processing: {filepath}")
    print(f"{'='*60}")
    
    # Read file
    with open(filepath, 'r') as f:
        lines = f.readlines()
    
    # Track changes
    changes_made = 0
    
    # Search for push_vla_scope function
    for i, line in enumerate(lines):
        # Look for the buggy line: cc->local_offset += 8;
        if 'cc->local_offset' in line and '+=' in line and '8' in line:
            # Verify this is inside push_vla_scope by checking nearby context
            context_start = max(0, i - 10)
            context_end = min(len(lines), i + 10)
            context = ''.join(lines[context_start:context_end])
            
            if 'push_vla_scope' in context or 'VLA' in context:
                # Found the buggy line!
                old_line = line
                # Replace += with -=
                new_line = line.replace('cc->local_offset += 8', 'cc->local_offset -= 8')
                
                if old_line != new_line:
                    lines[i] = new_line
                    changes_made += 1
                    
                    print(f"\n✓ Fixed at line {i+1}:")
                    print(f"  OLD: {old_line.rstrip()}")
                    print(f"  NEW: {new_line.rstrip()}")
    
    if changes_made == 0:
        print("⚠ No changes needed (already fixed or pattern not found)")
        return False
    
    # Write back
    backup_file(filepath)
    with open(filepath, 'w') as f:
        f.writelines(lines)
    
    print(f"\n✓ Applied {changes_made} fix(es) to {filepath}")
    return True

def verify_fix(filepath):
    """Verify the fix was applied correctly."""
    
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'r') as f:
        content = f.read()
    
    # Check that we have -= and not +=
    buggy_pattern = r'cc->local_offset\s*\+=\s*8'
    fixed_pattern = r'cc->local_offset\s*-=\s*8'
    
    has_bug = re.search(buggy_pattern, content)
    has_fix = re.search(fixed_pattern, content)
    
    if has_bug:
        print(f"✗ VERIFICATION FAILED: Still contains buggy pattern in {filepath}")
        return False
    
    if has_fix:
        print(f"✓ VERIFICATION PASSED: Fix confirmed in {filepath}")
        return True
    
    print(f"⚠ WARNING: Neither pattern found in {filepath}")
    return False

def main():
    print("""
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║  FIX VLA RSP CORRUPTION BUG (CG-IR-012)                         ║
║                                                                  ║
║  Changes: cc->local_offset += 8  →  cc->local_offset -= 8       ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
""")
    
    # Files to fix
    files_to_fix = [
        'part3.c',
        'zcc.c'  # Monolithic version
    ]
    
    success_count = 0
    
    for filepath in files_to_fix:
        if fix_vla_rsp_in_file(filepath):
            if verify_fix(filepath):
                success_count += 1
    
    print(f"\n{'='*60}")
    print(f"SUMMARY: Fixed {success_count}/{len(files_to_fix)} files")
    print(f"{'='*60}")
    
    if success_count == len(files_to_fix):
        print("\n✓ All files fixed successfully!")
        print("\nNext steps:")
        print("  1. Rebuild ZCC: gcc -O0 -w -o zcc zcc.c -lm")
        print("  2. Test exp3: ./zcc exp3_audio_visualizer.c -o exp3.s && gcc -o exp3 exp3.s -lm")
        print("  3. Run: ./exp3 > audio_output.ppm")
        return 0
    else:
        print("\n⚠ Some files could not be fixed. Check output above.")
        return 1

if __name__ == '__main__':
    sys.exit(main())
