int printf(const char *fmt, ...);
void *malloc(long size);

char *src;
int pos;

int is_digit(char c) { return c >= 48 && c <= 57; }
int is_alpha(char c) {
    return (c >= 65 && c <= 90) || (c >= 97 && c <= 122) || c == 95;
}
int is_space(char c) {
    return c == 32 || c == 9 || c == 10 || c == 13;
}

int TK_NUM=1, TK_IDENT=2, TK_PLUS=3, TK_MINUS=4,
    TK_STAR=5, TK_SLASH=6, TK_SEMI=7, TK_EOF=8,
    TK_LPAREN=9, TK_RPAREN=10, TK_EQ=11, TK_ASSIGN=12,
    TK_LBRACE=13, TK_RBRACE=14, TK_COMMA=15, TK_RETURN=16;

int tok;
int tok_num;
char tok_str[64];

void next() {
    int i;
    while (is_space(src[pos])) pos++;
    if (!src[pos]) { tok = TK_EOF; return; }
    if (is_digit(src[pos])) {
        tok_num = 0;
        while (is_digit(src[pos]))
            tok_num = tok_num * 10 + (src[pos++] - 48);
        tok = TK_NUM; return;
    }
    if (is_alpha(src[pos])) {
        i = 0;
        while (is_alpha(src[pos]) || is_digit(src[pos]))
            tok_str[i++] = src[pos++];
        tok_str[i] = 0;
        if (tok_str[0]==114&&tok_str[1]==101&&tok_str[2]==116&&
            tok_str[3]==117&&tok_str[4]==114&&tok_str[5]==110&&!tok_str[6])
            tok = TK_RETURN;
        else tok = TK_IDENT;
        return;
    }
    char c = src[pos++];
    if (c == 43) tok = TK_PLUS;
    else if (c == 45) tok = TK_MINUS;
    else if (c == 42) tok = TK_STAR;
    else if (c == 47) tok = TK_SLASH;
    else if (c == 59) tok = TK_SEMI;
    else if (c == 40) tok = TK_LPAREN;
    else if (c == 41) tok = TK_RPAREN;
    else if (c == 123) tok = TK_LBRACE;
    else if (c == 125) tok = TK_RBRACE;
    else if (c == 44) tok = TK_COMMA;
    else if (c == 61) {
        if (src[pos] == 61) { pos++; tok = TK_EQ; }
        else tok = TK_ASSIGN;
    }
}

int main() {
    char buf[256];
    int i;
    buf[0]=105;buf[1]=110;buf[2]=116;buf[3]=32;
    buf[4]=102;buf[5]=111;buf[6]=111;buf[7]=40;
    buf[8]=105;buf[9]=110;buf[10]=116;buf[11]=32;
    buf[12]=120;buf[13]=41;buf[14]=32;buf[15]=123;
    buf[16]=32;buf[17]=114;buf[18]=101;buf[19]=116;
    buf[20]=117;buf[21]=114;buf[22]=110;buf[23]=32;
    buf[24]=120;buf[25]=32;buf[26]=43;buf[27]=32;
    buf[28]=49;buf[29]=59;buf[30]=32;buf[31]=125;
    buf[32]=0;
    src = buf; pos = 0;
    printf("Lexing: int foo(int x) { return x + 1; }\n");
    next();
    while (tok != TK_EOF) {
        if (tok == TK_NUM) printf("NUM(%d) ", tok_num);
        else if (tok == TK_IDENT) printf("IDENT(%s) ", tok_str);
        else if (tok == TK_RETURN) printf("RETURN ");
        else if (tok == TK_PLUS) printf("PLUS ");
        else if (tok == TK_SEMI) printf("SEMI ");
        else if (tok == TK_LBRACE) printf("LBRACE ");
        else if (tok == TK_RBRACE) printf("RBRACE ");
        else if (tok == TK_LPAREN) printf("LPAREN ");
        else if (tok == TK_RPAREN) printf("RPAREN ");
        else printf("TOK(%d) ", tok);
        next();
    }
    printf("\nEOF\n");
    return 0;
}
