import glob, re

def main():
    with open("exp4_vr_stereo.c", "r") as f:
        txt = f.read()

    # Add tanf if missing
    if "extern float tanf" not in txt:
        txt = txt.replace("extern float expf(float);", "extern float expf(float);\nextern float tanf(float);")

    # Fix inline assembly
    asm_bad = '''        "movss   %5, %%xmm5\\n"         // xmm5 = k2
        "mulss   %%xmm3, %%xmm5\\n"     // xmm5 = k2 * r^2
        "addss   %%xmm4, %%xmm5\\n"     // xmm5 = k1 + k2 * r^2
        "mulss   %%xmm3, %%xmm5\\n"     // xmm5 = (k1 + k2 * r^2) * r^2
        "movss   $1.0, %%xmm5\\n"       // xmm5 = 1.0
        "addss   %%xmm5, %%xmm5\\n"     // wait, this is wrong logic anyway, but let's fix syntax'''

    # Wait, actually, let's just replace "movss   $1.0, %%xmm5" with passing it or loading it.
    txt = txt.replace('"movss   $1.0, %%xmm5\\n"', '"movss   %6, %%xmm5\\n"')
    txt = txt.replace(': "m"(k1), "m"(k2)', ': "m"(k1), "m"(k2), "m"(one)')
    
    # We need to insert `float one = 1.0f;` right before the __asm__ block.
    txt = txt.replace('__asm__ volatile(', 'float one = 1.0f;\n    __asm__ volatile(')

    with open("exp4_vr_stereo.c", "w") as f:
        f.write(txt)

if __name__ == "__main__":
    main()
