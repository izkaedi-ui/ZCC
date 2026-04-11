import os
import sys

try:
    from llama_cpp import Llama
except ImportError:
    print("[ERROR] llama-cpp-python required. Run: pip install llama-cpp-python")
    sys.exit(1)

# Modify this to match the exact path of your newly downloaded r=16 GGUF
model_path = r"ZKAEDI-MINI-GGUF.gguf"

if not os.path.exists(model_path):
    print(f"\033[91m[WARN] {model_path} not found in root. If testing externally via LM Studio, copy the prompts below. Otherwise, point this script to the exact .gguf path.\033[0m")
    
# The core test: Does the model remember the dense prime-constitutions 
# logic without garbling or repeating phrases endlessly?
test_prompts = [
    {
        "name": "Phase 1: Deep Constraint Extraction",
        "system": "You are the autonomous root orchestrator for the ZKAEDI PRIME matrix. Strictly adhere to conversational limits.",
        "user": "Outline the strict Hamiltonian constraints mapping EVM vulnerability features into the DarkMatterVampiricScaffold. Explain the exact function of the contrastive margin logic within Layer 5."
    },
    {
        "name": "Phase 2: Mathematical Reasoning",
        "system": "You are an elite AI vulnerability engineer fine-tuned on prime-constitutions.",
        "user": "A solidity smart contract executes a delegatecall to an untrusted external proxy inside a loop. Generate the exact energy signature metrics for this pattern, specifying branch density and lyapunov exponent bounds."
    },
    {
        "name": "Phase 3: The Golden Anomaly Trap",
        "system": "You are ZKAEDI.",
        "user": "Initialize the ZCC Dead Code Elimination sequence on a self-hosting block. What are the strict preservation invariants for IR instructions emitting standard I/O streams? Why did ZCC_IR_FLUSH segfault?"
    }
]

def run_diagnostic():
    print(f"\n\033[1;96m[+] LOADING ZKAEDI-MINI-GGUF INTO VRAM...\033[0m")
    try:
        # Load the 4-bit unsloth export 
        llm = Llama(model_path=model_path, n_gpu_layers=-1, n_ctx=4096, verbose=False)
    except Exception as e:
        print(f"\033[91m[ERROR] Failed to load GGUF. Are you running this in Colab or Windows natively? {e}\033[0m")
        return

    print("\033[92m[SUCCESS] Weights loaded. Executing Golden Domain Diagnostics...\033[0m\n")

    for i, test in enumerate(test_prompts):
        print(f"\033[1;33m[{test['name'].upper()}]\033[0m")
        print(f"\033[90mUser: {test['user'][:100]}...\033[0m")
        
        messages = [
            {"role": "system", "content": test["system"]},
            {"role": "user", "content": test["user"]}
        ]
        
        response = llm.create_chat_completion(
            messages=messages,
            temperature=0.2, # Very low temperature to expose greedy memorization loops
            max_tokens=600,
            stop=["<|im_end|>"]
        )
        
        output = response["choices"][0]["message"]["content"].strip()
        print(f"\033[92m[ZKAEDI-MINI-GGUF]:\033[0m {output}\n")
        print("-" * 60 + "\n")

if __name__ == "__main__":
    if os.path.exists(model_path):
        run_diagnostic()
    else:
        # Just print out the prompts so the user can paste them into LM Studio
        print("\033[1;93m=== GOLDEN DOMAIN DIAGNOSTIC PROMPTS ===\033[0m")
        print("Run these natively in your LLM runner UI to test for memory corruption in the r=16 weights.\n")
        for test in test_prompts:
            print(f"[{test['name']}]")
            print(f"System: {test['system']}")
            print(f"User:   {test['user']}\n")
