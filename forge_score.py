import sys
import json
import zcc_ir_forge

text = open('curl_ir_raw.txt', 'r', encoding='utf-8', errors='replace').read()
mod = zcc_ir_forge.parse_ir_text(text)
scorer = zcc_ir_forge.PRIMEScorer()
scorer.score_module(mod)

counted = 0
with open('curl_ir_scored.jsonl', 'w', encoding='utf-8') as f:
    for fn in mod.funcs:
        if fn.node_count > 0:
            f.write(json.dumps(fn.to_dict()) + '\n')
            counted += 1

print(f'Scored {counted} functions with >0 nodes.')
