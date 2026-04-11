#!/bin/bash
# test_pp.sh
PP="./zcc --pp-only"

echo "Test 1:"
echo 'int main(){return 0;}' > /tmp/test1.c
$PP /tmp/test1.c
echo ""

echo "Test 2:"
printf '#define X 42\nint main(){return X;}\n' > /tmp/test2.c
$PP /tmp/test2.c
echo ""

echo "Test 4:"
printf '#define FOO\n#ifdef FOO\nint yes;\n#endif\n' > /tmp/test4.c
$PP /tmp/test4.c
echo ""

echo "Test 5:"
printf '#ifndef BAR\nint yes;\n#endif\n' > /tmp/test5.c
$PP /tmp/test5.c
echo ""

echo "Test 7:"
printf '#define A\n#ifdef A\n#ifndef B\nint ok;\n#endif\n#endif\n' > /tmp/test7.c
$PP /tmp/test7.c
echo ""

echo "Test 8:"
printf '#define X 1\n#undef X\n#ifdef X\nbad\n#else\nint ok;\n#endif\n' > /tmp/test8.c
$PP /tmp/test8.c
echo ""
echo "Test 6:"
echo '#define HELLO 1' > /tmp/test_inc.h
printf '#include "/tmp/test_inc.h"\nint x = HELLO;\n' > /tmp/test6.c
$PP /tmp/test6.c
echo ""

echo "Test 9:"
printf '#include <stdio.h>\nint x = EOF;\n' > /tmp/test9.c
$PP /tmp/test9.c
echo ""
