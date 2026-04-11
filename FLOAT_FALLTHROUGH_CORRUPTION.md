# ZCC Switch Fallthrough Bit-Manipulation Corruption (Resolved)

## The Symptom
During execution of SQLite via ZCC, the bound floating-point parameter `3.14` was serialized correctly by `sqlite3_bind_double` but invariably became corrupted upon retrieval through `sqlite3_column_double` or any serialization pathway. 

Specifically, `3.14` (Hex: `40 09 1E B8 51 EB 85 1F`) was being restored as `-0.000000` (Hex: `96 62 A3 60 00 00 85 1F`). The lowest two bytes (`85 1F`) were perfectly intact, but the upper 6 bytes were utterly corrupt, containing either adjacent stack garbage or `00`.

## Root Cause Analysis
The issue lay in `sqlite3_zcc.c` at `sqlite3VdbeExec` inside `OP_MakeRecord`. 
SQLite packs integers and floats dynamically, serializing them down to big-endian byte streams using a heavily unrolled and fallthrough-dependent switch statement:

```c
len = sqlite3SmallTypeSizes[serial_type]; // For a double, len == 8
switch( len ){
  default: zPayload[7] = (u8)(v&0xff); v >>= 8;
           zPayload[6] = (u8)(v&0xff); v >>= 8;
  case 6: zPayload[5] = (u8)(v&0xff); v >>= 8;
           zPayload[4] = (u8)(v&0xff); v >>= 8;
  case 4: zPayload[3] = (u8)(v&0xff); v >>= 8;
  case 3: zPayload[2] = (u8)(v&0xff); v >>= 8;
  case 2: zPayload[1] = (u8)(v&0xff); v >>= 8;
  case 1: zPayload[0] = (u8)(v&0xff);
}
```

### The AST Parser Failure
In ZCC's original parsing mechanism (`part3.c`), the `TK_SWITCH` parser manually captured all statements until it encountered a token identifying a branch (e.g. `TK_CASE` or `TK_DEFAULT`). It explicitly extracted the `TK_CASE` blocks and threw them into an unlinked array `sw->cases[i]`. The `TK_DEFAULT` case was held entirely separate.

When ZCC's backend `part4.c` generated the codegen, it did so by iterating over the `cases` array manually:
```c
for (i = 0; i < ncase; i++) {
    codegen_stmt(cc, node->cases[i]->case_body);
}
if (node->default_case) codegen_stmt(cc, node->default_case->case_body);
```
**This completely disconnected standard C fallthrough between differing sections of a switch matrix.** 
If `default` was located at the *top* of the C source structure, it was forcefully re-ordered in the generated assembly line to be *at the very end of the `switch` block*, with an automatic exit branch out of the statement following its execution.

As a result, `OP_MakeRecord` for `len=8` jumped to `default:`, processed `zPayload[7]` and `zPayload[6]`, and then *completely bypassed* `case 6:` down to `case 1:`. The bytes `[0]` through `[5]` were never set, leaving raw heap garbage intact in those lower payload slots, resulting in corrupted double precision reconstitution.

## Resolution
The ZCC frontend (`part3.c`) was refactored to treat `TK_CASE` and `TK_DEFAULT` purely as standalone syntactic labels attaching to the immediately following statement subtree. The `NK_SWITCH` node ceased intercepting execution paths into fragmented out-of-band blocks, and instead natively delegates parsing to the global `parse_stmt` context, wrapping the whole switch scope dynamically.

The backend (`part4.c`) dispatch engine was recalibrated to only emit the GOTO-based conditional stub chains (the `cmpq` sequence), and rely cleanly upon standard recursive traversal `codegen_stmt(cc, node->body)` to emit the underlying linear AST block stream seamlessly. 

This flawlessly recreated the unadulterated linear memory execution, fully restoring inter-case System V fallthrough. As verified, SQLite correctly unpacked `3.140000` via accurate 64-bit shifting inside System V ABI boundaries.
