struct stat { int st_mode; int st_size; };
int stat(const char*, struct stat*);
int fstat(int, struct stat*);
