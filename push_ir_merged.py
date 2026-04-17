#!/usr/bin/env python3
"""Merge ZCC self-compile IR + curl IR and push to HuggingFace."""
import json
import os
from datasets import Dataset
import huggingface_hub

# --- CONFIG ---
TOKEN = os.environ.get("HF_TOKEN", "")
REPO = "zkaedi/zcc-ir-prime-v1"
BASE = "/mnt/h/__DOWNLOADS/selforglinux"

# Login
huggingface_hub.login(token=TOKEN)

# Load existing scored data
existing_file = os.path.join(BASE, "zcc_ir_scored.jsonl")
curl_file = os.path.join(BASE, "curl_ir_scored.jsonl")

rows = []

# Existing ZCC self-compile IR
if os.path.exists(existing_file):
    with open(existing_file) as f:
        for line in f:
            if line.strip():
                row = json.loads(line)
                row["source"] = "zcc-self-compile"
                rows.append(row)
    print(f"Loaded {len(rows)} from zcc_ir_scored.jsonl")

# Curl IR
curl_count = 0
if os.path.exists(curl_file):
    with open(curl_file) as f:
        for line in f:
            if line.strip():
                row = json.loads(line)
                row["source"] = "libcurl-8.7.1"
                rows.append(row)
                curl_count += 1
    print(f"Loaded {curl_count} from curl_ir_scored.jsonl")

print(f"Total: {len(rows)} functions")

# Push
ds = Dataset.from_list(rows)
ds.push_to_hub(REPO)
print("Done.")
