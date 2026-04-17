import os
from ir_parse import parse_ir
from extract_415_double_free import extract_415
from extract_476_null_check import extract_476
from extract_121_bounds import extract_121
from extract_121_memcpy_bounds import extract_121_memcpy

def validate(ir_path):
    print(f"=== Validating {os.path.basename(ir_path)} ===")
    funcs = parse_ir(ir_path)
    if not funcs:
        print(" [!] No functions parsed.")
        return
        
    for func in funcs.funcs:
        if 'socks' in func.name.lower() or 'connect' in func.name.lower():
            f415 = extract_415(func)
            f476 = extract_476(func)
            
            if f415.get('use_after_free') > 0 or f415.get('call_frees') > 0:
                print(f"  [CWE-416/415] {func.name} Vulnerable: {f415}")
            else:
                print(f"  [CWE-416/415] {func.name} SAFE: {f415}")
                
            if f476.get('deref_after_check_vuln') > 0 or f476.get('unguarded_derefs') > 0:
                print(f"  [CWE-476] {func.name} Vulnerable: {f476}")
            else:
                print(f"  [CWE-476] {func.name} SAFE: {f476}")
                
            f121 = extract_121(func)
            f121m = extract_121_memcpy(func)
            f121_merged = {
                **f121,
                "memcpy_alloca_min":  f121m["alloca_min"],
                "memcpy_max_call":    f121m["max_call_size"],
                "memcpy_overflow":    f121m["overflow"],
                "merged_overflow":    f121["overflow"] or f121m["overflow"]
            }
            if f121_merged["merged_overflow"]:
                print(f"  [CWE-121] {func.name} Vulnerable: {f121_merged}")
            else:
                print(f"  [CWE-121] {func.name} SAFE: {f121_merged}")

validate('cve_corpus/cve_ir_dumps/curl_CVE-2023-38545_pre_patch.ir')
validate('cve_corpus/cve_ir_dumps/curl_CVE-2023-38545_post_patch.ir')
