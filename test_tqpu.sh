#!/usr/bin/env bash
set -uo pipefail

ZCC=/mnt/h/__DOWNLOADS/selforglinux

echo "=== TQPU DEMO ELF ==="
timeout 5 $ZCC/tqpu_demo.elf 2>&1

echo
echo "=== LUA RP2040 QEMU ELF — full runtime ==="
echo 'print("TQPU:", math.pi*2)
local t = {}
for i=1,10 do t[i] = i*i end
print("squares:", t[5], t[10])
local s = 0
for _,v in ipairs(t) do s = s + v end
print("sum:", s)
print("pcall:", pcall(function() return 42 end))' | timeout 5 $ZCC/lua_rp2040_qemu.elf 2>&1

echo
echo "=== ZCC-TQPU-NEURAL compile test ==="
cat > /tmp/tqpu_test.c << 'CSRC'
int add(int a, int b) { return a + b; }
int main() { return add(1, 2); }
CSRC
timeout 10 $ZCC/zcc-tqpu-neural /tmp/tqpu_test.c -o /tmp/tqpu_test.s 2>&1 | head -20
echo "ASM lines: $(wc -l < /tmp/tqpu_test.s)"

echo
echo "=== ZCC-TQPU-OMEGA compile test ==="
timeout 10 $ZCC/zcc-tqpu-omega /tmp/tqpu_test.c -o /tmp/tqpu_omega.s 2>&1 | head -20
echo "OMEGA ASM lines: $(wc -l < /tmp/tqpu_omega.s)"

echo
echo "=== ZCC-TQPU-GEN2 (base TQPU compiler) ==="
timeout 10 $ZCC/zcc-tqpu-gen2 /tmp/tqpu_test.c -o /tmp/tqpu_gen2.s 2>&1 | head -20
echo "GEN2 ASM lines: $(wc -l < /tmp/tqpu_gen2.s)"
