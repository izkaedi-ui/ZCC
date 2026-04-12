struct dirent { char d_name[256]; };
typedef struct DIR DIR;
DIR* opendir(const char*);
struct dirent* readdir(DIR*);
int closedir(DIR*);
