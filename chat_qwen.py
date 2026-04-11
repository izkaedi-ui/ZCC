import os
import sys

try:
    from llama_cpp import Llama
except ImportError:
    print("\033[91m[ERROR] llama-cpp-python not found. Run: pip install llama-cpp-python\033[0m")
    sys.exit(1)

# Map the Windows file path intelligently whether you run this in CMD, Powershell or WSL
model_path = r"C:\Users\zkaed\Downloads\Qwen2.5-7B-Instruct.Q4_K_M.gguf"
if os.name == 'posix' and 'microsoft' in os.uname().release.lower():
    model_path = "/mnt/c/Users/zkaed/Downloads/Qwen2.5-7B-Instruct.Q4_K_M.gguf"

if not os.path.exists(model_path):
    print(f"\033[91m[FATAL] Model not found: {model_path}\033[0m")
    sys.exit(1)

print(f"\033[90m[SYSTEM] Injecting Qwen2.5-7B into VRAM (Smart Loading)...\033[0m")

# SMART RESOURCE ALLOCATION
# n_gpu_layers=-1 pushes the entire model into your NVIDIA GPU VRAM (fastest)
# n_ctx=8192 provides an 8,192 token conversation memory window
llm = Llama(
    model_path=model_path, 
    n_gpu_layers=-1, 
    n_ctx=8192, 
    verbose=False
)

# Start the conversation state
messages = [
    {"role": "system", "content": "You are a highly intelligent, bare-metal AI assistant named Qwen. You live inside the user's terminal. Provide concise, incredibly sharp, and helpful responses."}
]

print("\033[1;35m========================================================\033[0m")
print("\033[1;92m   QWEN 2.5 SECURE LINK ESTABLISHED. Type 'exit' to quit.\033[0m")
print("\033[1;35m========================================================\033[0m\n")

while True:
    try:
        # 1. Get user input
        user_input = input("\033[1;96m[YOU]>\033[0m ")
        if not user_input.strip():
            continue
            
        if user_input.strip().lower() in ["exit", "quit", "q"]:
            print("\033[90m[SYSTEM] Terminating connection...\033[0m")
            break
            
        # 2. Add your input to the conversation history
        messages.append({"role": "user", "content": user_input})
        
        print("\033[1;95m[QWEN]>\033[0m ", end="", flush=True)
        
        # 3. Stream the generation directly to the terminal (The ChatGPT effect)
        response_stream = llm.create_chat_completion(
            messages=messages,
            stream=True,
            temperature=0.7,
            max_tokens=4096
        )
        
        full_reply = ""
        for chunk in response_stream:
            if "choices" in chunk and len(chunk["choices"]) > 0:
                delta = chunk["choices"][0].get("delta", {})
                if "content" in delta:
                    token = delta["content"]
                    print(token, end="", flush=True)
                    full_reply += token
                    
        print("\n") # New line after the response is completely finished streaming
        
        # 4. Save Qwen's response to the memory bank so it remembers the context
        messages.append({"role": "assistant", "content": full_reply})
        
    except KeyboardInterrupt:
        print("\n\033[90m[SYSTEM] Link severed by user (Ctrl+C).\033[0m")
        break
    except Exception as e:
        print(f"\n\033[91m[ERROR]\033[0m {e}")
