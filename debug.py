import json
d = json.load(open('cve_corpus/ml_feature_vectors.json'))
print('Class distribution (full scale-out):')
print(f"{'CWE':<12} {'safe':>12} {'vuln':>12} {'total':>8} {'%vuln':>8}")
print('-'*55)
ts,tv=0,0
for cwe,samples in d.items():
    s=sum(1 for x in samples if x['label']==0)
    v=sum(1 for x in samples if x['label']==1)
    ts+=s; tv+=v
    r=v/(s+v) if (s+v) else 0
    print(f"{cwe:<12} {s:>12} {v:>12} {s+v:>8} {r:>8.1%}")
print('-'*55)
if ts+tv > 0:
    print(f"{'TOTAL':<12} {ts:>12} {tv:>12} {ts+tv:>8} {tv/(ts+tv):>8.1%}")
