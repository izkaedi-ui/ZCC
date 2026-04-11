#include <stdio.h>
typedef unsigned char u8;
int main() {
  u8 data = 0;
  int b = 256;
  if( (data = b) == 0 ) printf("ZERO\n");
  else printf("NOT ZERO (%d)\n", (data = b));
  return 0;
}
