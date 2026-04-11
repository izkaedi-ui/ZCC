#!/bin/bash
# ============================================================
# ZCC Level 4 — Language Implementation Test Harness
# ============================================================
# Tests: mini-forth, mini-lisp, brainfuck, tinycc
# Usage: ./test_level4.sh /path/to/zcc
# ============================================================

set +e

ZCC="${1:-}"
TOOLS_DIR="$(cd "$(dirname "$0")/tools" && pwd)"
BUILD_DIR="$(mktemp -d /tmp/zcc-level4-XXXXXX)"
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
    echo -e "${CYAN}╔══════════════════════════════════════════════════╗${RESET}"
    echo -e "${CYAN}║${BOLD}  ZCC LEVEL 4 — LANGUAGE IMPLEMENTATION TEST     ${CYAN}║${RESET}"
    echo -e "${CYAN}║${BOLD}          *** THE FINAL BOSS ***                 ${CYAN}║${RESET}"
    echo -e "${CYAN}╚══════════════════════════════════════════════════╝${RESET}"
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
        echo -e "${RED}FAIL${RESET} (output mismatch)"
        diff -u "$gcc_out" "$zcc_out" | head -10
        FAIL=$((FAIL+1))
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
TOOLS="miniforth minilisp bf tinycc"
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
# Test: mini-forth
# ============================================================
echo -e "${BOLD}── mini-forth ──${RESET}"
run_test "miniforth" "echo '2 3 + .'"              "" "forth_add"
run_test "miniforth" "echo '10 3 - .'"             "" "forth_sub"
run_test "miniforth" "echo '6 7 * .'"              "" "forth_mul"
run_test "miniforth" "echo '100 7 / .'"            "" "forth_div"
run_test "miniforth" "echo '17 5 mod .'"           "" "forth_mod"
run_test "miniforth" "echo '3 dup * .'"            "" "forth_dup"
run_test "miniforth" "echo '1 2 swap . .'"         "" "forth_swap"
run_test "miniforth" "echo '1 2 3 rot . . .'"      "" "forth_rot"
run_test "miniforth" "echo '5 3 over . . .'"       "" "forth_over"
run_test "miniforth" "echo '1 2 3 .s'"             "" "forth_dots"
run_test "miniforth" "printf ': square dup * ;\n5 square .\n'" "" "forth_define"
run_test "miniforth" "printf ': double 2 * ;\n: quad double double ;\n3 quad .\n'" "" "forth_compose"
run_test "miniforth" "echo '1 1 = .'"              "" "forth_eq_true"
run_test "miniforth" "echo '1 2 = .'"              "" "forth_eq_false"
run_test "miniforth" "echo '5 0 > if 1 . then'"    "" "forth_if_true"
run_test "miniforth" "echo '0 0 > if 1 . else 2 . then'" "" "forth_if_else"
run_test "miniforth" "echo '72 emit 105 emit cr'"  "" "forth_emit"
run_test "miniforth" "echo '-3 5 + .'"             "" "forth_neg"

# ============================================================
# Test: mini-lisp
# ============================================================
echo -e "${BOLD}── mini-lisp ──${RESET}"
run_test "minilisp" "echo '(+ 2 3)'"               "" "lisp_add"
run_test "minilisp" "echo '(- 10 3)'"              "" "lisp_sub"
run_test "minilisp" "echo '(* 6 7)'"               "" "lisp_mul"
run_test "minilisp" "echo '(/ 100 3)'"             "" "lisp_div"
run_test "minilisp" "echo '(+ (* 2 3) (* 4 5))'"  "" "lisp_nested"
run_test "minilisp" "echo '(if (> 5 3) 1 0)'"     "" "lisp_if_true"
run_test "minilisp" "echo '(if (< 5 3) 1 0)'"     "" "lisp_if_false"
run_test "minilisp" "printf '(define x 10)\n(+ x 5)\n'" "" "lisp_define"
run_test "minilisp" "echo '(cons 1 2)'"            "" "lisp_cons"
run_test "minilisp" "echo '(car (cons 1 2))'"      "" "lisp_car"
run_test "minilisp" "echo '(cdr (cons 1 2))'"      "" "lisp_cdr"
run_test "minilisp" "echo '(list 1 2 3)'"          "" "lisp_list"
run_test "minilisp" "echo '(null? (list))'"        "" "lisp_null"
run_test "minilisp" "echo '(mod 17 5)'"            "" "lisp_mod"
run_test "minilisp" "echo '42'"                    "" "lisp_atom"
run_test "minilisp" "echo '(= 3 3)'"              "" "lisp_eq"

# ============================================================
# Test: brainfuck
# ============================================================
echo -e "${BOLD}── brainfuck ──${RESET}"

# Hello World
HW='++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.'
printf "%s" "$HW" > "$BUILD_DIR/hello.bf"

# Add 2+3 and print as digit
ADD='++>+++<[->+<]>++++++++++++++++++++++++++++++++++++++++++++++++.'
printf "%s" "$ADD" > "$BUILD_DIR/add.bf"

# Print A (65)
PRINTA='+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.'
printf "%s" "$PRINTA" > "$BUILD_DIR/printa.bf"

# Simple loop: print 0-9
DIGITS='+++++++++++++++++++++++++++++++++++++++++++++++++[>++++++++++<-]>+++++++.+.+.+.+.+.+.+.+.+.'
printf "%s" "$DIGITS" > "$BUILD_DIR/digits.bf"

# Zero test
ZERO='[-].'
printf "%s" "$ZERO" > "$BUILD_DIR/zero.bf"

