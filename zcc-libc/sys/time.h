struct timeval { long tv_sec; long tv_usec; };
struct timezone { int tz_minuteswest; int tz_dsttime; };
int gettimeofday(struct timeval*, struct timezone*);
struct itimerval { struct timeval it_interval; struct timeval it_value; };
int setitimer(int, const struct itimerval*, struct itimerval*);
#define ITIMER_REAL 0
