import os
import sys
import json

try:
    from llama_cpp import Llama
except ImportError:
    print("[ERROR] llama-cpp-python not found. Run: pip install llama-cpp-python")
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
print("\033[1;96m    ZKAEDI OMNI-AGENT : AUTONOMOUS FILE NAVIGATOR\033[0m")
print("\033[1;35m========================================================\033[0m")

if not os.path.exists(model_path):
    print(f"[FATAL ZERO] Model not found: {model_path}")
    sys.exit(1)

print("\033[90m[SYSTEM] Injecting Qwen2.5-7B into VRAM (8K Context)...\033[0m")
llm = Llama(
    model_path=model_path,
    n_gpu_layers=-1,
    n_ctx=8192,   # 8K Context for heavy C files and codebase reads
    verbose=False
)

tools = [
    {
        "type": "function",
        "function": {
            "name": "list_directory",
            "description": "Lists all files and directories in the specified path.",
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {"type": "string", "description": "The directory path (e.g., '.' for current)."}
                },
                "required": ["path"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "read_file",
            "description": "Reads raw text content from a file on the physical disk.",
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {"type": "string", "description": "The exact path to the file."}
                },
                "required": ["path"]
            }
        }
    },
    {
        "type": "function",
        "function": {
            "name": "write_file",
            "description": "Overwrites a file on the disk with the provided text content.",
            "parameters": {
                "type": "object",
                "properties": {
                    "path": {"type": "string", "description": "The exact path to the file."},
                    "content": {"type": "string", "description": "The full exact text/code to write to the file."}
                },
                "required": ["path", "content"]
            }
        }
    }
]

messages = [
    {"role": "system", "content": "You are ZKAEDI PRIME, an elite cyber-intelligence LLM acting as an Autonomous Pair Programmer. You have access to list directories, read physical files, and write code back to disk autonomously. NEVER write code blocks to the user if you can just use `write_file` to physically save it to their drive."}
]

print("\033[92m[SYSTEM] Agent ONLINE. Enter your command, or type 'exit'.\033[0m")

while True:
    try:
        user_input = input("\n\033[1;36m[USER]>\033[0m ")
        if user_input.lower() in ['exit', 'quit']: break
        if not user_input.strip(): continue
        
        messages.append({"role": "user", "content": user_input})
        
        print("\033[90m[OMNI-CODER] Thinking...\033[0m")
        # Generate initial completion
        response = llm.create_chat_completion(
            messages=messages,
            tools=tools,
            temperature=0.2,
            max_tokens=2048
        )
        
        choice = response["choices"][0]
        message = choice["message"]
        
        # --- REGEX XML INTERCEPTOR ---
        import re
        content = message.get("content", "")
        if (not message.get("tool_calls")) and content and "<tool_call>" in content:
            match = re.search(r'<tool_call>\s*({.*?})\s*</tool_call>', content, re.DOTALL)
            if match:
                try:
                    t_data = json.loads(match.group(1))
                    message["tool_calls"] = [{
                        "id": "call_fallback",
                        "type": "function",
                        "function": { "name": t_data.get("name"), "arguments": json.dumps(t_data.get("arguments", {})) }
                    }]
                except: pass
        # -----------------------------
        
        # Recursive Tool Loop (Allows Qwen to chain multiple requests logically)
        while "tool_calls" in message and message["tool_calls"]:
            messages.append(message)
            
            for tool_call in message["tool_calls"]:
                name = tool_call["function"]["name"]
                
                try:
                    args = json.loads(tool_call["function"]["arguments"])
                except:
                    args = {}
                    
                path = args.get("path", "")
                
                if name == "list_directory":
                    print(f"\033[93m[TOOL EXECUTING] \033[90mlist_directory('{path}')\033[0m")
                    result = list_directory(path if path else ".")
                elif name == "read_file":
                    print(f"\033[93m[TOOL EXECUTING] \033[90mread_file('{path}')\033[0m")
                    result = read_file(path)
                elif name == "write_file":
                    print(f"\033[95m[TOOL INJECTING] \033[90mWriting code physically to -> {path}\033[0m")
                    result = write_file(path, args.get("content", ""))
                else:
                    result = f"Error: Unknown tool '{name}'."
                
                # Append the execution result back into the context
                messages.append({
                    "role": "tool",
                    "name": name,
                    "content": str(result),
                    "tool_call_id": tool_call.get("id", "tool_0")
                })
            
            print("\033[90m[OMNI-CODER] Synthesizing Sub-Systems & Loop Control...\033[0m")
            # Loop again (Qwen might throw ANOTHER tool call based on what it just read!)
            response = llm.create_chat_completion(
                messages=messages,
                tools=tools,
                temperature=0.2,
                max_tokens=2048
            )
            choice = response["choices"][0]
            message = choice["message"]
            
        print("\n\033[1;92m[QWEN CODER]>\033[0m", message.get("content", ""))
        messages.append(message)
            
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(f"\n\033[91m[SYSTEM CRASH] {e}\033[0m")
