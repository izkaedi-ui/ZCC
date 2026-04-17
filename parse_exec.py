import json
import statistics

with open('exec_scored.jsonl') as f:
    for line in f:
        d = json.loads(line)
        arr = d['field']
        if not arr: continue
        mean = sum(arr)/len(arr)
        max_v = max(arr)
        min_v = min(arr)
        stdev = statistics.stdev(arr) if len(arr) > 1 else 0
        print(f"Step {d.get('step')} ({d.get('pass_name', 'unk')}): nodes={len(arr)}, max={max_v:.2f}, min={min_v:.2f}, mean={mean:.2f}, span={max_v-min_v:.2f}, stdev={stdev:.2f}")
