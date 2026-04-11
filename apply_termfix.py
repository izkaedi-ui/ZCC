#!/usr/bin/env python3
"""
apply_termfix.py — ZCC CG-IR-005 patch template

Fixes applied:
  [1] ir_asm_emit_one_block: fall-through fixup after PGO reordering
  [2] ZND_IF then_body: skip OP_BR when body already terminates with OP_RET
  [3] ZND_IF else_body: same guard for else branch

USAGE:
    python3 apply_termfix.py --check   # detect whether fix is applied
    python3 apply_termfix.py --apply   # apply all fixes (bottom-to-top line order)
    python3 apply_termfix.py --repr N  # dump repr() of lines near line N for debugging

METHODOLOGY (learned the hard way across two sessions):
  ✓ Detect by short, unique SENTINEL strings — whitespace-immune
  ✓ Apply by line-number SPLICE (lines[start:end] = new) — always works
  ✗ Never string-match exact source — tabs/spaces kill it every time
  ✗ Never apply top-to-bottom splices on the same file — line numbers drift
     → Always apply BOTTOM-TO-TOP (highest line number first)
"""

import sys
import os

FILENAME = 'compiler_passes.c'

# ---------------------------------------------------------------------------
# SENTINELS — short, unique strings that exist only after the fix is applied
# ---------------------------------------------------------------------------
SENTINELS = [
    'op==OP_RET||_tt->op==OP_BR',   # ZND_IF then guard
    'op==OP_RET||_et->op==OP_BR',   # ZND_IF else guard
    'CG-IR-005 fall-through',        # ir_asm_emit_one_block fixup
]

# ---------------------------------------------------------------------------
# FIX DEFINITIONS — (label, find_sentinels_in_block, new_lines)
# Each fix provides strip-patterns to LOCATE the splice point, then new_lines.
# Applied bottom-to-top to avoid line drift.
# ---------------------------------------------------------------------------

FIX_FALLTHROUGH = {
    'label': 'Fix 1: ir_asm_emit_one_block fall-through fixup',
    'sentinel': 'CG-IR-005 fall-through',
    # Locate by finding 'ir_asm_lower_insn(ctx, ins, bid);\n    }\n}' 
    # in ir_asm_emit_one_block (first occurrence in the file)
    'find_stripped': [
        'ir_asm_lower_insn(ctx, ins, bid);',
        '}',
        '}',
    ],
    # Must appear together with the for-loop that processes ins
    'context_strip': 'for (; ins; ins = ins->next)',
    'new_suffix': [
        '    /* CG-IR-005 fall-through fixup: if the last instruction in this block is\n',
        '       not a terminator (OP_BR/OP_CONDBR/OP_RET), emit an explicit jmp to the\n',
        '       first successor.  After PGO reordering, empty merge blocks and phi-only\n',
        '       blocks can land anywhere in the emission order; without this they fall\n',
        '       through into whatever block happens to be next, causing infinite loops. */\n',
        '    {\n',
        '        Instr *tail = blk->tail;\n',
        '        while (tail && tail->op == OP_NOP) tail = tail->prev;\n',
        '        int need_jmp = 1;\n',
        '        if (tail) {\n',
        '            Opcode op = tail->op;\n',
        '            if (op == OP_BR || op == OP_CONDBR || op == OP_RET) need_jmp = 0;\n',
        '        }\n',
        '        if (need_jmp && blk->n_succs >= 1) {\n',
        '            BlockID succ0 = blk->succs[0];\n',
        '            ir_asm_emit_phi_edge_copy(ctx, bid, succ0);\n',
        '            fprintf(f, "    jmp .Lir_b_%d_%u\\n", ctx->func_label_id, (unsigned)succ0);\n',
        '        }\n',
        '    }\n',
    ],
}

# The then/else guards are pattern-matched by stripping, replacing 5-line blocks
THEN_GUARD_LINES = [
    '                 { Block *_tb=fn->blocks[ctx->cur_block]; Instr *_tt=_tb?_tb->tail:NULL;\n',
    '                   if(!(_tt&&(_tt->op==OP_RET||_tt->op==OP_BR||_tt->op==OP_CONDBR))){\n',
    '                     Instr *br_then=calloc(1,sizeof(Instr));\n',
    '                     br_then->id=ctx->next_instr_id++; br_then->op=OP_BR; br_then->src[0]=merge_blk; br_then->n_src=1; br_then->exec_freq=1.0;\n',
    '                     emit_instr(ctx,br_then);\n',
    '                     fn->blocks[ctx->cur_block]->succs[0]=merge_blk; fn->blocks[ctx->cur_block]->n_succs=1;\n',
    '                     fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++]=ctx->cur_block; } }\n',
]

