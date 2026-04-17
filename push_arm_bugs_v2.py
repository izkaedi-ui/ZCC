import json
import os
from datasets import load_dataset, Dataset
from huggingface_hub import login

login(token = os.environ.get("HF_TOKEN", ""))

# Load existing 16 entries from local cache/JSON since the HF one might have been overwritten
with open('zcc-compiler-bug-corpus.json', 'r') as f:
    local_data = json.load(f)

# The user states existing is 16 entries.
existing = local_data[:16]

# 7 new ARM bugs — detailed exactly as requested
new_bugs = [
    {
      "id": "CG-ARM-001",
      "title": "get_callee_reg() emits x86 register strings into ARM codegen path",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:get_callee_reg",
      "symptom": "ARM output contained movq %rax, %r12 — invalid on Cortex-M0+. arm-none-eabi-gcc rejects assembly.",
      "root_cause": "get_callee_reg() returned x86 register name strings unconditionally. No backend_ops guard existed for register string selection.",
      "detection_method": "grep -E 'movq|subq|rax|rbx' hello_zcc.s — x86 instructions found in ARM output",
      "fix_summary": "Added backend_ops guard: ARM path emits mov r0, r4 style via thumb_regs[] array instead of x86 names.",
      "affected_functions": ["get_callee_reg", "codegen_expr"],
      "lines_changed": 40,
      "fix_type": "backend_guard",
      "prime_energy_score": 8.2,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "register-allocation", "cross-compilation", "x86-leakage"],
      "before_pattern": "fprintf(cc->out, \"    movq %s, %%rax\\n\", reg);",
      "after_pattern": "if (backend_ops) fprintf(cc->out, \"    mov r0, %s\\n\", reg); else fprintf(cc->out, \"    movq %s, %%rax\\n\", reg);",
      "regression_test": "zcc hello.c -target thumbv6m-none-eabi && grep -c 'movq' hello.s == 0",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M2"
    },
    {
      "id": "CG-ARM-002",
      "title": "global __asm__(\"...\") blocks crash parser",
      "cwe": "CWE-228",
      "cwe_name": "Improper Handling of Syntactically Invalid Structure",
      "severity": "critical",
      "domain": "compiler_parser",
      "component": "part3.c:parse_program",
      "symptom": "\"expected ',' or ')' after parameter\" on hello.c:6",
      "root_cause": "ZCC did not understand top-level inline __asm__ syntax, leading to parse failures on required standard library or custom vector stubs.",
      "detection_method": "Parser crash on hello.c testing",
      "fix_summary": "parse_program() intercepts global __asm__, parsing the string and emitting raw payload to cc->out.",
      "affected_functions": ["parse_program"],
      "lines_changed": 15,
      "fix_type": "syntax_support",
      "prime_energy_score": 7.5,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "parser", "boot-vectors", "assembly"],
      "before_pattern": "/* no match for __asm__ */",
      "after_pattern": "if (match(\"__asm__\")) { consume_asm_string(); }",
      "regression_test": "compile boot vectors containing global asm without parser errors",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M2"
    },
    {
      "id": "CG-ARM-003",
      "title": "ND_SUB emits subq into ARM path",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:codegen_expr ND_SUB case",
      "symptom": "subq %r11, %rax in ARM output — illegal on M0+",
      "root_cause": "ND_SUB branch emitted raw x86 format unconditionally, ignoring backend_ops interface abstraction.",
      "detection_method": "arm-none-eabi-gcc assembler rejection",
      "fix_summary": "backend_ops guard routes to subs r0, r0, r1 instead of subq.",
      "affected_functions": ["codegen_expr"],
      "lines_changed": 5,
      "fix_type": "backend_guard",
      "prime_energy_score": 7.8,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "arithmetic", "x86-leakage"],
      "before_pattern": "fprintf(cc->out, \"    subq %%r11, %%rax\\n\");",
      "after_pattern": "if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB); else fprintf(cc->out, \"    subq %%r11, %%rax\\n\");",
      "regression_test": "build math.c on thumbv6m and assert zero subq presence",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M3"
    },
    {
      "id": "CG-ARM-004",
      "title": "ND_MUL emits imulq into ARM path",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:codegen_expr ND_MUL case",
      "symptom": "imulq %r11, %rax in ARM output",
      "root_cause": "The backend_ops->emit_binary_op hook was not applied to ND_MUL.",
      "detection_method": "arm-none-eabi-gcc assembler rejection",
      "fix_summary": "backend_ops guard routes to muls r0, r1, r0.",
      "affected_functions": ["codegen_expr"],
      "lines_changed": 5,
      "fix_type": "backend_guard",
      "prime_energy_score": 7.6,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "arithmetic", "x86-leakage"],
      "before_pattern": "fprintf(cc->out, \"    imulq %%r11, %%rax\\n\");",
      "after_pattern": "if (backend_ops) backend_ops->emit_binary_op(cc, ND_MUL); else fprintf(cc->out, \"    imulq %%r11, %%rax\\n\");",
      "regression_test": "compile stress_test and confirm absence of imulq",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M3"
    },
    {
      "id": "CG-ARM-005",
      "title": "Bitwise ops (AND/OR/XOR) emit x86 variants",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:codegen_expr ND_BAND/BOR/BXOR",
      "symptom": "andq/orq/xorq in ARM output",
      "root_cause": "Bitwise handlers generated x86 instructions irrespective of selected target interface.",
      "detection_method": "arm-none-eabi-gcc assembler rejection",
      "fix_summary": "backend_ops guards route to ands/orrs/eors respectively via emit_binary_op.",
      "affected_functions": ["codegen_expr"],
      "lines_changed": 15,
      "fix_type": "backend_guard",
      "prime_energy_score": 7.2,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "bitwise", "x86-leakage"],
      "before_pattern": "fprintf(cc->out, \"    andq %%r11, %%rax\\n\");",
      "after_pattern": "if (backend_ops) backend_ops->emit_binary_op(cc, ND_BAND); else fprintf(cc->out, \"    andq %%r11, %%rax\\n\");",
      "regression_test": "compile RP2040 MMIO assignments, zero x86 bitwise leaked",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M3"
    },
    {
      "id": "CG-ARM-006",
      "title": "Shift ops emit shlq/sarq/shrq into ARM path",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:codegen_expr ND_SHL/SHR",
      "symptom": "movq %r11, %rcx; shlq %cl, %rax in ARM output",
      "root_cause": "Power of 2 shift optimizations bypassed the basic backend emitter hooks completely.",
      "detection_method": "assembler rejection during MMIO base offset loads",
      "fix_summary": "backend_ops guard routes to lsls/lsrs r0, r1 architecture blocks explicitly avoiding %cl.",
      "affected_functions": ["codegen_expr"],
      "lines_changed": 12,
      "fix_type": "backend_guard",
      "prime_energy_score": 7.3,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "shift", "x86-leakage", "optimization-leak"],
      "before_pattern": "fprintf(cc->out, \"    shlq %%cl, %%rax\\n\");",
      "after_pattern": "if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHL); else ...",
      "regression_test": "no shlq instructions discovered in stress_test execution",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M3"
    },
    {
      "id": "CG-ARM-007",
      "title": "ND_TERNARY conditional branches not guarded",
      "cwe": "CWE-704",
      "cwe_name": "Incorrect Type Conversion or Cast",
      "severity": "critical",
      "domain": "compiler_codegen",
      "component": "part4.c:codegen_expr ND_TERNARY",
      "symptom": "cmpq $0, %rax; je .L in ARM output — illegal on M0+",
      "root_cause": "The ternary operator handler missed label abstractions, dropping direct x86 string arrays in the assembly output.",
      "detection_method": "ld failure during branch linkage due to missing .L parameters and cmpq rejection",
      "fix_summary": "backend_ops guard routes to cmp r0, #0; beq .L and guarantees labels pass correctly through format parameters.",
      "affected_functions": ["codegen_expr"],
      "lines_changed": 8,
      "fix_type": "backend_guard",
      "prime_energy_score": 8.0,
      "prime_phase": "bifurcation",
      "tags": ["ARM", "branching", "x86-leakage"],
      "before_pattern": "fprintf(cc->out, \"    cmpq $0, %%rax\\n    je .L%d\\n\", lbl1);",
      "after_pattern": "if (backend_ops) fprintf(cc->out, \"    cmp r0, #0\\n    beq .L%d\\n\", lbl1);",
      "regression_test": "ternary operator tests parse into correct bne/beq instructions",
      "target": "thumbv6m-none-eabi",
      "milestone": "ARM-M4"
    }
]

# Ensure we don't have overlapping IDs from the previous existing data:
# (If existing had CG-ARM-001 or CG-ARM-002, we remove them in favor of the detailed new_bugs)
cleaned_existing = [bug for bug in existing if bug['id'] not in [b['id'] for b in new_bugs]]

all_bugs = cleaned_existing + new_bugs
print(f"Total bugs to push: {len(all_bugs)}")

Dataset.from_list(all_bugs).push_to_hub(
    "zkaedi/zcc-compiler-bug-corpus",
    commit_message="Add CG-ARM-001 through CG-ARM-007: ARM codegen x86 leakage bugs"
)

# Keep the local cache up-to-date
with open('zcc-compiler-bug-corpus.json', 'w') as f:
    json.dump(all_bugs, f, indent=2)

print(f"Pushed {len(all_bugs)} total entries")
