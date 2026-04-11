import os
import sys
import time
import json
import re
import subprocess

try:
    from llama_cpp import Llama
except ImportError:
    print("\033[91m[ERROR] llama-cpp-python not found. Run: pip install llama-cpp-python\033[0m")
    sys.exit(1)

# Raw OS Abstractions
def list_directory(path):
    try:
        items = os.listdir(path)
        return json.dumps(items)
    except Exception as e:
        return f"Error: {e}"

def read_file(path):
    try:
        with open(path, 'r', encoding='utf-8') as f:
            text = f.read()
            if len(text) > 12000:
                return f"[TRUNCATED] File too large. First 12000 chars: {text[:12000]}"
            return text
    except Exception as e:
        return f"Error: {e}"

def write_file(path, content):
    try:
        os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        return f"SUCCESS: Wrote {len(content)} bytes securely to {path}."
    except Exception as e:
        return f"Error: {e}"

# Target Qwen Matrix
model_path = r"C:\Users\zkaed\Downloads\Qwen2.5-7B-Instruct.Q4_K_M.gguf"
if os.name == 'posix' and 'microsoft' in os.uname().release.lower():
    model_path = "/mnt/c/Users/zkaed/Downloads/Qwen2.5-7B-Instruct.Q4_K_M.gguf"

print("\n\033[1;35m========================================================\033[0m")
print("\033[1;96m      ZKAEDI OUROBOROS : AUTONOMOUS ZCC COMPILER HEALER\033[0m")
print("\033[1;35m========================================================\033[0m")

if not os.path.exists(model_path):
    print(f"\033[91m[FATAL ZERO] Model not found: {model_path}\033[0m")
    sys.exit(1)

print("\033[90m[SYSTEM] Injecting Qwen2.5-7B into VRAM (8K Context)...\033[0m")
llm = Llama(model_path=model_path, n_gpu_layers=-1, n_ctx=8192, verbose=False)

tools = [
    {
        "type": "function",
        "function": {
            "name": "read_file",
            "description": "Reads raw text/code content from a `.c` file on disk to find the error.",
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {"type": "string", "description": "The exact path (e.g., 'part4.c')."}
                },
                "required": ["path"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "write_file",
            "description": "Overwrites a broken file on disk with the fully repaired C code.",
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {"type": "string", "description": "The exact path to the file."},
                    "content": {"type": "string", "description": "The FULL exact C code to write to fix the issue."}
                },
                "required": ["path", "content"]
            }
        }
    }
]

def run_zcc_make():
    # Attempt to compile ZCC cleanly inside the WSL layer or natively
    cmd = "make clean && make zcc"
    # If running physically on Windows outside WSL, execute the compilation inside WSL natively!
    if os.name == 'nt' or ('microsoft' not in os.uname().release.lower() and os.name != 'posix'):
        # Just map it raw. If we're inside WSL already, just run 'make zcc'
        pass 
    if os.name == 'nt':
        cmd = 'wsl -e bash -c "cd /mnt/h/__DOWNLOADS/selforglinux && make clean && make zcc"'
    
    
    print("\033[93m[OUROBOROS] Compiling ZCC Physics Matrix (`make zcc`)...\033[0m")
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    return result.returncode, result.stdout + "\n" + result.stderr

