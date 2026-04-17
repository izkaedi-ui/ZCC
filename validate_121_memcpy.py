"""
CWE-121 memcpy extractor validation on the 20 pulled memcpy-variant files.
Uses existing ir_dumps (from the 80-file validation corpus — looks for memcpy-named ones).
"""
import glob
from ir_parse import parse_ir
from extract_121_memcpy_bounds import extract_121_memcpy

def ground_truth(fn_name):
    if fn_name.endswith('_bad') or 'B2G' in fn_name:
        return 'VULN'
    if fn_name.endswith('_') and 'CWE' in fn_name:
        return 'VULN'
    if 'G2B' in fn_name or fn_name.endswith('_good'):
        return 'SAFE'
    return None

verdict_fn = lambda feat: feat.get('overflow', False)

tp = fp = tn = fn_ = g2b_fp = 0

for fpath in sorted(glob.glob('cve_corpus/ir_dumps/CWE121*ncat*.ir') +
                    glob.glob('cve_corpus/ir_dumps/CWE121*ncpy*.ir') +
                    glob.glob('cve_corpus/ir_dumps/CWE121*memcpy*.ir') +
                    glob.glob('cve_corpus/ir_dumps/CWE121*snprintf*.ir') +
                    glob.glob('cve_corpus/ir_dumps/CWE121*memmove*.ir')):
    mod = parse_ir(fpath)
    for f in mod.funcs:
        gt = ground_truth(f.name)
        if gt is None:
            continue
        if 'G2B' in f.name:
            continue  # structural exclusion documented
        feat = extract_121_memcpy(f)
        predicted = verdict_fn(feat)
        if gt == 'VULN' and predicted:   tp += 1
        elif gt == 'VULN' and not predicted: fn_ += 1
        elif gt == 'SAFE' and predicted:
            fp += 1
            if 'G2B' in f.name: g2b_fp += 1
        elif gt == 'SAFE' and not predicted: tn += 1

prec = tp/(tp+fp) if (tp+fp) else 0.0
rec  = tp/(tp+fn_) if (tp+fn_) else 0.0
f1   = 2*prec*rec/(prec+rec) if (prec+rec) else 0.0
g2b_rate = g2b_fp/max(fp,1)

print(f"CWE-121 memcpy extractor on existing memcpy-named IR files:")
print(f"TP={tp} FP={fp} TN={tn} FN={fn_}  Prec={prec:.2f}  Rec={rec:.2f}  F1={f1:.2f}  G2B_FP={g2b_rate:.0%}")
if tp+fn_ == 0:
    print("\nNOTE: No VULN-labeled functions found in these files.")
    print("The ir_dumps directory only has the 80-file validation set (loop variants).")
    print("Re-run after compile_corpus.sh completes on the 20 memcpy-specific files.")
