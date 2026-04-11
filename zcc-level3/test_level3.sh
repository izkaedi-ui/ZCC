#!/bin/bash
# ============================================================
# ZCC Level 3 — System Tools Test Harness
# ============================================================
# Tests: calc, mini-make, mini-sh, ar-lite
# Usage: ./test_level3.sh /path/to/zcc
# ============================================================

set +e

ZCC="${1:-}"
TOOLS_DIR="$(cd "$(dirname "$0")/tools" && pwd)"
BUILD_DIR="$(mktemp -d /tmp/zcc-level3-XXXXXX)"
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
    echo -e "${CYAN}║${BOLD}     ZCC LEVEL 3 — SYSTEM TOOLS TEST         ${CYAN}║${RESET}"
    echo -e "${CYAN}╚══════════════════════════════════════════════╝${RESET}"
    echo ""
}

compile_gcc() { gcc -std=c89 -Wall -Wextra -o "$2" "$1" 2>/dev/null; }
compile_zcc() { [ -z "$ZCC" ] && return 1; "$ZCC" "$1" -o "$2" 2>/dev/null; }

run_test() {
    local name="$1" input_cmd="$2" args="$3" label="$4"
    local gcc_bin="$BUILD_DIR/${name}_gcc"
    local zcc_bin="$BUILD_DIR/${name}_zcc"
    local gcc_out="$BUILD_DIR/${label}_gcc.out"
    local zcc_out="$BUILD_DIR/${label}_zcc.out"
    local gcc_rc zcc_rc

    printf "  %-42s" "$name/$label"

    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$gcc_bin $args" > "$gcc_out" 2>/dev/null; gcc_rc=$?
    else
        eval "$gcc_bin $args" > "$gcc_out" 2>/dev/null; gcc_rc=$?
    fi

    if [ -z "$ZCC" ]; then
        echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1)); return
    fi

    if [ -n "$input_cmd" ]; then
        eval "$input_cmd" | eval "$zcc_bin $args" > "$zcc_out" 2>/dev/null; zcc_rc=$?
    else
        eval "$zcc_bin $args" > "$zcc_out" 2>/dev/null; zcc_rc=$?
    fi

    if [ "$gcc_rc" -ne "$zcc_rc" ]; then
        echo -e "${RED}FAIL${RESET} (exit: gcc=$gcc_rc zcc=$zcc_rc)"; FAIL=$((FAIL+1)); return
    fi

    if diff -q "$gcc_out" "$zcc_out" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET} (output mismatch)"; diff -u "$gcc_out" "$zcc_out" | head -10; FAIL=$((FAIL+1))
    fi
}

# ============================================================
banner

echo -e "${BOLD}Build directory:${RESET} $BUILD_DIR"
[ -n "$ZCC" ] && echo -e "${BOLD}ZCC binary:${RESET}      $ZCC" || echo -e "${YELLOW}Dry-run mode${RESET}"
echo ""

# ============================================================
# Compile
# ============================================================
echo -e "${BOLD}Compiling tools...${RESET}"
TOOLS="calc minimake minish arlite"
for tool in $TOOLS; do
    src="$TOOLS_DIR/${tool}.c"
    printf "  %-12s gcc... " "$tool"
    if compile_gcc "$src" "$BUILD_DIR/${tool}_gcc"; then
        echo -n -e "${GREEN}OK${RESET}"
    else
        echo -n -e "${RED}FAIL${RESET}"
    fi
    if [ -n "$ZCC" ]; then
        printf "  zcc... "
        if compile_zcc "$src" "$BUILD_DIR/${tool}_zcc"; then
            echo -e "${GREEN}OK${RESET}"
        else
            echo -e "${RED}FAIL${RESET} (compilation failed)"; FAIL=$((FAIL+1))
        fi
    else
        echo ""
    fi
done
echo ""