run_test "bf" "" "$BUILD_DIR/hello.bf"     "bf_hello"
run_test "bf" "" "$BUILD_DIR/add.bf"       "bf_add"
run_test "bf" "" "$BUILD_DIR/printa.bf"    "bf_printa"
run_test "bf" "" "$BUILD_DIR/digits.bf"    "bf_digits"
run_test "bf" "" "$BUILD_DIR/zero.bf"      "bf_zero"

# BF from stdin
run_test "bf" "printf '++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.'" "" "bf_stdin"

# ============================================================
# Test: tinycc (compile C -> asm -> binary, run, diff)
# ============================================================
echo -e "${BOLD}── tinycc (compiler-compiles-compiler) ──${RESET}"

# Test programs for tinycc to compile
cat > "$BUILD_DIR/t1.c" << 'CEOF'
int main() {
    printf("%d\n", 42);
    return 0;
}
CEOF

cat > "$BUILD_DIR/t2.c" << 'CEOF'
int main() {
    int x;
    x = 10;
    int y;
    y = 20;
    printf("%d\n", x + y);
    return 0;
}
CEOF

cat > "$BUILD_DIR/t3.c" << 'CEOF'
int main() {
    int x;
    x = 5;
    if (x > 3) {
        printf("yes\n");
    } else {
        printf("no\n");
    }
    return 0;
}
CEOF

cat > "$BUILD_DIR/t4.c" << 'CEOF'
int main() {
    int i;
    i = 0;
    while (i < 5) {
        printf("%d\n", i);
        i = i + 1;
    }
    return 0;
}
CEOF

cat > "$BUILD_DIR/t5.c" << 'CEOF'
int main() {
    int a;
    a = 2 * 3 + 4 * 5;
    printf("%d\n", a);
    return 0;
}
CEOF

# For each test: run gcc-compiled tinycc AND zcc-compiled tinycc on same input
# Both should produce identical assembly
for t in t1 t2 t3 t4 t5; do
    printf "  %-42s" "tinycc/$t"

    # GCC-compiled tinycc generates asm
    "$BUILD_DIR/tinycc_gcc" "$BUILD_DIR/$t.c" > "$BUILD_DIR/${t}_gcc.s" 2>/dev/null
    gcc_rc=$?

    if [ -z "$ZCC" ]; then
        echo -e "${YELLOW}SKIP${RESET}"; SKIP=$((SKIP+1)); continue
    fi

    # ZCC-compiled tinycc generates asm
    "$BUILD_DIR/tinycc_zcc" "$BUILD_DIR/$t.c" > "$BUILD_DIR/${t}_zcc.s" 2>/dev/null
    zcc_rc=$?

    if [ "$gcc_rc" -ne "$zcc_rc" ]; then
        echo -e "${RED}FAIL${RESET} (exit: gcc=$gcc_rc zcc=$zcc_rc)"; FAIL=$((FAIL+1)); continue
    fi

    # Compare assembly output
    if diff -q "$BUILD_DIR/${t}_gcc.s" "$BUILD_DIR/${t}_zcc.s" > /dev/null 2>&1; then
        echo -n -e "${GREEN}ASM-MATCH${RESET}"

        # Bonus: actually assemble and run the generated code
        if gcc -o "$BUILD_DIR/${t}_run" "$BUILD_DIR/${t}_gcc.s" -no-pie 2>/dev/null; then
            run_out=$("$BUILD_DIR/${t}_run" 2>/dev/null)
            echo -e " → output: ${CYAN}$(echo "$run_out" | tr '\n' ' ')${RESET}"
        else
            echo ""
        fi
        PASS=$((PASS+1))
    else
        echo -e "${RED}FAIL${RESET} (asm differs)"
        diff -u "$BUILD_DIR/${t}_gcc.s" "$BUILD_DIR/${t}_zcc.s" | head -10
        FAIL=$((FAIL+1))
    fi
done

# ============================================================
# Summary
# ============================================================
echo ""
echo -e "${CYAN}══════════════════════════════════════════════════${RESET}"
TOTAL=$((PASS + FAIL + SKIP))
echo -e "${BOLD}Results:${RESET}  $TOTAL tests"
echo -e "  ${GREEN}PASS:${RESET}  $PASS"
echo -e "  ${RED}FAIL:${RESET}  $FAIL"
echo -e "  ${YELLOW}SKIP:${RESET}  $SKIP"

if [ "$FAIL" -eq 0 ] && [ "$PASS" -gt 0 ]; then
    echo ""
    echo -e "${GREEN}${BOLD}🔱🔱🔱 LEVEL 4 CLEARED — ZCC COMPILES LANGUAGE IMPLEMENTATIONS 🔱🔱🔱${RESET}"
    echo -e "${CYAN}   ZCC is a real compiler. All 4 levels complete.${RESET}"
    echo ""
    echo -e "${BOLD}   Final Score:${RESET}"
    echo -e "   L1: Core Unix Utilities    ✅"
    echo -e "   L2: Text Processing        ✅"
    echo -e "   L3: System Tools           ✅"
    echo -e "   L4: Language Implementation ✅"
    echo ""
    echo -e "${CYAN}   ZCC compiled a Forth interpreter, a Lisp evaluator,${RESET}"
    echo -e "${CYAN}   a Brainfuck VM, and a C compiler — all producing${RESET}"
    echo -e "${CYAN}   bitwise-identical output to GCC.${RESET}"
elif [ "$FAIL" -eq 0 ]; then
    echo ""
    echo -e "${YELLOW}${BOLD}Dry run OK${RESET}"
else
    echo ""
    echo -e "${RED}${BOLD}Level 4 has failures — the boss fight continues${RESET}"
fi

echo ""
rm -rf "$BUILD_DIR"
