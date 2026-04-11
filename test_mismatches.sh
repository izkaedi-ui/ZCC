for f in fuzz_results_gods_eye/mismatches/*.c; do
  gcc -O0 "$f" -o tmp_gcc
  ./tmp_gcc > out_gcc.txt
  ./zcc2 "$f" -o tmp_zcc.s > /dev/null
  gcc -O0 tmp_zcc.s -o tmp_zcc -lm
  ./tmp_zcc > out_zcc.txt
  diff out_gcc.txt out_zcc.txt > /dev/null || echo "FAIL $f"
done
