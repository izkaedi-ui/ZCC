import sys
with open('miniz.c', 'rb') as f:
    text = f.read()

start = text.find(b'#define TINFL_HUFF_DECODE')
end = text.find(b'MZ_MACRO_END', start)
if start != -1 and end != -1:
    length = end + len(b'MZ_MACRO_END') - start
    print("Length of TINFL_HUFF_DECODE:", length)