ELSE_GUARD_LINES = [
    '                 { Block *_eb=fn->blocks[ctx->cur_block]; Instr *_et=_eb?_eb->tail:NULL;\n',
    '                   if(!(_et&&(_et->op==OP_RET||_et->op==OP_BR||_et->op==OP_CONDBR))){\n',
    '                     Instr *br_else=calloc(1,sizeof(Instr));\n',
    '                     br_else->id=ctx->next_instr_id++; br_else->op=OP_BR; br_else->src[0]=merge_blk; br_else->n_src=1; br_else->exec_freq=1.0;\n',
    '                     emit_instr(ctx,br_else);\n',
    '                     fn->blocks[ctx->cur_block]->succs[0]=merge_blk; fn->blocks[ctx->cur_block]->n_succs=1;\n',
    '                     fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++]=ctx->cur_block; } }\n',
]

# What to look for (strip-matched) and replace with the guard lines:
THEN_OLD_STRIPS = [
    'Instr *br_then = calloc(1, sizeof(Instr));',
    'br_then->id = ctx->next_instr_id++;',
    'emit_instr(ctx, br_then);',
    'fn->blocks[then_blk]->succs[0] = merge_blk;',
    'fn->blocks[merge_blk]->preds[',
]
ELSE_OLD_STRIPS = [
    'Instr *br_else = calloc(1, sizeof(Instr));',
    'br_else->id = ctx->next_instr_id++;',
    'emit_instr(ctx, br_else);',
    'fn->blocks[else_blk]->succs[0] = merge_blk;',
    'fn->blocks[merge_blk]->preds[',
]


# ---------------------------------------------------------------------------
# CORE HELPERS
# ---------------------------------------------------------------------------

def load():
    with open(FILENAME, 'r') as f:
        return f.readlines()

def save(lines):
    with open(FILENAME, 'w') as f:
        f.writelines(lines)

def check_sentinels(lines=None):
    if lines is None:
        lines = load()
    src = ''.join(lines)
    found = [(s, s in src) for s in SENTINELS]
    return found

def strip_match_block(lines, patterns, start=0):
    """Find first occurrence of N consecutive lines whose .strip() contains each pattern.
    Returns 0-indexed start line index or -1."""
    n = len(patterns)
    for i in range(start, len(lines) - n + 1):
        if all(patterns[j] in lines[i + j].strip() for j in range(n)):
            return i
    return -1

def repr_lines(lineno_1indexed, count=10):
    lines = load()
    start = max(0, lineno_1indexed - 1)
    end = min(start + count, len(lines))
    for i in range(start, end):
        print(f'  L{i+1}: {repr(lines[i])}')


# ---------------------------------------------------------------------------
# CHECK MODE
# ---------------------------------------------------------------------------

def do_check():
    found = check_sentinels()
    n_found = sum(1 for _, ok in found)
    total = len(found)
    print(f'{FILENAME}: {n_found}/{total} fix markers present')
    for s, ok in found:
        print(f'  {"✓" if ok else "✗"} {s!r}')
    if n_found == total:
        print('Status: FULLY APPLIED ✓')
    elif n_found == 0:
        print('Status: NOT APPLIED — run with --apply')
    else:
        print('Status: PARTIAL — inspect manually')
    return n_found == total


# ---------------------------------------------------------------------------
# APPLY MODE
# ---------------------------------------------------------------------------

