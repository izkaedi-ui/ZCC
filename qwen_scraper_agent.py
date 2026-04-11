import os
import sys
import json
import urllib.request
import urllib.error
from html.parser import HTMLParser

try:
    from llama_cpp import Llama
except ImportError:
    print("[ERROR] llama-cpp-python not found. Run: pip install llama-cpp-python")
    sys.exit(1)

# Zero-Dependency HTML Stripper
class TextExfilParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.text_data = []
        self.in_script = False
    
    def handle_starttag(self, tag, attrs):
        if tag in ['script', 'style', 'nav', 'footer', 'header', 'svg', 'button']:
            self.in_script = True
            
    def handle_endtag(self, tag):
        if tag in ['script', 'style', 'nav', 'footer', 'header', 'svg', 'button']:
            self.in_script = False
            
    def handle_data(self, data):
        if not self.in_script:
            txt = data.strip()
            if txt:
                self.text_data.append(txt)
                
    def get_text(self):
        return ' '.join(self.text_data)

def fetch_url_text(url):
    print(f"\n\033[96m[TOOL INVOCATION] \033[90mScraping physical data from: \033[36m{url}\033[0m")
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) ZKAEDI-OMNI/1.0'})
        with urllib.request.urlopen(req, timeout=10) as response:
            html = response.read().decode('utf-8', errors='ignore')
            parser = TextExfilParser()
            parser.feed(html)
            text = parser.get_text()
            
            # Crop to prevent context window implosion (2048 chars max to leave room for Qwen output)
            snippet = text[:3000]
            print(f"\033[92m[TOOL SUCCESS] \033[90mExtracted {len(text)} bytes. Feeding text to Qwen...\033[0m")
            return snippet
    except Exception as e:
        print(f"\033[91m[TOOL FAILED] \033[90m{e}\033[0m")
        return f"Error scraping {url}: {str(e)}"

# Target Qwen Matrix
model_path = r"C:\Users\zkaed\Downloads\Qwen2.5-7B-Instruct.Q4_K_M.gguf"
if os.name == 'posix' and 'microsoft' in os.uname().release.lower():
    model_path = "/mnt/c/Users/zkaed/Downloads/Qwen2.5-7B-Instruct.Q4_K_M.gguf"

print("\n\033[1;35m========================================================\033[0m")
print("\033[1;96m    ZKAEDI OMNI-AGENT : SMART WEB SCRAPER (QWEN)\033[0m")
print("\033[1;35m========================================================\033[0m")

if not os.path.exists(model_path):
    print(f"[FATAL ZERO] Model not found: {model_path}")
    sys.exit(1)

print("\033[90m[SYSTEM] Injecting Qwen2.5-7B into VRAM (Full Offload)...\033[0m")
llm = Llama(
    model_path=model_path,
    n_gpu_layers=-1,
    n_ctx=4096,   # Expanded context for web reading limits
    verbose=False # Keep terminal clean
)

# Standard OpenAI-Compatible JSON Tool Schema
tools = [{
    "type": "function",
    "function": {
        "name": "scrape_url",
        "description": "Fetches raw text content from a live URL on the internet. Use this to read articles, documentation, or search results.",
        "parameters": {
            "type": "object",
            "properties": {
                "url": {"type": "string", "description": "The exact https:// URL to extract data from"}
            },
            "required": ["url"]
        }
    }
}]

messages = [
    {"role": "system", "content": "You are ZKAEDI PRIME, an elite cyber-intelligence LLM. You have access to a live web scraping tool. If the user asks about something on the internet or provides a URL, you MUST call the `scrape_url` tool to read the page before answering. Do not hallucinate URLs."}
]

print("\033[92m[SYSTEM] Agent ONLINE. Enter your command, or type 'exit'.\033[0m")

while True:
    try:
        user_input = input("\n\033[1;36m[USER]>\033[0m ")
        if user_input.lower() in ['exit', 'quit']: break
        if not user_input.strip(): continue
        
        messages.append({"role": "user", "content": user_input})
        
        print("\033[90m[OMNI-AGENT] Thinking...\033[0m")
        # Generate completion, providing the tools JSON schema
        response = llm.create_chat_completion(
            messages=messages,
            tools=tools,
            temperature=0.3,
            max_tokens=1024
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
        
        # Determine if Qwen wants to call the tool!
        if "tool_calls" in message and message["tool_calls"]:
            for tool_call in message["tool_calls"]:
                if tool_call["function"]["name"] == "scrape_url":
                    # Parse Qwen's tool arguments dynamically
                    args = json.loads(tool_call["function"]["arguments"])
                    target_url = args.get("url")
                    
                    # 1. Execute physical Python tool
                    scraped_text = fetch_url_text(target_url)
                    
                    # 2. Append Qwen's request + the Tool's output to the history
                    messages.append(message)
                    messages.append({
                        "role": "tool",
                        "content": scraped_text,
                        "tool_call_id": tool_call.get("id", "tool_0")
                    })
                    
                    # 3. Call Qwen again with the scraped context injected!
                    print("\033[90m[OMNI-AGENT] Synthesizing physical web intelligence...\033[0m")
                    final_response = llm.create_chat_completion(
                        messages=messages,
                        tools=tools,
                        temperature=0.5,
                        max_tokens=1024
                    )
                    final_msg = final_response["choices"][0]["message"]
                    print("\n\033[1;95m[QWEN]>\033[0m", final_msg.get("content", ""))
                    messages.append(final_msg)
        else:
            # Standard Text Response
            print("\n\033[1;95m[QWEN]>\033[0m", message.get("content", ""))
            messages.append(message)
            
    except KeyboardInterrupt:
        break
    except Exception as e:
        print(f"\n\033[91m[SYSTEM CRASH] {e}\033[0m")
