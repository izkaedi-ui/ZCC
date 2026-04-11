int add(int a, int b){ return a+b; }
int main(){
  int (*fp)(int,int);
  fp = add;
  return fp(1,2);
}
