
## CG-001: Float Initializer Type Coercion Missing (April 14, 2026)
Float variable initialization with literals caused silent truncation.
Fix: Added ensure_type() call in parse_decl() initializer path.

## CG-002: Hardcoded Double-Precision SSE Suffixes (April 14, 2026)
Math operations used addsd/mulsd regardless of operand type.
Fix: Dynamic SSE suffix selection based on operand size.

## CG-003: Variadic Float Promotion Not Automatic (April 14, 2026)
printf with float arguments required manual double cast.
Fix: Auto-insert ND_CAST for float args to variadic functions.

## CG-004: ABI Parameter Type Promotion Failure (April 15, 2026)
Integer literals passed to double parameters caused System V ABI violation.
Fix: Auto-insert ND_CAST when arg type != param type in part3.c.
Impact: Lua 5.4.6 VM fully operational, math.sqrt works correctly.
