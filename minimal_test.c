#include <stdio.h>
typedef struct sqlite3 sqlite3;
int sqlite3_open(const char*, sqlite3**);
const char *sqlite3_errmsg(sqlite3*);
__asm__(".global __atomic_store_n\n__atomic_store_n:\nmovq %rsi, (%rdi)\nret\n");
__asm__(".global __atomic_load_n\n__atomic_load_n:\nmovq (%rdi), %rax\nret\n");
int main() {
  sqlite3 *db = 0;
  int rc = sqlite3_open(":memory:", &db);
  printf("rc = %d\n", rc);
  if (db) {
     printf("errmsg = %s\n", sqlite3_errmsg(db));
  }
  return rc;
}
