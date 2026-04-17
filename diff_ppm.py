import sys

def diff_ppms(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        lines1 = f1.readlines()
        lines2 = f2.readlines()
        
    diff_count = 0
    for i, (l1, l2) in enumerate(zip(lines1, lines2)):
        if l1 != l2:
            print(f"Line {i+1}:")
            print(f"ZCC: {l1.strip()}")
            print(f"GCC: {l2.strip()}")
            diff_count += 1
            if diff_count > 10:
                break
    print(f"Total lines checked: {len(lines1)}")

diff_ppms('character.ppm', 'char_gcc.ppm')
