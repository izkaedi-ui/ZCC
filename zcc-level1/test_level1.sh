#!/bin/bash
# ============================================================
# ZCC Level 1 — Real-World Tool Compilation Test Harness
# ============================================================
# Compiles 6 Unix utilities with both GCC (reference) and ZCC,
# runs identical inputs, diffs outputs. Green = ZCC matches GCC.
#
# Usage:
#   ./test_level1.sh /path/to/zcc
#
# If no ZCC path given, compiles sources only with GCC as a
# dry-run to verify the test fixtures themselves are correct.
# ============================================================

set -e

ZCC="${1:-}"
TOOLS_DIR="$(cd "$(dirname "$0")/tools" && pwd)"
BUILD_DIR="$(mktemp -d /tmp/zcc-level1-XXXXXX)"
PASS=0
FAIL=0
SKIP=0

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

banner() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════╗${RESET}"
    echo -e "${CYAN}║${BOLD}   ZCC LEVEL 1 — CORE UNIX UTILITIES TEST    ${CYAN}║${RESET}"
    echo -e "${CYAN}╚══════════════════════════════════════════════╝${RESET}"
    echo ""
}

compile_gcc() {
    local src="$1"
    local out="$2"
    gcc -std=c89 -Wall -Wextra -o "$out" "$src" 2>/dev/null
}

compile_zcc() {
    local src="$1"
    local out="$2"
    if [ -z "$ZCC" ]; then
        return 1
    fi
    "$ZCC" "$src" -o "$out" 2>/dev/null
}

run_test() {
    local name="$1"
    local desc="$2"
    local input_cmd="$3"     # command to generate stdin (empty = no stdin)
    local gcc_args="$4"      # args for the gcc-compiled binary
    local label="$5"         # sub-test label

    local gcc_bin="$BUILD_DIR/${name}_gcc"
    local zcc_bin="$BUILD_DIR/${name}_zcc"
    local gcc_out="$BUILD_DIR/${name}_gcc_${label}.out"
    local zcc_out="$BUILD_DIR/${name}_zcc_${label}.out"
    local gcc_rc zcc_rc

    printf "  %-42s" "$name/$label"

    # Run GCC version
    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$gcc_bin $gcc_args" > "$gcc_out" 2>&1
        gcc_rc=$?
    else
        eval "$gcc_bin $gcc_args" > "$gcc_out" 2>&1
        gcc_rc=$?
    fi

    if [ -z "$ZCC" ]; then
        echo -e "${YELLOW}SKIP${RESET} (no ZCC binary)"
        SKIP=$((SKIP + 1))
        return
    fi

    # Run ZCC version
    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$zcc_bin $gcc_args" > "$zcc_out" 2>&1
        zcc_rc=$?
    else
        eval "$zcc_bin $gcc_args" > "$zcc_out" 2>&1
        zcc_rc=$?
    fi

    # Compare
    if [ "$gcc_rc" -ne "$zcc_rc" ]; then
        echo -e "${RED}FAIL${RESET} (exit code: gcc=$gcc_rc zcc=$zcc_rc)"
        FAIL=$((FAIL + 1))
        return
    fi

    if diff -q "$gcc_out" "$zcc_out" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${RESET} (output mismatch)"
        diff -u "$gcc_out" "$zcc_out" | head -20
        FAIL=$((FAIL + 1))
    fi
}

# ============================================================
banner

echo -e "${BOLD}Build directory:${RESET} $BUILD_DIR"
if [ -n "$ZCC" ]; then
    echo -e "${BOLD}ZCC binary:${RESET}      $ZCC"
else
    echo -e "${YELLOW}No ZCC binary specified — dry-run mode (GCC only)${RESET}"
fi
echo ""

# Create test fixtures
FIXTURE_DIR="$BUILD_DIR/fixtures"
mkdir -p "$FIXTURE_DIR"

echo "Hello, world!" > "$FIXTURE_DIR/hello.txt"
printf "one\ntwo\nthree\nfour\nfive\n" > "$FIXTURE_DIR/lines.txt"
printf "the quick brown fox\njumps over\nthe lazy dog\n" > "$FIXTURE_DIR/words.txt"
printf "" > "$FIXTURE_DIR/empty.txt"
printf "no newline at end" > "$FIXTURE_DIR/nonewline.txt"
cat "$FIXTURE_DIR/hello.txt" "$FIXTURE_DIR/lines.txt" > "$FIXTURE_DIR/combined.txt"

# ============================================================
# Compile all tools
# ============================================================
echo -e "${BOLD}Compiling tools...${RESET}"

TOOLS="echo cat wc yes true false"
for tool in $TOOLS; do
    src="$TOOLS_DIR/${tool}.c"
    printf "  %-12s gcc... " "$tool"
    if compile_gcc "$src" "$BUILD_DIR/${tool}_gcc"; then
        echo -n -e "${GREEN}OK${RESET}"
    else
        echo -n -e "${RED}FAIL${RESET}"
        FAIL=$((FAIL + 1))
    fi

    if [ -n "$ZCC" ]; then
        printf "  zcc... "
        if compile_zcc "$src" "$BUILD_DIR/${tool}_zcc"; then
            echo -e "${GREEN}OK${RESET}"
        else
            echo -e "${RED}FAIL${RESET} (compilation failed)"
            FAIL=$((FAIL + 1))
        fi
    else
        echo ""
    fi
