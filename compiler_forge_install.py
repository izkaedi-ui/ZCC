#!/usr/bin/env python3
"""
ZKAEDI Compiler Forge — Install & Setup
Run this once to install dependencies and print your Cursor config.
"""
import subprocess, sys, shutil, json
from pathlib import Path

SCRIPT_DIR = Path(__file__).parent.resolve()

def check_tool(name):
    path = shutil.which(name)
    status = "✅" if path else "❌"
    print(f"  {status} {name:<12} {path or 'NOT FOUND'}")
    return bool(path)

print("\n🔱 ZKAEDI Compiler Forge — Setup\n")
print("System tools:")
gcc_ok   = check_tool("gcc")
objdump  = check_tool("objdump")
nm       = check_tool("nm")
readelf  = check_tool("readelf")

if not gcc_ok:
    print("\n  Install gcc: sudo apt install build-essential  (Ubuntu/Debian)")
    print("               brew install gcc                  (macOS)")
    sys.exit(1)

print("\nPython deps:")
sys.stdout.flush()
print("  Installing mcp...", end=" ", flush=True)
result = subprocess.run(
    [sys.executable, "-m", "pip", "install", "mcp", "--quiet"],
    capture_output=True, text=True, timeout=120
)
if result.returncode == 0:
    print("✅")
else:
    print("❌")
    print(f"  {result.stderr[:200] if result.stderr else result.stdout or 'pip failed'}")
    sys.exit(1)

# Verify server imports cleanly
result = subprocess.run(
    [sys.executable, "-c", "from compiler_forge_server import mcp; print('OK')"],
    capture_output=True, text=True, cwd=SCRIPT_DIR
)
if "OK" in result.stdout:
    print("  ✅ compiler_forge_server.py imports OK")
else:
    print(f"  ❌ compiler_forge_server error: {result.stderr[:200]}")
    sys.exit(1)

server_path = str(SCRIPT_DIR / "compiler_forge_server.py")

config = {
    "mcpServers": {
        "zkaedi-prime": {
            "url": "https://zkaedi-zkaedi-prime-mcp.hf.space/gradio_api/mcp/"
        },
        "zkaedi-compiler-forge": {
            "command": sys.executable,
            "args": [server_path]
        }
    }
}

print(f"""
✅ Setup complete!

──────────────────────────────────────────────────────────────
Add this to your Cursor MCP config (~/.cursor/mcp.json):
──────────────────────────────────────────────────────────────
{json.dumps(config, indent=2)}
──────────────────────────────────────────────────────────────

Or just run the server manually to test:
  {sys.executable} {server_path}

10 tools available:
  compile_c          — Compile C, get diagnostics
  compile_and_run    — Compile + execute, get stdout/stderr
  disassemble        — Objdump disassembly (Intel syntax)
  diff_assembly      — Diff assembly of two C snippets
  check_abi          — x86-64 System V ABI compliance check
  analyze_codegen    — Codegen analysis across -O levels
  bootstrap_verify   — Multi-stage self-hosting verification
  inspect_binary     — nm/readelf symbol + section inspection
  preprocess         — C preprocessor expansion
  compile_flags_cmp  — Compare assembly under two flag sets
""")
