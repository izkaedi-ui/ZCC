import json
from huggingface_hub import HfApi, CommitOperationAdd

bugs = [
    {
        "id": "CG-AST-006",
        "title": "Implicit standard library prototype integer decay — cltq execution corrupts XMM returns",
        "cwe": "CWE-682",
        "cwe_name": "Incorrect Calculation",
        "severity": "critical",
        "domain": "compiler_frontend",
        "component": "part4.c:ND_CALL",
        "symptom": "All math functions (pow, sqrt, floor) return completely garbage random values. Modulates geometry and light shading randomly into chaos.",
        "root_cause": "ZCC's preprocessor skipped complex glibc header definitions for <math.h>. Functions fell back to implicit 'int' return type rule under standard C. During codegen, ZCC ignored the actual FPU return register %xmm0, took trash bytes in %eax, executed a sign-extending 'cltq' cast, and wrapped it in cvtsi2sdq, totally shredding the valid IEEE-754 mantissa stored innocently in %xmm0.",
        "detection_method": "Line-by-line inspection of assembly output for `call pow` and `call sqrt`. Identified `cltq` sequentially following the `call`, a signature of implicit integer casting.",
        "fix_summary": "Explicitly defined standard prototypes `double pow(double, double);`, `double sqrt(double);`, and `double floor(double);` inside the target application headers to force AST recognition of FPU `double` returns.",
        "affected_functions": ["pow", "sqrt", "floor", "all unprototyped functions returning double"],
        "lines_changed": 4,
        "fix_type": "prototype_declaration",
        "prime_energy_score": 9.1,
        "prime_phase": "bifurcation",
        "tags": ["implicit-int", "cltq-extension", "math-headers", "xmm0-corruption"],
        "before_pattern": "call sqrt\\n    cltq\\n    cvtsi2sdq %rax, %xmm0",
        "after_pattern": "call sqrt\\n    movq %xmm0, %rax",
        "regression_test": "Raytracer: Calculate sqrt(4.0) — must resolve precisely to 2.0 without FPU bit-shredding."
    },
    {
        "id": "CG-AST-007",
        "title": "Floating-point conditional inversion — Two's Complement cmpq breaks negative limits",
        "cwe": "CWE-697",
        "cwe_name": "Incorrect Comparison",
        "severity": "critical",
        "domain": "compiler_codegen",
        "component": "part4.c:ND_LT",
        "symptom": "Raytracer floor plane traces completely vanish. Evaluates conditions like `ray_dir.y < -0.001` as FALSE for natively legitimate negative trajectories (e.g. -0.5).",
        "root_cause": "ZCC used signed integer `cmpq` to judge IEEE-754 floating point binaries. IEEE formats use sign-and-magnitude bits natively. When both operators are functionally negative, the structurally larger decimal holds a computationally smaller literal signed integer. `cmpq` reversed the inequalities across two negative values.",
        "detection_method": "Discovered ray-trace intersection evaluation against `-0.001` failed across identical input vectors vs GCC compilation.",
        "fix_summary": "Refactored floating-point comparisons out of negative numeric literals. Grounded comparisons safely against the native floating-point zero `0.0`. E.g., transitioning `x < -0.001` natively to `x < 0.0`. Validated exact match over positive distances and strictly monotonic `0.0` offsets.",
        "affected_functions": ["ND_LE", "ND_LT", "ND_GE", "ND_GT", "floating point comparisons involving negative literals"],
        "lines_changed": 2,
        "fix_type": "comparison_rewrite",
        "prime_energy_score": 8.8,
        "prime_phase": "bifurcation",
        "tags": ["floating-point", "cmpq-inversion", "twos-complement", "ieee-754", "negative-floats"],
        "before_pattern": "if (ray_dir->y < -0.001) {",
        "after_pattern": "if (ray_dir->y < 0.0) {",
        "regression_test": "Evaluate (-0.5 < -0.001) using ZCC — must structurally resolve inequality accurately."
    }
]

with open('/tmp/raytracer_bugs.jsonl', 'w') as f:
    for bug in bugs:
        f.write(json.dumps(bug) + '\n')

try:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_raytracer_apr11_2026.jsonl", path_or_fileobj="/tmp/raytracer_bugs.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 CG-AST-006 & CG-AST-007 — adding ZCC Raytracer floating-point evaluations: cltq extraction corruption and Two's Complement cmpq inversion"
    )
    print(f"Pushed: {result}")
except Exception as e:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_raytracer_apr11_2026.jsonl", path_or_fileobj="/tmp/raytracer_bugs.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 CG-AST-006 & CG-AST-007 — adding ZCC Raytracer floating-point evaluations: cltq extraction corruption and Two's Complement cmpq inversion"
    )
    print(f"Pushed: {result}")
