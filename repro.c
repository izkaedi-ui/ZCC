int foo(int x){ switch(x){ case 1: { int a=1; case 2: return a; } } return 0; }
int main() { return foo(2); }
