import sys
import re

if len(sys.argv) < 3:
    print("Usage: prep_lua_for_zcc.py in.i out.c")
    sys.exit(1)

infile = sys.argv[1]
outfile = sys.argv[2]

with open(infile, 'r') as f:
    lines = f.readlines()

# 1. Strip GCC #line directives correctly
clean_lines = []
for line in lines:
    if line.startswith('#'):
        continue
    clean_lines.append(line)

src = "".join(clean_lines)

# Robust attribute stripper
def strip_attributes(s):
    attr = '__attribute__'
    result = []
    i = 0
    while i < len(s):
        if s[i:i+len(attr)] == attr:
            i += len(attr)
            while i < len(s) and s[i] in ' \t\n': i += 1
            if i < len(s) and s[i] == '(':
                depth = 1; i += 1
                while i < len(s) and depth > 0:
                    if s[i] == '(': depth += 1
                    elif s[i] == ')': depth -= 1
                    i += 1
            continue
        result.append(s[i])
        i += 1
    return ''.join(result)

src = strip_attributes(src)

# Strip GNU GCC extensions 
src = re.sub(r'__asm__\s*\([^)]*\)', '', src)
src = re.sub(r'__asm\s*\([^)]*\)', '', src)
src = src.replace('__restrict', '')
src = src.replace('__extension__', '')
src = src.replace('__inline__', '')
src = src.replace('__inline', '')
src = re.sub(r'\bvolatile\b', '', src)
src = re.sub(r'\brestrict\b', '', src)

# Fix type ordering for ZCC
src = re.sub(r'\blong\s+unsigned\s+int\b', 'unsigned long int', src)
src = re.sub(r'\blong\s+unsigned\b', 'unsigned long', src)
src = re.sub(r'\bshort\s+unsigned\s+int\b', 'unsigned short int', src)
src = re.sub(r'\bshort\s+unsigned\b', 'unsigned short', src)

# Bypass OP_EXTRAARG enum parsing inside array allocation bounds
src = src.replace('((int)(OP_EXTRAARG) + 1)', '84')

# Handle 128-bit GCC Float types leaking into headers
src = re.sub(r'typedef\s+[^;]*\b_Float\w+\b[^;]*;', '', src)
src = re.sub(r'\blong\s+double\b', 'double', src)
src = re.sub(r'\b_Float\w+\b', 'double', src)

# Replace __builtin_offsetof
src = re.sub(r'__builtin_offsetof\s*\(\s*([^,]+)\s*,\s*([^)]+)\)', r'((long)&((\1*)0)->\2)', src)

# Precompute constant array sizes
def eval_arr(m):
    try:
        expr = m.group(1).replace('\n', ' ').strip()
        v = eval(expr)
        if isinstance(v,(int,float)) and v==int(v) and int(v)>0: return f'[{int(v)}]'
    except: pass
    return m.group(0)
src = re.sub(r'\[([^\]]+)\]', eval_arr, src)

# Fix sizeof in array sizes
def fix_sizeof_arrays(m):
    expr = m.group(1).replace('\n', ' ')
    expr = re.sub(r'sizeof\s*\(\s*int\s*\)', '4', expr)
    expr = re.sub(r'sizeof\s*\(\s*void\s*\*\s*\)', '8', expr)
    expr = re.sub(r'sizeof\s*\(\s*long\s*\)', '8', expr)
    expr = re.sub(r'sizeof\s*\(\s*size_t\s*\)', '8', expr)
    expr = re.sub(r'sizeof\s*\(\s*unsigned\s+long(\s+int)?\s*\)', '8', expr)
    expr = re.sub(r'sizeof\s*\(\s*unsigned\s+int\s*\)', '4', expr)
    expr = re.sub(r'\(\s*int\s*\)', '', expr)
    try:
        v = eval(expr)
        if isinstance(v,(int,float)) and v==int(v) and int(v)>0: return f'[{int(v)}]'
    except: pass
    return m.group(0)
src = re.sub(r'\[([^\]]*sizeof[^\]]*)\]', fix_sizeof_arrays, src)

# Strip redundant GNU va_list defs
src = re.sub(r'typedef\s+[^;]*\b__builtin_va_list\b[^;]*;', '', src)
src = re.sub(r'typedef\s+[^;]*\b__gnuc_va_list\b[^;]*;', '', src)
src = re.sub(r'typedef\s+[^;]*\bva_list\b[^;]*;', '', src)

# Add Shims for ZCC
shims = """/* ZCC compatibility shims */
typedef struct { unsigned int gp_offset; unsigned int fp_offset; void *overflow_arg_area; void *reg_save_area; } va_list[1];
typedef va_list __builtin_va_list;
typedef va_list __gnuc_va_list;

static unsigned int zcc_bswap32(unsigned int x){return((x>>24)&0xff)|((x>>8)&0xff00)|((x<<8)&0xff0000)|((x<<24)&0xff000000u);}
static unsigned short zcc_bswap16(unsigned short x){return(x>>8)|(x<<8);}
static unsigned long zcc_bswap64(unsigned long x){return zcc_bswap32(x>>32)|((unsigned long)zcc_bswap32(x)<<32);}
static int zcc_clzll(unsigned long long x){int n=0;if(!x)return 64;while(!(x&(1ULL<<63))){n++;x<<=1;}return n;}
"""

src = shims + src

# Strip parentheses around function names (LUA_API macro expansion artifacts)
src = re.sub(r'\bextern\s+([A-Za-z0-9_*\s]+?)\s*\(\s*([A-Za-z0-9_]+)\s*\)\s*\(', r'extern \1 \2(', src)

with open(outfile, 'w') as f:
    f.write(src)

print(f"Output: {outfile} — {src.count(chr(10))} lines, {len(src)} bytes")
