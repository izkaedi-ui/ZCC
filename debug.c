#include <stdio.h>
typedef unsigned short u16;
typedef unsigned char u8;
int main() {
  u8 data[16] = {0};
  u16 nCell = 263;
  
  ((&data[0+3])[0] = (u8)(nCell>>8), (&data[0+3])[1] = (u8)(nCell));
  
  printf("data[3]=%d data[4]=%d\n", data[3], data[4]);
  return 0;
}
