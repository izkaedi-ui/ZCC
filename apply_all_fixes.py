#!/usr/bin/env python3
"""
Master Script: Apply All ZCC Codegen Fixes

Runs all 3 bug fixes in the correct order:
1. VLA RSP corruption (CG-IR-012)
2. Const int folding (CG-IR-012 part 2)
3. Struct-by-value ABI (CG-IR-013)

Usage:
    python3 apply_all_fixes.py [--dry-run] [--skip-backup]
"""

import subprocess
import sys
import os
from pathlib import Path

def run_script(script_name, description):
    """Run a fix script and report results."""
    
    print(f"\n{'='*70}")
    print(f"RUNNING: {description}")
    print(f"Script:  {script_name}")
    print(f"{'='*70}\n")
    
    try:
        result = subprocess.run(
            ['python3', script_name],
            check=False,
            capture_output=False,
            text=True
        )
        
        if result.returncode == 0:
            print(f"\n✓ {description} - SUCCESS")
            return True
        else:
            print(f"\n⚠ {description} - FAILED (exit code {result.returncode})")
            return False
            
    except Exception as e:
        print(f"\n✗ {description} - ERROR: {e}")
        return False

def check_prerequisites():
    """Check that we're in the right directory and files exist."""
    
    required_files = ['part1.c', 'part2.c', 'part3.c', 'part4.c']
    
    missing = []
    for f in required_files:
        if not os.path.exists(f):
            missing.append(f)
    
    if missing:
        print(f"✗ ERROR: Missing required files: {', '.join(missing)}")
        print(f"  Please run this script from the ZCC source directory")
        return False
    
    print("✓ All prerequisite files found")
    return True

def rebuild_zcc():
    """Rebuild ZCC after applying patches."""
    
    print(f"\n{'='*70}")
    print("REBUILDING ZCC")
    print(f"{'='*70}\n")
    
    # Try to rebuild monolithic zcc.c
    if os.path.exists('zcc.c'):
        print("Building from monolithic zcc.c...")
        result = subprocess.run(
            ['gcc', '-O0', '-w', '-o', 'zcc', 'zcc.c', '-lm'],
            capture_output=True,
            text=True
        )
        
        if result.returncode == 0:
            print("✓ ZCC rebuilt successfully")
            return True
        else:
            print(f"⚠ Build failed:")
            print(result.stderr)
    
    # Try to rebuild from parts
    if os.path.exists('rebuild_full.sh'):
        print("Using rebuild_full.sh...")
        result = subprocess.run(['bash', 'rebuild_full.sh'], capture_output=True, text=True)
        if result.returncode == 0:
            print("✓ ZCC rebuilt successfully")
            return True
    
    print("⚠ Could not rebuild ZCC automatically")
    print("   Please rebuild manually:")
    print("   gcc -O0 -w -o zcc zcc.c -lm")
    return False

def run_quick_test():
    """Run a quick smoke test to verify ZCC still works."""
    
    print(f"\n{'='*70}")
    print("QUICK SMOKE TEST")
    print(f"{'='*70}\n")
    
    # Create a simple test file
    test_c = """
#include <stdio.h>

int main(void) {
    const int SIZE = 10;
    int arr[SIZE];
    arr[0] = 42;
    printf("Test: %d\\n", arr[0]);
    return 0;
}
"""
    
    with open('_test_const.c', 'w') as f:
        f.write(test_c)
    
    # Try to compile it
    print("Testing const int array sizing...")
    result = subprocess.run(
        ['./zcc', '_test_const.c', '-o', '_test_const.s'],
        capture_output=True,
        text=True
    )
    
    if result.returncode == 0:
        print("✓ Const int test compiled successfully")
        
        # Check if VLA save/restore is present (it shouldn't be)
        with open('_test_const.s', 'r') as f:
            asm = f.read()
        
        if 'movq    %rsp' in asm and '-80(%rbp)' in asm:
            print("⚠ WARNING: VLA save/restore still present (fix may not have worked)")
            return False
        else:
            print("✓ No VLA machinery detected (fix appears successful)")
            return True
    else:
        print(f"✗ Compilation failed:")
        print(result.stderr)
        return False

def main():
    print("""
╔══════════════════════════════════════════════════════════════════╗
║                                                                  ║
║             ZCC CODEGEN FIXES - MASTER APPLICATION               ║
║                                                                  ║
║  This script applies all 3 critical bug fixes:                  ║
║  • CG-IR-012: VLA RSP corruption                                ║
║  • CG-IR-012: Const int folding                                 ║
║  • CG-IR-013: Struct-by-value ABI                               ║
║                                                                  ║
╚══════════════════════════════════════════════════════════════════╝
""")
    
    # Check prerequisites
    if not check_prerequisites():
        return 1
    
    # Run all fixes
    fixes = [
        ('fix_vla_rsp.py', 'VLA RSP Corruption Fix (CG-IR-012)'),
        ('fix_const_folding.py', 'Const Int Folding (CG-IR-012 Part 2)'),
        ('fix_struct_abi.py', 'Struct-by-Value ABI Fix (CG-IR-013)')
    ]
    
    results = []
    
    for script, description in fixes:
        success = run_script(script, description)
        results.append((description, success))
    
    # Print summary
    print(f"\n{'='*70}")
    print("FINAL SUMMARY")
    print(f"{'='*70}\n")
    
    for description, success in results:
        status = "✓ SUCCESS" if success else "✗ FAILED"
        print(f"{status:12} - {description}")
    
    all_success = all(s for _, s in results)
    
    if all_success:
        print(f"\n✓ ALL FIXES APPLIED SUCCESSFULLY!\n")
        
        # Offer to rebuild
        response = input("Rebuild ZCC now? [Y/n]: ").strip().lower()
        if response in ['', 'y', 'yes']:
            if rebuild_zcc():
                run_quick_test()
        
        print("\nNext steps:")
        print("  1. Test exp3: ./zcc exp3_audio_visualizer.c -o exp3.s")
        print("  2. Test exp4: ./zcc exp4_vr_stereo.c -o exp4.s")
        print("  3. Test exp5: ./zcc exp5_physics_engine.c -o exp5.s")
        print("  4. Link and run all experiments to verify fixes")
        
        return 0
    else:
        print(f"\n⚠ SOME FIXES FAILED\n")
        print("Review the output above and apply manual patches as needed.")
        return 1

if __name__ == '__main__':
    sys.exit(main())