# ============================================================
# Test: calc
# ============================================================
echo -e "${BOLD}── calc ──${RESET}"
run_test "calc" "echo '2 + 3'"           "" "calc_add"
run_test "calc" "echo '10 - 7'"          "" "calc_sub"
run_test "calc" "echo '6 * 7'"           "" "calc_mul"
run_test "calc" "echo '100 / 3'"         "" "calc_div"
run_test "calc" "echo '17 % 5'"          "" "calc_mod"
run_test "calc" "echo '(2 + 3) * 4'"     "" "calc_paren"
run_test "calc" "echo '((1 + 2) * (3 + 4))'" "" "calc_nested"
run_test "calc" "echo '-5 + 3'"          "" "calc_neg"
run_test "calc" "printf '2 + 2\n3 * 3\n'" "" "calc_multi"
run_test "calc" "printf 'a = 10\nb = 20\na + b\n'" "" "calc_vars"
run_test "calc" "echo '1 + 2 * 3'"       "" "calc_prec"
run_test "calc" "echo '2 * 3 + 4 * 5'"   "" "calc_prec2"

# ============================================================
# Test: mini-make
# ============================================================
echo -e "${BOLD}── mini-make ──${RESET}"

# Create test makefiles
MK="$BUILD_DIR/mktest"
mkdir -p "$MK"

# Test 1: simple target with echo command
cat > "$MK/Makefile1" << 'MKEOF'
hello:
	echo "hello from make"
MKEOF

# Test 2: target with deps
cat > "$MK/Makefile2" << 'MKEOF'
all: step1
	echo "all done"
step1:
	echo "step1 done"
MKEOF

# Test 3: multiple commands
cat > "$MK/Makefile3" << 'MKEOF'
build:
	echo "compiling..."
	echo "linking..."
	echo "done"
MKEOF

# Test 4: target already exists (create the file first)
touch "$MK/existing_target"
cat > "$MK/Makefile4" << 'MKEOF'
existing_target:
	echo "should not run"
MKEOF

run_test "minimake" "" "-f $MK/Makefile1"        "make_simple"
run_test "minimake" "" "-f $MK/Makefile2"         "make_deps"
run_test "minimake" "" "-f $MK/Makefile3"         "make_multicmd"
run_test "minimake" "" "-f $MK/Makefile4"         "make_exists"

# ============================================================
# Test: mini-sh
# ============================================================
echo -e "${BOLD}── mini-sh ──${RESET}"

run_test "minish" "" "-c 'echo hello world'"                     "sh_echo"
run_test "minish" "" "-c 'echo one; echo two; echo three'"       "sh_chain"
run_test "minish" "" "-c 'echo hello; echo goodbye'"             "sh_multi"
run_test "minish" "printf 'echo line1\necho line2\n'" ""         "sh_stdin"
run_test "minish" "" "-c 'set a=42; get a'"                      "sh_vars"
run_test "minish" "" "-c 'exit 0'"                               "sh_exit0"
run_test "minish" "" "-c 'echo test; exit 3'"                    "sh_exit3"

# ============================================================
# Test: ar-lite
# ============================================================
echo -e "${BOLD}── ar-lite ──${RESET}"

AR="$BUILD_DIR/artest"
mkdir -p "$AR"

echo "Hello, World!" > "$AR/file1.txt"
echo "ZKAEDI PRIME" > "$AR/file2.txt"
printf "binary\x00data\x01\x02\xff" > "$AR/file3.bin"

# Test create + list (gcc)
printf "  %-42s" "arlite/create_list"
"$BUILD_DIR/arlite_gcc" c "$AR/test_gcc.ar" "$AR/file1.txt" "$AR/file2.txt" "$AR/file3.bin" > /dev/null 2>&1
"$BUILD_DIR/arlite_gcc" t "$AR/test_gcc.ar" > "$AR/list_gcc.out" 2>/dev/null

