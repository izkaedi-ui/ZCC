#!/usr/bin/env python3
"""Add curl milestone bugs to ZCC compiler bug corpus and push to HuggingFace."""
import json
import sys
import os

CORPUS_PATH = "/mnt/h/__DOWNLOADS/selforglinux/zcc-compiler-bug-corpus/zcc-compiler-bug-corpus.json"

CURL_BUGS = [
    {
        "id": "CG-CURL-001",
        "title": "ND_CAST wrapper in function-pointer global initializers",
        "symptom": "Curl_cmalloc = (curl_malloc_callback)malloc emits .quad 0 instead of .quad malloc relocation. Runtime crash: indirect call through NULL function pointer in curl_url() -> Curl_ccalloc.",
        "root_cause": "emit_global_var() in part4.c checked gvar->initializer->kind directly, missing the ND_CAST wrapper that the parser creates for function pointer casts. The dispatch chain (ND_NUM, ND_ADDR, ND_VAR, ND_STR, ND_INIT_LIST) never matched ND_CAST, falling through to .zero emission.",
        "fix": "Strip ND_CAST wrappers before dispatch: `Node *init = gvar->initializer; while (init && init->kind == ND_CAST) init = init->lhs;` Then use `init` for all kind checks and value reads.",
        "fix_diff": "--- a/part4.c\n+++ b/part4.c\n@@ emit_global_var\n-    if (gvar->initializer->kind == ND_NUM) {\n+    Node *init = gvar->initializer;\n+    while (init && init->kind == ND_CAST) init = init->lhs;\n+    if (init && init->kind == ND_NUM) {",
        "file": "part4.c",
        "function": "emit_global_var",
        "line_range": "3009-3013",
        "commit": "41389d6",
        "codebase": "libcurl-8.7.1",
        "category": "global_initializer",
        "severity": "runtime_crash",
        "discovery_method": "GDB backtrace: call *%r10 with r10=0x0, Curl_ccalloc in .data section all zeros",
        "verification": "objdump -r -j .data easy.o shows R_X86_64_64 malloc/free/calloc/realloc/strdup relocations",
        "related_bugs": [],
        "tags": ["function_pointer", "cast", "data_section", "relocation", "global_variable"]
    },
    {
        "id": "CG-CURL-002",
        "title": "SysV ABI va_list — void* pointer-arithmetic instead of 24-byte struct",
        "symptom": "curl_easy_setopt(easy, CURLOPT_URL, \"http://example.com\") crashes with strlen(NULL) inside Curl_vsetopt. va_arg(param, char*) extracts NULL instead of the URL string. Also causes garbled IP address formatting (snprintf va_list path).",
        "root_cause": "zcc-libc/stdarg.h defined: typedef void* va_list; #define va_start(v,l) (v=(void*)((char*)&l + sizeof(l))). This pointer-arithmetic approach assumes variadic args are contiguous on the stack after the last named param. On x86-64 SysV ABI, the first 6 integer args are in registers (rdi,rsi,rdx,rcx,r8,r9), not on the stack. Additionally, zcc_compat.h redefined __builtin_va_list to void*, preventing the correct __builtin_va_start codegen (already implemented in part4.c) from being reached during GCC preprocessing.",
        "fix": "Rewrote zcc-libc/stdarg.h with proper SysV ABI 24-byte va_list struct: typedef struct { unsigned int gp_offset; unsigned int fp_offset; void *overflow_arg_area; void *reg_save_area; } __va_list_tag; typedef __va_list_tag va_list[1]; va_start uses __builtin_va_start (ZCC codegen sets gp_offset=N*8, fp_offset=48+M*16, overflow=16(%rbp), reg_save=save_base(%rbp)). va_arg macro reads from reg_save_area+gp_offset, advances gp_offset+=8. Removed __builtin_va_list=void* from zcc_compat.h.",
        "fix_diff": "--- a/zcc-libc/stdarg.h\n+++ b/zcc-libc/stdarg.h\n-typedef void* va_list;\n-#define va_start(v,l) (v=(void*)((char*)&l + sizeof(l)))\n-#define va_arg(v, l) (*(l*)((v=(void*)((char*)v + sizeof(l))) - sizeof(l)))\n+typedef struct { unsigned int gp_offset; unsigned int fp_offset; void *overflow_arg_area; void *reg_save_area; } __va_list_tag;\n+typedef __va_list_tag va_list[1];\n+#define va_start(v,l) __builtin_va_start(v, l)\n+#define va_arg(v, type) (*(type *)((char *)(v)[0].reg_save_area + ((v)[0].gp_offset += 8, (v)[0].gp_offset - 8)))",
        "file": "zcc-libc/stdarg.h, zcc-libc/zcc_compat.h",
        "function": "N/A (header macros + __builtin_va_start in codegen_call)",
        "line_range": "part4.c:2079-2098",
        "commit": "0f4864d",
        "codebase": "libcurl-8.7.1",
        "category": "abi_calling_convention",
        "severity": "runtime_crash",
        "discovery_method": "GDB: break Curl_vsetopt, info registers shows rdx=va_list_ptr but va_arg reads garbage from stack instead of register save area",
        "verification": "Standalone test: sum_args(3,10,20,30)=60, get_str(0,\"hello world\")=\"hello world\". curl_easy_setopt(URL)=0, curl_easy_strerror(0)=\"No error\"",
        "related_bugs": ["CG-DOOM-005"],
        "tags": ["va_list", "variadic", "sysv_abi", "register_save_area", "x86_64", "calling_convention", "stdarg"]
    }
]

def main():
    # Load existing corpus
    with open(CORPUS_PATH, 'r') as f:
        corpus = json.load(f)
    
    existing_ids = {b.get('id', b.get('bug_id', '')) for b in corpus}
    print(f"Existing corpus: {len(corpus)} bugs")
    print(f"IDs: {sorted(existing_ids)}")
    
    # Add new bugs (skip if already present)
    added = 0
    for bug in CURL_BUGS:
        if bug['id'] not in existing_ids:
            corpus.append(bug)
            added += 1
            print(f"  Added: {bug['id']} — {bug['title']}")
        else:
            print(f"  Skipped (exists): {bug['id']}")
    
    # Write updated corpus
    with open(CORPUS_PATH, 'w') as f:
        json.dump(corpus, f, indent=2)
    
    print(f"\nUpdated corpus: {len(corpus)} bugs ({added} added)")
    
    # Also update the standalone copy
    standalone = "/mnt/h/__DOWNLOADS/selforglinux/zcc-compiler-bug-corpus.json"
    if os.path.exists(standalone):
        with open(standalone, 'w') as f:
            json.dump(corpus, f, indent=2)
        print(f"Updated standalone copy: {standalone}")

if __name__ == '__main__':
    main()
