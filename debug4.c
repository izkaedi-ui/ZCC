#include <stdio.h>
typedef unsigned char u8;
int main() {
  u8 arr[10] = {0};
  u8 *data = arr;
  data[4] = 255;
  data[3] = 0;
  
  if( (++data[4]) == 0 ) data[3]++;
  
  printf("data[3]=%d data[4]=%d\n", data[3], data[4]);
  return 0;
}
