#include <X11/Xlib.h>
typedef struct { int shmid; char *shmaddr; int readOnly; } XShmSegmentInfo;