if [ -n "$ZCC" ]; then
    "$BUILD_DIR/arlite_zcc" c "$AR/test_zcc.ar" "$AR/file1.txt" "$AR/file2.txt" "$AR/file3.bin" > /dev/null 2>&1
    "$BUILD_DIR/arlite_zcc" t "$AR/test_zcc.ar" > "$AR/list_zcc.out" 2>/dev/null

    if diff -q "$AR/list_gcc.out" "$AR/list_zcc.out" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET} (list mismatch)"; FAIL=$((FAIL+1))
    fi
else
    echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1))
fi

# Test create + extract roundtrip (gcc creates, zcc extracts and vice versa)
printf "  %-42s" "arlite/roundtrip_gcc_create"
mkdir -p "$AR/ext_gcc" "$AR/ext_zcc"

cd "$AR/ext_gcc"
"$BUILD_DIR/arlite_gcc" x "$AR/test_gcc.ar" > /dev/null 2>&1
gcc_match=1
diff -q "$AR/file1.txt" "$AR/ext_gcc/file1.txt" > /dev/null 2>&1 || gcc_match=0
diff -q "$AR/file2.txt" "$AR/ext_gcc/file2.txt" > /dev/null 2>&1 || gcc_match=0

if [ "$gcc_match" -eq 1 ]; then
    echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
else
    echo -e "${RED}FAIL${RESET} (extract mismatch)"; FAIL=$((FAIL+1))
fi

printf "  %-42s" "arlite/roundtrip_zcc"
if [ -n "$ZCC" ]; then
    cd "$AR/ext_zcc"
    "$BUILD_DIR/arlite_zcc" x "$AR/test_zcc.ar" > /dev/null 2>&1
    zcc_match=1
    diff -q "$AR/file1.txt" "$AR/ext_zcc/file1.txt" > /dev/null 2>&1 || zcc_match=0
    diff -q "$AR/file2.txt" "$AR/ext_zcc/file2.txt" > /dev/null 2>&1 || zcc_match=0

    if [ "$zcc_match" -eq 1 ]; then
        echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET} (extract mismatch)"; FAIL=$((FAIL+1))
    fi
else
    echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1))
fi

# Test: archives are binary-identical between gcc and zcc
printf "  %-42s" "arlite/archive_identical"
if [ -n "$ZCC" ]; then
    if diff -q "$AR/test_gcc.ar" "$AR/test_zcc.ar" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET} (archives differ)"; FAIL=$((FAIL+1))
    fi
else
    echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1))
fi

# Test empty archive
printf "  %-42s" "arlite/empty_list"
"$BUILD_DIR/arlite_gcc" c "$AR/empty_gcc.ar" > /dev/null 2>&1
"$BUILD_DIR/arlite_gcc" t "$AR/empty_gcc.ar" > "$AR/elist_gcc.out" 2>/dev/null
if [ -n "$ZCC" ]; then
    "$BUILD_DIR/arlite_zcc" c "$AR/empty_zcc.ar" > /dev/null 2>&1
    "$BUILD_DIR/arlite_zcc" t "$AR/empty_zcc.ar" > "$AR/elist_zcc.out" 2>/dev/null
    if diff -q "$AR/elist_gcc.out" "$AR/elist_zcc.out" > /dev/null 2>&1; then
        echo -e "${GREEN}PASS${RESET}"; PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET}"; FAIL=$((FAIL+1))
    fi
else
    echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1))
fi

cd /tmp

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
    echo -e "${GREEN}${BOLD}🔱 LEVEL 3 CLEARED — ZCC compiles system tools${RESET}"
    echo -e "${CYAN}   Ready for Level 4: Language Implementation${RESET}"
elif [ "$FAIL" -eq 0 ]; then
    echo ""
    echo -e "${YELLOW}${BOLD}Dry run OK${RESET}"
else
    echo ""
    echo -e "${RED}${BOLD}Level 3 has failures — fix before advancing${RESET}"
fi

echo ""
rm -rf "$BUILD_DIR"