def apply_then_else_guards(lines):
    """Strip-match and replace br_then / br_else 5-line blocks with guarded versions.
    Multiple occurrences of each pattern are replaced (there are 2x then, 1x else).
    Returns (new_lines, then_count, else_count).
    """
    new_lines = []
    i = 0
    then_count = 0
    else_count = 0
    while i < len(lines):
        # Try then-match
        if i + len(THEN_OLD_STRIPS) <= len(lines):
            if all(THEN_OLD_STRIPS[j] in lines[i + j].strip()
                   for j in range(len(THEN_OLD_STRIPS))):
                new_lines.extend(THEN_GUARD_LINES)
                i += len(THEN_OLD_STRIPS)
                then_count += 1
                continue
        # Try else-match
        if i + len(ELSE_OLD_STRIPS) <= len(lines):
            if all(ELSE_OLD_STRIPS[j] in lines[i + j].strip()
                   for j in range(len(ELSE_OLD_STRIPS))):
                new_lines.extend(ELSE_GUARD_LINES)
                i += len(ELSE_OLD_STRIPS)
                else_count += 1
                continue
        new_lines.append(lines[i])
        i += 1
    return new_lines, then_count, else_count


def apply_fallthrough_fix(lines):
    """Find the closing of ir_asm_emit_one_block's for-loop and insert the fixup.
    Locates: ir_asm_lower_insn line, then the '}' of the for-loop, then '}' of function.
    Inserts the fixup block between the for-loop close and function close.
    Returns (new_lines, applied_bool).
    """
    # Find: '    ir_asm_lower_insn(ctx, ins, bid);\n    }\n}\n'
    # but only inside ir_asm_emit_one_block (first occurrence after function def)
    func_def_line = -1
    for i, l in enumerate(lines):
        if 'ir_asm_emit_one_block' in l and '{' in l:
            func_def_line = i
            break
    if func_def_line == -1:
        return lines, False

    # From func_def_line, find ir_asm_lower_insn
    for i in range(func_def_line, min(func_def_line + 200, len(lines))):
        if 'ir_asm_lower_insn(ctx, ins, bid);' in lines[i]:
            # Next non-blank line should be '}' (for-loop), then '}' (function)
            j = i + 1
            if (j < len(lines) and lines[j].strip() == '}' and
                    j + 1 < len(lines) and lines[j + 1].strip() == '}'):
                # Insert fixup between j (for-loop close) and j+1 (function close)
                new_lines = lines[:j + 1] + ['\n'] + FIX_FALLTHROUGH['new_suffix'] + lines[j + 1:]
                return new_lines, True
    return lines, False


def do_apply():
    lines = load()

    # Check what's already applied
    sentinels = check_sentinels(lines)
    already = {s for s, ok in sentinels if ok}

    # Fix 2 & 3: then/else guards (apply first — they're in lower line numbers)
    # Then fix 1: fall-through (higher numbers, and we go bottom-to-top by applying last)
    # But since apply_then_else_guards does a full sweep and apply_fallthrough_fix
    # works on line numbers AFTER the then/else splice (which is in a different region),
    # order here is: then/else sweep first, then fallthrough splice.

    if 'op==OP_RET||_tt->op==OP_BR' not in already:
        new_lines, tc, ec = apply_then_else_guards(lines)
        print(f'  ZND_IF guards: replaced {tc} br_then and {ec} br_else blocks')
        lines = new_lines
    else:
        print('  ZND_IF guards: already present, skipping')

    if 'CG-IR-005 fall-through' not in already:
        new_lines, ok = apply_fallthrough_fix(lines)
        if ok:
            print('  Fall-through fixup: inserted')
            lines = new_lines
        else:
            print('  Fall-through fixup: FAILED to locate insertion point — insert manually')
    else:
        print('  Fall-through fixup: already present, skipping')

    save(lines)

    # Verify
    print()
    all_ok = do_check()
    if not all_ok:
        print()
        print('WARNING: Not all markers found after apply. Check the file manually.')
        print('Tip: run  python3 apply_termfix.py --repr <lineno>  to inspect exact bytes.')
    return all_ok


# ---------------------------------------------------------------------------
# MAIN
# ---------------------------------------------------------------------------

if __name__ == '__main__':
    mode = sys.argv[1] if len(sys.argv) > 1 else '--check'

    if not os.path.exists(FILENAME):
        print(f'Error: {FILENAME} not found. Run from the selforglinux repo root.')
        sys.exit(1)

    if mode == '--check':
        ok = do_check()
        sys.exit(0 if ok else 1)
    elif mode == '--apply':
        ok = do_apply()
        sys.exit(0 if ok else 1)
    elif mode == '--repr' and len(sys.argv) > 2:
        repr_lines(int(sys.argv[2]))
    else:
        print(__doc__)
        sys.exit(1)
