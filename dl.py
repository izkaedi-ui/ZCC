from huggingface_hub import snapshot_download
import sys

try:
    snapshot_download(repo_id="zkaedi/solidity-prime-tools", repo_type="dataset", local_dir="./zcc_restore")
    print("Download complete")
except Exception as e:
    print(f"Error: {e}")
    sys.exit(1)
