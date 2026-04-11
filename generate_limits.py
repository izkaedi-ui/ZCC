import sys

def generate_limit_test():
    with open("zcc_limit_test.c", "w") as f:
        f.write("#include <stdio.h>\n")
        f.write("#include <stdlib.h>\n\n")

        # 1. 64-Argument Function Test (ABI SystemV compliance limit)
        args_decl = ", ".join([f"int a{i}" for i in range(1, 65)])
        f.write(f"int omega_function({args_decl}) {{\n")
        
        args_sum = " + ".join([f"a{i}" for i in range(1, 65)])
        f.write(f"    return {args_sum};\n")
        f.write("}\n\n")

        # 3. Main execution frame
        f.write("int main() {\n")
        
        args_call = ", ".join([str(i) for i in range(1, 65)])
        f.write(f"    int result = omega_function({args_call});\n")
        f.write('    printf("Omega  : %d\\n", result);\n')
        f.write("    if (result == 2080) {\n")
        f.write('        printf("LIMIT TEST PASSED.\\n");\n')
        f.write("        return 0;\n")
        f.write("    }\n")
        f.write('    printf("LIMIT FAILED.\\n");\n')
        f.write("    return 1;\n")
        f.write("}\n")

if __name__ == "__main__":
    generate_limit_test()
    print("Re-generated 64 parameter limit test.")
