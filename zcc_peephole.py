#!/usr/bin/env python3
import sys
import os

def run_peephole(input_file, output_file):
    with open(input_file, 'r') as f:
        lines = f.readlines()

    optimized = []
    i = 0
    n = len(lines)
    eliminated = 0

    while i < n:
        line = lines[i]
        stripped = line.strip()

        # 1. Redundant Push/Pop Elimination
        # pushq %rax
        # popq %rcx  -> movq %rax, %rcx
        if stripped.startswith("pushq ") and i + 1 < n:
            next_stripped = lines[i+1].strip()
            if next_stripped.startswith("popq "):
                reg1 = stripped.split()[1]
                reg2 = next_stripped.split()[1]
                if reg1 == reg2:
                    # push %rax, pop %rax is just dead code
                    eliminated += 2
                    i += 2
                    continue
                else:
                    # push %rax, pop %rcx -> movq %rax, %rcx
                    optimized.append(f"    movq {reg1}, {reg2}\n")
                    eliminated += 2
                    i += 2
                    continue

        # 2. Arithmetic Nullification
        if stripped in ["addq $0, %rax", "subq $0, %rax", "addq $0, %rsp", "subq $0, %rsp"]:
            eliminated += 1
            i += 1
            continue

        # 3. Push/Lea/Pop Triad (Common in ZCC parameter extraction)
        # pushq %rax
        # leaq -8(%rbp), %rax
        # popq %r11
        if stripped == "pushq %rax" and i + 2 < n:
            l2 = lines[i+1].strip()
            l3 = lines[i+2].strip()
            if l2.startswith("leaq ") and l2.endswith(", %rax") and l3.startswith("popq "):
                reg_pop = l3.split()[1]
                # Translated to:
                # movq %rax, %reg
                # leaq -X(%rbp), %rax
                optimized.append(f"    movq %rax, {reg_pop}\n")
                optimized.append(lines[i+1])
                eliminated += 3
                i += 3
                continue

        optimized.append(line)
        i += 1

    with open(output_file, 'w') as f:
        f.writelines(optimized)

    return eliminated

if __name__ == "__main__":
    if len(sys.argv) < 3:
        input_s = "zcc2.s"
        output_s = "zcc2_opt.s"
    else:
        input_s = sys.argv[1]
        output_s = sys.argv[2]
        
    if not os.path.exists(input_s):
        print(f"Error: {input_s} not found.")
        sys.exit(1)

    elim = run_peephole(input_s, output_s)
    print(f"🔱 ZCC Peephole Optimizer:")
    print(f"  Input:  {input_s}")
    print(f"  Output: {output_s}")
    print(f"  Result: Eliminated {elim} redundant ASM instructions. ✅")
