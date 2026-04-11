import os
import sys

# Attempt dynamic load of llama_cpp
try:
    from llama_cpp import Llama
except ImportError:
    print("[ERROR] llama-cpp-python not found in this environment. Run: pip install llama-cpp-python")
    sys.exit(1)

# Map Qwen 2.5 exactly as configured in Phase 12 ZKAEDI pool_sync.py
model_path = r"C:\Users\zkaed\Downloads\Qwen2.5-7B-Instruct.Q4_K_M.gguf"
if os.name == 'posix' and 'microsoft' in os.uname().release.lower():
    model_path = "/mnt/c/Users/zkaed/Downloads/Qwen2.5-7B-Instruct.Q4_K_M.gguf"

print(f"==================================================")
print(f" [ZKAEDI LLM DIAGNOSTIC PROBE] ")
print(f"==================================================")
print(f"Targeting Weights: {model_path}")

if not os.path.exists(model_path):
    print(f"[FATAL ZERO] The weight matrices do not exist at {model_path}!")
    sys.exit(1)

try:
    print("[MAPPING GPU] Spinning up llama_cpp with full n_gpu_layers offloading...")
    llm = Llama(
        model_path=model_path,
        n_gpu_layers=-1, # Target the RTX 5070 natively
        n_ctx=2048,
        verbose=False
    )
    print("\n[VITAL SIGNS LOCKED] Memory loaded successfully. Firing synthetic prompt...\n")
    
    raw_prompt = "<|im_start|>system\nYou are ZKAEDI PRIME, a cyber-intelligence matrix.<|im_end|>\n<|im_start|>user\nGive me a 1 sentence status report.<|im_end|>\n<|im_start|>assistant\n"
    
    output = llm(
        prompt=raw_prompt,
        max_tokens=64,
        temperature=0.7,
        stop=["<|im_end|>"]
    )
    
    print("\n<<< QWEN 2.5 (ZKAEDI PRIME) RESPONSE >>>")
    print(output['choices'][0]['text'].strip())
    print("<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>\n")
    print("[PROBE COMPLETE] LLM is fully operational and capable of real-time telemetry inference.")
    
except Exception as e:
    print(f"\n[CATASTROPHIC FAILURE] {e}")
