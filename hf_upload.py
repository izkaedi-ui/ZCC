from huggingface_hub import HfApi
from huggingface_hub import CommitOperationAdd
import os

api = HfApi(token="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")

print("Ensuring repo exists...")
api.create_repo('zkaedi/zcc-ir-prime-v1', repo_type='dataset', exist_ok=True)
print("Repo ready.")

# Upload updated compiler source files
files = [
    "part1.c", "part2.c", "part3.c", "part4.c", "part5.c",
    "zcc.c", "ir.c", "ir.h", "ir_bridge.h", "ir_to_x86.c",
    "compiler_passes.c", "compiler_passes_ir.c", "Makefile"
]

ops = []
for f in files:
    if os.path.exists(f):
        ops.append(CommitOperationAdd(path_in_repo=f"compiler/{f}", path_or_fileobj=f))
    else:
        print(f"skip {f} (not found)")

# Also push the scored IR dataset if it exists
if os.path.exists("zcc_ir_scored.jsonl"):
    ops.append(CommitOperationAdd(path_in_repo="data/zcc_ir_scored.jsonl", path_or_fileobj="zcc_ir_scored.jsonl"))

if len(ops) > 0:
    print(f"Uploading {len(ops)} files to Hugging Face...")
    result = api.create_commit(
        repo_id="zkaedi/zcc-ir-prime-v1",
        repo_type="dataset",
        operations=ops,
        commit_message=(
            "🔱 ZCC Apr 11 2026 — switch fallthrough fix, SysV ABI float registers, "
            "SQLite 6-level full pass (CREATE/INSERT/SELECT/JOIN/triggers/LIKE with correct floats), "
            "Lua 5.4.6 fully operational, 53/53 fuzz, self-host verified"
        )
    )
    print(f"Pushed: {result}")
else:
    print("No files to upload.")
