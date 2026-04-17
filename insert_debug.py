content = open('zcc.c', 'rb').read().decode('utf-8', errors='replace')
# Replace the three fprintf(stderr) calls in the security block
content = content.replace(
    '              fprintf(stderr,\r\n                "ZCC SECURITY [CWE-416] use-after-free in \'%s\' line %d: "\r\n                "slot %s accessed after free()\\n",',
    '              printf(\r\n                "ZCC SECURITY [CWE-416] use-after-free in \'%s\' line %d: "\r\n                "slot %s accessed after free()\\n",'
)
content = content.replace(
    '      fprintf(stderr, "ZCC SECURITY: %d use-after-free violation(s) found. "\r\n              "Compilation succeeded but binary may be unsafe.\\n", total_violations);',
    '      printf("ZCC SECURITY: %d use-after-free violation(s) found. "\r\n              "Compilation succeeded but binary may be unsafe.\\n", total_violations);'
)
content = content.replace(
    '      fprintf(stderr, "ZCC SECURITY [CWE-416]: No intra-function UAF detected.\\n");',
    '      printf("ZCC SECURITY [CWE-416]: No intra-function UAF detected.\\n");'
)
open('zcc.c', 'wb').write(content.encode('utf-8'))
# Verify
import re
for m in re.finditer(r'(fprintf.*?stderr.*?SECURITY|printf.*?SECURITY)', content):
    print(m.group())
