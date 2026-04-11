/* 🔱 ZCC Fuzz Program — seed=91 */
int printf(const char *fmt, ...);

static int f1();

static int f1(int a, int b, int c) {
    long v1 = (long)((-(((((-(((unsigned long)(4187u))))) != (8035u)) ? (-(569)) : ((long)((((5718u) > (8965u)) ? 769916582u : 6123u)))))));
    unsigned int v2 = (unsigned int)(((unsigned int)((~(((-108) << (((((678) % (((((-686) | 1)) == 0 ? 1 : (((-686) | 1)))))) & 15))))))));
    unsigned int v3 = (unsigned int)(((((unsigned int)((((((unsigned long)(-633))) <= (((unsigned long)(634933657u)))) ? 2147483647 : (-(-674)))))) >> ((((-(((((~(-569))) <= (((328) | (-499)))) ? ((3429u) >> (((4674u) & 15))) : ((unsigned int)(765176227u)))))) & 15))));
    long v4 = (long)(272);
    unsigned long v5 = (unsigned long)(((unsigned long)((((~((((6942u) >= (997425470u)) ? -283659946 : -894)))) & (-213580760)))));
    unsigned long v6 = (unsigned long)((~(((unsigned long)((((-539) != ((~(127)))) ? ((unsigned long)(256u)) : ((unsigned long)(39))))))));
    if (((((((unsigned int)(0))) ^ (((((((438) != (0)) ? -766 : -187)) <= ((-(518583288)))) ? ((8373u) & (1637u)) : (((-433506624) != (-173417051)) ? 4476u : 2750u))))) >= (9979u))) {
        if (((((((((~(127))) != ((-(-699)))) ? 1673u : ((6509u) ^ (256u)))) ^ (8926u))) <= (((((255u) - ((-(1u))))) + (9507u))))) {
            { int v7; for (v7 = 0; v7 < 17; v7++) {
                a = (int)(((((394) - (-257200998))) % ((((((-(-412))) | 1)) == 0 ? 1 : ((((-(-412))) | 1))))));
            } }
        } else {
            { int v8; for (v8 = 0; v8 < 15; v8++) {
                b = (int)((((~(-482093024))) & ((~(932)))));
            } }
        }
    } else {
        c = (int)((((((575) - (((int)(0x80000000u))))) < (-205)) ? 157 : (((~(648))) / (((((((int)(1153071496u))) | 1)) == 0 ? 1 : (((((int)(1153071496u))) | 1)))))));
    }
    if (((1) <= ((~((((~(1))) * (((long)(690))))))))) {
        a = (int)((((((979) != (-917)) ? (((876578552) <= (802)) ? 608 : 323) : -314)) ^ (((int)(491)))));
    } else {
        c = (int)((c >> 12) | (c << 4));
    }
    v4 = (long)(((long)(((((~(-863))) >= (((long)(0x7FFFFFFFu)))) ? (-(7700u)) : 255u))));
    v1 += (long)(-86);
    printf("v1=%ld v2=%u v3=%u a=%d b=%d c=%d\n", v1, v2, v3, a, b, c);
    return ((int)v1 + (int)v2 + (int)v3) + a + b + c;
}

int main(void) {
    long checksum = 0;
    checksum += (long)f1(1, 2, 3);
    printf("CHECKSUM=%ld\n", checksum);
    return 0;
}