import os
import sys
import glob
from ir_parse import parse_ir
from extract_476_null_check import extract_476
from extract_190_overflow import extract_190
from extract_121_bounds import extract_121
from extract_121_memcpy_bounds import extract_121_memcpy
from extract_415_double_free import extract_415
import json

def process_batch(ir_dump_dir="cve_corpus/ir_dumps"):
    # We will output a JSON file containing the feature vectors for all functions.
    output_data = {
        "CWE476": [],
        "CWE190": [],
        "CWE121": [],
        "CWE416": []
    }
    
    ir_files = [
        os.path.join(ir_dump_dir, f)
        for f in os.listdir(ir_dump_dir)
        if f.endswith('.ir')
    ]
    print(f"Discovered {len(ir_files)} IR dumps for extraction.")
    
    for fpath in ir_files:
        fname = os.path.basename(fpath)
        try:
            mod = parse_ir(fpath)
        except Exception as e:
            print(f"[!] Error parsing {fname}: {e}")
            continue
            
        # Route to appropriate extractor based on file name prefix
        if "CWE476" in fname:
            target_class = "CWE476"
            extractor = extract_476
        elif "CWE190" in fname:
            target_class = "CWE190"
            extractor = extract_190
        elif "CWE121" in fname:
            target_class = "CWE121"
            extractor = extract_121
        elif "CWE416" in fname or "CWE415" in fname:
            target_class = "CWE416"
            extractor = extract_415
        else:
            continue
            
        for func in mod.funcs:
            if func.name in ("main", "printLine", "printHexCharLine",
                              "printIntLine", "printLongLongLine"):
                continue

            # For CWE-121 merge both loop-bound and memcpy-bound extractors
            if target_class == "CWE121":
                feat_loop = extract_121(func)
                feat_memcpy = extract_121_memcpy(func)
                # Merge: overflow if EITHER analysis signals it
                features = {
                    **feat_loop,
                    "memcpy_alloca_min":  feat_memcpy["alloca_min"],
                    "memcpy_max_call":    feat_memcpy["max_call_size"],
                    "memcpy_overflow":    feat_memcpy["overflow"],
                    "overflow": feat_loop["overflow"] or feat_memcpy["overflow"],
                }
            else:
                features = extractor(func)

            # Label derivation from Juliet naming convention:
            #   _bad or trailing _ (CWE-121 module-named fns) or B2G -> VULN (1)
            #   G2B, _good, or no vulnerability pattern found    -> SAFE (0)
            fn_lower = func.name.lower()
            is_vuln = (
                fn_lower.endswith('_bad')
                or 'badsink' in fn_lower    # dispatch pattern: bad sink contains the vuln
                or fn_lower.endswith('_')   # CWE-121: bad fn named after module
                or 'b2g' in fn_lower        # bad source variants
            )
            label = 1 if is_vuln else 0

            # ALWAYS emit a vector. Functions where the extractor finds nothing
            # are the negative class (safe IR). Dropping them kills TN and makes
            # the training set have no negative examples.
            # Only skip structural dispatchers (good/bad call wrappers with 0-2 nodes).
            if len(func.nodes) <= 2 and func.name.endswith(('_good', '_bad')):
                continue  # pure dispatcher, no IR content

            sample = {
                "source_file": fname,
                "function": func.name,
                "label": label,
                "features": features
            }
            output_data[target_class].append(sample)
                
    for cwe, samples in output_data.items():
        print(f"Extracted {len(samples)} valid feature vectors for {cwe}")
        
    with open("cve_corpus/ml_feature_vectors.json", "w") as f:
        json.dump(output_data, f, indent=2)
        
    print("Batch extraction complete -> cve_corpus/ml_feature_vectors.json")

if __name__ == "__main__":
    process_batch()
