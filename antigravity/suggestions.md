# Suggestions for IR/AST Hybrid Path Optimization

Based on your commit message `IR backend: blacklist 3 high-spill functions`, here are some suggestions grouped into architectural next steps, compiler theory optimizations, and procedural improvements.

## 1. Commit Message Refinement

Your current commit summary is succinct and provides concrete metrics (which is excellent). You might consider adding a bit more context on *why* these specific functions were spilling for future reference:

```text
IR backend: blacklist 3 high-spill functions

Functions `lookup_keyword_fallback`, `parse_stmt`, and `next_token`
exhibited excessive register spilling due to high branch cyclomatic 
complexity and deep nesting. They have been routed to the pure AST 
path.

The IR hybrid now outperforms pure AST on memory ops: 
37794 vs 38816 (-1022)
```

## 2. Analyzing the "High-Spill" Causes

These three functions are classically state-heavy in C compilers (ZCC context):

*   **`parse_stmt`**: Typically involves very deep recursive descent and large `switch` statements over token types, forcing the register allocator to spill heavily across call boundaries.
*   **`lookup_keyword_fallback` / `next_token`**: Lexical analysis functions usually contain tight loops with heavy pointer arithmetic (e.g., advancing through string buffers) and frequent memory access, causing high register pressure.

**Suggestion:** Dump the intermediate representation (and interference graph, if you have one) for these specific routines to see if the spills are caused by *caller-saved register clobbering* around recursive calls or simply running out of physical registers.

## 3. Register Allocator Improvements (Mid-to-Long Term)

If the IR backend is failing on complex functions, the register allocator might be the bottleneck:
*   **Spill Weights:** Are you using a sophisticated heuristic for spill weights (e.g., placing variables used in inner loops into registers, and spilling variables used in outer scopes)?
*   **Live Interval Splitting:** If doing Linear Scan, you might be keeping intervals alive across function calls (`parse_stmt` is heavily recursive). Splitting the intervals at call boundaries might allow the IR backend to handle them.

## 4. Automated Hot/Cold Partitioning (Dynamic Blacklisting)

Instead of hardcoding the blacklist for these three functions:
*   Could you introduce a **spill-threshold** during the IR generation phase? If `spilled_registers > THRESHOLD`, the backend automatically falls back to emitting the AST path for that function. This prevents you from needing to manually manage the blacklist as the compiler evolves.

## 5. Next Optimization Targets

*   **Peephole Optimization over Memory Ops:** Now that you're saving 1022 memory ops, verify if the remaining memory operations are reducible via basic peephole optimizations (e.g., `mov register to stack` followed immediately by `mov stack to register`).
*   **Verifying ABI and Stack Alignment:** Make sure that mixing AST-generated assembly and IR-generated assembly isn't misaligning the stack. Ensure the boundaries between an IR function calling an AST function follow System V ABI perfectly.
