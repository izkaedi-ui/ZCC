import json
from huggingface_hub import HfApi, CommitOperationAdd

bugs = [
    {
        "id": "CG-AST-001",
        "title": "Indirect call ABI alignment — extra_pushes modulo parity inversion",
        "cwe": "CWE-682",
        "cwe_name": "Incorrect Calculation",
        "severity": "critical",
        "domain": "compiler_codegen",
        "component": "part4.c:ND_CALL",
        "symptom": "Every indirect function call misaligns RSP by 8 bytes. Lua VM segfaults in vsnprintf movaps. SQLite segfaults in sqlite3RegisterBuiltinFunctions.",
        "root_cause": "extra_pushes counted callee pointer in stack parity modulo, but callee was popped (pop %r10) before call *%r10. Parity check included a push that no longer existed at call time.",
        "detection_method": "GDB RSP tracking through Lua call chain: tostringbuff -> addnum2buff -> luaO_pushvfstring -> luaG_addinfo -> lexerror -> luaD_rawrunprotected. RSP off by 8 at every indirect call.",
        "fix_summary": "Removed extra_pushes from alignment modulo calculation in both zcc.c and part4.c.",
        "affected_functions": ["all indirect calls", "function pointer calls through structs", "Lua f_call", "SQLite VFS dispatch"],
        "lines_changed": 4,
        "fix_type": "alignment_fix",
        "prime_energy_score": 9.7,
        "prime_phase": "bifurcation_critical",
        "tags": ["ABI-alignment", "indirect-call", "movaps-segfault", "16-byte-boundary", "stack-parity"],
        "before_pattern": "if ((cc->stack_depth + args_on_stack + extra_pushes) % 2 != 0) alignment_pad = 8;",
        "after_pattern": "if ((cc->stack_depth + args_on_stack) % 2 != 0) alignment_pad = 8;",
        "regression_test": "Lua 5.4.6: print('hello') — must not segfault in vsnprintf"
    },
    {
        "id": "CG-AST-002",
        "title": "Static struct pointer initializer enum drift — ND_CAST/ND_ADDR hardcoded as wrong values",
        "cwe": "CWE-682",
        "cwe_name": "Incorrect Calculation",
        "severity": "critical",
        "domain": "compiler_codegen",
        "component": "part4.c:emit_struct_fields",
        "symptom": "SQLite SIGSEGV in sqlite3IsLikeFunction. FuncDef aBuiltinFunc[] pointer fields (void*)&globInfo emit as .zero 8 instead of .quad globInfo.",
        "root_cause": "emit_struct_fields used hardcoded node kind values 26 and 12 for ND_CAST and ND_ADDR. After AST enum expansion, ND_CAST=44 and ND_ADDR=28. Values 26/12 now matched ND_SHR and ND_EQ, causing pointer initializers to fall through to zero-padding.",
        "detection_method": "Assembly inspection: grep globInfo aBuiltinFunc.txt showed .zero 8 where .quad globInfo expected. Traced to hardcoded if(elem->kind == 26) in part4.c.",
        "fix_summary": "Replaced magic numbers 26 and 12 with symbolic ND_CAST and ND_ADDR enum references.",
        "affected_functions": ["all static struct initializers with pointer fields", "SQLite aBuiltinFunc[]", "SQLite FuncDef arrays"],
        "lines_changed": 4,
        "fix_type": "enum_symbolic",
        "prime_energy_score": 9.5,
        "prime_phase": "bifurcation",
        "tags": ["magic-number", "enum-drift", "struct-initializer", "static-data", "NULL-pointer"],
        "before_pattern": "while (elem && elem->kind == 26) { elem = elem->lhs; } if (elem->kind == 12 && ...)",
        "after_pattern": "while (elem && elem->kind == ND_CAST) { elem = elem->lhs; } if (elem->kind == ND_ADDR && ...)",
        "regression_test": "SQLite: SELECT * FROM t WHERE name LIKE 'h%'; — must not segfault in sqlite3IsLikeFunction"
    },
    {
        "id": "CG-AST-003",
        "title": "SysV ABI float/int register assignment — floats consuming integer register slots",
        "cwe": "CWE-628",
        "cwe_name": "Function Call with Incorrectly Specified Arguments",
        "severity": "critical",
        "domain": "compiler_codegen",
        "component": "part4.c:ND_CALL + codegen_func",
        "symptom": "printf with mixed float/int args prints garbage for integers. SQLite internal formatting produces wrong float text representations.",
        "root_cause": "Call codegen popped all args into integer registers sequentially, then copied floats to XMM. Float args occupied integer slots, shifting subsequent integer args. Callee side had same issue — argregs[i] where i counted all params including floats.",
        "detection_method": "test_setdouble: u.r=3.140000 flags=1374389535. flags=0x51EB851F = low 32 bits of 3.14 IEEE754, proving printf read double bits as int.",
        "fix_summary": "Caller: unified pop loop with independent gp_idx/fp_idx counters. Callee: same independent counters for parameter receiving. Float return bridge: movq %xmm0, %rax after calls returning double.",
        "affected_functions": ["all functions with mixed float/int arguments", "printf", "fprintf", "sqlite3MPrintf"],
        "lines_changed": 40,
        "fix_type": "abi_register_assignment",
        "prime_energy_score": 9.3,
        "prime_phase": "bifurcation",
        "tags": ["SysV-ABI", "float-register", "XMM", "argument-passing", "mixed-type-args"],
        "before_pattern": "for (i = 0; i < nargs && i < 6; i++) pop_reg(cc, argregs[i]); /* then copy floats to xmm */",
        "after_pattern": "int gp_idx=0, fp_idx=0; for (i=0; i<nargs; i++) { if (is_float) popq->xmm[fp_idx++]; else pop_reg(argregs[gp_idx++]); }",
        "regression_test": "printf('r=%.2f flags=%d', 3.14, 8) — flags must print 8, not 1374389535"
    },
    {
        "id": "CG-AST-004",
        "title": "Switch statement fallthrough broken — case labels isolated into separate blocks",
        "cwe": "CWE-484",
        "cwe_name": "Omitted Break Statement in a Switch",
        "severity": "critical",
        "domain": "compiler_parser",
        "component": "part3.c:parse_switch + part4.c:ND_SWITCH codegen",
        "symptom": "SQLite stores all REAL values as 0.0 or garbage. SELECT 3.14 returns correct value but INSERT/SELECT round-trip corrupts floats.",
        "root_cause": "ZCC parser decoupled case labels into isolated Node arrays with forced jumps between them. Default case pushed to bottom. SQLite OP_MakeRecord uses Duff's device fallthrough (default: zPayload[7]=...; case 6: zPayload[5]=...) to pack double bytes. ZCC emitted jmp after default, skipping cases 6-1. Only 2 of 8 bytes written.",
        "detection_method": "test_serial: text=4.908e-310 (garbage). Isolated SELECT 3.14 worked (no serialization). Traced to OP_MakeRecord byte-packing switch with fallthrough. test_fallthrough.c confirmed ZCC breaks case fallthrough.",
        "fix_summary": "Rewrote switch parsing to preserve continuous block structure. Case/default labels parsed as goto targets within single body. Codegen emits cmpq dispatch stubs jumping to labels within continuous code, preserving fallthrough semantics.",
        "affected_functions": ["all switch statements with fallthrough", "SQLite OP_MakeRecord", "SQLite serialization", "Duff's device patterns"],
        "lines_changed": 60,
        "fix_type": "parser_rewrite",
        "prime_energy_score": 9.9,
        "prime_phase": "bifurcation_critical",
        "tags": ["switch-fallthrough", "Duffs-device", "serialization", "byte-packing", "parser-bug"],
        "before_pattern": "/* Each case parsed as isolated node array with implicit break */",
        "after_pattern": "/* Cases parsed as labels within continuous body block — fallthrough preserved */",
        "regression_test": "SQLite: INSERT INTO t VALUES(3.14); SELECT val FROM t; — must return 3.14, not 0.0"
    },
    {
        "id": "CG-AST-005",
        "title": "SQLITE_OMIT_FLOATING_POINT in preprocessing — tokenizer rejects float literals",
        "cwe": "CWE-1078",
        "cwe_name": "Inappropriate Source Code Style or Formatting",
        "severity": "high",
        "domain": "build_pipeline",
        "component": "prep_sqlite_for_zcc.py",
        "symptom": "INSERT INTO t VALUES(1,'hello',3.14) returns 'near \".\": syntax error'. Integer inserts work.",
        "root_cause": "prep_sqlite_for_zcc.py passed -DSQLITE_OMIT_FLOATING_POINT to gcc -E, causing SQLite preprocessor to strip CC_DOT fallthrough in sqlite3GetToken. Tokenizer correctly rejected float literals per the OMIT_FLOATING_POINT specification.",
        "detection_method": "aiClass[46]=26 verified correct. Char comparison test passed. gcc -o test_gcc sqlite3_zcc.c showed float parsing worked under GCC. Diffed preprocessor flags.",
        "fix_summary": "Removed -DSQLITE_OMIT_FLOATING_POINT from prep_sqlite_for_zcc.py. Added __builtin_inff() -> (1e300) regex replacement for ZCC compatibility.",
        "affected_functions": ["sqlite3GetToken", "all SQL float literal parsing"],
        "lines_changed": 3,
        "fix_type": "build_config",
        "prime_energy_score": 6.5,
        "prime_phase": "converging",
        "tags": ["preprocessor", "build-pipeline", "float-tokenizer", "OMIT-flag"],
        "before_pattern": "gcc -E -P -DSQLITE_OMIT_FLOATING_POINT ... sqlite3.c",
        "after_pattern": "gcc -E -P ... sqlite3.c  /* no OMIT_FLOATING_POINT */",
        "regression_test": "SQLite: INSERT INTO t VALUES(3.14); — must not return syntax error"
    }
]

with open('/tmp/new_bugs.jsonl', 'w') as f:
    for bug in bugs:
        f.write(json.dumps(bug) + '\n')

try:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_apr11_2026.jsonl", path_or_fileobj="/tmp/new_bugs.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 5 new bugs from Apr 11 session: indirect call ABI, enum drift, float registers, switch fallthrough, preprocessor flag"
    )
    print(f"Pushed: {result}")
except Exception as e:
    api = HfApi(token = os.environ.get("HF_TOKEN", ""))
    ops = [CommitOperationAdd(path_in_repo="bugs_apr11_2026.jsonl", path_or_fileobj="/tmp/new_bugs.jsonl")]
    result = api.create_commit(
        repo_id="zkaedi/zcc-compiler-bug-corpus",
        repo_type="dataset",
        operations=ops,
        commit_message="🔱 5 new bugs from Apr 11 session: indirect call ABI, enum drift, float registers, switch fallthrough, preprocessor flag"
    )
    print(f"Pushed: {result}")
