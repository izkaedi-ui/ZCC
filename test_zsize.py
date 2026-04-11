import subprocess
import os

with open('part1.c', 'r') as f:
    content = f.read()

content += """
void printf();
int main() {
    printf("%d %d\\n", sizeof(struct Symbol), sizeof(struct Compiler));
    return 0;
}
"""

with open('test_zsize.c', 'w') as f:
    f.write(content)

subprocess.check_call(['./zcc', 'test_zsize.c', '-o', 'test_zsize.s'])
subprocess.check_call(['gcc', 'test_zsize.s', '-o', 'ez'])
subprocess.run(['./ez'])
