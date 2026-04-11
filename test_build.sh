#!/bin/bash
python3 -c "
with open('zkaedi_core_fixed.c') as f1, open('test_c_inference.c') as f2:
    with open('test_zcc_in.c', 'w') as out:
        out.write(f1.read())
        out.write(f2.read().replace('#include \"zkaedi_core_fixed.c\"', ''))
"
./zcc test_zcc_in.c -o test_c_inference_zcc.s
if [ $? -ne 0 ]; then
    echo "ZCC Compilation Failed"
    exit 1
fi
gcc test_c_inference_zcc.s zkaedi_weights.s -o test_c_inference_zcc
if [ $? -ne 0 ]; then
    echo "GCC Linking Failed"
    exit 1
fi
./test_c_inference_zcc
