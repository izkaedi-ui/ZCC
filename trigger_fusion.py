import subprocess
import sys

files = [
    "A_AdamW_seed2.pt", "A_AdamW_seed3.pt", "A_AdamW_seed4.pt", "A_AdamW_seed5.pt",
    "A_AdamW_seed6.pt", "A_AdamW_seed7.pt", "A_AdamW_seed8.pt", "A_AdamW_seed9.pt",
    "A_AdamW_seed10.pt", "A_AdamW_seed11.pt", "A_AdamW_seed12.pt", "A_AdamW_seed13.pt",
    "A_AdamW_seed14.pt", "A_AdamW_seed15.pt", "A_AdamW_seed16.pt", "A_AdamW_seed17.pt",
    "A_AdamW_seed18.pt", "A_AdamW_seed19.pt",
    "A_SGD_seed0.pt", "A_SGD_seed1.pt", "A_SGD_seed2.pt", "A_SGD_seed3.pt",
    "A_SGD_seed4.pt", "A_SGD_seed5.pt", "A_SGD_seed6.pt", "A_SGD_seed7.pt",
    "A_SGD_seed8.pt", "A_SGD_seed9.pt", "A_SGD_seed10.pt", "A_SGD_seed11.pt"
]

cmd = [sys.executable, "zkaedi_prime_fuser.py", "--genetic-soup"] + files + ["ZKAEDI_MASTER_FUSED.pt", "--out", "ZKAEDI_ULTIMA_FUSED.pt", "--steps", "200"]

print(f"Launching Fusion Array on {len(files)} matrices...")
result = subprocess.run(cmd)
print(f"Fusion exit code: {result.returncode}")
