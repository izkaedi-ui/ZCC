typedef unsigned long shmatt_t;
struct ipc_perm { int cuid; int cgid; int uid; int gid; unsigned short mode; };
struct shmid_ds { struct ipc_perm shm_perm; unsigned long shm_segsz; int shm_cpid; int shm_lpid; unsigned long shm_nattch; };
#define IPC_CREAT 01000
#define IPC_RMID 0
int shmget(int, unsigned long, int);
void* shmat(int, const void*, int);
int shmdt(const void*);
int shmctl(int, int, struct shmid_ds*);
