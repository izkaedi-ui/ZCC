typedef void (*sighandler_t)(int);
#define SIGINT 2
#define SIGILL 4
#define SIGFPE 8
#define SIGSEGV 11
#define SIGTERM 15
sighandler_t signal(int, sighandler_t);
struct sigaction { sighandler_t sa_handler; unsigned long sa_flags; unsigned long sa_mask; };
int sigaction(int, const struct sigaction*, struct sigaction*);
