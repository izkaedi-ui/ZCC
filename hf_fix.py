import os
from huggingface_hub import HfApi

TOKEN = os.environ.get('HF_TOKEN', "")
api = HfApi(token=TOKEN)

new_readme = """---
license: mit
---
# ZCC Compiler Bug Corpus

A growing dataset of confirmed, ground-truth C compiler codegen bugs, AST traversal faults, and SysV ABI violations discovered during the creation of the ZCC compiler.

## Provenance Codebases (Stress Categories)
1. Baseline Arithmetic
2. Memory Allocation
3. Complex Expressions
4. SQLite 3.45.0
5. DOOM 1.10
6. Lua 5.4.6
7. **libcurl-8.7.1** (Network/IO)
"""
api.upload_file(
    path_or_fileobj=new_readme.encode('utf-8'),
    path_in_repo='README.md',
    repo_id='zkaedi/zcc-compiler-bug-corpus',
    repo_type='dataset'
)
print('Fixed zcc-compiler-bug-corpus README')
