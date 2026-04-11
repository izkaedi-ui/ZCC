int printf(const char *fmt, ...);
char *src2;
int pos;
char output[8192];
int out_pos;
void emit(char *s){int i=0;while(s[i])output[out_pos++]=s[i++];}
void emit_int(int n){char buf[32];int i=0,neg=0;if(n<0){neg=1;n=-n;}if(n==0){buf[i++]=48;}else{while(n){buf[i++]=48+(n%10);n/=10;}}if(neg)buf[i++]=45;int j=0,k=i-1;while(j<k){char t=buf[j];buf[j]=buf[k];buf[k]=t;j++;k--;}buf[i]=0;emit(buf);}
int is_digit(char c){return c>=48&&c<=57;}
int is_alpha(char c){return(c>=65&&c<=90)||(c>=97&&c<=122)||c==95;}
int is_space(char c){return c==32||c==9||c==10||c==13;}
int tok,tok_num;char tok_str[64];
void next(){int i;while(is_space(src2[pos]))pos++;if(src2[pos]==0){tok=8;return;}if(is_digit(src2[pos])){tok_num=0;while(is_digit(src2[pos]))tok_num=tok_num*10+(src2[pos++]-48);tok=1;return;}if(is_alpha(src2[pos])){i=0;while(is_alpha(src2[pos])||is_digit(src2[pos]))tok_str[i++]=src2[pos++];tok_str[i]=0;if(tok_str[0]==114&&tok_str[1]==101&&tok_str[2]==116&&tok_str[3]==117&&tok_str[4]==114&&tok_str[5]==110&&tok_str[6]==0)tok=13;else if(tok_str[0]==105&&tok_str[1]==110&&tok_str[2]==116&&tok_str[3]==0)tok=14;else tok=2;return;}char c=src2[pos++];if(c==43)tok=3;else if(c==45)tok=4;else if(c==59)tok=7;else if(c==40)tok=9;else if(c==41)tok=10;else if(c==123)tok=11;else if(c==125)tok=12;else if(c==61)tok=15;}
void parse_expr(){if(tok==1){emit("    movq $");emit_int(tok_num);emit(", %rax
");next();}else if(tok==2){emit("    movq %rdi, %rax
");next();}while(tok==3||tok==4){int op=tok;next();if(tok==1){int v=tok_num;next();if(op==3){emit("    addq $");emit_int(v);emit(", %rax
");}else{emit("    subq $");emit_int(v);emit(", %rax
");}}}} 
void parse_func(){next();char fname[64];int i=0;while(tok_str[i]){fname[i]=tok_str[i];i++;}fname[i]=0;next();next();next();next();next();emit("    .globl ");emit(fname);emit("
");emit(fname);emit(":
");emit("    pushq %rbp
");emit("    movq %rsp, %rbp
");while(tok!=12){if(tok==13){next();parse_expr();emit("    popq %rbp
");emit("    ret
");next();}else next();}next();}
int main(){char buf[64];buf[0]=105;buf[1]=110;buf[2]=116;buf[3]=32;buf[4]=97;buf[5]=100;buf[6]=100;buf[7]=40;buf[8]=105;buf[9]=110;buf[10]=116;buf[11]=32;buf[12]=120;buf[13]=41;buf[14]=32;buf[15]=123;buf[16]=32;buf[17]=114;buf[18]=101;buf[19]=116;buf[20]=117;buf[21]=114;buf[22]=110;buf[23]=32;buf[24]=120;buf[25]=43;buf[26]=49;buf[27]=59;buf[28]=32;buf[29]=125;buf[30]=0;src2=buf;pos=0;out_pos=0;emit("    .text
");next();while(tok!=8){if(tok==14)parse_func();else next();}output[out_pos]=0;printf("%s",output);return 0;}
