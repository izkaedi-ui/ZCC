## Update: PP-HEADERS-023A closure + PARSER-001 scoping (2026-04-19)

PP-HEADERS-023A committed at `373afc5` with HONEST 8% partial result 
documented in commit body. LLONG_MAX/MIN/ULLONG_MAX pre-registered in 
PPState macro table (not just stddef stub — they fire before any 
#include reaches the stub injector).

Diagnostic probe for residual 22,398 errors isolated PARSER-001: 
parenthesized declarator form `type *(name)(params)` — patterns 1/3/4 
in `tools/battle/paren_decl_probe.c` fail with unexpected-token-86, 
patterns 2/5 pass. Every Lua API function in `lua.h` uses the failing 
form, cascading 130-500+ errors per Lua source file.

Next session: PARSER-001. Starting state is `373afc5`.
