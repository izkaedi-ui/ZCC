/* CG-IR-019-RECON Phase 1A-RECON: Gold Standard ABI Classification Lattice Verification */

struct IntInt { long a; long b; };
/* Expected: size=16, align=8 -> lo:1 (INTEGER), hi:1 (INTEGER) */

struct DblDbl { double a; double b; };
/* Expected: size=16, align=8 -> lo:2 (SSE), hi:2 (SSE) */

struct DblInt { double a; long b; };
/* Expected: size=16, align=8 -> lo:2 (SSE), hi:1 (INTEGER) */

struct IntDbl { long a; double b; };
/* Expected: size=16, align=8 -> lo:1 (INTEGER), hi:2 (SSE) */

union  Val    { long i; double d; };
/* Expected: size=8, align=8 -> lo:1 (INTEGER), hi:0 (NO_CLASS) */

struct TValue { union Val v; char t; };
/* Expected: size=16, align=8 -> lo:1 (INTEGER), hi:1 (INTEGER) 
 * (Val(8) + char(1) = 9, padded to 16. Lattice join INTEGER ⊔ SSE = INTEGER) */

struct TooBig { long a, b, c; };
/* Expected: size=24, align=8 -> lo:3 (MEMORY), hi:3 (MEMORY) */

__attribute__((packed)) struct Packed9 { char a; long b; };
/* Expected: size=9, align=1 -> lo:3 (MEMORY), hi:3 (MEMORY) 
 * (Unaligned field 'b' forces MEMORY class) */

int main() {
    return 0;
}
