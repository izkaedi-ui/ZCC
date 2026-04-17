# ZCC Debug Protocol v1.0: AST Memory Deallocation Failure

## Phase 0: Intent
* **Bug Statement**: When orchestrating ZCC compilation natively through WSL (`scripts/use_zcc.sh hello.c`), the GCC linker stage collapses due to an `undefined reference to 'zcc_node_free'` during the final object compilation step.
* **Repro Command**: `wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && make zcc && ./zcc hello.c -o hello.s && gcc -o hello.exe hello.s -lm"`

## Phase 1: Landscape (10+ Failure Modes)
The `zcc_node_free` symbol is attempting to be called but cannot be located by the linker. Here are the 10 structural fault nodes:

| # | Subsystem | Failure Node | Probability |
|---|-----------|--------------|-------------|
| 1 | AST / Memory Arena | `zcc_node_free` is declared in a header but the function definition was deleted or `#ifdef`-ed out in the C implementation. | Highly Likely |
| 2 | Linker / Build System | The `part4.c` or equivalent file containing `zcc_node_free` is not being included in the `make zcc` GCC dependency chain. | Likely |
| 3 | AST / Memory Arena | ZCC transitioned to a pure Arena Allocator (`void-check` invariant) but legacy `zcc_node_free` calls were left behind in the parser teardown logic. | Highly Likely |
| 4 | Codegen | The Code Generator is trying to explicitly free AST nodes mid-compilation after emitting IR, violating the Arena architecture. | Likely |
| 5 | Lexer / Parser | An error-recovery routine calls `zcc_node_free` on an invalid token sequence to prevent memory leaks, but the function doesn't exist. | Unlikely |
| 6 | AST | `zcc_node_free` is formally defined as an `inline` or `static` block in one C-file but trying to be linked globally by another. | Likely |
| 7 | Linker | The WSL linker is trying to resolve a standard `free` alias but a macro redefined it to `zcc_node_free` improperly. | Unlikely |
| 8 | Preprocessor | A compilation flag (e.g., `-DZCC_DEBUG`) is missing, stripping the `zcc_node_free` debugger out of the final compiled source. | Possible |
| 9 | AST / Parser | The root `program` struct teardown loops through child nodes calling `zcc_node_free` rather than just resetting the bump-pointer allocator. | Highly Likely |
| 10 | Garbage Collection | A rogue memory management layer was partially implemented to clean up intermediate IR nodes but abandoned mid-development. | Possible |
