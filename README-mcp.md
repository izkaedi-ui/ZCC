# MCP setup (Zkaedi Prime + Compiler Forge)

These MCP tools are powered by **Gemma 7B** trained on our Solidity vulnerability energy-signatures dataset:
- **Model:** [zkaedi/gemma-7b-solidity-energy-signatures](https://hf.co/zkaedi/gemma-7b-solidity-energy-signatures)
- **Dataset:** [zkaedi/solidity-vulnerability-energy-signatures](https://hf.co/datasets/zkaedi/solidity-vulnerability-energy-signatures)

## Quick config

1. **Install Compiler Forge deps and get your config:**
   ```bash
   python compiler_forge_install.py
   ```
   It prints a ready-to-paste `mcpServers` block with the **absolute path** to `compiler_forge_server.py` on your machine.

2. **Add to Cursor MCP config:**
   - Open Cursor → Settings → MCP (or edit `~/.cursor/mcp.json`).
   - Paste the block from step 1, or merge with your existing `mcpServers`.

## Example (you must replace the path)

```json
{
  "mcpServers": {
    "zkaedi-prime": {
      "url": "https://zkaedi-zkaedi-prime-mcp.hf.space/gradio_api/mcp/"
    },
    "zkaedi-compiler-forge": {
      "command": "python",
      "args": ["D:/__DOWNLOADS/selforglinux/compiler_forge_server.py"]
    }
  }
}
```

On Windows use forward slashes or escaped backslashes in the path. Run `python compiler_forge_install.py` to get the exact path for your system.

## .env (Hugging Face token)

If you use the HF MCP and need a token, create `.env` from `.env.example` and set `HF_TOKEN=hf_...`. `.env` is in `.gitignore`. The **Compiler Forge** server does not use the HF token; it runs locally (compile_c, disassemble, etc.).

**Check profile:** from repo root run `python scripts/check_hf_profile.py` — it reads `.env` and prints your HF username (e.g. `OK — authenticated as: zkaedi`).

## Hugging Face Spaces

| Space | Description |
|-------|-------------|
| [ZKAEDI PRIME MCP](https://hf.co/spaces/zkaedi/zkaedi-prime-mcp) | Hamiltonian dynamics for smart contract security (MCP / Cursor). |
| [ZKAEDI PRIME — Solidity Energy Auditor](https://hf.co/spaces/zkaedi/solidity-energy-auditor) | Scan Solidity contracts for vulnerabilities in the browser. |
| [ZKAEDI-CC Error Agent Team](https://hf.co/spaces/zkaedi/zkaedi-cc) | ZCC error agent team. |

**How to utilize them**

- **ZKAEDI PRIME MCP** — Already in use when Cursor talks to the HF MCP: use it for smart-contract security analysis, Solidity reasoning, or any task that benefits from the Gemma model (Hamiltonian / energy-signature angle). No extra step; it’s your MCP server.
- **Solidity Energy Auditor** — Use when you want a quick browser scan without Cursor: paste a contract (or snippet), get vulnerability hints. Good for sharing with others or a second opinion. Same model as the MCP, different UI.
- **ZKAEDI-CC Error Agent Team** — Use for **ZCC compiler bugs**: at the start of the [Debug Protocol](docs/DEBUG_PROTOCOL.md) (Phase 0/1) or when stuck, paste the **error message**, **last crumb** (`tail -n 1 debug.log`), or **minimal repro** into the Space. The error agent team can suggest failure modes, likely phases, or next steps; then continue locally with phases 4–7 (repro, locate, stub, fix, Squasher).
