#!/usr/bin/env bash
# gatebuild.sh — Atomic marker-check + build + selfhost gate
#
# Usage:
#   ./gatebuild.sh FILE SENTINEL [SENTINEL...] -- [MAKE_TARGET]
#
# Examples:
#   ./gatebuild.sh compiler_passes.c "op==OP_RET" "need_jmp" "CG-IR-005"
#   ./gatebuild.sh compiler_passes.c "op==OP_RET" "CG-IR-005" -- ir-verify
#
# Exit codes:
#   0  GATE PASS — markers found, build succeeded, selfhost verified
#   1  GATE FAIL — missing markers, build error, or selfhost mismatch
#
# Add to Makefile:
#   make-gate: ./gatebuild.sh compiler_passes.c "op==OP_RET" "CG-IR-005"

set -euo pipefail

FILE=""
MARKERS=()
EXTRA_TARGET=""

# Parse args: everything before '--' is FILE + MARKERS; after '--' is extra target
parsing_markers=true
for arg in "$@"; do
    if [[ "$arg" == "--" ]]; then
        parsing_markers=false
        continue
    fi
    if $parsing_markers; then
        if [[ -z "$FILE" ]]; then
            FILE="$arg"
        else
            MARKERS+=("$arg")
        fi
    else
        EXTRA_TARGET="$arg"
    fi
done

if [[ -z "$FILE" ]]; then
    echo "Usage: $0 FILE SENTINEL [SENTINEL...] [-- EXTRA_TARGET]"
    exit 1
fi

if [[ ! -f "$FILE" ]]; then
    echo "GATE FAIL: $FILE not found"
    exit 1
fi

# --- Check markers ---
FOUND=0
for m in "${MARKERS[@]:-}"; do
    if grep -q "$m" "$FILE" 2>/dev/null; then
        echo "  ✓ $m"
        ((FOUND++)) || true
    else
        echo "  ✗ $m  ← MISSING"
    fi
done

TOTAL=${#MARKERS[@]:-0}
echo "$FOUND/$TOTAL markers in $FILE"

if [[ $TOTAL -gt 0 && $FOUND -ne $TOTAL ]]; then
    echo "GATE FAIL: missing markers — run apply_termfix.py --apply first"
    exit 1
fi

# --- Build ---
echo ""
echo "=== make zcc_full ==="
if ! make zcc_full; then
    echo "GATE FAIL: build error"
    exit 1
fi

# --- Selfhost ---
echo ""
echo "=== make selfhost (timeout 90s) ==="
if ! timeout 90 make selfhost; then
    echo "GATE FAIL: selfhost failed or timed out"
    exit 1
fi

# --- Optional extra target ---
if [[ -n "$EXTRA_TARGET" ]]; then
    echo ""
    echo "=== make $EXTRA_TARGET ==="
    if ! make "$EXTRA_TARGET"; then
        echo "GATE FAIL: $EXTRA_TARGET failed"
        exit 1
    fi
fi

echo ""
echo "GATE PASS ✓"
exit 0
