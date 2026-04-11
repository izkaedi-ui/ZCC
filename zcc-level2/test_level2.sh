#!/bin/bash
# ============================================================
# ZCC Level 2 — Text Processing Tools Test Harness
# ============================================================
# Compiles 5 text tools with GCC (reference) and ZCC,
# runs identical inputs, diffs outputs.
#
# Usage:  ./test_level2.sh /path/to/zcc
# ============================================================

set +e

ZCC="${1:-}"
TOOLS_DIR="$(cd "$(dirname "$0")/tools" && pwd)"
BUILD_DIR="$(mktemp -d /tmp/zcc-level2-XXXXXX)"
PASS=0
FAIL=0
SKIP=0

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

banner() {
    echo ""
    echo -e "${CYAN}╔══════════════════════════════════════════════╗${RESET}"
    echo -e "${CYAN}║${BOLD}  ZCC LEVEL 2 — TEXT PROCESSING TOOLS TEST   ${CYAN}║${RESET}"
    echo -e "${CYAN}╚══════════════════════════════════════════════╝${RESET}"
    echo ""
}

compile_gcc() {
    gcc -std=c89 -Wall -Wextra -o "$2" "$1" 2>/dev/null
}

compile_zcc() {
    [ -z "$ZCC" ] && return 1
    "$ZCC" "$1" -o "$2" 2>/dev/null
}

run_test() {
    local name="$1"
    local input_cmd="$2"
    local args="$3"
    local label="$4"

    local gcc_bin="$BUILD_DIR/${name}_gcc"
    local zcc_bin="$BUILD_DIR/${name}_zcc"
    local gcc_out="$BUILD_DIR/${name}_gcc_${label}.out"
    local zcc_out="$BUILD_DIR/${name}_zcc_${label}.out"
    local gcc_rc zcc_rc

    printf "  %-42s" "$name/$label"

    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$gcc_bin $args" > "$gcc_out" 2>/dev/null
        gcc_rc=$?
    else
        eval "$gcc_bin $args" > "$gcc_out" 2>/dev/null
        gcc_rc=$?
    fi

    if [ -z "$ZCC" ]; then
        echo -e "${YELLOW}SKIP${RESET}"
        SKIP=$((SKIP + 1))
        return
    fi

    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$zcc_bin $args" > "$zcc_out" 2>/dev/null
        zcc_rc=$?
    else
        eval "$zcc_bin $args" > "$zcc_out" 2>/dev/null
        zcc_rc=$?
    fi

    if [ "$gcc_rc" -ne "$zcc_rc" ]; then
        echo -e "${RED}FAIL${RESET} (exit: gcc=$gcc_rc zcc=$zcc_rc)"
        FAIL=$((FAIL + 1))
        return
    fi

    if diff -q "$gcc_out" "$zcc_out" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"
        PASS=$((PASS + 1))
    else
        echo -e "${RED}FAIL${RESET} (output mismatch)"
        diff -u "$gcc_out" "$zcc_out" | head -15
        FAIL=$((FAIL + 1))
    fi
}

# ============================================================
banner

echo -e "${BOLD}Build directory:${RESET} $BUILD_DIR"
if [ -n "$ZCC" ]; then
    echo -e "${BOLD}ZCC binary:${RESET}      $ZCC"
else
    echo -e "${YELLOW}Dry-run mode (GCC only)${RESET}"
fi
echo ""

# Create fixtures
FIX="$BUILD_DIR/fixtures"
mkdir -p "$FIX"

printf "the quick brown fox\njumps over the lazy dog\nthe fox ran away\nhello world\n" > "$FIX/text.txt"
printf "alice:100:admin\nbob:200:user\ncharlie:300:user\ndave:400:admin\n" > "$FIX/csv.txt"
printf "banana\napple\ncherry\napple\ndate\nbanana\ncherry\ncherry\n" > "$FIX/fruits.txt"
printf "aaa\naaa\nbbb\nccc\nccc\nccc\nddd\n" > "$FIX/dupes.txt"
printf "ZKAEDI\x00PRIME\x01\x02\xff\xfe\n\thello\r\nworld" > "$FIX/binary.bin"
printf "one\ntwo\nthree\n" > "$FIX/small.txt"
printf "" > "$FIX/empty.txt"
printf "single\nsingle\nsingle\n" > "$FIX/allsame.txt"
printf "z\na\nm\nb\ny\nc\n" > "$FIX/letters.txt"
printf "hello\tworld\tfoo\nbar\tbaz\tqux\n" > "$FIX/tabs.txt"

