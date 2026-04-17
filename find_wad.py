import sys
try:
    with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
        for i, line in enumerate(f):
            if '1wad' in line or 'doom1.wad' in line or 'IdentifyVersion' in line:
                print(f"{i+1}: {line.strip()}")
except Exception as e:
    print("Error:", e)
