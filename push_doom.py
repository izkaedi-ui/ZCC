import json
from huggingface_hub import HfApi, CommitOperationAdd

bugs = [
    {
        "id": "CG-AST-008",
        "title": "DOOM 1.10 Parsing Boundary — _Float128 and X11 System Header Overload",
        "cwe": "CWE-1164",
        "cwe_name": "Irrelevant Code",
        "severity": "info",
        "domain": "compiler_parser",
        "component": "part3.c:parse_type",
        "symptom": "DOOM preprocessing succeeds, but ZCC produces ~2500 errors attempting to compile. Primary failures target GCC internal compiler types (`_Float128`), untyped `size_t` fallbacks over macro strip-out, and recursive untracked structs inside <X11/Xlib.h>.",
        "root_cause": "The DOOM Linux port heavily depends on GNU libc compiler extensions and extremely dense UNIX header inclusions (<sys/uio.h>, <X11/Xshm.h>). When concatenated, ZCC's current parser evaluates thousands of unfamiliar GNU macro stubs as syntax errors instead of gracefully skipping.",
        "detection_method": "Attempting `./zcc doom_pp.c -o doom.s 2> doom_errors.log`.",
        "fix_summary": "Identified as a Graduation Constraint. To compile monolithic legacy X11 applications, ZCC must either deploy robust C preprocessor stub-out passes for internal GCC primitives (`__builtin_va_list`, `_Float128`) or the target application must be aggressively `#define`-isolated.",
        "affected_functions": ["parse_type", "parse_declarator"],
        "lines_changed": 0,
        "fix_type": "architectural_limitation",
        "prime_energy_score": 9.9,
        "prime_phase": "graduation_constraint",
        "tags": ["doom-compile", "_Float128", "x11-headers", "libc", "parser-boundary"],
        "before_pattern": "./zcc doom_pp.c",
        "after_pattern": "FAIL: 2092 syntax/type errors",
        "regression_test": "DOOM Compilation"
    }
]

with open('/tmp/doom_fail.jsonl', 'w') as f:
    for bug in bugs:
        f.write(json.dumps(bug) + '\n')

try:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_doom_apr11_2026.jsonl", path_or_fileobj="/tmp/doom_fail.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 CG-AST-008 — DOOM 1.10 Compilation constraints: _Float128 and GNU X11 Headers"
    )
    print(f"Pushed: {result}")
except Exception as e:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_doom_apr11_2026.jsonl", path_or_fileobj="/tmp/doom_fail.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 CG-AST-008 — DOOM 1.10 Compilation constraints: _Float128 and GNU X11 Headers"
    )
    print(f"Pushed: {result}")
