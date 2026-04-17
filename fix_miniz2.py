import re

with open("/mnt/h/__DOWNLOADS/selforglinux/miniz_test_pp.c", "r") as f:
    text = f.read()

# Fix the broken extern void line from previous run
text = text.replace('extern void ((void)0)', 'extern void __assert_fail(const char *, const char *, unsigned int, const char *)')

# Also fix any __builtin_bswap that didn't get caught
# Use simple function replacement
text = text.replace('__builtin_bswap16', 'miniz_bswap16')
text = text.replace('__builtin_bswap32', 'miniz_bswap32')
text = text.replace('__builtin_bswap64', 'miniz_bswap64')

# Add inline implementations at the top
bswap_impl = """
static unsigned short miniz_bswap16(unsigned short x) { return (x >> 8) | (x << 8); }
static unsigned int miniz_bswap32(unsigned int x) { return ((x>>24)&0xFF)|((x>>8)&0xFF00)|((x<<8)&0xFF0000)|((x<<24)&0xFF000000U); }
static unsigned long long miniz_bswap64(unsigned long long x) { return ((unsigned long long)miniz_bswap32(x&0xFFFFFFFF)<<32)|miniz_bswap32(x>>32); }
"""

# Insert after first typedef
idx = text.find('typedef')
if idx != -1:
    text = text[:idx] + bswap_impl + text[idx:]

with open("/mnt/h/__DOWNLOADS/selforglinux/miniz_test_pp.c", "w") as f:
    f.write(text)
print("fixed")
