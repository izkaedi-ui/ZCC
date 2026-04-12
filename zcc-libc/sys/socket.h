struct sockaddr { unsigned short sa_family; char sa_data[14]; };
struct msghdr { void* msg_name; int msg_namelen; void* msg_iov; int msg_iovlen; void* msg_control; int msg_controllen; int msg_flags; };
struct sockaddr_in { short sin_family; unsigned short sin_port; struct in_addr { unsigned long s_addr; } sin_addr; char sin_zero[8]; };
#define AF_INET 2
#define SOCK_DGRAM 2
#define INADDR_ANY 0
int socket(int, int, int);
int bind(int, const struct sockaddr*, int);
int recvfrom(int, void*, int, int, struct sockaddr*, int*);
int sendto(int, const void*, int, int, const struct sockaddr*, int);
int getsockname(int, struct sockaddr*, int*);
