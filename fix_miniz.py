import re

with open("/mnt/h/__DOWNLOADS/selforglinux/miniz_test_pp.c", "r") as f:
    text = f.read()

# Kill GCC statement-expression asserts
text = re.sub(r'\(\(void\)\s*sizeof\s*\([^,]+,\s*\(\{[^}]*__assert_fail[^}]*\}\)\)', '((void)0)', text)

# Kill remaining __assert_fail
text = re.sub(r'__assert_fail\s*\([^)]*\)', '((void)0)', text)

# Kill __PRETTY_FUNCTION__
text = text.replace('__PRETTY_FUNCTION__', '""')

# Replace __builtin_bswap
text = re.sub(r'__builtin_bswap16\(([^)]*)\)', r'((((\1)>>8)&0xFF)|((\1)<<8))', text)
text = re.sub(r'__builtin_bswap32\(([^)]*)\)', r'((((\1)>>24)&0xFF)|((((\1)>>16)&0xFF)<<8)|((((\1)>>8)&0xFF)<<16)|(((\1)&0xFF)<<24))', text)
text = re.sub(r'__builtin_bswap64\(([^)]*)\)', r'(\1)', text)

with open("/mnt/h/__DOWNLOADS/selforglinux/miniz_test_pp.c", "w") as f:
    f.write(text)
print("cleaned")
