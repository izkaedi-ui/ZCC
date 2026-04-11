import os

def check():
    for f in os.listdir('.'):
        sz = os.path.getsize(f)
        if '\uf00d' in f:
            print(f"FOUND: {ascii(f)} (size {sz})")
        elif '\r' in f:
            print(f"FOUND_CR: {ascii(f)} (size {sz})")
        elif sz == 0 and f.endswith('.c'):
            print(f"EMPTY_C: {ascii(f)}")

if __name__ == '__main__':
    check()