done

echo ""

# ============================================================
# Test: true
# ============================================================
echo -e "${BOLD}── true ──${RESET}"
printf "  %-42s" "true/exit_code"
"$BUILD_DIR/true_gcc"; gcc_rc=$?
if [ -n "$ZCC" ]; then
    "$BUILD_DIR/true_zcc"; zcc_rc=$?
    if [ "$gcc_rc" -eq 0 ] && [ "$zcc_rc" -eq 0 ]; then
        echo -e "${GREEN}PASS${RESET}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${RESET} (gcc=$gcc_rc zcc=$zcc_rc)"
        FAIL=$((FAIL + 1))
    fi
else
    if [ "$gcc_rc" -eq 0 ]; then
        echo -e "${YELLOW}SKIP${RESET} (gcc=$gcc_rc)"
    fi
    SKIP=$((SKIP + 1))
fi

# ============================================================
# Test: false
# ============================================================
echo -e "${BOLD}── false ──${RESET}"
printf "  %-42s" "false/exit_code"
"$BUILD_DIR/false_gcc" || gcc_rc=$?
if [ -n "$ZCC" ]; then
    "$BUILD_DIR/false_zcc" || zcc_rc=$?
    if [ "$gcc_rc" -eq 1 ] && [ "$zcc_rc" -eq 1 ]; then
        echo -e "${GREEN}PASS${RESET}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${RESET} (gcc=$gcc_rc zcc=$zcc_rc)"
        FAIL=$((FAIL + 1))
    fi
else
    echo -e "${YELLOW}SKIP${RESET}"
    SKIP=$((SKIP + 1))
fi

# ============================================================
# Test: echo
# ============================================================
echo -e "${BOLD}── echo ──${RESET}"
run_test "echo" "basic args"        "" "hello world"              "basic"
run_test "echo" "no args"           "" ""                         "noargs"
run_test "echo" "-n flag"           "" "-n no newline"            "nflag"
run_test "echo" "special chars"     "" "'hello   world'"          "spaces"
run_test "echo" "many args"         "" "a b c d e f g h"         "many"

# ============================================================
# Test: cat
# ============================================================
echo -e "${BOLD}── cat ──${RESET}"
run_test "cat" "single file"        "" "$FIXTURE_DIR/hello.txt"   "single"
run_test "cat" "multiple files"     "" "$FIXTURE_DIR/hello.txt $FIXTURE_DIR/lines.txt" "multi"
run_test "cat" "empty file"         "" "$FIXTURE_DIR/empty.txt"   "empty"
run_test "cat" "no newline at end"  "" "$FIXTURE_DIR/nonewline.txt" "nonl"
run_test "cat" "stdin pipe"         "echo piped_input" ""          "stdin"

# ============================================================
# Test: wc
# ============================================================
echo -e "${BOLD}── wc ──${RESET}"
run_test "wc" "single file"         "" "$FIXTURE_DIR/words.txt"   "single"
run_test "wc" "multi file + total"  "" "$FIXTURE_DIR/hello.txt $FIXTURE_DIR/lines.txt" "multi"
run_test "wc" "empty file"          "" "$FIXTURE_DIR/empty.txt"   "empty"
run_test "wc" "stdin"               "echo hello world" ""         "stdin"
run_test "wc" "no trailing newline" "" "$FIXTURE_DIR/nonewline.txt" "nonl"

# ============================================================
# Test: yes
# ============================================================
echo -e "${BOLD}── yes ──${RESET}"
run_test "yes" "default y"          "" ""                         "default"
run_test "yes" "custom string"      "" "ZKAEDI"                   "custom"
run_test "yes" "multi word"         "" "hello world"              "multi"

# ============================================================
# Summary
# ============================================================
echo ""
echo -e "${CYAN}══════════════════════════════════════════════${RESET}"
TOTAL=$((PASS + FAIL + SKIP))
echo -e "${BOLD}Results:${RESET}  $TOTAL tests"
echo -e "  ${GREEN}PASS:${RESET}  $PASS"
echo -e "  ${RED}FAIL:${RESET}  $FAIL"
echo -e "  ${YELLOW}SKIP:${RESET}  $SKIP"

if [ "$FAIL" -eq 0 ] && [ "$PASS" -gt 0 ]; then
    echo ""
    echo -e "${GREEN}${BOLD}🔱 LEVEL 1 CLEARED — ZCC compiles real Unix tools${RESET}"
    echo -e "${CYAN}   Ready for Level 2: Text Processing${RESET}"
elif [ "$FAIL" -eq 0 ]; then
    echo ""
    echo -e "${YELLOW}${BOLD}Dry run complete — all GCC baselines verified${RESET}"
    echo -e "${CYAN}   Re-run with: ./test_level1.sh /path/to/zcc${RESET}"
else
    echo ""
    echo -e "${RED}${BOLD}Level 1 has failures — fix before advancing${RESET}"
fi

echo ""

# Cleanup
rm -rf "$BUILD_DIR"