# ============================================================
# Compile
# ============================================================
echo -e "${BOLD}Compiling tools...${RESET}"

TOOLS="grep cut sort uniq hexdump"
COMPILE_FAIL=0
for tool in $TOOLS; do
    src="$TOOLS_DIR/${tool}.c"
    printf "  %-12s gcc... " "$tool"
    if compile_gcc "$src" "$BUILD_DIR/${tool}_gcc"; then
        echo -n -e "${GREEN}OK${RESET}"
    else
        echo -n -e "${RED}FAIL${RESET}"
        COMPILE_FAIL=1
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
# Test: grep
# ============================================================
echo -e "${BOLD}── grep ──${RESET}"
run_test "grep" "" "fox $FIX/text.txt"                   "basic"
run_test "grep" "" "the $FIX/text.txt"                   "multi_match"
run_test "grep" "" "NOTFOUND $FIX/text.txt"              "no_match"
run_test "grep" "" "hello $FIX/text.txt $FIX/small.txt"  "multi_file"
run_test "grep" "echo 'find me here'" "me"               "stdin"
run_test "grep" "" "apple $FIX/fruits.txt"               "fruit"

# ============================================================
# Test: cut
# ============================================================
echo -e "${BOLD}── cut ──${RESET}"
run_test "cut" "" "-d: -f1 $FIX/csv.txt"                 "field1"
run_test "cut" "" "-d: -f2 $FIX/csv.txt"                 "field2"
run_test "cut" "" "-d: -f3 $FIX/csv.txt"                 "field3"
run_test "cut" "printf 'a,b,c\nd,e,f\n'" "-d, -f2"      "stdin_comma"
run_test "cut" "" "-f1 $FIX/tabs.txt"                    "tab_default"
run_test "cut" "" "-d: -f9 $FIX/csv.txt"                 "missing_field"

# ============================================================
# Test: sort
# ============================================================
echo -e "${BOLD}── sort ──${RESET}"
run_test "sort" "" "$FIX/fruits.txt"                     "basic"
run_test "sort" "" "-r $FIX/fruits.txt"                  "reverse"
run_test "sort" "" "-u $FIX/fruits.txt"                  "unique"
run_test "sort" "" "$FIX/letters.txt"                    "letters"
run_test "sort" "printf 'z\na\nm\n'" ""                  "stdin"
run_test "sort" "" "$FIX/empty.txt"                      "empty"
run_test "sort" "" "$FIX/allsame.txt"                    "allsame"

# ============================================================
# Test: uniq
# ============================================================
echo -e "${BOLD}── uniq ──${RESET}"
run_test "uniq" "" "$FIX/dupes.txt"                      "basic"
run_test "uniq" "" "-c $FIX/dupes.txt"                   "count"
run_test "uniq" "" "-d $FIX/dupes.txt"                   "dups_only"
run_test "uniq" "" "-u $FIX/dupes.txt"                   "unique_only"
run_test "uniq" "printf 'x\nx\ny\ny\ny\nz\n'" ""        "stdin"
run_test "uniq" "" "$FIX/allsame.txt"                    "allsame"

# ============================================================
# Test: hexdump
# ============================================================
echo -e "${BOLD}── hexdump ──${RESET}"
run_test "hexdump" "" "$FIX/small.txt"                   "text"
run_test "hexdump" "" "$FIX/binary.bin"                  "binary"
run_test "hexdump" "" "$FIX/empty.txt"                   "empty"
run_test "hexdump" "printf 'ABCDEFGHIJKLMNOP'" ""        "stdin_16"
run_test "hexdump" "printf 'short'" ""                   "stdin_short"

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
    echo -e "${GREEN}${BOLD}🔱 LEVEL 2 CLEARED — ZCC handles text processing${RESET}"
    echo -e "${CYAN}   Ready for Level 3: System Tools${RESET}"
elif [ "$FAIL" -eq 0 ]; then
    echo ""
    echo -e "${YELLOW}${BOLD}Dry run OK — re-run with: ./test_level2.sh /path/to/zcc${RESET}"
else
    echo ""
    echo -e "${RED}${BOLD}Level 2 has failures — fix before advancing${RESET}"
fi

echo ""
rm -rf "$BUILD_DIR"
