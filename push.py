import os
from huggingface_hub import HfApi

print('Combining datasets...')
with open('zcc_ir_scored.jsonl', 'rb') as f1:
    data1 = f1.read()
with open('sqlite3_scored.jsonl', 'rb') as f2:
    data2 = f2.read()

with open('combined_dataset.jsonl', 'wb') as out:
    out.write(data1)
    if not data1.endswith(b'\n'):
        out.write(b'\n')
    out.write(data2)

lines = sum(1 for _ in open('combined_dataset.jsonl', 'rb'))
print(f'Combined lines: {lines}')

print('Uploading to HuggingFace...')
api = HfApi()
api.upload_file(
    path_or_fileobj='combined_dataset.jsonl',
    path_in_repo='zcc_ir_prime_v2.jsonl',
    repo_id='zkaedi/zcc-ir-prime-v1',
    repo_type='dataset',
    commit_message='v2: add 2306 SQLite functions, IR_MAX_FUNCS=8192, post-va_arg and array fixes'
)
print('Upload complete')