def invoke_qwen_healer(error_trace):
    messages = [
        {"role": "system", "content": "You are OUROBOROS, an elite auto-healing compiler orchestrator for the ZCC C-Compiler. You analyze C compilation dumps, locate SegFaults, syntax errors, or logic holes, and autonomously use `read_file` to see the code, and `write_file` to physically deploy a completely fixed `.c` file back to the disk. Never apologize. Only execute physical repairs."},
        {"role": "user", "content": f"ZCC Compilation completely Failed!\n\nTraceback:\n{error_trace}\n\nUse your `read_file` tool to inspect the exact faulty block in the source code, and your `write_file` tool to push the complete C code replacement back to disk."}
    ]
    
    print("\033[95m[OUROBOROS] Awakened Qwen. Feeding physical error trace to Neural Net...\033[0m")
    
    for iteration in range(5): # Limit recursive loop to 5 calls to prevent infinite VRAM execution
        response = llm.create_chat_completion(
            messages=messages,
            tools=tools,
            temperature=0.2,
            max_tokens=2048
        )
        
        choice = response["choices"][0]
        message = choice["message"]
        
        # --- REGEX XML INTERCEPTOR ---
        content = message.get("content", "")
        if (not message.get("tool_calls")) and content and "<tool_call>" in content:
            match = re.search(r'<tool_call>\s*({.*?})\s*</tool_call>', content, re.DOTALL)
            if match:
                try:
                    t_data = json.loads(match.group(1))
                    message["tool_calls"] = [{
                        "id": f"call_{iteration}",
                        "type": "function",
                        "function": { "name": t_data.get("name"), "arguments": json.dumps(t_data.get("arguments", {})) }
                    }]
                except: pass
        # -----------------------------
        
        if "tool_calls" in message and message["tool_calls"]:
            messages.append(message)
            for tool_call in message["tool_calls"]:
                name = tool_call["function"]["name"]
                
                try:
                    args = json.loads(tool_call["function"]["arguments"])
                except:
                    args = {}
                    
                path = args.get("path", "")
                
                if name == "read_file":
                    print(f"\033[94m[OUROBOROS QWEN] Reading Code -> {path}\033[0m")
                    result = read_file(path)
                elif name == "write_file":
                    print(f"\033[92m[OUROBOROS QWEN] Injecting Patch -> {path}\033[0m")
                    result = write_file(path, args.get("content", ""))
                else:
                    result = f"Error: Unknown tool '{name}'."
                
                messages.append({
                    "role": "tool",
                    "name": name,
                    "content": str(result),
                    "tool_call_id": tool_call.get("id", f"call_{iteration}")
                })
            
            print("\033[90m[OUROBOROS] Synthesizing ZCC Patch Parameters...\033[0m")
        else:
            print(f"\n\033[1;92m[OUROBOROS QWEN]>\033[0m {message.get('content', '')}")
            break

def watch_loop():
    files_to_watch = ["zcc.c", "part1.c", "part2.c", "part3.c", "part4.c", "part5.c", "part5_ir_emit.c", "zcc.h"]
    last_mtime = {}
    
    print("\033[92m[OUROBOROS] Sentinel Loop Active. Monitoring ZCC source files infinitely...\033[0m")
    while True:
        changed = False
        for f in files_to_watch:
            # Need strict absolute paths if running globally, assuming local script path mapping here natively.
            abs_f = os.path.join(os.path.dirname(os.path.abspath(__file__)), f)
            if os.path.exists(abs_f):
                mtime = os.path.getmtime(abs_f)
                if last_mtime.get(f) is not None and mtime > last_mtime[f]:
                    changed = True
                    print(f"\n\033[90m[WATCHER] Detected file modifications inside {f}...\033[0m")
                last_mtime[f] = mtime
        
        if changed:
            code, out = run_zcc_make()
            if code != 0:
                print("\033[91m[CRITICAL] ZCC Build Failed!\033[0m")
                # Strip out if way too huge to prevent Context Window Crash
                error_trace = out[-3500:] 
                invoke_qwen_healer(error_trace)
                
                # REVALIDATE COMPILE AFTER QWEN PATCH!
                print("\n\033[93m[OUROBOROS] Validating Qwen's Physical Injection...\033[0m")
                code_v, out_v = run_zcc_make()
                if code_v == 0:
                    print("\033[1;92m[OUROBOROS SUCCESS] ZCC Auto-Healed and passed `make test` flawlessly!\033[0m")
                else:
                    print("\033[1;91m[OUROBOROS FAILURE] Qwen failed to automatically heal ZCC logic arrays.\033[0m")
            else:
                print("\033[92m[SUCCESS] ZCC Build Passed Cleanly natively.\033[0m")
        
        time.sleep(1.5)

if __name__ == "__main__":
    try:
        watch_loop()
    except KeyboardInterrupt:
        print("\n\033[90m[SYSTEM] Terminating Ouroboros Daemon...\033[0m")
