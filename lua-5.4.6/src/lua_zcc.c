/* ZCC compatibility shims */
typedef struct { unsigned int gp_offset; unsigned int fp_offset; void *overflow_arg_area; void *reg_save_area; } va_list[1];
typedef va_list __builtin_va_list;
typedef va_list __gnuc_va_list;

static unsigned int zcc_bswap32(unsigned int x){return((x>>24)&0xff)|((x>>8)&0xff00)|((x<<8)&0xff0000)|((x<<24)&0xff000000u);}
static unsigned short zcc_bswap16(unsigned short x){return(x>>8)|(x<<8);}
static unsigned long zcc_bswap64(unsigned long x){return zcc_bswap32(x>>32)|((unsigned long)zcc_bswap32(x)<<32);}
static int zcc_clzll(unsigned long long x){int n=0;if(!x)return 64;while(!(x&(1ULL<<63))){n++;x<<=1;}return n;}
























typedef unsigned long int size_t;
extern void *memcpy (void * __dest, const void * __src,
       size_t __n) ;


extern void *memmove (void *__dest, const void *__src, size_t __n)
     ;





extern void *memccpy (void * __dest, const void * __src,
        int __c, size_t __n)
    ;




extern void *memset (void *__s, int __c, size_t __n) ;


extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     ;
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     ;
extern void *memchr (const void *__s, int __c, size_t __n)
      ;
extern char *strcpy (char * __dest, const char * __src)
     ;

extern char *strncpy (char * __dest,
        const char * __src, size_t __n)
     ;


extern char *strcat (char * __dest, const char * __src)
     ;

extern char *strncat (char * __dest, const char * __src,
        size_t __n) ;


extern int strcmp (const char *__s1, const char *__s2)
     ;

extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     ;


extern int strcoll (const char *__s1, const char *__s2)
     ;

extern size_t strxfrm (char * __dest,
         const char * __src, size_t __n)
    ;
extern char *strdup (const char *__s)
     ;
extern char *strchr (const char *__s, int __c)
     ;
extern char *strrchr (const char *__s, int __c)
     ;
extern size_t strcspn (const char *__s, const char *__reject)
     ;


extern size_t strspn (const char *__s, const char *__accept)
     ;
extern char *strpbrk (const char *__s, const char *__accept)
     ;
extern char *strstr (const char *__haystack, const char *__needle)
     ;




extern char *strtok (char * __s, const char * __delim)
     ;



extern char *__strtok_r (char * __s,
    const char * __delim,
    char ** __save_ptr)
     ;

extern char *strtok_r (char * __s, const char * __delim,
         char ** __save_ptr)
     ;
extern size_t strlen (const char *__s)
     ;
extern char *strerror (int __errnum) ;
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen)
     ;


typedef long int ptrdiff_t;
typedef int wchar_t;
typedef struct {
  long long __max_align_ll ;
  double __max_align_ld ;
} max_align_t;




typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;

typedef signed long int __int64_t;
typedef unsigned long int __uint64_t;






typedef __int8_t __int_least8_t;
typedef __uint8_t __uint_least8_t;
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
typedef __int64_t __int_least64_t;
typedef __uint64_t __uint_least64_t;



typedef long int __quad_t;
typedef unsigned long int __u_quad_t;







typedef long int __intmax_t;
typedef unsigned long int __uintmax_t;


typedef unsigned long int __dev_t;
typedef unsigned int __uid_t;
typedef unsigned int __gid_t;
typedef unsigned long int __ino_t;
typedef unsigned long int __ino64_t;
typedef unsigned int __mode_t;
typedef unsigned long int __nlink_t;
typedef long int __off_t;
typedef long int __off64_t;
typedef int __pid_t;
typedef struct { int __val[2]; } __fsid_t;
typedef long int __clock_t;
typedef unsigned long int __rlim_t;
typedef unsigned long int __rlim64_t;
typedef unsigned int __id_t;
typedef long int __time_t;
typedef unsigned int __useconds_t;
typedef long int __suseconds_t;
typedef long int __suseconds64_t;

typedef int __daddr_t;
typedef int __key_t;


typedef int __clockid_t;


typedef void * __timer_t;


typedef long int __blksize_t;




typedef long int __blkcnt_t;
typedef long int __blkcnt64_t;


typedef unsigned long int __fsblkcnt_t;
typedef unsigned long int __fsblkcnt64_t;


typedef unsigned long int __fsfilcnt_t;
typedef unsigned long int __fsfilcnt64_t;


typedef long int __fsword_t;

typedef long int __ssize_t;


typedef long int __syscall_slong_t;

typedef unsigned long int __syscall_ulong_t;



typedef __off64_t __loff_t;
typedef char *__caddr_t;


typedef long int __intptr_t;


typedef unsigned int __socklen_t;




typedef int __sig_atomic_t;




typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;


typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;



typedef __int_least8_t int_least8_t;
typedef __int_least16_t int_least16_t;
typedef __int_least32_t int_least32_t;
typedef __int_least64_t int_least64_t;


typedef __uint_least8_t uint_least8_t;
typedef __uint_least16_t uint_least16_t;
typedef __uint_least32_t uint_least32_t;
typedef __uint_least64_t uint_least64_t;





typedef signed char int_fast8_t;

typedef long int int_fast16_t;
typedef long int int_fast32_t;
typedef long int int_fast64_t;
typedef unsigned char uint_fast8_t;

typedef unsigned long int uint_fast16_t;
typedef unsigned long int uint_fast32_t;
typedef unsigned long int uint_fast64_t;
typedef long int intptr_t;


typedef unsigned long int uintptr_t;
typedef __intmax_t intmax_t;
typedef __uintmax_t uintmax_t;

typedef struct lua_State lua_State;
typedef double lua_Number;



typedef long long lua_Integer;


typedef unsigned long long lua_Unsigned;


typedef intptr_t lua_KContext;





typedef int (*lua_CFunction) (lua_State *L);




typedef int (*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);





typedef const char * (*lua_Reader) (lua_State *L, void *ud, size_t *sz);

typedef int (*lua_Writer) (lua_State *L, const void *p, size_t sz, void *ud);





typedef void * (*lua_Alloc) (void *ud, void *ptr, size_t osize, size_t nsize);





typedef void (*lua_WarnFunction) (void *ud, const char *msg, int tocont);





typedef struct lua_Debug lua_Debug;





typedef void (*lua_Hook) (lua_State *L, lua_Debug *ar);
extern const char lua_ident[];





extern lua_State * lua_newstate(lua_Alloc f, void *ud);
extern void lua_close(lua_State *L);
extern lua_State * lua_newthread(lua_State *L);
extern int lua_closethread(lua_State *L, lua_State *from);
extern int lua_resetthread(lua_State *L);

extern lua_CFunction lua_atpanic(lua_State *L, lua_CFunction panicf);


extern lua_Number lua_version(lua_State *L);





extern int lua_absindex(lua_State *L, int idx);
extern int lua_gettop(lua_State *L);
extern void lua_settop(lua_State *L, int idx);
extern void lua_pushvalue(lua_State *L, int idx);
extern void lua_rotate(lua_State *L, int idx, int n);
extern void lua_copy(lua_State *L, int fromidx, int toidx);
extern int lua_checkstack(lua_State *L, int n);

extern void lua_xmove(lua_State *from, lua_State *to, int n);






extern int lua_isnumber(lua_State *L, int idx);
extern int lua_isstring(lua_State *L, int idx);
extern int lua_iscfunction(lua_State *L, int idx);
extern int lua_isinteger(lua_State *L, int idx);
extern int lua_isuserdata(lua_State *L, int idx);
extern int lua_type(lua_State *L, int idx);
extern const char * lua_typename(lua_State *L, int tp);

extern lua_Number lua_tonumberx(lua_State *L, int idx, int *isnum);
extern lua_Integer lua_tointegerx(lua_State *L, int idx, int *isnum);
extern int lua_toboolean(lua_State *L, int idx);
extern const char * lua_tolstring(lua_State *L, int idx, size_t *len);
extern lua_Unsigned lua_rawlen(lua_State *L, int idx);
extern lua_CFunction lua_tocfunction(lua_State *L, int idx);
extern void * lua_touserdata(lua_State *L, int idx);
extern lua_State * lua_tothread(lua_State *L, int idx);
extern const void * lua_topointer(lua_State *L, int idx);
extern void lua_arith(lua_State *L, int op);





extern int lua_rawequal(lua_State *L, int idx1, int idx2);
extern int lua_compare(lua_State *L, int idx1, int idx2, int op);





extern void lua_pushnil(lua_State *L);
extern void lua_pushnumber(lua_State *L, lua_Number n);
extern void lua_pushinteger(lua_State *L, lua_Integer n);
extern const char * lua_pushlstring(lua_State *L, const char *s, size_t len);
extern const char * lua_pushstring(lua_State *L, const char *s);
extern const char * lua_pushvfstring(lua_State *L, const char *fmt,
                                                      va_list argp);
extern const char * lua_pushfstring(lua_State *L, const char *fmt, ...);
extern void lua_pushcclosure(lua_State *L, lua_CFunction fn, int n);
extern void lua_pushboolean(lua_State *L, int b);
extern void lua_pushlightuserdata(lua_State *L, void *p);
extern int lua_pushthread(lua_State *L);





extern int lua_getglobal(lua_State *L, const char *name);
extern int lua_gettable(lua_State *L, int idx);
extern int lua_getfield(lua_State *L, int idx, const char *k);
extern int lua_geti(lua_State *L, int idx, lua_Integer n);
extern int lua_rawget(lua_State *L, int idx);
extern int lua_rawgeti(lua_State *L, int idx, lua_Integer n);
extern int lua_rawgetp(lua_State *L, int idx, const void *p);

extern void lua_createtable(lua_State *L, int narr, int nrec);
extern void * lua_newuserdatauv(lua_State *L, size_t sz, int nuvalue);
extern int lua_getmetatable(lua_State *L, int objindex);
extern int lua_getiuservalue(lua_State *L, int idx, int n);





extern void lua_setglobal(lua_State *L, const char *name);
extern void lua_settable(lua_State *L, int idx);
extern void lua_setfield(lua_State *L, int idx, const char *k);
extern void lua_seti(lua_State *L, int idx, lua_Integer n);
extern void lua_rawset(lua_State *L, int idx);
extern void lua_rawseti(lua_State *L, int idx, lua_Integer n);
extern void lua_rawsetp(lua_State *L, int idx, const void *p);
extern int lua_setmetatable(lua_State *L, int objindex);
extern int lua_setiuservalue(lua_State *L, int idx, int n);





extern void lua_callk(lua_State *L, int nargs, int nresults,
                           lua_KContext ctx, lua_KFunction k);


extern int lua_pcallk(lua_State *L, int nargs, int nresults, int errfunc,
                            lua_KContext ctx, lua_KFunction k);


extern int lua_load(lua_State *L, lua_Reader reader, void *dt,
                          const char *chunkname, const char *mode);

extern int lua_dump(lua_State *L, lua_Writer writer, void *data, int strip);





extern int lua_yieldk(lua_State *L, int nresults, lua_KContext ctx,
                               lua_KFunction k);
extern int lua_resume(lua_State *L, lua_State *from, int narg,
                               int *nres);
extern int lua_status(lua_State *L);
extern int lua_isyieldable(lua_State *L);







extern void lua_setwarnf(lua_State *L, lua_WarnFunction f, void *ud);
extern void lua_warning(lua_State *L, const char *msg, int tocont);
extern int lua_gc(lua_State *L, int what, ...);






extern int lua_error(lua_State *L);

extern int lua_next(lua_State *L, int idx);

extern void lua_concat(lua_State *L, int n);
extern void lua_len(lua_State *L, int idx);

extern size_t lua_stringtonumber(lua_State *L, const char *s);

extern lua_Alloc lua_getallocf(lua_State *L, void **ud);
extern void lua_setallocf(lua_State *L, lua_Alloc f, void *ud);

extern void lua_toclose(lua_State *L, int idx);
extern void lua_closeslot(lua_State *L, int idx);
extern int lua_getstack(lua_State *L, int level, lua_Debug *ar);
extern int lua_getinfo(lua_State *L, const char *what, lua_Debug *ar);
extern const char * lua_getlocal(lua_State *L, const lua_Debug *ar, int n);
extern const char * lua_setlocal(lua_State *L, const lua_Debug *ar, int n);
extern const char * lua_getupvalue(lua_State *L, int funcindex, int n);
extern const char * lua_setupvalue(lua_State *L, int funcindex, int n);

extern void * lua_upvalueid(lua_State *L, int fidx, int n);
extern void lua_upvaluejoin(lua_State *L, int fidx1, int n1,
                                               int fidx2, int n2);

extern void lua_sethook(lua_State *L, lua_Hook func, int mask, int count);
extern lua_Hook lua_gethook(lua_State *L);
extern int lua_gethookmask(lua_State *L);
extern int lua_gethookcount(lua_State *L);

extern int lua_setcstacklimit(lua_State *L, unsigned int limit);

struct lua_Debug {
  int event;
  const char *name;
  const char *namewhat;
  const char *what;
  const char *source;
  size_t srclen;
  int currentline;
  int linedefined;
  int lastlinedefined;
  unsigned char nups;
  unsigned char nparams;
  char isvararg;
  char istailcall;
  unsigned short ftransfer;
  unsigned short ntransfer;
  char short_src[60];

  struct CallInfo *i_ci;
};

typedef size_t lu_mem;
typedef ptrdiff_t l_mem;







typedef unsigned char lu_byte;
typedef signed char ls_byte;
typedef double l_uacNumber;
typedef long long l_uacInt;
typedef unsigned int l_uint32;




typedef l_uint32 Instruction;
typedef struct CallInfo CallInfo;


typedef union Value {
  struct GCObject *gc;
  void *p;
  lua_CFunction f;
  lua_Integer i;
  lua_Number n;

  lu_byte ub;
} Value;
typedef struct TValue {
  Value value_; lu_byte tt_;
} TValue;
typedef union StackValue {
  TValue val;
  struct {
    Value value_; lu_byte tt_;
    unsigned short delta;
  } tbclist;
} StackValue;



typedef StackValue *StkId;






typedef union {
  StkId p;
  ptrdiff_t offset;
} StkIdRel;
typedef struct GCObject {
  struct GCObject *next; lu_byte tt; lu_byte marked;
} GCObject;
typedef struct TString {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  lu_byte extra;
  lu_byte shrlen;
  unsigned int hash;
  union {
    size_t lnglen;
    struct TString *hnext;
  } u;
  char contents[1];
} TString;
typedef union UValue {
  TValue uv;
  lua_Number n; double u; void *s; lua_Integer i; long l;
} UValue;






typedef struct Udata {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  unsigned short nuvalue;
  size_t len;
  struct Table *metatable;
  GCObject *gclist;
  UValue uv[1];
} Udata;
typedef struct Udata0 {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  unsigned short nuvalue;
  size_t len;
  struct Table *metatable;
  union {lua_Number n; double u; void *s; lua_Integer i; long l;} bindata;
} Udata0;
typedef struct Upvaldesc {
  TString *name;
  lu_byte instack;
  lu_byte idx;
  lu_byte kind;
} Upvaldesc;






typedef struct LocVar {
  TString *varname;
  int startpc;
  int endpc;
} LocVar;
typedef struct AbsLineInfo {
  int pc;
  int line;
} AbsLineInfo;




typedef struct Proto {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  lu_byte numparams;
  lu_byte is_vararg;
  lu_byte maxstacksize;
  int sizeupvalues;
  int sizek;
  int sizecode;
  int sizelineinfo;
  int sizep;
  int sizelocvars;
  int sizeabslineinfo;
  int linedefined;
  int lastlinedefined;
  TValue *k;
  Instruction *code;
  struct Proto **p;
  Upvaldesc *upvalues;
  ls_byte *lineinfo;
  AbsLineInfo *abslineinfo;
  LocVar *locvars;
  TString *source;
  GCObject *gclist;
} Proto;
typedef struct UpVal {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  union {
    TValue *p;
    ptrdiff_t offset;
  } v;
  union {
    struct {
      struct UpVal *next;
      struct UpVal **previous;
    } open;
    TValue value;
  } u;
} UpVal;






typedef struct CClosure {
  struct GCObject *next; lu_byte tt; lu_byte marked; lu_byte nupvalues; GCObject *gclist;
  lua_CFunction f;
  TValue upvalue[1];
} CClosure;


typedef struct LClosure {
  struct GCObject *next; lu_byte tt; lu_byte marked; lu_byte nupvalues; GCObject *gclist;
  struct Proto *p;
  UpVal *upvals[1];
} LClosure;


typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;
typedef union Node {
  struct NodeKey {
    Value value_; lu_byte tt_;
    lu_byte key_tt;
    int next;
    Value key_val;
  } u;
  TValue i_val;
} Node;
typedef struct Table {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  lu_byte flags;
  lu_byte lsizenode;
  unsigned int alimit;
  TValue *array;
  Node *node;
  Node *lastfree;
  struct Table *metatable;
  GCObject *gclist;
} Table;
extern int luaO_utf8esc (char *buff, unsigned long x);
extern int luaO_ceillog2 (unsigned int x);
extern int luaO_rawarith (lua_State *L, int op, const TValue *p1,
                             const TValue *p2, TValue *res);
extern void luaO_arith (lua_State *L, int op, const TValue *p1,
                           const TValue *p2, StkId res);
extern size_t luaO_str2num (const char *s, TValue *o);
extern int luaO_hexavalue (int c);
extern void luaO_tostring (lua_State *L, TValue *obj);
extern const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
extern const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
extern void luaO_chunkid (char *out, const char *source, size_t srclen);






typedef enum {
  TM_INDEX,
  TM_NEWINDEX,
  TM_GC,
  TM_MODE,
  TM_LEN,
  TM_EQ,
  TM_ADD,
  TM_SUB,
  TM_MUL,
  TM_MOD,
  TM_POW,
  TM_DIV,
  TM_IDIV,
  TM_BAND,
  TM_BOR,
  TM_BXOR,
  TM_SHL,
  TM_SHR,
  TM_UNM,
  TM_BNOT,
  TM_LT,
  TM_LE,
  TM_CONCAT,
  TM_CALL,
  TM_CLOSE,
  TM_N
} TMS;
extern const char *const luaT_typenames_[12];


extern const char *luaT_objtypename (lua_State *L, const TValue *o);

extern const TValue *luaT_gettm (Table *events, TMS event, TString *ename);
extern const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o,
                                                       TMS event);
extern void luaT_init (lua_State *L);

extern void luaT_callTM (lua_State *L, const TValue *f, const TValue *p1,
                            const TValue *p2, const TValue *p3);
extern void luaT_callTMres (lua_State *L, const TValue *f,
                            const TValue *p1, const TValue *p2, StkId p3);
extern void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
                              StkId res, TMS event);
extern void luaT_tryconcatTM (lua_State *L);
extern void luaT_trybinassocTM (lua_State *L, const TValue *p1,
       const TValue *p2, int inv, StkId res, TMS event);
extern void luaT_trybiniTM (lua_State *L, const TValue *p1, lua_Integer i2,
                               int inv, StkId res, TMS event);
extern int luaT_callorderTM (lua_State *L, const TValue *p1,
                                const TValue *p2, TMS event);
extern int luaT_callorderiTM (lua_State *L, const TValue *p1, int v2,
                                 int inv, int isfloat, TMS event);

extern void luaT_adjustvarargs (lua_State *L, int nfixparams,
                                   CallInfo *ci, const Proto *p);
extern void luaT_getvarargs (lua_State *L, CallInfo *ci,
                                              StkId where, int wanted);
extern void luaM_toobig (lua_State *L);


extern void *luaM_realloc_ (lua_State *L, void *block, size_t oldsize,
                                                          size_t size);
extern void *luaM_saferealloc_ (lua_State *L, void *block, size_t oldsize,
                                                              size_t size);
extern void luaM_free_ (lua_State *L, void *block, size_t osize);
extern void *luaM_growaux_ (lua_State *L, void *block, int nelems,
                               int *size, int size_elem, int limit,
                               const char *what);
extern void *luaM_shrinkvector_ (lua_State *L, void *block, int *nelem,
                                    int final_n, int size_elem);
extern void *luaM_malloc_ (lua_State *L, size_t size, int tag);




typedef struct Zio ZIO;




typedef struct Mbuffer {
  char *buffer;
  size_t n;
  size_t buffsize;
} Mbuffer;
extern void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader,
                                        void *data);
extern size_t luaZ_read (ZIO* z, void *b, size_t n);





struct Zio {
  size_t n;
  const char *p;
  lua_Reader reader;
  void *data;
  lua_State *L;
};


extern int luaZ_fill (ZIO *z);
struct lua_longjmp;



















typedef __sig_atomic_t sig_atomic_t;









typedef struct
{
  unsigned long int __val[16];
} __sigset_t;


typedef __sigset_t sigset_t;




typedef __pid_t pid_t;





typedef __uid_t uid_t;











typedef __time_t time_t;



struct timespec
{



  __time_t tv_sec;




  __syscall_slong_t tv_nsec;
};







union sigval
{
  int sival_int;
  void *sival_ptr;
};

typedef union sigval __sigval_t;
typedef struct
  {
    int si_signo;

    int si_errno;

    int si_code;





    int __pad0;


    union
      {
 int _pad[28];


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
   } _kill;


 struct
   {
     int si_tid;
     int si_overrun;
     __sigval_t si_sigval;
   } _timer;


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
     __sigval_t si_sigval;
   } _rt;


 struct
   {
     __pid_t si_pid;
     __uid_t si_uid;
     int si_status;
     __clock_t si_utime;
     __clock_t si_stime;
   } _sigchld;


 struct
   {
     void *si_addr;
    
     short int si_addr_lsb;
     union
       {

  struct
    {
      void *_lower;
      void *_upper;
    } _addr_bnd;

  __uint32_t _pkey;
       } _bounds;
   } _sigfault;


 struct
   {
     long int si_band;
     int si_fd;
   } _sigpoll;



 struct
   {
     void *_call_addr;
     int _syscall;
     unsigned int _arch;
   } _sigsys;

      } _sifields;
  } siginfo_t ;
enum
{
  SI_ASYNCNL = -60,
  SI_DETHREAD = -7,

  SI_TKILL,
  SI_SIGIO,

  SI_ASYNCIO,
  SI_MESGQ,
  SI_TIMER,





  SI_QUEUE,
  SI_USER,
  SI_KERNEL = 0x80
};




enum
{
  ILL_ILLOPC = 1,

  ILL_ILLOPN,

  ILL_ILLADR,

  ILL_ILLTRP,

  ILL_PRVOPC,

  ILL_PRVREG,

  ILL_COPROC,

  ILL_BADSTK,

  ILL_BADIADDR

};


enum
{
  FPE_INTDIV = 1,

  FPE_INTOVF,

  FPE_FLTDIV,

  FPE_FLTOVF,

  FPE_FLTUND,

  FPE_FLTRES,

  FPE_FLTINV,

  FPE_FLTSUB,

  FPE_FLTUNK = 14,

  FPE_CONDTRAP

};


enum
{
  SEGV_MAPERR = 1,

  SEGV_ACCERR,

  SEGV_BNDERR,

  SEGV_PKUERR,

  SEGV_ACCADI,

  SEGV_ADIDERR,

  SEGV_ADIPERR,

  SEGV_MTEAERR,

  SEGV_MTESERR,

  SEGV_CPERR

};


enum
{
  BUS_ADRALN = 1,

  BUS_ADRERR,

  BUS_OBJERR,

  BUS_MCEERR_AR,

  BUS_MCEERR_AO

};




enum
{
  TRAP_BRKPT = 1,

  TRAP_TRACE,

  TRAP_BRANCH,

  TRAP_HWBKPT,

  TRAP_UNK

};




enum
{
  CLD_EXITED = 1,

  CLD_KILLED,

  CLD_DUMPED,

  CLD_TRAPPED,

  CLD_STOPPED,

  CLD_CONTINUED

};


enum
{
  POLL_IN = 1,

  POLL_OUT,

  POLL_MSG,

  POLL_ERR,

  POLL_PRI,

  POLL_HUP

};










typedef union pthread_attr_t pthread_attr_t;




typedef struct sigevent
  {
    __sigval_t sigev_value;
    int sigev_signo;
    int sigev_notify;

    union
      {
 int _pad[12];



 __pid_t _tid;

 struct
   {
     void (*_function) (__sigval_t);
     pthread_attr_t *_attribute;
   } _sigev_thread;
      } _sigev_un;
  } sigevent_t;
enum
{
  SIGEV_SIGNAL = 0,

  SIGEV_NONE,

  SIGEV_THREAD,


  SIGEV_THREAD_ID = 4


};




typedef void (*__sighandler_t) (int);




extern __sighandler_t __sysv_signal (int __sig, __sighandler_t __handler)
     ;
extern __sighandler_t bsd_signal (int __sig, __sighandler_t __handler)
     ;






extern int kill (__pid_t __pid, int __sig) ;






extern int killpg (__pid_t __pgrp, int __sig) ;



extern int raise (int __sig) ;
extern int __sigpause (int __sig_or_mask, int __is_sig);
extern int sigemptyset (sigset_t *__set) ;


extern int sigfillset (sigset_t *__set) ;


extern int sigaddset (sigset_t *__set, int __signo) ;


extern int sigdelset (sigset_t *__set, int __signo) ;


extern int sigismember (const sigset_t *__set, int __signo)
     ;
struct sigaction
  {


    union
      {

 __sighandler_t sa_handler;

 void (*sa_sigaction) (int, siginfo_t *, void *);
      }
    __sigaction_handler;







    __sigset_t sa_mask;


    int sa_flags;


    void (*sa_restorer) (void);
  };


extern int sigprocmask (int __how, const sigset_t * __set,
   sigset_t * __oset) ;






extern int sigsuspend (const sigset_t *__set) ;


extern int sigaction (int __sig, const struct sigaction * __act,
        struct sigaction * __oact) ;


extern int sigpending (sigset_t *__set) ;







extern int sigwait (const sigset_t * __set, int * __sig)
     ;







extern int sigwaitinfo (const sigset_t * __set,
   siginfo_t * __info) ;







extern int sigtimedwait (const sigset_t * __set,
    siginfo_t * __info,
    const struct timespec * __timeout)
     ;
extern int sigqueue (__pid_t __pid, int __sig, const union sigval __val)
     ;



typedef struct
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t;


 typedef long long int greg_t;
typedef greg_t gregset_t[23];
struct _libc_fpxreg
{
  unsigned short int __significand[4];
  unsigned short int __exponent;
  unsigned short int __glibc_reserved1[3];
};

struct _libc_xmmreg
{
  __uint32_t __element[4];
};

struct _libc_fpstate
{

  __uint16_t __cwd;
  __uint16_t __swd;
  __uint16_t __ftw;
  __uint16_t __fop;
  __uint64_t __rip;
  __uint64_t __rdp;
  __uint32_t __mxcsr;
  __uint32_t __mxcr_mask;
  struct _libc_fpxreg _st[8];
  struct _libc_xmmreg _xmm[16];
  __uint32_t __glibc_reserved1[24];
};


typedef struct _libc_fpstate *fpregset_t;


typedef struct
  {
    gregset_t __gregs;

    fpregset_t __fpregs;
    unsigned long long __reserved1 [8];
} mcontext_t;


typedef struct ucontext_t
  {
    unsigned long int __uc_flags;
    struct ucontext_t *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
    unsigned long long int __ssp[4];
  } ucontext_t;







extern int siginterrupt (int __sig, int __interrupt)
  ;

enum
{
  SS_ONSTACK = 1,

  SS_DISABLE

};



extern int sigaltstack (const stack_t * __ss,
   stack_t * __oss) ;




struct sigstack
  {
    void *ss_sp;
    int ss_onstack;
  };
extern int sighold (int __sig)
  ;


extern int sigrelse (int __sig)
  ;


extern int sigignore (int __sig)
  ;


extern __sighandler_t sigset (int __sig, __sighandler_t __disp)
 
                                                        ;






typedef union
{
  unsigned long long int __value64;
  struct
  {
    unsigned int __low;
    unsigned int __high;
  } __value32;
} __atomic_wide_counter;




typedef struct __pthread_internal_list
{
  struct __pthread_internal_list *__prev;
  struct __pthread_internal_list *__next;
} __pthread_list_t;

typedef struct __pthread_internal_slist
{
  struct __pthread_internal_slist *__next;
} __pthread_slist_t;
struct __pthread_mutex_s
{
  int __lock;
  unsigned int __count;
  int __owner;

  unsigned int __nusers;



  int __kind;

  short __spins;
  short __elision;
  __pthread_list_t __list;
};
struct __pthread_rwlock_arch_t
{
  unsigned int __readers;
  unsigned int __writers;
  unsigned int __wrphase_futex;
  unsigned int __writers_futex;
  unsigned int __pad3;
  unsigned int __pad4;

  int __cur_writer;
  int __shared;
  signed char __rwelision;




  unsigned char __pad1[7];


  unsigned long int __pad2;


  unsigned int __flags;
};




struct __pthread_cond_s
{
  __atomic_wide_counter __wseq;
  __atomic_wide_counter __g1_start;
  unsigned int __g_refs[2] ;
  unsigned int __g_size[2];
  unsigned int __g1_orig_size;
  unsigned int __wrefs;
  unsigned int __g_signals[2];
};

typedef unsigned int __tss_t;
typedef unsigned long int __thrd_t;

typedef struct
{
  int __data ;
} __once_flag;



typedef unsigned long int pthread_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_mutexattr_t;




typedef union
{
  char __size[4];
  int __align;
} pthread_condattr_t;



typedef unsigned int pthread_key_t;



typedef int pthread_once_t;


union pthread_attr_t
{
  char __size[56];
  long int __align;
};






typedef union
{
  struct __pthread_mutex_s __data;
  char __size[40];
  long int __align;
} pthread_mutex_t;


typedef union
{
  struct __pthread_cond_s __data;
  char __size[48];
  long long int __align;
} pthread_cond_t;





typedef union
{
  struct __pthread_rwlock_arch_t __data;
  char __size[56];
  long int __align;
} pthread_rwlock_t;

typedef union
{
  char __size[8];
  long int __align;
} pthread_rwlockattr_t;





typedef  int pthread_spinlock_t;




typedef union
{
  char __size[32];
  long int __align;
} pthread_barrier_t;

typedef union
{
  char __size[4];
  int __align;
} pthread_barrierattr_t;
extern int pthread_sigmask (int __how,
       const __sigset_t * __newmask,
       __sigset_t * __oldmask);


extern int pthread_kill (pthread_t __threadid, int __signo) ;






extern int __libc_current_sigrtmin (void) ;

extern int __libc_current_sigrtmax (void) ;








typedef struct stringtable {
  TString **hash;
  int nuse;
  int size;
} stringtable;
struct CallInfo {
  StkIdRel func;
  StkIdRel top;
  struct CallInfo *previous, *next;
  union {
    struct {
      const Instruction *savedpc;
       sig_atomic_t trap;
      int nextraargs;
    } l;
    struct {
      lua_KFunction k;
      ptrdiff_t old_errfunc;
      lua_KContext ctx;
    } c;
  } u;
  union {
    int funcidx;
    int nyield;
    int nres;
    struct {
      unsigned short ftransfer;
      unsigned short ntransfer;
    } transferinfo;
  } u2;
  short nresults;
  unsigned short callstatus;
};
typedef struct global_State {
  lua_Alloc frealloc;
  void *ud;
  l_mem totalbytes;
  l_mem GCdebt;
  lu_mem GCestimate;
  lu_mem lastatomic;
  stringtable strt;
  TValue l_registry;
  TValue nilvalue;
  unsigned int seed;
  lu_byte currentwhite;
  lu_byte gcstate;
  lu_byte gckind;
  lu_byte gcstopem;
  lu_byte genminormul;
  lu_byte genmajormul;
  lu_byte gcstp;
  lu_byte gcemergency;
  lu_byte gcpause;
  lu_byte gcstepmul;
  lu_byte gcstepsize;
  GCObject *allgc;
  GCObject **sweepgc;
  GCObject *finobj;
  GCObject *gray;
  GCObject *grayagain;
  GCObject *weak;
  GCObject *ephemeron;
  GCObject *allweak;
  GCObject *tobefnz;
  GCObject *fixedgc;

  GCObject *survival;
  GCObject *old1;
  GCObject *reallyold;
  GCObject *firstold1;
  GCObject *finobjsur;
  GCObject *finobjold1;
  GCObject *finobjrold;
  struct lua_State *twups;
  lua_CFunction panic;
  struct lua_State *mainthread;
  TString *memerrmsg;
  TString *tmname[TM_N];
  struct Table *mt[9];
  TString *strcache[53][2];
  lua_WarnFunction warnf;
  void *ud_warn;
} global_State;





struct lua_State {
  struct GCObject *next; lu_byte tt; lu_byte marked;
  lu_byte status;
  lu_byte allowhook;
  unsigned short nci;
  StkIdRel top;
  global_State *l_G;
  CallInfo *ci;
  StkIdRel stack_last;
  StkIdRel stack;
  UpVal *openupval;
  StkIdRel tbclist;
  GCObject *gclist;
  struct lua_State *twups;
  struct lua_longjmp *errorJmp;
  CallInfo base_ci;
   lua_Hook hook;
  ptrdiff_t errfunc;
  l_uint32 nCcalls;
  int oldpc;
  int basehookcount;
  int hookcount;
   sig_atomic_t hookmask;
};
union GCUnion {
  GCObject gc;
  struct TString ts;
  struct Udata u;
  union Closure cl;
  struct Table h;
  struct Proto p;
  struct lua_State th;
  struct UpVal upv;
};
extern void luaE_setdebt (global_State *g, l_mem debt);
extern void luaE_freethread (lua_State *L, lua_State *L1);
extern CallInfo *luaE_extendCI (lua_State *L);
extern void luaE_freeCI (lua_State *L);
extern void luaE_shrinkCI (lua_State *L);
extern void luaE_checkcstack (lua_State *L);
extern void luaE_incCstack (lua_State *L);
extern void luaE_warning (lua_State *L, const char *msg, int tocont);
extern void luaE_warnerror (lua_State *L, const char *where);
extern int luaE_resetthread (lua_State *L, int status);
extern int luaG_getfuncline (const Proto *f, int pc);
extern const char *luaG_findlocal (lua_State *L, CallInfo *ci, int n,
                                                    StkId *pos);
extern void luaG_typeerror (lua_State *L, const TValue *o,
                                                const char *opname);
extern void luaG_callerror (lua_State *L, const TValue *o);
extern void luaG_forerror (lua_State *L, const TValue *o,
                                               const char *what);
extern void luaG_concaterror (lua_State *L, const TValue *p1,
                                                  const TValue *p2);
extern void luaG_opinterror (lua_State *L, const TValue *p1,
                                                 const TValue *p2,
                                                 const char *msg);
extern void luaG_tointerror (lua_State *L, const TValue *p1,
                                                 const TValue *p2);
extern void luaG_ordererror (lua_State *L, const TValue *p1,
                                                 const TValue *p2);
extern void luaG_runerror (lua_State *L, const char *fmt, ...);
extern const char *luaG_addinfo (lua_State *L, const char *msg,
                                                  TString *src, int line);
extern void luaG_errormsg (lua_State *L);
extern int luaG_traceexec (lua_State *L, const Instruction *pc);
typedef void (*Pfunc) (lua_State *L, void *ud);

extern void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop);
extern int luaD_protectedparser (lua_State *L, ZIO *z, const char *name,
                                                  const char *mode);
extern void luaD_hook (lua_State *L, int event, int line,
                                        int fTransfer, int nTransfer);
extern void luaD_hookcall (lua_State *L, CallInfo *ci);
extern int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                              int narg1, int delta);
extern CallInfo *luaD_precall (lua_State *L, StkId func, int nResults);
extern void luaD_call (lua_State *L, StkId func, int nResults);
extern void luaD_callnoyield (lua_State *L, StkId func, int nResults);
extern StkId luaD_tryfuncTM (lua_State *L, StkId func);
extern int luaD_closeprotected (lua_State *L, ptrdiff_t level, int status);
extern int luaD_pcall (lua_State *L, Pfunc func, void *u,
                                        ptrdiff_t oldtop, ptrdiff_t ef);
extern void luaD_poscall (lua_State *L, CallInfo *ci, int nres);
extern int luaD_reallocstack (lua_State *L, int newsize, int raiseerror);
extern int luaD_growstack (lua_State *L, int n, int raiseerror);
extern void luaD_shrinkstack (lua_State *L);
extern void luaD_inctop (lua_State *L);

extern void luaD_throw (lua_State *L, int errcode);
extern int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud);
extern Proto *luaF_newproto (lua_State *L);
extern CClosure *luaF_newCclosure (lua_State *L, int nupvals);
extern LClosure *luaF_newLclosure (lua_State *L, int nupvals);
extern void luaF_initupvals (lua_State *L, LClosure *cl);
extern UpVal *luaF_findupval (lua_State *L, StkId level);
extern void luaF_newtbcupval (lua_State *L, StkId level);
extern void luaF_closeupval (lua_State *L, StkId level);
extern StkId luaF_close (lua_State *L, StkId level, int status, int yy);
extern void luaF_unlinkupval (UpVal *uv);
extern void luaF_freeproto (lua_State *L, Proto *f);
extern const char *luaF_getlocalname (const Proto *func, int local_number,
                                         int pc);
extern void luaC_fix (lua_State *L, GCObject *o);
extern void luaC_freeallobjects (lua_State *L);
extern void luaC_step (lua_State *L);
extern void luaC_runtilstate (lua_State *L, int statesmask);
extern void luaC_fullgc (lua_State *L, int isemergency);
extern GCObject *luaC_newobj (lua_State *L, int tt, size_t sz);
extern GCObject *luaC_newobjdt (lua_State *L, int tt, size_t sz,
                                                 size_t offset);
extern void luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v);
extern void luaC_barrierback_ (lua_State *L, GCObject *o);
extern void luaC_checkfinalizer (lua_State *L, GCObject *o, Table *mt);
extern void luaC_changemode (lua_State *L, int newmode);



extern unsigned int luaS_hash (const char *str, size_t l, unsigned int seed);
extern unsigned int luaS_hashlongstr (TString *ts);
extern int luaS_eqlngstr (TString *a, TString *b);
extern void luaS_resize (lua_State *L, int newsize);
extern void luaS_clearcache (global_State *g);
extern void luaS_init (lua_State *L);
extern void luaS_remove (lua_State *L, TString *ts);
extern Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue);
extern TString *luaS_newlstr (lua_State *L, const char *str, size_t l);
extern TString *luaS_new (lua_State *L, const char *str);
extern TString *luaS_createlngstrobj (lua_State *L, size_t l);
extern const TValue *luaH_getint (Table *t, lua_Integer key);
extern void luaH_setint (lua_State *L, Table *t, lua_Integer key,
                                                    TValue *value);
extern const TValue *luaH_getshortstr (Table *t, TString *key);
extern const TValue *luaH_getstr (Table *t, TString *key);
extern const TValue *luaH_get (Table *t, const TValue *key);
extern void luaH_newkey (lua_State *L, Table *t, const TValue *key,
                                                    TValue *value);
extern void luaH_set (lua_State *L, Table *t, const TValue *key,
                                                 TValue *value);
extern void luaH_finishset (lua_State *L, Table *t, const TValue *key,
                                       const TValue *slot, TValue *value);
extern Table *luaH_new (lua_State *L);
extern void luaH_resize (lua_State *L, Table *t, unsigned int nasize,
                                                    unsigned int nhsize);
extern void luaH_resizearray (lua_State *L, Table *t, unsigned int nasize);
extern void luaH_free (lua_State *L, Table *t);
extern int luaH_next (lua_State *L, Table *t, StkId key);
extern lua_Unsigned luaH_getn (Table *t);
extern unsigned int luaH_realasize (const Table *t);

extern LClosure* luaU_undump (lua_State* L, ZIO* Z, const char* name);


extern int luaU_dump (lua_State* L, const Proto* f, lua_Writer w,
                         void* data, int strip);
typedef enum {
  F2Ieq,
  F2Ifloor,
  F2Iceil
} F2Imod;
extern int luaV_equalobj (lua_State *L, const TValue *t1, const TValue *t2);
extern int luaV_lessthan (lua_State *L, const TValue *l, const TValue *r);
extern int luaV_lessequal (lua_State *L, const TValue *l, const TValue *r);
extern int luaV_tonumber_ (const TValue *obj, lua_Number *n);
extern int luaV_tointeger (const TValue *obj, lua_Integer *p, F2Imod mode);
extern int luaV_tointegerns (const TValue *obj, lua_Integer *p,
                                F2Imod mode);
extern int luaV_flttointeger (lua_Number n, lua_Integer *p, F2Imod mode);
extern void luaV_finishget (lua_State *L, const TValue *t, TValue *key,
                               StkId val, const TValue *slot);
extern void luaV_finishset (lua_State *L, const TValue *t, TValue *key,
                               TValue *val, const TValue *slot);
extern void luaV_finishOp (lua_State *L);
extern void luaV_execute (lua_State *L, CallInfo *ci);
extern void luaV_concat (lua_State *L, int total);
extern lua_Integer luaV_idiv (lua_State *L, lua_Integer x, lua_Integer y);
extern lua_Integer luaV_mod (lua_State *L, lua_Integer x, lua_Integer y);
extern lua_Number luaV_modf (lua_State *L, lua_Number x, lua_Number y);
extern lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y);
extern void luaV_objlen (lua_State *L, StkId ra, const TValue *rb);



const char lua_ident[] =
  "$LuaVersion: " "Lua " "5" "." "4" "." "6" "  Copyright (C) 1994-2023 Lua.org, PUC-Rio" " $"
  "$LuaAuthors: " "R. Ierusalimschy, L. H. de Figueiredo, W. Celes" " $";
static TValue *index2value (lua_State *L, int idx) {
  CallInfo *ci = L->ci;
  if (idx > 0) {
    StkId o = ci->func.p + idx;
    ((void)L, ((void)0));
    if (o >= L->top.p) return &(L->l_G)->nilvalue;
    else return (&(o)->val);
  }
  else if (!((idx) <= (-1000000 - 1000))) {
    ((void)L, ((void)0))
                                 ;
    return (&(L->top.p + idx)->val);
  }
  else if (idx == (-1000000 - 1000))
    return &(L->l_G)->l_registry;
  else {
    idx = (-1000000 - 1000) - idx;
    ((void)L, ((void)0));
    if ((((((&(ci->func.p)->val)))->tt_) == (((((6) | ((2) << 4))) | (1 << 6))))) {
      CClosure *func = ((&((((union GCUnion *)(((((&(ci->func.p)->val))->value_).gc))))->cl.c)));
      return (idx <= func->nupvalues) ? &func->upvalue[idx-1]
                                      : &(L->l_G)->nilvalue;
    }
    else {
      ((void)L, ((void)0));
      return &(L->l_G)->nilvalue;
    }
  }
}






static inline StkId index2stack (lua_State *L, int idx) {
  CallInfo *ci = L->ci;
  if (idx > 0) {
    StkId o = ci->func.p + idx;
    ((void)L, ((void)0));
    return o;
  }
  else {
    ((void)L, ((void)0))
                                 ;
    ((void)L, ((void)0));
    return L->top.p + idx;
  }
}


extern int lua_checkstack (lua_State *L, int n) {
  int res;
  CallInfo *ci;
  ((void) 0);
  ci = L->ci;
  ((void)L, ((void)0));
  if (L->stack_last.p - L->top.p > n)
    res = 1;
  else
    res = luaD_growstack(L, n, 0);
  if (res && ci->top.p < L->top.p + n)
    ci->top.p = L->top.p + n;
  ((void) 0);
  return res;
}


extern void lua_xmove (lua_State *from, lua_State *to, int n) {
  int i;
  if (from == to) return;
  ((void) 0);
  ((void)from, ((void)0));
  ((void)from, ((void)0));
  ((void)from, ((void)0));
  from->top.p -= n;
  for (i = 0; i < n; i++) {
    { TValue *io1=((&(to->top.p)->val)); const TValue *io2=((&(from->top.p + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)to, ((void)0)); ((void)0); };
    to->top.p++;
  }
  ((void) 0);
}


extern lua_CFunction lua_atpanic (lua_State *L, lua_CFunction panicf) {
  lua_CFunction old;
  ((void) 0);
  old = (L->l_G)->panic;
  (L->l_G)->panic = panicf;
  ((void) 0);
  return old;
}


extern lua_Number lua_version (lua_State *L) {
  ((void)(L));
  return 504;
}
extern int lua_absindex (lua_State *L, int idx) {
  return (idx > 0 || ((idx) <= (-1000000 - 1000)))
         ? idx
         : ((int)((L->top.p - L->ci->func.p))) + idx;
}


extern int lua_gettop (lua_State *L) {
  return ((int)((L->top.p - (L->ci->func.p + 1))));
}


extern void lua_settop (lua_State *L, int idx) {
  CallInfo *ci;
  StkId func, newtop;
  ptrdiff_t diff;
  ((void) 0);
  ci = L->ci;
  func = ci->func.p;
  if (idx >= 0) {
    ((void)L, ((void)0));
    diff = ((func + 1) + idx) - L->top.p;
    for (; diff > 0; diff--)
      (((&(L->top.p++)->val))->tt_=(((0) | ((0) << 4))));
  }
  else {
    ((void)L, ((void)0));
    diff = idx + 1;
  }
  ((void)L, ((void)0));
  newtop = L->top.p + diff;
  if (diff < 0 && L->tbclist.p >= newtop) {
    ((void)0);
    newtop = luaF_close(L, newtop, (-1), 0);
  }
  L->top.p = newtop;
  ((void) 0);
}


extern void lua_closeslot (lua_State *L, int idx) {
  StkId level;
  ((void) 0);
  level = index2stack(L, idx);
  ((void)L, ((void)0))
                                           ;
  level = luaF_close(L, level, (-1), 0);
  (((&(level)->val))->tt_=(((0) | ((0) << 4))));
  ((void) 0);
}
static inline void reverse (lua_State *L, StkId from, StkId to) {
  for (; from < to; from++, to--) {
    TValue temp;
    { TValue *io1=(&temp); const TValue *io2=((&(from)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    { TValue *io1=((&(from)->val)); const TValue *io2=((&(to)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    { TValue *io1=((&(to)->val)); const TValue *io2=(&temp); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  }
}






extern void lua_rotate (lua_State *L, int idx, int n) {
  StkId p, t, m;
  ((void) 0);
  t = L->top.p - 1;
  p = index2stack(L, idx);
  ((void)L, ((void)0));
  m = (n >= 0 ? t - n : p - n - 1);
  reverse(L, p, m);
  reverse(L, m + 1, t);
  reverse(L, p, t);
  ((void) 0);
}


extern void lua_copy (lua_State *L, int fromidx, int toidx) {
  TValue *fr, *to;
  ((void) 0);
  fr = index2value(L, fromidx);
  to = index2value(L, toidx);
  ((void)L, ((void)0));
  { TValue *io1=(to); const TValue *io2=(fr); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  if (((toidx) < (-1000000 - 1000)))
    ( (((fr)->tt_) & (1 << 6)) ? ( ((((((&((((union GCUnion *)(((((&(L->ci->func.p)->val))->value_).gc))))->cl.c))))->marked) & ((1<<(5)))) && ((((((fr)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((((&((((union GCUnion *)(((((&(L->ci->func.p)->val))->value_).gc))))->cl.c))))))->gc)),(&(((union GCUnion *)(((((fr)->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));


  ((void) 0);
}


extern void lua_pushvalue (lua_State *L, int idx) {
  ((void) 0);
  { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(index2value(L, idx)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}
extern int lua_type (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return ((!(((((((o))->tt_)) & 0x0F)) == (0)) || o != &(L->l_G)->nilvalue) ? (((((o)->tt_)) & 0x0F)) : (-1));
}


extern const char *lua_typename (lua_State *L, int t) {
  ((void)(L));
  ((void)L, ((void)0));
  return luaT_typenames_[(t) + 1];
}


extern int lua_iscfunction (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (((((o))->tt_) == (((6) | ((1) << 4)))) || (((((o))->tt_) == (((((6) | ((2) << 4))) | (1 << 6))))));
}


extern int lua_isinteger (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return ((((o))->tt_) == (((3) | ((0) << 4))));
}


extern int lua_isnumber (lua_State *L, int idx) {
  lua_Number n;
  const TValue *o = index2value(L, idx);
  return (((((o))->tt_) == (((3) | ((1) << 4)))) ? (*(&n) = (((o)->value_).n), 1) : luaV_tonumber_(o,&n));
}


extern int lua_isstring (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return ((((((((o))->tt_)) & 0x0F)) == (4)) || (((((((o))->tt_)) & 0x0F)) == (3)));
}


extern int lua_isuserdata (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (((((o))->tt_) == (((((7) | ((0) << 4))) | (1 << 6)))) || ((((o))->tt_) == (((2) | ((0) << 4)))));
}


extern int lua_rawequal (lua_State *L, int index1, int index2) {
  const TValue *o1 = index2value(L, index1);
  const TValue *o2 = index2value(L, index2);
  return ((!(((((((o1))->tt_)) & 0x0F)) == (0)) || o1 != &(L->l_G)->nilvalue) && (!(((((((o2))->tt_)) & 0x0F)) == (0)) || o2 != &(L->l_G)->nilvalue)) ? luaV_equalobj(
                                             ((void *)0)
                                             ,o1,o2) : 0;
}


extern void lua_arith (lua_State *L, int op) {
  ((void) 0);
  if (op != 12 && op != 13)
    ((void)L, ((void)0));
  else {
    ((void)L, ((void)0));
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    {L->top.p++; ((void)L, ((void)0));};
  }

  luaO_arith(L, op, (&(L->top.p - 2)->val), (&(L->top.p - 1)->val), L->top.p - 2);
  L->top.p--;
  ((void) 0);
}


extern int lua_compare (lua_State *L, int index1, int index2, int op) {
  const TValue *o1;
  const TValue *o2;
  int i = 0;
  ((void) 0);
  o1 = index2value(L, index1);
  o2 = index2value(L, index2);
  if ((!(((((((o1))->tt_)) & 0x0F)) == (0)) || o1 != &(L->l_G)->nilvalue) && (!(((((((o2))->tt_)) & 0x0F)) == (0)) || o2 != &(L->l_G)->nilvalue)) {
    switch (op) {
      case 0: i = luaV_equalobj(L, o1, o2); break;
      case 1: i = luaV_lessthan(L, o1, o2); break;
      case 2: i = luaV_lessequal(L, o1, o2); break;
      default: ((void)L, ((void)0));
    }
  }
  ((void) 0);
  return i;
}


extern size_t lua_stringtonumber (lua_State *L, const char *s) {
  size_t sz = luaO_str2num(s, (&(L->top.p)->val));
  if (sz != 0)
    {L->top.p++; ((void)L, ((void)0));};
  return sz;
}


extern lua_Number lua_tonumberx (lua_State *L, int idx, int *pisnum) {
  lua_Number n = 0;
  const TValue *o = index2value(L, idx);
  int isnum = (((((o))->tt_) == (((3) | ((1) << 4)))) ? (*(&n) = (((o)->value_).n), 1) : luaV_tonumber_(o,&n));
  if (pisnum)
    *pisnum = isnum;
  return n;
}


extern lua_Integer lua_tointegerx (lua_State *L, int idx, int *pisnum) {
  lua_Integer res = 0;
  const TValue *o = index2value(L, idx);
  int isnum = ((((((o))->tt_) == (((3) | ((0) << 4))))) ? (*(&res) = (((o)->value_).i), 1) : luaV_tointeger(o,&res,F2Ieq));
  if (pisnum)
    *pisnum = isnum;
  return res;
}


extern int lua_toboolean (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return !(((((o))->tt_) == (((1) | ((0) << 4)))) || (((((((o))->tt_)) & 0x0F)) == (0)));
}


extern const char *lua_tolstring (lua_State *L, int idx, size_t *len) {
  TValue *o;
  ((void) 0);
  o = index2value(L, idx);
  if (!(((((((o))->tt_)) & 0x0F)) == (4))) {
    if (!(((((((o))->tt_)) & 0x0F)) == (3))) {
      if (len != 
                ((void *)0)
                    ) *len = 0;
      ((void) 0);
      return 
            ((void *)0)
                ;
    }
    luaO_tostring(L, o);
    { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
    o = index2value(L, idx);
  }
  if (len != 
            ((void *)0)
                )
    *len = ((((&((((union GCUnion *)((((o)->value_).gc))))->ts))))->tt == ((4) | ((0) << 4)) ? (((&((((union GCUnion *)((((o)->value_).gc))))->ts))))->shrlen : (((&((((union GCUnion *)((((o)->value_).gc))))->ts))))->u.lnglen);
  ((void) 0);
  return ((((&((((union GCUnion *)((((o)->value_).gc))))->ts))))->contents);
}


extern lua_Unsigned lua_rawlen (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  switch (((((o)->tt_)) & 0x3F)) {
    case ((4) | ((0) << 4)): return ((&((((union GCUnion *)((((o)->value_).gc))))->ts)))->shrlen;
    case ((4) | ((1) << 4)): return ((&((((union GCUnion *)((((o)->value_).gc))))->ts)))->u.lnglen;
    case ((7) | ((0) << 4)): return ((&((((union GCUnion *)((((o)->value_).gc))))->u)))->len;
    case ((5) | ((0) << 4)): return luaH_getn(((&((((union GCUnion *)((((o)->value_).gc))))->h))));
    default: return 0;
  }
}


extern lua_CFunction lua_tocfunction (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  if (((((o))->tt_) == (((6) | ((1) << 4))))) return (((o)->value_).f);
  else if (((((o))->tt_) == (((((6) | ((2) << 4))) | (1 << 6)))))
    return ((&((((union GCUnion *)((((o)->value_).gc))))->cl.c)))->f;
  else return 
             ((void *)0)
                 ;
}


static inline void *touserdata (const TValue *o) {
  switch ((((((o)->tt_)) & 0x0F))) {
    case 7: return (((char *)((((&((((union GCUnion *)((((o)->value_).gc))))->u)))))) + (((((&((((union GCUnion *)((((o)->value_).gc))))->u))))->nuvalue) == 0 ? 
                              ((long)&((Udata0
                              *)0)->bindata
                              ) 
                              : 
                              ((long)&((Udata
                              *)0)->uv
                              ) 
                              + (sizeof(UValue) * ((((&((((union GCUnion *)((((o)->value_).gc))))->u))))->nuvalue))));
    case 2: return (((o)->value_).p);
    default: return 
                   ((void *)0)
                       ;
  }
}


extern void *lua_touserdata (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return touserdata(o);
}


extern lua_State *lua_tothread (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  return (!((((o))->tt_) == (((((8) | ((0) << 4))) | (1 << 6))))) ? 
                           ((void *)0) 
                                : ((&((((union GCUnion *)((((o)->value_).gc))))->th)));
}
extern const void *lua_topointer (lua_State *L, int idx) {
  const TValue *o = index2value(L, idx);
  switch (((((o)->tt_)) & 0x3F)) {
    case ((6) | ((1) << 4)): return ((void *)((((size_t)(((((o)->value_).f)))))));
    case ((7) | ((0) << 4)): case ((2) | ((0) << 4)):
      return touserdata(o);
    default: {
      if ((((o)->tt_) & (1 << 6)))
        return (((o)->value_).gc);
      else
        return 
              ((void *)0)
                  ;
    }
  }
}
extern void lua_pushnil (lua_State *L) {
  ((void) 0);
  (((&(L->top.p)->val))->tt_=(((0) | ((0) << 4))));
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}


extern void lua_pushnumber (lua_State *L, lua_Number n) {
  ((void) 0);
  { TValue *io=((&(L->top.p)->val)); ((io)->value_).n=(n); ((io)->tt_=(((3) | ((1) << 4)))); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}


extern void lua_pushinteger (lua_State *L, lua_Integer n) {
  ((void) 0);
  { TValue *io=((&(L->top.p)->val)); ((io)->value_).i=(n); ((io)->tt_=(((3) | ((0) << 4)))); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}







extern const char *lua_pushlstring (lua_State *L, const char *s, size_t len) {
  TString *ts;
  ((void) 0);
  ts = (len == 0) ? luaS_new(L, "") : luaS_newlstr(L, s, len);
  { TValue *io = ((&(L->top.p)->val)); TString *x_ = (ts); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
  return ((ts)->contents);
}


extern const char *lua_pushstring (lua_State *L, const char *s) {
  ((void) 0);
  if (s == 
          ((void *)0)
              )
    (((&(L->top.p)->val))->tt_=(((0) | ((0) << 4))));
  else {
    TString *ts;
    ts = luaS_new(L, s);
    { TValue *io = ((&(L->top.p)->val)); TString *x_ = (ts); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    s = ((ts)->contents);
  }
  {L->top.p++; ((void)L, ((void)0));};
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
  return s;
}


extern const char *lua_pushvfstring (lua_State *L, const char *fmt,
                                      va_list argp) {
  const char *ret;
  ((void) 0);
  ret = luaO_pushvfstring(L, fmt, argp);
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
  return ret;
}


extern const char *lua_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *ret;
  va_list argp;
  ((void) 0);
  
 __builtin_va_start(
 argp
 ,
 fmt
 )
                    ;
  ret = luaO_pushvfstring(L, fmt, argp);
  
 __builtin_va_end(
 argp
 )
             ;
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
  return ret;
}


extern void lua_pushcclosure (lua_State *L, lua_CFunction fn, int n) {
  ((void) 0);
  if (n == 0) {
    { TValue *io=((&(L->top.p)->val)); ((io)->value_).f=(fn); ((io)->tt_=(((6) | ((1) << 4)))); };
    {L->top.p++; ((void)L, ((void)0));};
  }
  else {
    CClosure *cl;
    ((void)L, ((void)0));
    ((void)L, ((void)0));
    cl = luaF_newCclosure(L, n);
    cl->f = fn;
    L->top.p -= n;
    while (n--) {
      { TValue *io1=(&cl->upvalue[n]); const TValue *io2=((&(L->top.p + n)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };

      ((void)0);
    }
    { TValue *io = ((&(L->top.p)->val)); CClosure *x_ = (cl); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((6) | ((2) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
    { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  }
  ((void) 0);
}


extern void lua_pushboolean (lua_State *L, int b) {
  ((void) 0);
  if (b)
    (((&(L->top.p)->val))->tt_=(((1) | ((1) << 4))));
  else
    (((&(L->top.p)->val))->tt_=(((1) | ((0) << 4))));
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}


extern void lua_pushlightuserdata (lua_State *L, void *p) {
  ((void) 0);
  { TValue *io=((&(L->top.p)->val)); ((io)->value_).p=(p); ((io)->tt_=(((2) | ((0) << 4)))); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}


extern int lua_pushthread (lua_State *L) {
  ((void) 0);
  { TValue *io = ((&(L->top.p)->val)); lua_State *x_ = (L); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((8) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
  return ((L->l_G)->mainthread == L);
}
static inline int auxgetstr (lua_State *L, const TValue *t, const char *k) {
  const TValue *slot;
  TString *str = luaS_new(L, k);
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = luaH_getstr(((&((((union GCUnion *)((((t)->value_).gc))))->h))), str), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    {L->top.p++; ((void)L, ((void)0));};
  }
  else {
    { TValue *io = ((&(L->top.p)->val)); TString *x_ = (str); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
    luaV_finishget(L, t, (&(L->top.p - 1)->val), L->top.p - 1, slot);
  }
  ((void) 0);
  return ((((((&(L->top.p - 1)->val))->tt_)) & 0x0F));
}
extern int lua_getglobal (lua_State *L, const char *name) {
  const TValue *G;
  ((void) 0);
  G = (&((&((((union GCUnion *)((((&(L->l_G)->l_registry)->value_).gc))))->h)))->array[1]);
  return auxgetstr(L, G, name);
}


extern int lua_gettable (lua_State *L, int idx) {
  const TValue *slot;
  TValue *t;
  ((void) 0);
  t = index2value(L, idx);
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = luaH_get(((&((((union GCUnion *)((((t)->value_).gc))))->h))), (&(L->top.p - 1)->val)), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { TValue *io1=((&(L->top.p - 1)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  }
  else
    luaV_finishget(L, t, (&(L->top.p - 1)->val), L->top.p - 1, slot);
  ((void) 0);
  return ((((((&(L->top.p - 1)->val))->tt_)) & 0x0F));
}


extern int lua_getfield (lua_State *L, int idx, const char *k) {
  ((void) 0);
  return auxgetstr(L, index2value(L, idx), k);
}


extern int lua_geti (lua_State *L, int idx, lua_Integer n) {
  TValue *t;
  const TValue *slot;
  ((void) 0);
  t = index2value(L, idx);
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = (((lua_Unsigned)(n)) - 1u < ((&((((union GCUnion *)((((t)->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)((((t)->value_).gc))))->h)))->array[n - 1] : luaH_getint(((&((((union GCUnion *)((((t)->value_).gc))))->h))), n), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  }
  else {
    TValue aux;
    { TValue *io=(&aux); ((io)->value_).i=(n); ((io)->tt_=(((3) | ((0) << 4)))); };
    luaV_finishget(L, t, &aux, L->top.p, slot);
  }
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
  return ((((((&(L->top.p - 1)->val))->tt_)) & 0x0F));
}


static inline int finishrawget (lua_State *L, const TValue *val) {
  if ((((((((val))->tt_)) & 0x0F)) == (0)))
    (((&(L->top.p)->val))->tt_=(((0) | ((0) << 4))));
  else
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(val); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
  return ((((((&(L->top.p - 1)->val))->tt_)) & 0x0F));
}


static Table *gettable (lua_State *L, int idx) {
  TValue *t = index2value(L, idx);
  ((void)L, ((void)0));
  return ((&((((union GCUnion *)((((t)->value_).gc))))->h)));
}


extern int lua_rawget (lua_State *L, int idx) {
  Table *t;
  const TValue *val;
  ((void) 0);
  ((void)L, ((void)0));
  t = gettable(L, idx);
  val = luaH_get(t, (&(L->top.p - 1)->val));
  L->top.p--;
  return finishrawget(L, val);
}


extern int lua_rawgeti (lua_State *L, int idx, lua_Integer n) {
  Table *t;
  ((void) 0);
  t = gettable(L, idx);
  return finishrawget(L, luaH_getint(t, n));
}


extern int lua_rawgetp (lua_State *L, int idx, const void *p) {
  Table *t;
  TValue k;
  ((void) 0);
  t = gettable(L, idx);
  { TValue *io=(&k); ((io)->value_).p=(((void *)((p)))); ((io)->tt_=(((2) | ((0) << 4)))); };
  return finishrawget(L, luaH_get(t, &k));
}


extern void lua_createtable (lua_State *L, int narray, int nrec) {
  Table *t;
  ((void) 0);
  t = luaH_new(L);
  { TValue *io = ((&(L->top.p)->val)); Table *x_ = (t); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  if (narray > 0 || nrec > 0)
    luaH_resize(L, t, narray, nrec);
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
}


extern int lua_getmetatable (lua_State *L, int objindex) {
  const TValue *obj;
  Table *mt;
  int res = 0;
  ((void) 0);
  obj = index2value(L, objindex);
  switch ((((((obj)->tt_)) & 0x0F))) {
    case 5:
      mt = ((&((((union GCUnion *)((((obj)->value_).gc))))->h)))->metatable;
      break;
    case 7:
      mt = ((&((((union GCUnion *)((((obj)->value_).gc))))->u)))->metatable;
      break;
    default:
      mt = (L->l_G)->mt[(((((obj)->tt_)) & 0x0F))];
      break;
  }
  if (mt != 
           ((void *)0)
               ) {
    { TValue *io = ((&(L->top.p)->val)); Table *x_ = (mt); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
    res = 1;
  }
  ((void) 0);
  return res;
}


extern int lua_getiuservalue (lua_State *L, int idx, int n) {
  TValue *o;
  int t;
  ((void) 0);
  o = index2value(L, idx);
  ((void)L, ((void)0));
  if (n <= 0 || n > ((&((((union GCUnion *)((((o)->value_).gc))))->u)))->nuvalue) {
    (((&(L->top.p)->val))->tt_=(((0) | ((0) << 4))));
    t = (-1);
  }
  else {
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(&((&((((union GCUnion *)((((o)->value_).gc))))->u)))->uv[n - 1].uv); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    t = ((((((&(L->top.p)->val))->tt_)) & 0x0F));
  }
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
  return t;
}
static void auxsetstr (lua_State *L, const TValue *t, const char *k) {
  const TValue *slot;
  TString *str = luaS_new(L, k);
  ((void)L, ((void)0));
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = luaH_getstr(((&((((union GCUnion *)((((t)->value_).gc))))->h))), str), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { { TValue *io1=(((TValue *)(slot))); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((((t)->value_).gc))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((t)->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
    L->top.p--;
  }
  else {
    { TValue *io = ((&(L->top.p)->val)); TString *x_ = (str); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
    luaV_finishset(L, t, (&(L->top.p - 1)->val), (&(L->top.p - 2)->val), slot);
    L->top.p -= 2;
  }
  ((void) 0);
}


extern void lua_setglobal (lua_State *L, const char *name) {
  const TValue *G;
  ((void) 0);
  G = (&((&((((union GCUnion *)((((&(L->l_G)->l_registry)->value_).gc))))->h)))->array[1]);
  auxsetstr(L, G, name);
}


extern void lua_settable (lua_State *L, int idx) {
  TValue *t;
  const TValue *slot;
  ((void) 0);
  ((void)L, ((void)0));
  t = index2value(L, idx);
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = luaH_get(((&((((union GCUnion *)((((t)->value_).gc))))->h))), (&(L->top.p - 2)->val)), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { { TValue *io1=(((TValue *)(slot))); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((((t)->value_).gc))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((t)->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
  }
  else
    luaV_finishset(L, t, (&(L->top.p - 2)->val), (&(L->top.p - 1)->val), slot);
  L->top.p -= 2;
  ((void) 0);
}


extern void lua_setfield (lua_State *L, int idx, const char *k) {
  ((void) 0);
  auxsetstr(L, index2value(L, idx), k);
}


extern void lua_seti (lua_State *L, int idx, lua_Integer n) {
  TValue *t;
  const TValue *slot;
  ((void) 0);
  ((void)L, ((void)0));
  t = index2value(L, idx);
  if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
     ((void *)0)
     , 0) : (slot = (((lua_Unsigned)(n)) - 1u < ((&((((union GCUnion *)((((t)->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)((((t)->value_).gc))))->h)))->array[n - 1] : luaH_getint(((&((((union GCUnion *)((((t)->value_).gc))))->h))), n), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
    { { TValue *io1=(((TValue *)(slot))); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((((t)->value_).gc))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((t)->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
  }
  else {
    TValue aux;
    { TValue *io=(&aux); ((io)->value_).i=(n); ((io)->tt_=(((3) | ((0) << 4)))); };
    luaV_finishset(L, t, &aux, (&(L->top.p - 1)->val), slot);
  }
  L->top.p--;
  ((void) 0);
}


static void aux_rawset (lua_State *L, int idx, TValue *key, int n) {
  Table *t;
  ((void) 0);
  ((void)L, ((void)0));
  t = gettable(L, idx);
  luaH_set(L, t, key, (&(L->top.p - 1)->val));
  ((t)->flags &= ~(~(~0u << (TM_EQ + 1))));
  ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((&(((union GCUnion *)((t)))->gc)))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(&(((union GCUnion *)((t)))->gc))) : ((void)((0)))) : ((void)((0))));
  L->top.p -= n;
  ((void) 0);
}


extern void lua_rawset (lua_State *L, int idx) {
  aux_rawset(L, idx, (&(L->top.p - 2)->val), 2);
}


extern void lua_rawsetp (lua_State *L, int idx, const void *p) {
  TValue k;
  { TValue *io=(&k); ((io)->value_).p=(((void *)((p)))); ((io)->tt_=(((2) | ((0) << 4)))); };
  aux_rawset(L, idx, &k, 1);
}


extern void lua_rawseti (lua_State *L, int idx, lua_Integer n) {
  Table *t;
  ((void) 0);
  ((void)L, ((void)0));
  t = gettable(L, idx);
  luaH_setint(L, t, n, (&(L->top.p - 1)->val));
  ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((&(((union GCUnion *)((t)))->gc)))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(&(((union GCUnion *)((t)))->gc))) : ((void)((0)))) : ((void)((0))));
  L->top.p--;
  ((void) 0);
}


extern int lua_setmetatable (lua_State *L, int objindex) {
  TValue *obj;
  Table *mt;
  ((void) 0);
  ((void)L, ((void)0));
  obj = index2value(L, objindex);
  if (((((((((&(L->top.p - 1)->val)))->tt_)) & 0x0F)) == (0)))
    mt = 
        ((void *)0)
            ;
  else {
    ((void)L, ((void)0));
    mt = ((&((((union GCUnion *)(((((&(L->top.p - 1)->val))->value_).gc))))->h)));
  }
  switch ((((((obj)->tt_)) & 0x0F))) {
    case 5: {
      ((&((((union GCUnion *)((((obj)->value_).gc))))->h)))->metatable = mt;
      if (mt) {
        ( (((((((obj)->value_).gc))->marked) & ((1<<(5)))) && (((mt)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)(((((obj)->value_).gc))))->gc)),(&(((union GCUnion *)((mt)))->gc))) : ((void)((0))));
        luaC_checkfinalizer(L, (((obj)->value_).gc), mt);
      }
      break;
    }
    case 7: {
      ((&((((union GCUnion *)((((obj)->value_).gc))))->u)))->metatable = mt;
      if (mt) {
        ( ((((((&((((union GCUnion *)((((obj)->value_).gc))))->u))))->marked) & ((1<<(5)))) && (((mt)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((((&((((union GCUnion *)((((obj)->value_).gc))))->u))))))->gc)),(&(((union GCUnion *)((mt)))->gc))) : ((void)((0))));
        luaC_checkfinalizer(L, (((obj)->value_).gc), mt);
      }
      break;
    }
    default: {
      (L->l_G)->mt[(((((obj)->tt_)) & 0x0F))] = mt;
      break;
    }
  }
  L->top.p--;
  ((void) 0);
  return 1;
}


extern int lua_setiuservalue (lua_State *L, int idx, int n) {
  TValue *o;
  int res;
  ((void) 0);
  ((void)L, ((void)0));
  o = index2value(L, idx);
  ((void)L, ((void)0));
  if (!(((unsigned int)((n))) - 1u < ((unsigned int)((((&((((union GCUnion *)((((o)->value_).gc))))->u)))->nuvalue)))))
    res = 0;
  else {
    { TValue *io1=(&((&((((union GCUnion *)((((o)->value_).gc))))->u)))->uv[n - 1].uv); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    ( ((((&(L->top.p - 1)->val))->tt_) & (1 << 6)) ? ( (((((((o)->value_).gc))->marked) & ((1<<(5)))) && (((((((&(L->top.p - 1)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((o)->value_).gc)) : ((void)((0)))) : ((void)((0))));
    res = 1;
  }
  L->top.p--;
  ((void) 0);
  return res;
}
extern void lua_callk (lua_State *L, int nargs, int nresults,
                        lua_KContext ctx, lua_KFunction k) {
  StkId func;
  ((void) 0);
  ((void)L, ((void)0))
                                            ;
  ((void)L, ((void)0));
  ((void)L, ((void)0));
  ((void)L, ((void)0));
  func = L->top.p - (nargs+1);
  if (k != 
          ((void *)0) 
               && (((L)->nCcalls & 0xffff0000) == 0)) {
    L->ci->u.c.k = k;
    L->ci->u.c.ctx = ctx;
    luaD_call(L, func, nresults);
  }
  else
    luaD_callnoyield(L, func, nresults);
  { if ((nresults) <= (-1) && L->ci->top.p < L->top.p) L->ci->top.p = L->top.p; };
  ((void) 0);
}






struct CallS {
  StkId func;
  int nresults;
};


static void f_call (lua_State *L, void *ud) {
  struct CallS *c = ((struct CallS *)(ud));
  luaD_callnoyield(L, c->func, c->nresults);
}



extern int lua_pcallk (lua_State *L, int nargs, int nresults, int errfunc,
                        lua_KContext ctx, lua_KFunction k) {
  struct CallS c;
  int status;
  ptrdiff_t func;
  ((void) 0);
  ((void)L, ((void)0))
                                            ;
  ((void)L, ((void)0));
  ((void)L, ((void)0));
  ((void)L, ((void)0));
  if (errfunc == 0)
    func = 0;
  else {
    StkId o = index2stack(L, errfunc);
    ((void)L, ((void)0));
    func = (((char *)((o))) - ((char *)((L->stack.p))));
  }
  c.func = L->top.p - (nargs+1);
  if (k == 
          ((void *)0) 
               || !(((L)->nCcalls & 0xffff0000) == 0)) {
    c.nresults = nresults;
    status = luaD_pcall(L, f_call, &c, (((char *)((c.func))) - ((char *)((L->stack.p)))), func);
  }
  else {
    CallInfo *ci = L->ci;
    ci->u.c.k = k;
    ci->u.c.ctx = ctx;

    ci->u2.funcidx = ((int)(((((char *)((c.func))) - ((char *)((L->stack.p)))))));
    ci->u.c.old_errfunc = L->errfunc;
    L->errfunc = func;
    ((ci->callstatus) = ((ci->callstatus) & ~(1<<0)) | (L->allowhook));
    ci->callstatus |= (1<<4);
    luaD_call(L, c.func, nresults);
    ci->callstatus &= ~(1<<4);
    L->errfunc = ci->u.c.old_errfunc;
    status = 0;
  }
  { if ((nresults) <= (-1) && L->ci->top.p < L->top.p) L->ci->top.p = L->top.p; };
  ((void) 0);
  return status;
}


extern int lua_load (lua_State *L, lua_Reader reader, void *data,
                      const char *chunkname, const char *mode) {
  ZIO z;
  int status;
  ((void) 0);
  if (!chunkname) chunkname = "?";
  luaZ_init(L, &z, reader, data);
  status = luaD_protectedparser(L, &z, chunkname, mode);
  if (status == 0) {
    LClosure *f = ((&((((union GCUnion *)(((((&(L->top.p - 1)->val))->value_).gc))))->cl.l)));
    if (f->nupvalues >= 1) {

      const TValue *gt = (&((&((((union GCUnion *)((((&(L->l_G)->l_registry)->value_).gc))))->h)))->array[1]);

      { TValue *io1=(f->upvals[0]->v.p); const TValue *io2=(gt); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      ( (((gt)->tt_) & (1 << 6)) ? ( ((((f->upvals[0])->marked) & ((1<<(5)))) && ((((((gt)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((f->upvals[0])))->gc)),(&(((union GCUnion *)(((((gt)->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));
    }
  }
  ((void) 0);
  return status;
}


extern int lua_dump (lua_State *L, lua_Writer writer, void *data, int strip) {
  int status;
  TValue *o;
  ((void) 0);
  ((void)L, ((void)0));
  o = (&(L->top.p - 1)->val);
  if (((((o))->tt_) == (((((6) | ((0) << 4))) | (1 << 6)))))
    status = luaU_dump(L, (((&((((union GCUnion *)((((o)->value_).gc))))->cl.l)))->p), writer, data, strip);
  else
    status = 1;
  ((void) 0);
  return status;
}


extern int lua_status (lua_State *L) {
  return L->status;
}





extern int lua_gc (lua_State *L, int what, ...) {
  va_list argp;
  int res = 0;
  global_State *g = (L->l_G);
  if (g->gcstp & 2)
    return -1;
  ((void) 0);
  
 __builtin_va_start(
 argp
 ,
 what
 )
                     ;
  switch (what) {
    case 0: {
      g->gcstp = 1;
      break;
    }
    case 1: {
      luaE_setdebt(g, 0);
      g->gcstp = 0;
      break;
    }
    case 2: {
      luaC_fullgc(L, 0);
      break;
    }
    case 3: {

      res = ((int)((((lu_mem)((g)->totalbytes + (g)->GCdebt)) >> 10)));
      break;
    }
    case 4: {
      res = ((int)((((lu_mem)((g)->totalbytes + (g)->GCdebt)) & 0x3ff)));
      break;
    }
    case 5: {
      int data = 
                __builtin_va_arg(
                argp
                ,
                int
                )
                                 ;
      l_mem debt = 1;
      lu_byte oldstp = g->gcstp;
      g->gcstp = 0;
      if (data == 0) {
        luaE_setdebt(g, 0);
        luaC_step(L);
      }
      else {
        debt = ((l_mem)(data)) * 1024 + g->GCdebt;
        luaE_setdebt(g, debt);
        { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
      }
      g->gcstp = oldstp;
      if (debt > 0 && g->gcstate == 8)
        res = 1;
      break;
    }
    case 6: {
      int data = 
                __builtin_va_arg(
                argp
                ,
                int
                )
                                 ;
      res = ((g->gcpause) * 4);
      ((g->gcpause) = (data) / 4);
      break;
    }
    case 7: {
      int data = 
                __builtin_va_arg(
                argp
                ,
                int
                )
                                 ;
      res = ((g->gcstepmul) * 4);
      ((g->gcstepmul) = (data) / 4);
      break;
    }
    case 9: {
      res = ((g)->gcstp == 0);
      break;
    }
    case 10: {
      int minormul = 
                    __builtin_va_arg(
                    argp
                    ,
                    int
                    )
                                     ;
      int majormul = 
                    __builtin_va_arg(
                    argp
                    ,
                    int
                    )
                                     ;
      res = (g->gckind == 1 || g->lastatomic != 0) ? 10 : 11;
      if (minormul != 0)
        g->genminormul = minormul;
      if (majormul != 0)
        ((g->genmajormul) = (majormul) / 4);
      luaC_changemode(L, 1);
      break;
    }
    case 11: {
      int pause = 
                 __builtin_va_arg(
                 argp
                 ,
                 int
                 )
                                  ;
      int stepmul = 
                   __builtin_va_arg(
                   argp
                   ,
                   int
                   )
                                    ;
      int stepsize = 
                    __builtin_va_arg(
                    argp
                    ,
                    int
                    )
                                     ;
      res = (g->gckind == 1 || g->lastatomic != 0) ? 10 : 11;
      if (pause != 0)
        ((g->gcpause) = (pause) / 4);
      if (stepmul != 0)
        ((g->gcstepmul) = (stepmul) / 4);
      if (stepsize != 0)
        g->gcstepsize = stepsize;
      luaC_changemode(L, 0);
      break;
    }
    default: res = -1;
  }
  
 __builtin_va_end(
 argp
 )
             ;
  ((void) 0);
  return res;
}
extern int lua_error (lua_State *L) {
  TValue *errobj;
  ((void) 0);
  errobj = (&(L->top.p - 1)->val);
  ((void)L, ((void)0));

  if (((((errobj))->tt_) == (((((4) | ((0) << 4))) | (1 << 6)))) && ((((&((((union GCUnion *)((((errobj)->value_).gc))))->ts)))) == ((L->l_G)->memerrmsg)))
    luaD_throw(L, 4);
  else
    luaG_errormsg(L);

  return 0;
}


extern int lua_next (lua_State *L, int idx) {
  Table *t;
  int more;
  ((void) 0);
  ((void)L, ((void)0));
  t = gettable(L, idx);
  more = luaH_next(L, t, L->top.p - 1);
  if (more) {
    {L->top.p++; ((void)L, ((void)0));};
  }
  else
    L->top.p -= 1;
  ((void) 0);
  return more;
}


extern void lua_toclose (lua_State *L, int idx) {
  int nresults;
  StkId o;
  ((void) 0);
  o = index2stack(L, idx);
  nresults = L->ci->nresults;
  ((void)L, ((void)0));
  luaF_newtbcupval(L, o);
  if (!((nresults) < (-1)))
    L->ci->nresults = (-(nresults) - 3);
  ((void)0);
  ((void) 0);
}


extern void lua_concat (lua_State *L, int n) {
  ((void) 0);
  ((void)L, ((void)0));
  if (n > 0)
    luaV_concat(L, n);
  else {
    { TValue *io = ((&(L->top.p)->val)); TString *x_ = (luaS_newlstr(L, "", 0)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
  }
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
}


extern void lua_len (lua_State *L, int idx) {
  TValue *t;
  ((void) 0);
  t = index2value(L, idx);
  luaV_objlen(L, L->top.p, t);
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
}


extern lua_Alloc lua_getallocf (lua_State *L, void **ud) {
  lua_Alloc f;
  ((void) 0);
  if (ud) *ud = (L->l_G)->ud;
  f = (L->l_G)->frealloc;
  ((void) 0);
  return f;
}


extern void lua_setallocf (lua_State *L, lua_Alloc f, void *ud) {
  ((void) 0);
  (L->l_G)->ud = ud;
  (L->l_G)->frealloc = f;
  ((void) 0);
}


void lua_setwarnf (lua_State *L, lua_WarnFunction f, void *ud) {
  ((void) 0);
  (L->l_G)->ud_warn = ud;
  (L->l_G)->warnf = f;
  ((void) 0);
}


void lua_warning (lua_State *L, const char *msg, int tocont) {
  ((void) 0);
  luaE_warning(L, msg, tocont);
  ((void) 0);
}



extern void *lua_newuserdatauv (lua_State *L, size_t size, int nuvalue) {
  Udata *u;
  ((void) 0);
  ((void)L, ((void)0));
  u = luaS_newudata(L, size, nuvalue);
  { TValue *io = ((&(L->top.p)->val)); Udata *x_ = (u); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((7) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  ((void) 0);
  return (((char *)((u))) + (((u)->nuvalue) == 0 ? 
        ((long)&((Udata0
        *)0)->bindata
        ) 
        : 
        ((long)&((Udata
        *)0)->uv
        ) 
        + (sizeof(UValue) * ((u)->nuvalue))));
}



static const char *aux_upvalue (TValue *fi, int n, TValue **val,
                                GCObject **owner) {
  switch (((((fi)->tt_)) & 0x3F)) {
    case ((6) | ((2) << 4)): {
      CClosure *f = ((&((((union GCUnion *)((((fi)->value_).gc))))->cl.c)));
      if (!(((unsigned int)((n))) - 1u < ((unsigned int)((f->nupvalues)))))
        return 
              ((void *)0)
                  ;
      *val = &f->upvalue[n-1];
      if (owner) *owner = (&(((union GCUnion *)((f)))->gc));
      return "";
    }
    case ((6) | ((0) << 4)): {
      LClosure *f = ((&((((union GCUnion *)((((fi)->value_).gc))))->cl.l)));
      TString *name;
      Proto *p = f->p;
      if (!(((unsigned int)((n))) - 1u < ((unsigned int)((p->sizeupvalues)))))
        return 
              ((void *)0)
                  ;
      *val = f->upvals[n-1]->v.p;
      if (owner) *owner = (&(((union GCUnion *)((f->upvals[n - 1])))->gc));
      name = p->upvalues[n-1].name;
      return (name == 
                     ((void *)0)
                         ) ? "(no name)" : ((name)->contents);
    }
    default: return 
                   ((void *)0)
                       ;
  }
}


extern const char *lua_getupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val = 
               ((void *)0)
                   ;
  ((void) 0);
  name = aux_upvalue(index2value(L, funcindex), n, &val, 
                                                        ((void *)0)
                                                            );
  if (name) {
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(val); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    {L->top.p++; ((void)L, ((void)0));};
  }
  ((void) 0);
  return name;
}


extern const char *lua_setupvalue (lua_State *L, int funcindex, int n) {
  const char *name;
  TValue *val = 
               ((void *)0)
                   ;
  GCObject *owner = 
                   ((void *)0)
                       ;
  TValue *fi;
  ((void) 0);
  fi = index2value(L, funcindex);
  ((void)L, ((void)0));
  name = aux_upvalue(fi, n, &val, &owner);
  if (name) {
    L->top.p--;
    { TValue *io1=(val); const TValue *io2=((&(L->top.p)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    ( (((val)->tt_) & (1 << 6)) ? ( ((((owner)->marked) & ((1<<(5)))) && ((((((val)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((owner)))->gc)),(&(((union GCUnion *)(((((val)->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));
  }
  ((void) 0);
  return name;
}


static UpVal **getupvalref (lua_State *L, int fidx, int n, LClosure **pf) {
  static const UpVal *const nullup = 
                                    ((void *)0)
                                        ;
  LClosure *f;
  TValue *fi = index2value(L, fidx);
  ((void)L, ((void)0));
  f = ((&((((union GCUnion *)((((fi)->value_).gc))))->cl.l)));
  if (pf) *pf = f;
  if (1 <= n && n <= f->p->sizeupvalues)
    return &f->upvals[n - 1];
  else
    return (UpVal**)&nullup;
}


extern void *lua_upvalueid (lua_State *L, int fidx, int n) {
  TValue *fi = index2value(L, fidx);
  switch (((((fi)->tt_)) & 0x3F)) {
    case ((6) | ((0) << 4)): {
      return *getupvalref(L, fidx, n, 
                                     ((void *)0)
                                         );
    }
    case ((6) | ((2) << 4)): {
      CClosure *f = ((&((((union GCUnion *)((((fi)->value_).gc))))->cl.c)));
      if (1 <= n && n <= f->nupvalues)
        return &f->upvalue[n - 1];

    }
    case ((6) | ((1) << 4)):
      return 
            ((void *)0)
                ;
    default: {
      ((void)L, ((void)0));
      return 
            ((void *)0)
                ;
    }
  }
}


extern void lua_upvaluejoin (lua_State *L, int fidx1, int n1,
                                            int fidx2, int n2) {
  LClosure *f1;
  UpVal **up1 = getupvalref(L, fidx1, n1, &f1);
  UpVal **up2 = getupvalref(L, fidx2, n2, 
                                         ((void *)0)
                                             );
  ((void)L, ((void)0));
  *up1 = *up2;
  ( ((((f1)->marked) & ((1<<(5)))) && (((*up1)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((f1)))->gc)),(&(((union GCUnion *)((*up1)))->gc))) : ((void)((0))));
}



















typedef float float_t;
typedef double double_t;
extern int __fpclassify (double __value)
     ;


extern int __signbit (double __value)
     ;



extern int __isinf (double __value)
  ;


extern int __finite (double __value)
  ;


extern int __isnan (double __value)
  ;


extern int __iseqsig (double __x, double __y) ;


extern int __issignaling (double __value)
     ;
 extern double acos (double __x) ; extern double __acos (double __x) ;

 extern double asin (double __x) ; extern double __asin (double __x) ;

 extern double atan (double __x) ; extern double __atan (double __x) ;

 extern double atan2 (double __y, double __x) ; extern double __atan2 (double __y, double __x) ;


 extern double cos (double __x) ; extern double __cos (double __x) ;

 extern double sin (double __x) ; extern double __sin (double __x) ;

 extern double tan (double __x) ; extern double __tan (double __x) ;




 extern double cosh (double __x) ; extern double __cosh (double __x) ;

 extern double sinh (double __x) ; extern double __sinh (double __x) ;

 extern double tanh (double __x) ; extern double __tanh (double __x) ;
 extern double acosh (double __x) ; extern double __acosh (double __x) ;

 extern double asinh (double __x) ; extern double __asinh (double __x) ;

 extern double atanh (double __x) ; extern double __atanh (double __x) ;





 extern double exp (double __x) ; extern double __exp (double __x) ;


extern double frexp (double __x, int *__exponent) ; extern double __frexp (double __x, int *__exponent) ;


extern double ldexp (double __x, int __exponent) ; extern double __ldexp (double __x, int __exponent) ;


 extern double log (double __x) ; extern double __log (double __x) ;


 extern double log10 (double __x) ; extern double __log10 (double __x) ;


extern double modf (double __x, double *__iptr) ; extern double __modf (double __x, double *__iptr) ;
 extern double expm1 (double __x) ; extern double __expm1 (double __x) ;


 extern double log1p (double __x) ; extern double __log1p (double __x) ;


extern double logb (double __x) ; extern double __logb (double __x) ;




 extern double exp2 (double __x) ; extern double __exp2 (double __x) ;


 extern double log2 (double __x) ; extern double __log2 (double __x) ;






 extern double pow (double __x, double __y) ; extern double __pow (double __x, double __y) ;


extern double sqrt (double __x) ; extern double __sqrt (double __x) ;



 extern double hypot (double __x, double __y) ; extern double __hypot (double __x, double __y) ;




 extern double cbrt (double __x) ; extern double __cbrt (double __x) ;






extern double ceil (double __x) ; extern double __ceil (double __x) ;


extern double fabs (double __x) ; extern double __fabs (double __x) ;


extern double floor (double __x) ; extern double __floor (double __x) ;


extern double fmod (double __x, double __y) ; extern double __fmod (double __x, double __y) ;
extern double copysign (double __x, double __y) ; extern double __copysign (double __x, double __y) ;




extern double nan (const char *__tagb) ; extern double __nan (const char *__tagb) ;
extern double j0 (double) ; extern double __j0 (double) ;
extern double j1 (double) ; extern double __j1 (double) ;
extern double jn (int, double) ; extern double __jn (int, double) ;
extern double y0 (double) ; extern double __y0 (double) ;
extern double y1 (double) ; extern double __y1 (double) ;
extern double yn (int, double) ; extern double __yn (int, double) ;





 extern double erf (double) ; extern double __erf (double) ;
 extern double erfc (double) ; extern double __erfc (double) ;
extern double lgamma (double) ; extern double __lgamma (double) ;




extern double tgamma (double) ; extern double __tgamma (double) ;
extern double rint (double __x) ; extern double __rint (double __x) ;


extern double nextafter (double __x, double __y) ; extern double __nextafter (double __x, double __y) ;

extern double nexttoward (double __x, double __y) ; extern double __nexttoward (double __x, double __y) ;
extern double remainder (double __x, double __y) ; extern double __remainder (double __x, double __y) ;



extern double scalbn (double __x, int __n) ; extern double __scalbn (double __x, int __n) ;



extern int ilogb (double __x) ; extern int __ilogb (double __x) ;
extern double scalbln (double __x, long int __n) ; extern double __scalbln (double __x, long int __n) ;



extern double nearbyint (double __x) ; extern double __nearbyint (double __x) ;



extern double round (double __x) ; extern double __round (double __x) ;



extern double trunc (double __x) ; extern double __trunc (double __x) ;




extern double remquo (double __x, double __y, int *__quo) ; extern double __remquo (double __x, double __y, int *__quo) ;






extern long int lrint (double __x) ; extern long int __lrint (double __x) ;

extern long long int llrint (double __x) ; extern long long int __llrint (double __x) ;



extern long int lround (double __x) ; extern long int __lround (double __x) ;

extern long long int llround (double __x) ; extern long long int __llround (double __x) ;



extern double fdim (double __x, double __y) ; extern double __fdim (double __x, double __y) ;



extern double fmax (double __x, double __y) ; extern double __fmax (double __x, double __y) ;


extern double fmin (double __x, double __y) ; extern double __fmin (double __x, double __y) ;



extern double fma (double __x, double __y, double __z) ; extern double __fma (double __x, double __y, double __z) ;
extern double scalb (double __x, double __n) ; extern double __scalb (double __x, double __n) ;
extern int __fpclassifyf (float __value)
     ;


extern int __signbitf (float __value)
     ;



extern int __isinff (float __value)
  ;


extern int __finitef (float __value)
  ;


extern int __isnanf (float __value)
  ;


extern int __iseqsigf (float __x, float __y) ;


extern int __issignalingf (float __value)
     ;
 extern float acosf (float __x) ; extern float __acosf (float __x) ;

 extern float asinf (float __x) ; extern float __asinf (float __x) ;

 extern float atanf (float __x) ; extern float __atanf (float __x) ;

 extern float atan2f (float __y, float __x) ; extern float __atan2f (float __y, float __x) ;


 extern float cosf (float __x) ; extern float __cosf (float __x) ;

 extern float sinf (float __x) ; extern float __sinf (float __x) ;

 extern float tanf (float __x) ; extern float __tanf (float __x) ;




 extern float coshf (float __x) ; extern float __coshf (float __x) ;

 extern float sinhf (float __x) ; extern float __sinhf (float __x) ;

 extern float tanhf (float __x) ; extern float __tanhf (float __x) ;
 extern float acoshf (float __x) ; extern float __acoshf (float __x) ;

 extern float asinhf (float __x) ; extern float __asinhf (float __x) ;

 extern float atanhf (float __x) ; extern float __atanhf (float __x) ;





 extern float expf (float __x) ; extern float __expf (float __x) ;


extern float frexpf (float __x, int *__exponent) ; extern float __frexpf (float __x, int *__exponent) ;


extern float ldexpf (float __x, int __exponent) ; extern float __ldexpf (float __x, int __exponent) ;


 extern float logf (float __x) ; extern float __logf (float __x) ;


 extern float log10f (float __x) ; extern float __log10f (float __x) ;


extern float modff (float __x, float *__iptr) ; extern float __modff (float __x, float *__iptr) ;
 extern float expm1f (float __x) ; extern float __expm1f (float __x) ;


 extern float log1pf (float __x) ; extern float __log1pf (float __x) ;


extern float logbf (float __x) ; extern float __logbf (float __x) ;




 extern float exp2f (float __x) ; extern float __exp2f (float __x) ;


 extern float log2f (float __x) ; extern float __log2f (float __x) ;






 extern float powf (float __x, float __y) ; extern float __powf (float __x, float __y) ;


extern float sqrtf (float __x) ; extern float __sqrtf (float __x) ;



 extern float hypotf (float __x, float __y) ; extern float __hypotf (float __x, float __y) ;




 extern float cbrtf (float __x) ; extern float __cbrtf (float __x) ;






extern float ceilf (float __x) ; extern float __ceilf (float __x) ;


extern float fabsf (float __x) ; extern float __fabsf (float __x) ;


extern float floorf (float __x) ; extern float __floorf (float __x) ;


extern float fmodf (float __x, float __y) ; extern float __fmodf (float __x, float __y) ;
extern float copysignf (float __x, float __y) ; extern float __copysignf (float __x, float __y) ;




extern float nanf (const char *__tagb) ; extern float __nanf (const char *__tagb) ;
 extern float erff (float) ; extern float __erff (float) ;
 extern float erfcf (float) ; extern float __erfcf (float) ;
extern float lgammaf (float) ; extern float __lgammaf (float) ;




extern float tgammaf (float) ; extern float __tgammaf (float) ;
extern float rintf (float __x) ; extern float __rintf (float __x) ;


extern float nextafterf (float __x, float __y) ; extern float __nextafterf (float __x, float __y) ;

extern float nexttowardf (float __x, double __y) ; extern float __nexttowardf (float __x, double __y) ;
extern float remainderf (float __x, float __y) ; extern float __remainderf (float __x, float __y) ;



extern float scalbnf (float __x, int __n) ; extern float __scalbnf (float __x, int __n) ;



extern int ilogbf (float __x) ; extern int __ilogbf (float __x) ;
extern float scalblnf (float __x, long int __n) ; extern float __scalblnf (float __x, long int __n) ;



extern float nearbyintf (float __x) ; extern float __nearbyintf (float __x) ;



extern float roundf (float __x) ; extern float __roundf (float __x) ;



extern float truncf (float __x) ; extern float __truncf (float __x) ;




extern float remquof (float __x, float __y, int *__quo) ; extern float __remquof (float __x, float __y, int *__quo) ;






extern long int lrintf (float __x) ; extern long int __lrintf (float __x) ;

extern long long int llrintf (float __x) ; extern long long int __llrintf (float __x) ;



extern long int lroundf (float __x) ; extern long int __lroundf (float __x) ;

extern long long int llroundf (float __x) ; extern long long int __llroundf (float __x) ;



extern float fdimf (float __x, float __y) ; extern float __fdimf (float __x, float __y) ;



extern float fmaxf (float __x, float __y) ; extern float __fmaxf (float __x, float __y) ;


extern float fminf (float __x, float __y) ; extern float __fminf (float __x, float __y) ;



extern float fmaf (float __x, float __y, float __z) ; extern float __fmaf (float __x, float __y, float __z) ;
extern int __fpclassifyl (double __value)
     ;


extern int __signbitl (double __value)
     ;



extern int __isinfl (double __value)
  ;


extern int __finitel (double __value)
  ;


extern int __isnanl (double __value)
  ;


extern int __iseqsigl (double __x, double __y) ;


extern int __issignalingl (double __value)
     ;
 extern double acosl (double __x) ; extern double __acosl (double __x) ;

 extern double asinl (double __x) ; extern double __asinl (double __x) ;

 extern double atanl (double __x) ; extern double __atanl (double __x) ;

 extern double atan2l (double __y, double __x) ; extern double __atan2l (double __y, double __x) ;


 extern double cosl (double __x) ; extern double __cosl (double __x) ;

 extern double sinl (double __x) ; extern double __sinl (double __x) ;

 extern double tanl (double __x) ; extern double __tanl (double __x) ;




 extern double coshl (double __x) ; extern double __coshl (double __x) ;

 extern double sinhl (double __x) ; extern double __sinhl (double __x) ;

 extern double tanhl (double __x) ; extern double __tanhl (double __x) ;
 extern double acoshl (double __x) ; extern double __acoshl (double __x) ;

 extern double asinhl (double __x) ; extern double __asinhl (double __x) ;

 extern double atanhl (double __x) ; extern double __atanhl (double __x) ;





 extern double expl (double __x) ; extern double __expl (double __x) ;


extern double frexpl (double __x, int *__exponent) ; extern double __frexpl (double __x, int *__exponent) ;


extern double ldexpl (double __x, int __exponent) ; extern double __ldexpl (double __x, int __exponent) ;


 extern double logl (double __x) ; extern double __logl (double __x) ;


 extern double log10l (double __x) ; extern double __log10l (double __x) ;


extern double modfl (double __x, double *__iptr) ; extern double __modfl (double __x, double *__iptr) ;
 extern double expm1l (double __x) ; extern double __expm1l (double __x) ;


 extern double log1pl (double __x) ; extern double __log1pl (double __x) ;


extern double logbl (double __x) ; extern double __logbl (double __x) ;




 extern double exp2l (double __x) ; extern double __exp2l (double __x) ;


 extern double log2l (double __x) ; extern double __log2l (double __x) ;






 extern double powl (double __x, double __y) ; extern double __powl (double __x, double __y) ;


extern double sqrtl (double __x) ; extern double __sqrtl (double __x) ;



 extern double hypotl (double __x, double __y) ; extern double __hypotl (double __x, double __y) ;




 extern double cbrtl (double __x) ; extern double __cbrtl (double __x) ;






extern double ceill (double __x) ; extern double __ceill (double __x) ;


extern double fabsl (double __x) ; extern double __fabsl (double __x) ;


extern double floorl (double __x) ; extern double __floorl (double __x) ;


extern double fmodl (double __x, double __y) ; extern double __fmodl (double __x, double __y) ;
extern double copysignl (double __x, double __y) ; extern double __copysignl (double __x, double __y) ;




extern double nanl (const char *__tagb) ; extern double __nanl (const char *__tagb) ;
 extern double erfl (double) ; extern double __erfl (double) ;
 extern double erfcl (double) ; extern double __erfcl (double) ;
extern double lgammal (double) ; extern double __lgammal (double) ;




extern double tgammal (double) ; extern double __tgammal (double) ;
extern double rintl (double __x) ; extern double __rintl (double __x) ;


extern double nextafterl (double __x, double __y) ; extern double __nextafterl (double __x, double __y) ;

extern double nexttowardl (double __x, double __y) ; extern double __nexttowardl (double __x, double __y) ;
extern double remainderl (double __x, double __y) ; extern double __remainderl (double __x, double __y) ;



extern double scalbnl (double __x, int __n) ; extern double __scalbnl (double __x, int __n) ;



extern int ilogbl (double __x) ; extern int __ilogbl (double __x) ;
extern double scalblnl (double __x, long int __n) ; extern double __scalblnl (double __x, long int __n) ;



extern double nearbyintl (double __x) ; extern double __nearbyintl (double __x) ;



extern double roundl (double __x) ; extern double __roundl (double __x) ;



extern double truncl (double __x) ; extern double __truncl (double __x) ;




extern double remquol (double __x, double __y, int *__quo) ; extern double __remquol (double __x, double __y, int *__quo) ;






extern long int lrintl (double __x) ; extern long int __lrintl (double __x) ;

extern long long int llrintl (double __x) ; extern long long int __llrintl (double __x) ;



extern long int lroundl (double __x) ; extern long int __lroundl (double __x) ;

extern long long int llroundl (double __x) ; extern long long int __llroundl (double __x) ;



extern double fdiml (double __x, double __y) ; extern double __fdiml (double __x, double __y) ;



extern double fmaxl (double __x, double __y) ; extern double __fmaxl (double __x, double __y) ;


extern double fminl (double __x, double __y) ; extern double __fminl (double __x, double __y) ;



extern double fmal (double __x, double __y, double __z) ; extern double __fmal (double __x, double __y, double __z) ;
extern int signgam;
enum
  {
    FP_NAN =

      0,
    FP_INFINITE =

      1,
    FP_ZERO =

      2,
    FP_SUBNORMAL =

      3,
    FP_NORMAL =

      4
  };













typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;





 typedef struct
  {
    long long int quot;
    long long int rem;
  } lldiv_t;
extern size_t __ctype_get_mb_cur_max (void) ;



extern double atof (const char *__nptr)
     ;

extern int atoi (const char *__nptr)
     ;

extern long int atol (const char *__nptr)
     ;



 extern long long int atoll (const char *__nptr)
     ;



extern double strtod (const char * __nptr,
        char ** __endptr)
     ;



extern float strtof (const char * __nptr,
       char ** __endptr) ;

extern double strtold (const char * __nptr,
       char ** __endptr)
     ;
extern long int strtol (const char * __nptr,
   char ** __endptr, int __base)
     ;

extern unsigned long int strtoul (const char * __nptr,
      char ** __endptr, int __base)
     ;

extern long long int strtoll (const char * __nptr,
         char ** __endptr, int __base)
     ;


extern unsigned long long int strtoull (const char * __nptr,
     char ** __endptr, int __base)
     ;
extern char *l64a (long int __n) ;


extern long int a64l (const char *__s)
     ;





typedef __ino64_t ino_t;




typedef __ino64_t ino64_t;




typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;
typedef __off64_t off_t;




typedef __off64_t off64_t;
typedef __id_t id_t;




typedef __ssize_t ssize_t;
typedef __key_t key_t;










typedef __clock_t clock_t;







typedef __clockid_t clockid_t;







typedef __timer_t timer_t;



typedef __useconds_t useconds_t;



typedef __suseconds_t suseconds_t;





typedef __uint8_t u_int8_t;
typedef __uint16_t u_int16_t;
typedef __uint32_t u_int32_t;
typedef __uint64_t u_int64_t;




typedef int register_t;
typedef __blksize_t blksize_t;
typedef __blkcnt64_t blkcnt_t;



typedef __fsblkcnt64_t fsblkcnt_t;



typedef __fsfilcnt64_t fsfilcnt_t;





typedef __blkcnt64_t blkcnt64_t;
typedef __fsblkcnt64_t fsblkcnt64_t;
typedef __fsfilcnt64_t fsfilcnt64_t;







extern long int random (void) ;


extern void srandom (unsigned int __seed) ;





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) ;



extern char *setstate (char *__statebuf) ;
extern int rand (void) ;

extern void srand (unsigned int __seed) ;



extern int rand_r (unsigned int *__seed) ;







extern double drand48 (void) ;
extern double erand48 (unsigned short int __xsubi[3]) ;


extern long int lrand48 (void) ;
extern long int nrand48 (unsigned short int __xsubi[3])
     ;


extern long int mrand48 (void) ;
extern long int jrand48 (unsigned short int __xsubi[3])
     ;


extern void srand48 (long int __seedval) ;
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     ;
extern void lcong48 (unsigned short int __param[7]) ;
extern void *malloc (size_t __size)
     ;

extern void *calloc (size_t __nmemb, size_t __size)
     ;






extern void *realloc (void *__ptr, size_t __size)
     ;


extern void free (void *__ptr) ;
extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     ;




extern void *aligned_alloc (size_t __alignment, size_t __size)
    
     ;



extern void abort (void) ;



extern int atexit (void (*__func) (void)) ;







extern int at_quick_exit (void (*__func) (void)) ;
extern void exit (int __status) ;





extern void quick_exit (int __status) ;





extern void _Exit (int __status) ;




extern char *getenv (const char *__name) ;
extern int putenv (char *__string) ;





extern int setenv (const char *__name, const char *__value, int __replace)
     ;


extern int unsetenv (const char *__name) ;
extern char *mktemp (char *__template) ;
extern int mkstemp64 (char *__template) ;
extern int system (const char *__command) ;
extern char *realpath (const char * __name,
         char * __resolved) ;






typedef int (*__compar_fn_t) (const void *, const void *);
extern void *bsearch (const void *__key, const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     ;







extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) ;
extern int abs (int __x) ;
extern long int labs (long int __x) ;


 extern long long int llabs (long long int __x)
     ;






extern div_t div (int __numer, int __denom)
     ;
extern ldiv_t ldiv (long int __numer, long int __denom)
     ;


 extern lldiv_t lldiv (long long int __numer,
        long long int __denom)
     ;
extern char *ecvt (double __value, int __ndigit, int * __decpt,
     int * __sign) ;




extern char *fcvt (double __value, int __ndigit, int * __decpt,
     int * __sign) ;




extern char *gcvt (double __value, int __ndigit, char *__buf)
     ;
extern int mblen (const char *__s, size_t __n) ;


extern int mbtowc (wchar_t * __pwc,
     const char * __s, size_t __n) ;


extern int wctomb (char *__s, wchar_t __wchar) ;



extern size_t mbstowcs (wchar_t * __pwcs,
   const char * __s, size_t __n)
    ;

extern size_t wcstombs (char * __s,
   const wchar_t * __pwcs, size_t __n)
    
 
  ;
extern int getsubopt (char ** __optionp,
        char *const * __tokens,
        char ** __valuep)
     ;







extern int posix_openpt (int __oflag) ;







extern int grantpt (int __fd) ;



extern int unlockpt (int __fd) ;




extern char *ptsname (int __fd) ;





enum RESERVED {

  TK_AND = (
          (0x7f * 2 + 1) 
          + 1), TK_BREAK,
  TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
  TK_GOTO, TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
  TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,

  TK_IDIV, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE,
  TK_SHL, TK_SHR,
  TK_DBCOLON, TK_EOS,
  TK_FLT, TK_INT, TK_NAME, TK_STRING
};





typedef union {
  lua_Number r;
  lua_Integer i;
  TString *ts;
} SemInfo;


typedef struct Token {
  int token;
  SemInfo seminfo;
} Token;




typedef struct LexState {
  int current;
  int linenumber;
  int lastline;
  Token t;
  Token lookahead;
  struct FuncState *fs;
  struct lua_State *L;
  ZIO *z;
  Mbuffer *buff;
  Table *h;
  struct Dyndata *dyd;
  TString *source;
  TString *envn;
} LexState;


extern void luaX_init (lua_State *L);
extern void luaX_setinput (lua_State *L, LexState *ls, ZIO *z,
                              TString *source, int firstchar);
extern TString *luaX_newstring (LexState *ls, const char *str, size_t l);
extern void luaX_next (LexState *ls);
extern int luaX_lookahead (LexState *ls);
extern void luaX_syntaxerror (LexState *ls, const char *s);
extern const char *luaX_token2str (LexState *ls, int token);

enum OpMode {iABC, iABx, iAsBx, iAx, isJ};
typedef enum {



OP_MOVE,
OP_LOADI,
OP_LOADF,
OP_LOADK,
OP_LOADKX,
OP_LOADFALSE,
OP_LFALSESKIP,
OP_LOADTRUE,
OP_LOADNIL,
OP_GETUPVAL,
OP_SETUPVAL,

OP_GETTABUP,
OP_GETTABLE,
OP_GETI,
OP_GETFIELD,

OP_SETTABUP,
OP_SETTABLE,
OP_SETI,
OP_SETFIELD,

OP_NEWTABLE,

OP_SELF,

OP_ADDI,

OP_ADDK,
OP_SUBK,
OP_MULK,
OP_MODK,
OP_POWK,
OP_DIVK,
OP_IDIVK,

OP_BANDK,
OP_BORK,
OP_BXORK,

OP_SHRI,
OP_SHLI,

OP_ADD,
OP_SUB,
OP_MUL,
OP_MOD,
OP_POW,
OP_DIV,
OP_IDIV,

OP_BAND,
OP_BOR,
OP_BXOR,
OP_SHL,
OP_SHR,

OP_MMBIN,
OP_MMBINI,
OP_MMBINK,

OP_UNM,
OP_BNOT,
OP_NOT,
OP_LEN,

OP_CONCAT,

OP_CLOSE,
OP_TBC,
OP_JMP,
OP_EQ,
OP_LT,
OP_LE,

OP_EQK,
OP_EQI,
OP_LTI,
OP_LEI,
OP_GTI,
OP_GEI,

OP_TEST,
OP_TESTSET,

OP_CALL,
OP_TAILCALL,

OP_RETURN,
OP_RETURN0,
OP_RETURN1,

OP_FORLOOP,
OP_FORPREP,


OP_TFORPREP,
OP_TFORCALL,
OP_TFORLOOP,

OP_SETLIST,

OP_CLOSURE,

OP_VARARG,

OP_VARARGPREP,

OP_EXTRAARG
} OpCode;
extern const lu_byte luaP_opmodes[84];
typedef enum {
  VVOID,

  VNIL,
  VTRUE,
  VFALSE,
  VK,
  VKFLT,
  VKINT,
  VKSTR,

  VNONRELOC,

  VLOCAL,

  VUPVAL,
  VCONST,

  VINDEXED,


  VINDEXUP,


  VINDEXI,


  VINDEXSTR,


  VJMP,

  VRELOC,

  VCALL,
  VVARARG
} expkind;






typedef struct expdesc {
  expkind k;
  union {
    lua_Integer ival;
    lua_Number nval;
    TString *strval;
    int info;
    struct {
      short idx;
      lu_byte t;
    } ind;
    struct {
      lu_byte ridx;
      unsigned short vidx;
    } var;
  } u;
  int t;
  int f;
} expdesc;
typedef union Vardesc {
  struct {
    Value value_; lu_byte tt_;
    lu_byte kind;
    lu_byte ridx;
    short pidx;
    TString *name;
  } vd;
  TValue k;
} Vardesc;




typedef struct Labeldesc {
  TString *name;
  int pc;
  int line;
  lu_byte nactvar;
  lu_byte close;
} Labeldesc;



typedef struct Labellist {
  Labeldesc *arr;
  int n;
  int size;
} Labellist;



typedef struct Dyndata {
  struct {
    Vardesc *arr;
    int n;
    int size;
  } actvar;
  Labellist gt;
  Labellist label;
} Dyndata;



struct BlockCnt;



typedef struct FuncState {
  Proto *f;
  struct FuncState *prev;
  struct LexState *ls;
  struct BlockCnt *bl;
  int pc;
  int lasttarget;
  int previousline;
  int nk;
  int np;
  int nabslineinfo;
  int firstlocal;
  int firstlabel;
  short ndebugvars;
  lu_byte nactvar;
  lu_byte nups;
  lu_byte freereg;
  lu_byte iwthabs;
  lu_byte needclose;
} FuncState;


extern int luaY_nvarstack (FuncState *fs);
extern LClosure *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff,
                                 Dyndata *dyd, const char *name, int firstchar);
typedef enum BinOpr {

  OPR_ADD, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
  OPR_DIV, OPR_IDIV,

  OPR_BAND, OPR_BOR, OPR_BXOR,
  OPR_SHL, OPR_SHR,

  OPR_CONCAT,

  OPR_EQ, OPR_LT, OPR_LE,
  OPR_NE, OPR_GT, OPR_GE,

  OPR_AND, OPR_OR,
  OPR_NOBINOPR
} BinOpr;
typedef enum UnOpr { OPR_MINUS, OPR_BNOT, OPR_NOT, OPR_LEN, OPR_NOUNOPR } UnOpr;
extern int luaK_code (FuncState *fs, Instruction i);
extern int luaK_codeABx (FuncState *fs, OpCode o, int A, unsigned int Bx);
extern int luaK_codeAsBx (FuncState *fs, OpCode o, int A, int Bx);
extern int luaK_codeABCk (FuncState *fs, OpCode o, int A,
                                            int B, int C, int k);
extern int luaK_isKint (expdesc *e);
extern int luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v);
extern void luaK_fixline (FuncState *fs, int line);
extern void luaK_nil (FuncState *fs, int from, int n);
extern void luaK_reserveregs (FuncState *fs, int n);
extern void luaK_checkstack (FuncState *fs, int n);
extern void luaK_int (FuncState *fs, int reg, lua_Integer n);
extern void luaK_dischargevars (FuncState *fs, expdesc *e);
extern int luaK_exp2anyreg (FuncState *fs, expdesc *e);
extern void luaK_exp2anyregup (FuncState *fs, expdesc *e);
extern void luaK_exp2nextreg (FuncState *fs, expdesc *e);
extern void luaK_exp2val (FuncState *fs, expdesc *e);
extern int luaK_exp2RK (FuncState *fs, expdesc *e);
extern void luaK_self (FuncState *fs, expdesc *e, expdesc *key);
extern void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k);
extern void luaK_goiftrue (FuncState *fs, expdesc *e);
extern void luaK_goiffalse (FuncState *fs, expdesc *e);
extern void luaK_storevar (FuncState *fs, expdesc *var, expdesc *e);
extern void luaK_setreturns (FuncState *fs, expdesc *e, int nresults);
extern void luaK_setoneret (FuncState *fs, expdesc *e);
extern int luaK_jump (FuncState *fs);
extern void luaK_ret (FuncState *fs, int first, int nret);
extern void luaK_patchlist (FuncState *fs, int list, int target);
extern void luaK_patchtohere (FuncState *fs, int list);
extern void luaK_concat (FuncState *fs, int *l1, int l2);
extern int luaK_getlabel (FuncState *fs);
extern void luaK_prefix (FuncState *fs, UnOpr op, expdesc *v, int line);
extern void luaK_infix (FuncState *fs, BinOpr op, expdesc *v);
extern void luaK_posfix (FuncState *fs, BinOpr op, expdesc *v1,
                            expdesc *v2, int line);
extern void luaK_settablesize (FuncState *fs, int pc,
                                  int ra, int asize, int hsize);
extern void luaK_setlist (FuncState *fs, int base, int nelems, int tostore);
extern void luaK_finish (FuncState *fs);
extern void luaK_semerror (LexState *ls, const char *msg);
static int codesJ (FuncState *fs, OpCode o, int sj, int k);




void luaK_semerror (LexState *ls, const char *msg) {
  ls->t.token = 0;
  luaX_syntaxerror(ls, msg);
}






static int tonumeral (const expdesc *e, TValue *v) {
  if (((e)->t != (e)->f))
    return 0;
  switch (e->k) {
    case VKINT:
      if (v) { TValue *io=(v); ((io)->value_).i=(e->u.ival); ((io)->tt_=(((3) | ((0) << 4)))); };
      return 1;
    case VKFLT:
      if (v) { TValue *io=(v); ((io)->value_).n=(e->u.nval); ((io)->tt_=(((3) | ((1) << 4)))); };
      return 1;
    default: return 0;
  }
}





static TValue *const2val (FuncState *fs, const expdesc *e) {
  ((void)0);
  return &fs->ls->dyd->actvar.arr[e->u.info].k;
}






int luaK_exp2const (FuncState *fs, const expdesc *e, TValue *v) {
  if (((e)->t != (e)->f))
    return 0;
  switch (e->k) {
    case VFALSE:
      ((v)->tt_=(((1) | ((0) << 4))));
      return 1;
    case VTRUE:
      ((v)->tt_=(((1) | ((1) << 4))));
      return 1;
    case VNIL:
      ((v)->tt_=(((0) | ((0) << 4))));
      return 1;
    case VKSTR: {
      { TValue *io = (v); TString *x_ = (e->u.strval); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)fs->ls->L, ((void)0)); };
      return 1;
    }
    case VCONST: {
      { TValue *io1=(v); const TValue *io2=(const2val(fs, e)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)fs->ls->L, ((void)0)); ((void)0); };
      return 1;
    }
    default: return tonumeral(e, v);
  }
}
static Instruction *previousinstruction (FuncState *fs) {
  static const Instruction invalidinstruction = ~(Instruction)0;
  if (fs->pc > fs->lasttarget)
    return &fs->f->code[fs->pc - 1];
  else
    return ((Instruction*)(&invalidinstruction));
}
void luaK_nil (FuncState *fs, int from, int n) {
  int l = from + n - 1;
  Instruction *previous = previousinstruction(fs);
  if ((((OpCode)(((*previous)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) == OP_LOADNIL) {
    int pfrom = (((int)((((*previous)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))));
    int pl = pfrom + ((((int)((((*previous)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
    if ((pfrom <= from && from <= pl + 1) ||
        (from <= pfrom && pfrom <= l + 1)) {
      if (pfrom < from) from = pfrom;
      if (pl > l) l = pl;
      ((*previous) = (((*previous)&(~((~((~(Instruction)0)<<(8)))<<((0 + 7))))) | ((((Instruction)(from))<<(0 + 7))&((~((~(Instruction)0)<<(8)))<<((0 + 7))))));
      ((*previous) = (((*previous)&(~((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))) | ((((Instruction)(l - from))<<(((0 + 7) + 8) + 1))&((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))));
      return;
    }
  }
  luaK_codeABCk(fs,OP_LOADNIL,from,n - 1,0,0);
}






static int getjump (FuncState *fs, int pc) {
  int offset = ((((int)((((fs->f->code[pc])>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1));
  if (offset == (-1))
    return (-1);
  else
    return (pc+1)+offset;
}






static void fixjump (FuncState *fs, int pc, int dest) {
  Instruction *jmp = &fs->f->code[pc];
  int offset = dest - (pc + 1);
  ((void)0);
  if (!(-(((1 << ((8 + 8 + 1) + 8)) - 1) >> 1) <= offset && offset <= ((1 << ((8 + 8 + 1) + 8)) - 1) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)))
    luaX_syntaxerror(fs->ls, "control structure too long");
  ((void)0);
  ((*jmp) = (((*jmp)&(~((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<((0 + 7))))) | ((((Instruction)(((unsigned int)(((offset)+(((1 << ((8 + 8 + 1) + 8)) - 1) >> 1))))))<<(0 + 7))&((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<((0 + 7))))));
}





void luaK_concat (FuncState *fs, int *l1, int l2) {
  if (l2 == (-1)) return;
  else if (*l1 == (-1))
    *l1 = l2;
  else {
    int list = *l1;
    int next;
    while ((next = getjump(fs, list)) != (-1))
      list = next;
    fixjump(fs, list, l2);
  }
}






int luaK_jump (FuncState *fs) {
  return codesJ(fs, OP_JMP, (-1), 0);
}





void luaK_ret (FuncState *fs, int first, int nret) {
  OpCode op;
  switch (nret) {
    case 0: op = OP_RETURN0; break;
    case 1: op = OP_RETURN1; break;
    default: op = OP_RETURN; break;
  }
  luaK_codeABCk(fs,op,first,nret + 1,0,0);
}






static int condjump (FuncState *fs, OpCode op, int A, int B, int C, int k) {
  luaK_codeABCk(fs, op, A, B, C, k);
  return luaK_jump(fs);
}






int luaK_getlabel (FuncState *fs) {
  fs->lasttarget = fs->pc;
  return fs->pc;
}







static Instruction *getjumpcontrol (FuncState *fs, int pc) {
  Instruction *pi = &fs->f->code[pc];
  if (pc >= 1 && (luaP_opmodes[(((OpCode)(((*(pi-1))>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))] & (1 << 4)))
    return pi-1;
  else
    return pi;
}
static int patchtestreg (FuncState *fs, int node, int reg) {
  Instruction *i = getjumpcontrol(fs, node);
  if ((((OpCode)(((*i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) != OP_TESTSET)
    return 0;
  if (reg != ((1<<8)-1) && reg != ((((int)((((*i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))
    ((*i) = (((*i)&(~((~((~(Instruction)0)<<(8)))<<((0 + 7))))) | ((((Instruction)(reg))<<(0 + 7))&((~((~(Instruction)0)<<(8)))<<((0 + 7))))));
  else {


    *i = ((((Instruction)(OP_TEST))<<0) | (((Instruction)(((((int)((((*i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))<<(0 + 7)) | (((Instruction)(0))<<(((0 + 7) + 8) + 1)) | (((Instruction)(0))<<((((0 + 7) + 8) + 1) + 8)) | (((Instruction)(((((int)((((*i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))))<<((0 + 7) + 8)));
  }
  return 1;
}





static void removevalues (FuncState *fs, int list) {
  for (; list != (-1); list = getjump(fs, list))
      patchtestreg(fs, list, ((1<<8)-1));
}







static void patchlistaux (FuncState *fs, int list, int vtarget, int reg,
                          int dtarget) {
  while (list != (-1)) {
    int next = getjump(fs, list);
    if (patchtestreg(fs, list, reg))
      fixjump(fs, list, vtarget);
    else
      fixjump(fs, list, dtarget);
    list = next;
  }
}







void luaK_patchlist (FuncState *fs, int list, int target) {
  ((void)0);
  patchlistaux(fs, list, target, ((1<<8)-1), target);
}


void luaK_patchtohere (FuncState *fs, int list) {
  int hr = luaK_getlabel(fs);
  luaK_patchlist(fs, list, hr);
}
static void savelineinfo (FuncState *fs, Proto *f, int line) {
  int linedif = line - fs->previousline;
  int pc = fs->pc - 1;
  if (abs(linedif) >= 0x80 || fs->iwthabs++ >= 128) {
    ((f->abslineinfo)=((AbsLineInfo *)(luaM_growaux_(fs->ls->L,f->abslineinfo,fs->nabslineinfo,&(f->sizeabslineinfo),sizeof(AbsLineInfo), ((((size_t)((0x7fffffff
   ))) <= ((size_t)(~(size_t)0))/sizeof(AbsLineInfo)) ? (0x7fffffff
   ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(AbsLineInfo)))))),"lines"))))
                                                                      ;
    f->abslineinfo[fs->nabslineinfo].pc = pc;
    f->abslineinfo[fs->nabslineinfo++].line = line;
    linedif = (-0x80);
    fs->iwthabs = 1;
  }
  ((f->lineinfo)=((ls_byte *)(luaM_growaux_(fs->ls->L,f->lineinfo,pc,&(f->sizelineinfo),sizeof(ls_byte), ((((size_t)((0x7fffffff
 ))) <= ((size_t)(~(size_t)0))/sizeof(ls_byte)) ? (0x7fffffff
 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(ls_byte)))))),"opcodes"))))
                                     ;
  f->lineinfo[pc] = linedif;
  fs->previousline = line;
}
static void removelastlineinfo (FuncState *fs) {
  Proto *f = fs->f;
  int pc = fs->pc - 1;
  if (f->lineinfo[pc] != (-0x80)) {
    fs->previousline -= f->lineinfo[pc];
    fs->iwthabs--;
  }
  else {
    ((void)0);
    fs->nabslineinfo--;
    fs->iwthabs = 128 + 1;
  }
}






static void removelastinstruction (FuncState *fs) {
  removelastlineinfo(fs);
  fs->pc--;
}






int luaK_code (FuncState *fs, Instruction i) {
  Proto *f = fs->f;

  ((f->code)=((Instruction *)(luaM_growaux_(fs->ls->L,f->code,fs->pc,&(f->sizecode),sizeof(Instruction), ((((size_t)((0x7fffffff
 ))) <= ((size_t)(~(size_t)0))/sizeof(Instruction)) ? (0x7fffffff
 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Instruction)))))),"opcodes"))))
                                     ;
  f->code[fs->pc++] = i;
  savelineinfo(fs, f, fs->ls->lastline);
  return fs->pc - 1;
}






int luaK_codeABCk (FuncState *fs, OpCode o, int a, int b, int c, int k) {
  ((void)0);
  ((void)0)
                                            ;
  return luaK_code(fs, ((((Instruction)(o))<<0) | (((Instruction)(a))<<(0 + 7)) | (((Instruction)(b))<<(((0 + 7) + 8) + 1)) | (((Instruction)(c))<<((((0 + 7) + 8) + 1) + 8)) | (((Instruction)(k))<<((0 + 7) + 8))));
}





int luaK_codeABx (FuncState *fs, OpCode o, int a, unsigned int bc) {
  ((void)0);
  ((void)0);
  return luaK_code(fs, ((((Instruction)(o))<<0) | (((Instruction)(a))<<(0 + 7)) | (((Instruction)(bc))<<((0 + 7) + 8))));
}





int luaK_codeAsBx (FuncState *fs, OpCode o, int a, int bc) {
  unsigned int b = bc + (((1<<(8 + 8 + 1))-1)>>1);
  ((void)0);
  ((void)0);
  return luaK_code(fs, ((((Instruction)(o))<<0) | (((Instruction)(a))<<(0 + 7)) | (((Instruction)(b))<<((0 + 7) + 8))));
}





static int codesJ (FuncState *fs, OpCode o, int sj, int k) {
  unsigned int j = sj + (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1);
  ((void)0);
  ((void)0);
  return luaK_code(fs, ((((Instruction)(o)) << 0) | (((Instruction)(j)) << (0 + 7)) | (((Instruction)(k)) << ((0 + 7) + 8))));
}





static int codeextraarg (FuncState *fs, int a) {
  ((void)0);
  return luaK_code(fs, ((((Instruction)(OP_EXTRAARG))<<0) | (((Instruction)(a))<<(0 + 7))));
}







static int luaK_codek (FuncState *fs, int reg, int k) {
  if (k <= ((1<<(8 + 8 + 1))-1))
    return luaK_codeABx(fs, OP_LOADK, reg, k);
  else {
    int p = luaK_codeABx(fs, OP_LOADKX, reg, 0);
    codeextraarg(fs, k);
    return p;
  }
}






void luaK_checkstack (FuncState *fs, int n) {
  int newstack = fs->freereg + n;
  if (newstack > fs->f->maxstacksize) {
    if (newstack >= 255)
      luaX_syntaxerror(fs->ls,
        "function or expression needs too many registers");
    fs->f->maxstacksize = ((lu_byte)((newstack)));
  }
}





void luaK_reserveregs (FuncState *fs, int n) {
  luaK_checkstack(fs, n);
  fs->freereg += n;
}







static void freereg (FuncState *fs, int reg) {
  if (reg >= luaY_nvarstack(fs)) {
    fs->freereg--;
    ((void)0);
  }
}





static void freeregs (FuncState *fs, int r1, int r2) {
  if (r1 > r2) {
    freereg(fs, r1);
    freereg(fs, r2);
  }
  else {
    freereg(fs, r2);
    freereg(fs, r1);
  }
}





static void freeexp (FuncState *fs, expdesc *e) {
  if (e->k == VNONRELOC)
    freereg(fs, e->u.info);
}






static void freeexps (FuncState *fs, expdesc *e1, expdesc *e2) {
  int r1 = (e1->k == VNONRELOC) ? e1->u.info : -1;
  int r2 = (e2->k == VNONRELOC) ? e2->u.info : -1;
  freeregs(fs, r1, r2);
}
static int addk (FuncState *fs, TValue *key, TValue *v) {
  TValue val;
  lua_State *L = fs->ls->L;
  Proto *f = fs->f;
  const TValue *idx = luaH_get(fs->ls->h, key);
  int k, oldsize;
  if (((((idx))->tt_) == (((3) | ((0) << 4))))) {
    k = ((int)(((((idx)->value_).i))));

    if (k < fs->nk && ((((&f->k[k])->tt_)) & 0x3F) == ((((v)->tt_)) & 0x3F) &&
                      luaV_equalobj(
                     ((void *)0)
                     ,&f->k[k],v))
      return k;
  }

  oldsize = f->sizek;
  k = fs->nk;


  { TValue *io=(&val); ((io)->value_).i=(k); ((io)->tt_=(((3) | ((0) << 4)))); };
  luaH_finishset(L, fs->ls->h, key, idx, &val);
  ((f->k)=((TValue *)(luaM_growaux_(L,f->k,k,&(f->sizek),sizeof(TValue), ((((size_t)((((1<<((8 + 8 + 1) + 8))-1)))) <= ((size_t)(~(size_t)0))/sizeof(TValue)) ? (((1<<((8 + 8 + 1) + 8))-1)) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(TValue)))))),"constants"))));
  while (oldsize < f->sizek) ((&f->k[oldsize++])->tt_=(((0) | ((0) << 4))));
  { TValue *io1=(&f->k[k]); const TValue *io2=(v); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  fs->nk++;
  ( (((v)->tt_) & (1 << 6)) ? ( ((((f)->marked) & ((1<<(5)))) && ((((((v)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((f)))->gc)),(&(((union GCUnion *)(((((v)->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));
  return k;
}





static int stringK (FuncState *fs, TString *s) {
  TValue o;
  { TValue *io = (&o); TString *x_ = (s); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)fs->ls->L, ((void)0)); };
  return addk(fs, &o, &o);
}





static int luaK_intK (FuncState *fs, lua_Integer n) {
  TValue o;
  { TValue *io=(&o); ((io)->value_).i=(n); ((io)->tt_=(((3) | ((0) << 4)))); };
  return addk(fs, &o, &o);
}
static int luaK_numberK (FuncState *fs, lua_Number r) {
  TValue o;
  lua_Integer ik;
  { TValue *io=(&o); ((io)->value_).n=(r); ((io)->tt_=(((3) | ((1) << 4)))); };
  if (!luaV_flttointeger(r, &ik, F2Ieq))
    return addk(fs, &o, &o);
  else {
    const int nbm = (53
                   );
    const lua_Number q = ldexp(1.0, -nbm + 1);
    const lua_Number k = (ik == 0) ? q : r + r*q;
    TValue kv;
    { TValue *io=(&kv); ((io)->value_).n=(k); ((io)->tt_=(((3) | ((1) << 4)))); };

    ((void)0)
                                                   ;
    return addk(fs, &kv, &o);
  }
}





static int boolF (FuncState *fs) {
  TValue o;
  ((&o)->tt_=(((1) | ((0) << 4))));
  return addk(fs, &o, &o);
}





static int boolT (FuncState *fs) {
  TValue o;
  ((&o)->tt_=(((1) | ((1) << 4))));
  return addk(fs, &o, &o);
}





static int nilK (FuncState *fs) {
  TValue k, v;
  ((&v)->tt_=(((0) | ((0) << 4))));

  { TValue *io = (&k); Table *x_ = (fs->ls->h); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)fs->ls->L, ((void)0)); };
  return addk(fs, &k, &v);
}







static int fitsC (lua_Integer i) {
  return (((lua_Unsigned)(i)) + (((1<<8)-1) >> 1) <= ((unsigned int)((((1<<8)-1)))));
}





static int fitsBx (lua_Integer i) {
  return (-(((1<<(8 + 8 + 1))-1)>>1) <= i && i <= ((1<<(8 + 8 + 1))-1) - (((1<<(8 + 8 + 1))-1)>>1));
}


void luaK_int (FuncState *fs, int reg, lua_Integer i) {
  if (fitsBx(i))
    luaK_codeAsBx(fs, OP_LOADI, reg, ((int)((i))));
  else
    luaK_codek(fs, reg, luaK_intK(fs, i));
}


static void luaK_float (FuncState *fs, int reg, lua_Number f) {
  lua_Integer fi;
  if (luaV_flttointeger(f, &fi, F2Ieq) && fitsBx(fi))
    luaK_codeAsBx(fs, OP_LOADF, reg, ((int)((fi))));
  else
    luaK_codek(fs, reg, luaK_numberK(fs, f));
}





static void const2exp (TValue *v, expdesc *e) {
  switch (((((v)->tt_)) & 0x3F)) {
    case ((3) | ((0) << 4)):
      e->k = VKINT; e->u.ival = (((v)->value_).i);
      break;
    case ((3) | ((1) << 4)):
      e->k = VKFLT; e->u.nval = (((v)->value_).n);
      break;
    case ((1) | ((0) << 4)):
      e->k = VFALSE;
      break;
    case ((1) | ((1) << 4)):
      e->k = VTRUE;
      break;
    case ((0) | ((0) << 4)):
      e->k = VNIL;
      break;
    case ((4) | ((0) << 4)): case ((4) | ((1) << 4)):
      e->k = VKSTR; e->u.strval = ((&((((union GCUnion *)((((v)->value_).gc))))->ts)));
      break;
    default: ((void)0);
  }
}






void luaK_setreturns (FuncState *fs, expdesc *e, int nresults) {
  Instruction *pc = &((fs)->f->code[(e)->u.info]);
  if (e->k == VCALL)
    ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))) | ((((Instruction)(nresults + 1))<<((((0 + 7) + 8) + 1) + 8))&((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))));
  else {
    ((void)0);
    ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))) | ((((Instruction)(nresults + 1))<<((((0 + 7) + 8) + 1) + 8))&((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))));
    ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(8)))<<((0 + 7))))) | ((((Instruction)(fs->freereg))<<(0 + 7))&((~((~(Instruction)0)<<(8)))<<((0 + 7))))));
    luaK_reserveregs(fs, 1);
  }
}





static void str2K (FuncState *fs, expdesc *e) {
  ((void)0);
  e->u.info = stringK(fs, e->u.strval);
  e->k = VK;
}
void luaK_setoneret (FuncState *fs, expdesc *e) {
  if (e->k == VCALL) {

    ((void)0);
    e->k = VNONRELOC;
    e->u.info = (((int)((((((fs)->f->code[(e)->u.info]))>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))));
  }
  else if (e->k == VVARARG) {
    ((((fs)->f->code[(e)->u.info])) = (((((fs)->f->code[(e)->u.info]))&(~((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))) | ((((Instruction)(2))<<((((0 + 7) + 8) + 1) + 8))&((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))));
    e->k = VRELOC;
  }
}






void luaK_dischargevars (FuncState *fs, expdesc *e) {
  switch (e->k) {
    case VCONST: {
      const2exp(const2val(fs, e), e);
      break;
    }
    case VLOCAL: {
      e->u.info = e->u.var.ridx;
      e->k = VNONRELOC;
      break;
    }
    case VUPVAL: {
      e->u.info = luaK_codeABCk(fs,OP_GETUPVAL,0,e->u.info,0,0);
      e->k = VRELOC;
      break;
    }
    case VINDEXUP: {
      e->u.info = luaK_codeABCk(fs,OP_GETTABUP,0,e->u.ind.t,e->u.ind.idx,0);
      e->k = VRELOC;
      break;
    }
    case VINDEXI: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABCk(fs,OP_GETI,0,e->u.ind.t,e->u.ind.idx,0);
      e->k = VRELOC;
      break;
    }
    case VINDEXSTR: {
      freereg(fs, e->u.ind.t);
      e->u.info = luaK_codeABCk(fs,OP_GETFIELD,0,e->u.ind.t,e->u.ind.idx,0);
      e->k = VRELOC;
      break;
    }
    case VINDEXED: {
      freeregs(fs, e->u.ind.t, e->u.ind.idx);
      e->u.info = luaK_codeABCk(fs,OP_GETTABLE,0,e->u.ind.t,e->u.ind.idx,0);
      e->k = VRELOC;
      break;
    }
    case VVARARG: case VCALL: {
      luaK_setoneret(fs, e);
      break;
    }
    default: break;
  }
}







static void discharge2reg (FuncState *fs, expdesc *e, int reg) {
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case VNIL: {
      luaK_nil(fs, reg, 1);
      break;
    }
    case VFALSE: {
      luaK_codeABCk(fs,OP_LOADFALSE,reg,0,0,0);
      break;
    }
    case VTRUE: {
      luaK_codeABCk(fs,OP_LOADTRUE,reg,0,0,0);
      break;
    }
    case VKSTR: {
      str2K(fs, e);
    }
    case VK: {
      luaK_codek(fs, reg, e->u.info);
      break;
    }
    case VKFLT: {
      luaK_float(fs, reg, e->u.nval);
      break;
    }
    case VKINT: {
      luaK_int(fs, reg, e->u.ival);
      break;
    }
    case VRELOC: {
      Instruction *pc = &((fs)->f->code[(e)->u.info]);
      ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(8)))<<((0 + 7))))) | ((((Instruction)(reg))<<(0 + 7))&((~((~(Instruction)0)<<(8)))<<((0 + 7))))));
      break;
    }
    case VNONRELOC: {
      if (reg != e->u.info)
        luaK_codeABCk(fs,OP_MOVE,reg,e->u.info,0,0);
      break;
    }
    default: {
      ((void)0);
      return;
    }
  }
  e->u.info = reg;
  e->k = VNONRELOC;
}







static void discharge2anyreg (FuncState *fs, expdesc *e) {
  if (e->k != VNONRELOC) {
    luaK_reserveregs(fs, 1);
    discharge2reg(fs, e, fs->freereg-1);
  }
}


static int code_loadbool (FuncState *fs, int A, OpCode op) {
  luaK_getlabel(fs);
  return luaK_codeABCk(fs,op,A,0,0,0);
}






static int need_value (FuncState *fs, int list) {
  for (; list != (-1); list = getjump(fs, list)) {
    Instruction i = *getjumpcontrol(fs, list);
    if ((((OpCode)(((i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) != OP_TESTSET) return 1;
  }
  return 0;
}
static void exp2reg (FuncState *fs, expdesc *e, int reg) {
  discharge2reg(fs, e, reg);
  if (e->k == VJMP)
    luaK_concat(fs, &e->t, e->u.info);
  if (((e)->t != (e)->f)) {
    int final;
    int p_f = (-1);
    int p_t = (-1);
    if (need_value(fs, e->t) || need_value(fs, e->f)) {
      int fj = (e->k == VJMP) ? (-1) : luaK_jump(fs);
      p_f = code_loadbool(fs, reg, OP_LFALSESKIP);
      p_t = code_loadbool(fs, reg, OP_LOADTRUE);

      luaK_patchtohere(fs, fj);
    }
    final = luaK_getlabel(fs);
    patchlistaux(fs, e->f, final, reg, p_f);
    patchlistaux(fs, e->t, final, reg, p_t);
  }
  e->f = e->t = (-1);
  e->u.info = reg;
  e->k = VNONRELOC;
}





void luaK_exp2nextreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  freeexp(fs, e);
  luaK_reserveregs(fs, 1);
  exp2reg(fs, e, fs->freereg - 1);
}






int luaK_exp2anyreg (FuncState *fs, expdesc *e) {
  luaK_dischargevars(fs, e);
  if (e->k == VNONRELOC) {
    if (!((e)->t != (e)->f))
      return e->u.info;
    if (e->u.info >= luaY_nvarstack(fs)) {
      exp2reg(fs, e, e->u.info);
      return e->u.info;
    }



  }
  luaK_exp2nextreg(fs, e);
  return e->u.info;
}






void luaK_exp2anyregup (FuncState *fs, expdesc *e) {
  if (e->k != VUPVAL || ((e)->t != (e)->f))
    luaK_exp2anyreg(fs, e);
}






void luaK_exp2val (FuncState *fs, expdesc *e) {
  if (((e)->t != (e)->f))
    luaK_exp2anyreg(fs, e);
  else
    luaK_dischargevars(fs, e);
}






static int luaK_exp2K (FuncState *fs, expdesc *e) {
  if (!((e)->t != (e)->f)) {
    int info;
    switch (e->k) {
      case VTRUE: info = boolT(fs); break;
      case VFALSE: info = boolF(fs); break;
      case VNIL: info = nilK(fs); break;
      case VKINT: info = luaK_intK(fs, e->u.ival); break;
      case VKFLT: info = luaK_numberK(fs, e->u.nval); break;
      case VKSTR: info = stringK(fs, e->u.strval); break;
      case VK: info = e->u.info; break;
      default: return 0;
    }
    if (info <= ((1<<8)-1)) {
      e->k = VK;
      e->u.info = info;
      return 1;
    }
  }

  return 0;
}
int luaK_exp2RK (FuncState *fs, expdesc *e) {
  if (luaK_exp2K(fs, e))
    return 1;
  else {
    luaK_exp2anyreg(fs, e);
    return 0;
  }
}


static void codeABRK (FuncState *fs, OpCode o, int a, int b,
                      expdesc *ec) {
  int k = luaK_exp2RK(fs, ec);
  luaK_codeABCk(fs, o, a, b, ec->u.info, k);
}





void luaK_storevar (FuncState *fs, expdesc *var, expdesc *ex) {
  switch (var->k) {
    case VLOCAL: {
      freeexp(fs, ex);
      exp2reg(fs, ex, var->u.var.ridx);
      return;
    }
    case VUPVAL: {
      int e = luaK_exp2anyreg(fs, ex);
      luaK_codeABCk(fs,OP_SETUPVAL,e,var->u.info,0,0);
      break;
    }
    case VINDEXUP: {
      codeABRK(fs, OP_SETTABUP, var->u.ind.t, var->u.ind.idx, ex);
      break;
    }
    case VINDEXI: {
      codeABRK(fs, OP_SETI, var->u.ind.t, var->u.ind.idx, ex);
      break;
    }
    case VINDEXSTR: {
      codeABRK(fs, OP_SETFIELD, var->u.ind.t, var->u.ind.idx, ex);
      break;
    }
    case VINDEXED: {
      codeABRK(fs, OP_SETTABLE, var->u.ind.t, var->u.ind.idx, ex);
      break;
    }
    default: ((void)0);
  }
  freeexp(fs, ex);
}





void luaK_self (FuncState *fs, expdesc *e, expdesc *key) {
  int ereg;
  luaK_exp2anyreg(fs, e);
  ereg = e->u.info;
  freeexp(fs, e);
  e->u.info = fs->freereg;
  e->k = VNONRELOC;
  luaK_reserveregs(fs, 2);
  codeABRK(fs, OP_SELF, e->u.info, ereg, key);
  freeexp(fs, key);
}





static void negatecondition (FuncState *fs, expdesc *e) {
  Instruction *pc = getjumpcontrol(fs, e->u.info);
  ((void)0)
                                                                      ;
  ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(1)))<<(((0 + 7) + 8))))) | ((((Instruction)((((((int)((((*pc)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0))))))) ^ 1)))<<((0 + 7) + 8))&((~((~(Instruction)0)<<(1)))<<(((0 + 7) + 8))))));
}
static int jumponcond (FuncState *fs, expdesc *e, int cond) {
  if (e->k == VRELOC) {
    Instruction ie = ((fs)->f->code[(e)->u.info]);
    if ((((OpCode)(((ie)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) == OP_NOT) {
      removelastinstruction(fs);
      return condjump(fs, OP_TEST, ((((int)((((ie)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))), 0, 0, !cond);
    }

  }
  discharge2anyreg(fs, e);
  freeexp(fs, e);
  return condjump(fs, OP_TESTSET, ((1<<8)-1), e->u.info, 0, cond);
}





void luaK_goiftrue (FuncState *fs, expdesc *e) {
  int pc;
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case VJMP: {
      negatecondition(fs, e);
      pc = e->u.info;
      break;
    }
    case VK: case VKFLT: case VKINT: case VKSTR: case VTRUE: {
      pc = (-1);
      break;
    }
    default: {
      pc = jumponcond(fs, e, 0);
      break;
    }
  }
  luaK_concat(fs, &e->f, pc);
  luaK_patchtohere(fs, e->t);
  e->t = (-1);
}





void luaK_goiffalse (FuncState *fs, expdesc *e) {
  int pc;
  luaK_dischargevars(fs, e);
  switch (e->k) {
    case VJMP: {
      pc = e->u.info;
      break;
    }
    case VNIL: case VFALSE: {
      pc = (-1);
      break;
    }
    default: {
      pc = jumponcond(fs, e, 1);
      break;
    }
  }
  luaK_concat(fs, &e->t, pc);
  luaK_patchtohere(fs, e->f);
  e->f = (-1);
}





static void codenot (FuncState *fs, expdesc *e) {
  switch (e->k) {
    case VNIL: case VFALSE: {
      e->k = VTRUE;
      break;
    }
    case VK: case VKFLT: case VKINT: case VKSTR: case VTRUE: {
      e->k = VFALSE;
      break;
    }
    case VJMP: {
      negatecondition(fs, e);
      break;
    }
    case VRELOC:
    case VNONRELOC: {
      discharge2anyreg(fs, e);
      freeexp(fs, e);
      e->u.info = luaK_codeABCk(fs,OP_NOT,0,e->u.info,0,0);
      e->k = VRELOC;
      break;
    }
    default: ((void)0);
  }

  { int temp = e->f; e->f = e->t; e->t = temp; }
  removevalues(fs, e->f);
  removevalues(fs, e->t);
}





static int isKstr (FuncState *fs, expdesc *e) {
  return (e->k == VK && !((e)->t != (e)->f) && e->u.info <= ((1<<8)-1) &&
          ((((&fs->f->k[e->u.info]))->tt_) == (((((4) | ((0) << 4))) | (1 << 6)))));
}




int luaK_isKint (expdesc *e) {
  return (e->k == VKINT && !((e)->t != (e)->f));
}






static int isCint (expdesc *e) {
  return luaK_isKint(e) && (((lua_Unsigned)(e->u.ival)) <= ((lua_Unsigned)(((1<<8)-1))));
}






static int isSCint (expdesc *e) {
  return luaK_isKint(e) && fitsC(e->u.ival);
}






static int isSCnumber (expdesc *e, int *pi, int *isfloat) {
  lua_Integer i;
  if (e->k == VKINT)
    i = e->u.ival;
  else if (e->k == VKFLT && luaV_flttointeger(e->u.nval, &i, F2Ieq))
    *isfloat = 1;
  else
    return 0;
  if (!((e)->t != (e)->f) && fitsC(i)) {
    *pi = ((((int)((i)))) + (((1<<8)-1) >> 1));
    return 1;
  }
  else
    return 0;
}
void luaK_indexed (FuncState *fs, expdesc *t, expdesc *k) {
  if (k->k == VKSTR)
    str2K(fs, k);
  ((void)0)
                                                                     ;
  if (t->k == VUPVAL && !isKstr(fs, k))
    luaK_exp2anyreg(fs, t);
  if (t->k == VUPVAL) {
    t->u.ind.t = t->u.info;
    t->u.ind.idx = k->u.info;
    t->k = VINDEXUP;
  }
  else {

    t->u.ind.t = (t->k == VLOCAL) ? t->u.var.ridx: t->u.info;
    if (isKstr(fs, k)) {
      t->u.ind.idx = k->u.info;
      t->k = VINDEXSTR;
    }
    else if (isCint(k)) {
      t->u.ind.idx = ((int)((k->u.ival)));
      t->k = VINDEXI;
    }
    else {
      t->u.ind.idx = luaK_exp2anyreg(fs, k);
      t->k = VINDEXED;
    }
  }
}







static int validop (int op, TValue *v1, TValue *v2) {
  switch (op) {
    case 7: case 8: case 9:
    case 10: case 11: case 13: {
      lua_Integer i;
      return (luaV_tointegerns(v1, &i, F2Ieq) &&
              luaV_tointegerns(v2, &i, F2Ieq));
    }
    case 5: case 6: case 3:
      return (((((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((lua_Number)(((((v2)->value_).i)))) : (((v2)->value_).n))) != 0);
    default: return 1;
  }
}






static int constfolding (FuncState *fs, int op, expdesc *e1,
                                        const expdesc *e2) {
  TValue v1, v2, res;
  if (!tonumeral(e1, &v1) || !tonumeral(e2, &v2) || !validop(op, &v1, &v2))
    return 0;
  luaO_rawarith(fs->ls->L, op, &v1, &v2, &res);
  if (((((&res))->tt_) == (((3) | ((0) << 4))))) {
    e1->k = VKINT;
    e1->u.ival = (((&res)->value_).i);
  }
  else {
    lua_Number n = (((&res)->value_).n);
    if ((!(((n))==((n)))) || n == 0)
      return 0;
    e1->k = VKFLT;
    e1->u.nval = n;
  }
  return 1;
}





static inline OpCode binopr2op (BinOpr opr, BinOpr baser, OpCode base) {
  ((void)0)

                                                 ;
  return ((OpCode)((((int)((opr))) - ((int)((baser)))) + ((int)((base)))));
}





static inline OpCode unopr2op (UnOpr opr) {
  return ((OpCode)((((int)((opr))) - ((int)((OPR_MINUS)))) + ((int)((OP_UNM)))))
                                                        ;
}





static inline TMS binopr2TM (BinOpr opr) {
  ((void)0);
  return ((TMS)((((int)((opr))) - ((int)((OPR_ADD)))) + ((int)((TM_ADD)))));
}







static void codeunexpval (FuncState *fs, OpCode op, expdesc *e, int line) {
  int r = luaK_exp2anyreg(fs, e);
  freeexp(fs, e);
  e->u.info = luaK_codeABCk(fs,op,0,r,0,0);
  e->k = VRELOC;
  luaK_fixline(fs, line);
}
static void finishbinexpval (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, int v2, int flip, int line,
                             OpCode mmop, TMS event) {
  int v1 = luaK_exp2anyreg(fs, e1);
  int pc = luaK_codeABCk(fs, op, 0, v1, v2, 0);
  freeexps(fs, e1, e2);
  e1->u.info = pc;
  e1->k = VRELOC;
  luaK_fixline(fs, line);
  luaK_codeABCk(fs, mmop, v1, v2, event, flip);
  luaK_fixline(fs, line);
}






static void codebinexpval (FuncState *fs, BinOpr opr,
                           expdesc *e1, expdesc *e2, int line) {
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADD);
  int v2 = luaK_exp2anyreg(fs, e2);

  ((void)0)
                                                   ;
  ((void)0);
  finishbinexpval(fs, e1, e2, op, v2, 0, line, OP_MMBIN, binopr2TM(opr));
}





static void codebini (FuncState *fs, OpCode op,
                       expdesc *e1, expdesc *e2, int flip, int line,
                       TMS event) {
  int v2 = ((((int)((e2->u.ival)))) + (((1<<8)-1) >> 1));
  ((void)0);
  finishbinexpval(fs, e1, e2, op, v2, flip, line, OP_MMBINI, event);
}





static void codebinK (FuncState *fs, BinOpr opr,
                      expdesc *e1, expdesc *e2, int flip, int line) {
  TMS event = binopr2TM(opr);
  int v2 = e2->u.info;
  OpCode op = binopr2op(opr, OPR_ADD, OP_ADDK);
  finishbinexpval(fs, e1, e2, op, v2, flip, line, OP_MMBINK, event);
}





static int finishbinexpneg (FuncState *fs, expdesc *e1, expdesc *e2,
                             OpCode op, int line, TMS event) {
  if (!luaK_isKint(e2))
    return 0;
  else {
    lua_Integer i2 = e2->u.ival;
    if (!(fitsC(i2) && fitsC(-i2)))
      return 0;
    else {
      int v2 = ((int)((i2)));
      finishbinexpval(fs, e1, e2, op, ((-v2) + (((1<<8)-1) >> 1)), 0, line, OP_MMBINI, event);

      ((fs->f->code[fs->pc - 1]) = (((fs->f->code[fs->pc - 1])&(~((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))) | ((((Instruction)(((v2) + (((1<<8)-1) >> 1))))<<(((0 + 7) + 8) + 1))&((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))));
      return 1;
    }
  }
}


static void swapexps (expdesc *e1, expdesc *e2) {
  expdesc temp = *e1; *e1 = *e2; *e2 = temp;
}





static void codebinNoK (FuncState *fs, BinOpr opr,
                        expdesc *e1, expdesc *e2, int flip, int line) {
  if (flip)
    swapexps(e1, e2);
  codebinexpval(fs, opr, e1, e2, line);
}






static void codearith (FuncState *fs, BinOpr opr,
                       expdesc *e1, expdesc *e2, int flip, int line) {
  if (tonumeral(e2, 
                   ((void *)0)
                       ) && luaK_exp2K(fs, e2))
    codebinK(fs, opr, e1, e2, flip, line);
  else
    codebinNoK(fs, opr, e1, e2, flip, line);
}







static void codecommutative (FuncState *fs, BinOpr op,
                             expdesc *e1, expdesc *e2, int line) {
  int flip = 0;
  if (tonumeral(e1, 
                   ((void *)0)
                       )) {
    swapexps(e1, e2);
    flip = 1;
  }
  if (op == OPR_ADD && isSCint(e2))
    codebini(fs, OP_ADDI, e1, e2, flip, line, TM_ADD);
  else
    codearith(fs, op, e1, e2, flip, line);
}






static void codebitwise (FuncState *fs, BinOpr opr,
                         expdesc *e1, expdesc *e2, int line) {
  int flip = 0;
  if (e1->k == VKINT) {
    swapexps(e1, e2);
    flip = 1;
  }
  if (e2->k == VKINT && luaK_exp2K(fs, e2))
    codebinK(fs, opr, e1, e2, flip, line);
  else
    codebinNoK(fs, opr, e1, e2, flip, line);
}






static void codeorder (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  int r1, r2;
  int im;
  int isfloat = 0;
  OpCode op;
  if (isSCnumber(e2, &im, &isfloat)) {

    r1 = luaK_exp2anyreg(fs, e1);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_LTI);
  }
  else if (isSCnumber(e1, &im, &isfloat)) {

    r1 = luaK_exp2anyreg(fs, e2);
    r2 = im;
    op = binopr2op(opr, OPR_LT, OP_GTI);
  }
  else {
    r1 = luaK_exp2anyreg(fs, e1);
    r2 = luaK_exp2anyreg(fs, e2);
    op = binopr2op(opr, OPR_LT, OP_LT);
  }
  freeexps(fs, e1, e2);
  e1->u.info = condjump(fs, op, r1, r2, isfloat, 1);
  e1->k = VJMP;
}






static void codeeq (FuncState *fs, BinOpr opr, expdesc *e1, expdesc *e2) {
  int r1, r2;
  int im;
  int isfloat = 0;
  OpCode op;
  if (e1->k != VNONRELOC) {
    ((void)0);
    swapexps(e1, e2);
  }
  r1 = luaK_exp2anyreg(fs, e1);
  if (isSCnumber(e2, &im, &isfloat)) {
    op = OP_EQI;
    r2 = im;
  }
  else if (luaK_exp2RK(fs, e2)) {
    op = OP_EQK;
    r2 = e2->u.info;
  }
  else {
    op = OP_EQ;
    r2 = luaK_exp2anyreg(fs, e2);
  }
  freeexps(fs, e1, e2);
  e1->u.info = condjump(fs, op, r1, r2, isfloat, (opr == OPR_EQ));
  e1->k = VJMP;
}





void luaK_prefix (FuncState *fs, UnOpr opr, expdesc *e, int line) {
  static const expdesc ef = {VKINT, {0}, (-1), (-1)};
  luaK_dischargevars(fs, e);
  switch (opr) {
    case OPR_MINUS: case OPR_BNOT:
      if (constfolding(fs, opr + 12, e, &ef))
        break;

    case OPR_LEN:
      codeunexpval(fs, unopr2op(opr), e, line);
      break;
    case OPR_NOT: codenot(fs, e); break;
    default: ((void)0);
  }
}






void luaK_infix (FuncState *fs, BinOpr op, expdesc *v) {
  luaK_dischargevars(fs, v);
  switch (op) {
    case OPR_AND: {
      luaK_goiftrue(fs, v);
      break;
    }
    case OPR_OR: {
      luaK_goiffalse(fs, v);
      break;
    }
    case OPR_CONCAT: {
      luaK_exp2nextreg(fs, v);
      break;
    }
    case OPR_ADD: case OPR_SUB:
    case OPR_MUL: case OPR_DIV: case OPR_IDIV:
    case OPR_MOD: case OPR_POW:
    case OPR_BAND: case OPR_BOR: case OPR_BXOR:
    case OPR_SHL: case OPR_SHR: {
      if (!tonumeral(v, 
                       ((void *)0)
                           ))
        luaK_exp2anyreg(fs, v);


      break;
    }
    case OPR_EQ: case OPR_NE: {
      if (!tonumeral(v, 
                       ((void *)0)
                           ))
        luaK_exp2RK(fs, v);

      break;
    }
    case OPR_LT: case OPR_LE:
    case OPR_GT: case OPR_GE: {
      int dummy, dummy2;
      if (!isSCnumber(v, &dummy, &dummy2))
        luaK_exp2anyreg(fs, v);

      break;
    }
    default: ((void)0);
  }
}






static void codeconcat (FuncState *fs, expdesc *e1, expdesc *e2, int line) {
  Instruction *ie2 = previousinstruction(fs);
  if ((((OpCode)(((*ie2)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) == OP_CONCAT) {
    int n = ((((int)((((*ie2)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
    ((void)0);
    freeexp(fs, e2);
    ((*ie2) = (((*ie2)&(~((~((~(Instruction)0)<<(8)))<<((0 + 7))))) | ((((Instruction)(e1->u.info))<<(0 + 7))&((~((~(Instruction)0)<<(8)))<<((0 + 7))))));
    ((*ie2) = (((*ie2)&(~((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))) | ((((Instruction)(n + 1))<<(((0 + 7) + 8) + 1))&((~((~(Instruction)0)<<(8)))<<((((0 + 7) + 8) + 1))))));
  }
  else {
    luaK_codeABCk(fs,OP_CONCAT,e1->u.info,2,0,0);
    freeexp(fs, e2);
    luaK_fixline(fs, line);
  }
}





void luaK_posfix (FuncState *fs, BinOpr opr,
                  expdesc *e1, expdesc *e2, int line) {
  luaK_dischargevars(fs, e2);
  if (((opr) <= OPR_SHR) && constfolding(fs, opr + 0, e1, e2))
    return;
  switch (opr) {
    case OPR_AND: {
      ((void)0);
      luaK_concat(fs, &e2->f, e1->f);
      *e1 = *e2;
      break;
    }
    case OPR_OR: {
      ((void)0);
      luaK_concat(fs, &e2->t, e1->t);
      *e1 = *e2;
      break;
    }
    case OPR_CONCAT: {
      luaK_exp2nextreg(fs, e2);
      codeconcat(fs, e1, e2, line);
      break;
    }
    case OPR_ADD: case OPR_MUL: {
      codecommutative(fs, opr, e1, e2, line);
      break;
    }
    case OPR_SUB: {
      if (finishbinexpneg(fs, e1, e2, OP_ADDI, line, TM_SUB))
        break;

    }
    case OPR_DIV: case OPR_IDIV: case OPR_MOD: case OPR_POW: {
      codearith(fs, opr, e1, e2, 0, line);
      break;
    }
    case OPR_BAND: case OPR_BOR: case OPR_BXOR: {
      codebitwise(fs, opr, e1, e2, line);
      break;
    }
    case OPR_SHL: {
      if (isSCint(e1)) {
        swapexps(e1, e2);
        codebini(fs, OP_SHLI, e1, e2, 1, line, TM_SHL);
      }
      else if (finishbinexpneg(fs, e1, e2, OP_SHRI, line, TM_SHL)) {
                                 ;
      }
      else
       codebinexpval(fs, opr, e1, e2, line);
      break;
    }
    case OPR_SHR: {
      if (isSCint(e2))
        codebini(fs, OP_SHRI, e1, e2, 0, line, TM_SHR);
      else
        codebinexpval(fs, opr, e1, e2, line);
      break;
    }
    case OPR_EQ: case OPR_NE: {
      codeeq(fs, opr, e1, e2);
      break;
    }
    case OPR_GT: case OPR_GE: {

      swapexps(e1, e2);
      opr = ((BinOpr)((opr - OPR_GT) + OPR_LT));
    }
    case OPR_LT: case OPR_LE: {
      codeorder(fs, opr, e1, e2);
      break;
    }
    default: ((void)0);
  }
}






void luaK_fixline (FuncState *fs, int line) {
  removelastlineinfo(fs);
  savelineinfo(fs, fs->f, line);
}


void luaK_settablesize (FuncState *fs, int pc, int ra, int asize, int hsize) {
  Instruction *inst = &fs->f->code[pc];
  int rb = (hsize != 0) ? luaO_ceillog2(hsize) + 1 : 0;
  int extra = asize / (((1<<8)-1) + 1);
  int rc = asize % (((1<<8)-1) + 1);
  int k = (extra > 0);
  *inst = ((((Instruction)(OP_NEWTABLE))<<0) | (((Instruction)(ra))<<(0 + 7)) | (((Instruction)(rb))<<(((0 + 7) + 8) + 1)) | (((Instruction)(rc))<<((((0 + 7) + 8) + 1) + 8)) | (((Instruction)(k))<<((0 + 7) + 8)));
  *(inst + 1) = ((((Instruction)(OP_EXTRAARG))<<0) | (((Instruction)(extra))<<(0 + 7)));
}
void luaK_setlist (FuncState *fs, int base, int nelems, int tostore) {
  ((void)0);
  if (tostore == (-1))
    tostore = 0;
  if (nelems <= ((1<<8)-1))
    luaK_codeABCk(fs,OP_SETLIST,base,tostore,nelems,0);
  else {
    int extra = nelems / (((1<<8)-1) + 1);
    nelems %= (((1<<8)-1) + 1);
    luaK_codeABCk(fs, OP_SETLIST, base, tostore, nelems, 1);
    codeextraarg(fs, extra);
  }
  fs->freereg = base + 1;
}





static int finaltarget (Instruction *code, int i) {
  int count;
  for (count = 0; count < 100; count++) {
    Instruction pc = code[i];
    if ((((OpCode)(((pc)>>0) & ((~((~(Instruction)0)<<(7)))<<(0))))) != OP_JMP)
      break;
     else
       i += ((((int)((((pc)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1;
  }
  return i;
}






void luaK_finish (FuncState *fs) {
  int i;
  Proto *p = fs->f;
  for (i = 0; i < fs->pc; i++) {
    Instruction *pc = &p->code[i];
    ((void)0);
    switch ((((OpCode)(((*pc)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))) {
      case OP_RETURN0: case OP_RETURN1: {
        if (!(fs->needclose || p->is_vararg))
          break;

        ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(7)))<<(0)))) | ((((Instruction)(OP_RETURN))<<0)&((~((~(Instruction)0)<<(7)))<<(0)))));
      }
      case OP_RETURN: case OP_TAILCALL: {
        if (fs->needclose)
          ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(1)))<<(((0 + 7) + 8))))) | ((((Instruction)(1))<<((0 + 7) + 8))&((~((~(Instruction)0)<<(1)))<<(((0 + 7) + 8))))));
        if (p->is_vararg)
          ((*pc) = (((*pc)&(~((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))) | ((((Instruction)(p->numparams + 1))<<((((0 + 7) + 8) + 1) + 8))&((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))));
        break;
      }
      case OP_JMP: {
        int target = finaltarget(p->code, i);
        fixjump(fs, i, target);
        break;
      }
      default: break;
    }
  }
}
extern const lu_byte luai_ctype_[257];



 const lu_byte luai_ctype_[257] = {
  0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16,
  0x16, 0x16, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
  0x04, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x05,
  0x04, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
  0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const char *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const char **name);


static int currentpc (CallInfo *ci) {
  ((void)0);
  return (((int)(((ci->u.l.savedpc) - ((((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p)->code))) - 1);
}
static int getbaseline (const Proto *f, int pc, int *basepc) {
  if (f->sizeabslineinfo == 0 || pc < f->abslineinfo[0].pc) {
    *basepc = -1;
    return f->linedefined;
  }
  else {
    int i = ((unsigned int)((pc))) / 128 - 1;

    ((void)0)
                                                                     ;
    while (i + 1 < f->sizeabslineinfo && pc >= f->abslineinfo[i + 1].pc)
      i++;
    *basepc = f->abslineinfo[i].pc;
    return f->abslineinfo[i].line;
  }
}







int luaG_getfuncline (const Proto *f, int pc) {
  if (f->lineinfo == 
                    ((void *)0)
                        )
    return -1;
  else {
    int basepc;
    int baseline = getbaseline(f, pc, &basepc);
    while (basepc++ < pc) {
      ((void)0);
      baseline += f->lineinfo[basepc];
    }
    return baseline;
  }
}


static int getcurrentline (CallInfo *ci) {
  return luaG_getfuncline((((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p, currentpc(ci));
}
static void settraps (CallInfo *ci) {
  for (; ci != 
              ((void *)0)
                  ; ci = ci->previous)
    if ((!((ci)->callstatus & (1<<1))))
      ci->u.l.trap = 1;
}
extern void lua_sethook (lua_State *L, lua_Hook func, int mask, int count) {
  if (func == 
             ((void *)0) 
                  || mask == 0) {
    mask = 0;
    func = 
          ((void *)0)
              ;
  }
  L->hook = func;
  L->basehookcount = count;
  (L->hookcount = L->basehookcount);
  L->hookmask = ((lu_byte)((mask)));
  if (mask)
    settraps(L->ci);
}


extern lua_Hook lua_gethook (lua_State *L) {
  return L->hook;
}


extern int lua_gethookmask (lua_State *L) {
  return L->hookmask;
}


extern int lua_gethookcount (lua_State *L) {
  return L->basehookcount;
}


extern int lua_getstack (lua_State *L, int level, lua_Debug *ar) {
  int status;
  CallInfo *ci;
  if (level < 0) return 0;
  ((void) 0);
  for (ci = L->ci; level > 0 && ci != &L->base_ci; ci = ci->previous)
    level--;
  if (level == 0 && ci != &L->base_ci) {
    status = 1;
    ar->i_ci = ci;
  }
  else status = 0;
  ((void) 0);
  return status;
}


static const char *upvalname (const Proto *p, int uv) {
  TString *s = (p->upvalues[uv].name);
  if (s == 
          ((void *)0)
              ) return "?";
  else return ((s)->contents);
}


static const char *findvararg (CallInfo *ci, int n, StkId *pos) {
  if (((&((((union GCUnion *)(((((&(ci->func.p)->val))->value_).gc))))->cl.l)))->p->is_vararg) {
    int nextra = ci->u.l.nextraargs;
    if (n >= -nextra) {
      *pos = ci->func.p - nextra - (n + 1);
      return "(vararg)";
    }
  }
  return 
        ((void *)0)
            ;
}


const char *luaG_findlocal (lua_State *L, CallInfo *ci, int n, StkId *pos) {
  StkId base = ci->func.p + 1;
  const char *name = 
                    ((void *)0)
                        ;
  if ((!((ci)->callstatus & (1<<1)))) {
    if (n < 0)
      return findvararg(ci, n, pos);
    else
      name = luaF_getlocalname((((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p, n, currentpc(ci));
  }
  if (name == 
             ((void *)0)
                 ) {
    StkId limit = (ci == L->ci) ? L->top.p : ci->next->func.p;
    if (limit - base >= n && n > 0) {

      name = (!((ci)->callstatus & (1<<1))) ? "(temporary)" : "(C temporary)";
    }
    else
      return 
            ((void *)0)
                ;
  }
  if (pos)
    *pos = base + (n - 1);
  return name;
}


extern const char *lua_getlocal (lua_State *L, const lua_Debug *ar, int n) {
  const char *name;
  ((void) 0);
  if (ar == 
           ((void *)0)
               ) {
    if (!(((((&(L->top.p - 1)->val)))->tt_) == (((((6) | ((0) << 4))) | (1 << 6)))))
      name = 
            ((void *)0)
                ;
    else
      name = luaF_getlocalname(((&((((union GCUnion *)(((((&(L->top.p - 1)->val))->value_).gc))))->cl.l)))->p, n, 0);
  }
  else {
    StkId pos = 
               ((void *)0)
                   ;
    name = luaG_findlocal(L, ar->i_ci, n, &pos);
    if (name) {
      { TValue *io1=((&(L->top.p)->val)); const TValue *io2=((&(pos)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      {L->top.p++; ((void)L, ((void)0));};
    }
  }
  ((void) 0);
  return name;
}


extern const char *lua_setlocal (lua_State *L, const lua_Debug *ar, int n) {
  StkId pos = 
             ((void *)0)
                 ;
  const char *name;
  ((void) 0);
  name = luaG_findlocal(L, ar->i_ci, n, &pos);
  if (name) {
    { TValue *io1=((&(pos)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    L->top.p--;
  }
  ((void) 0);
  return name;
}


static void funcinfo (lua_Debug *ar, Closure *cl) {
  if (((cl) == 
     ((void *)0) 
     || (cl)->c.tt == ((6) | ((2) << 4)))) {
    ar->source = "=[C]";
    ar->srclen = (sizeof("=[C]")/sizeof(char) - 1);
    ar->linedefined = -1;
    ar->lastlinedefined = -1;
    ar->what = "C";
  }
  else {
    const Proto *p = cl->l.p;
    if (p->source) {
      ar->source = ((p->source)->contents);
      ar->srclen = ((p->source)->tt == ((4) | ((0) << 4)) ? (p->source)->shrlen : (p->source)->u.lnglen);
    }
    else {
      ar->source = "=?";
      ar->srclen = (sizeof("=?")/sizeof(char) - 1);
    }
    ar->linedefined = p->linedefined;
    ar->lastlinedefined = p->lastlinedefined;
    ar->what = (ar->linedefined == 0) ? "main" : "Lua";
  }
  luaO_chunkid(ar->short_src, ar->source, ar->srclen);
}


static int nextline (const Proto *p, int currentline, int pc) {
  if (p->lineinfo[pc] != (-0x80))
    return currentline + p->lineinfo[pc];
  else
    return luaG_getfuncline(p, pc);
}


static void collectvalidlines (lua_State *L, Closure *f) {
  if (((f) == 
     ((void *)0) 
     || (f)->c.tt == ((6) | ((2) << 4)))) {
    (((&(L->top.p)->val))->tt_=(((0) | ((0) << 4))));
    {L->top.p++; ((void)L, ((void)0));};
  }
  else {
    int i;
    TValue v;
    const Proto *p = f->l.p;
    int currentline = p->linedefined;
    Table *t = luaH_new(L);
    { TValue *io = ((&(L->top.p)->val)); Table *x_ = (t); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
    {L->top.p++; ((void)L, ((void)0));};
    ((&v)->tt_=(((1) | ((1) << 4))));
    if (!p->is_vararg)
      i = 0;
    else {
      ((void)0);
      currentline = nextline(p, currentline, 0);
      i = 1;
    }
    for (; i < p->sizelineinfo; i++) {
      currentline = nextline(p, currentline, i);
      luaH_setint(L, t, currentline, &v);
    }
  }
}


static const char *getfuncname (lua_State *L, CallInfo *ci, const char **name) {

  if (ci != 
           ((void *)0) 
                && !(ci->callstatus & (1<<5)))
    return funcnamefromcall(L, ci->previous, name);
  else return 
             ((void *)0)
                 ;
}


static int auxgetinfo (lua_State *L, const char *what, lua_Debug *ar,
                       Closure *f, CallInfo *ci) {
  int status = 1;
  for (; *what; what++) {
    switch (*what) {
      case 'S': {
        funcinfo(ar, f);
        break;
      }
      case 'l': {
        ar->currentline = (ci && (!((ci)->callstatus & (1<<1)))) ? getcurrentline(ci) : -1;
        break;
      }
      case 'u': {
        ar->nups = (f == 
                        ((void *)0)
                            ) ? 0 : f->c.nupvalues;
        if (((f) == 
           ((void *)0) 
           || (f)->c.tt == ((6) | ((2) << 4)))) {
          ar->isvararg = 1;
          ar->nparams = 0;
        }
        else {
          ar->isvararg = f->l.p->is_vararg;
          ar->nparams = f->l.p->numparams;
        }
        break;
      }
      case 't': {
        ar->istailcall = (ci) ? ci->callstatus & (1<<5) : 0;
        break;
      }
      case 'n': {
        ar->namewhat = getfuncname(L, ci, &ar->name);
        if (ar->namewhat == 
                           ((void *)0)
                               ) {
          ar->namewhat = "";
          ar->name = 
                    ((void *)0)
                        ;
        }
        break;
      }
      case 'r': {
        if (ci == 
                 ((void *)0) 
                      || !(ci->callstatus & (1<<8)))
          ar->ftransfer = ar->ntransfer = 0;
        else {
          ar->ftransfer = ci->u2.transferinfo.ftransfer;
          ar->ntransfer = ci->u2.transferinfo.ntransfer;
        }
        break;
      }
      case 'L':
      case 'f':
        break;
      default: status = 0;
    }
  }
  return status;
}


extern int lua_getinfo (lua_State *L, const char *what, lua_Debug *ar) {
  int status;
  Closure *cl;
  CallInfo *ci;
  TValue *func;
  ((void) 0);
  if (*what == '>') {
    ci = 
        ((void *)0)
            ;
    func = (&(L->top.p - 1)->val);
    ((void)L, ((void)0));
    what++;
    L->top.p--;
  }
  else {
    ci = ar->i_ci;
    func = (&(ci->func.p)->val);
    ((void)0);
  }
  cl = (((((func))->tt_) == (((((6) | ((0) << 4))) | (1 << 6)))) || ((((func))->tt_) == (((((6) | ((2) << 4))) | (1 << 6))))) ? ((&((((union GCUnion *)((((func)->value_).gc))))->cl))) : 
                                          ((void *)0)
                                              ;
  status = auxgetinfo(L, what, ar, cl, ci);
  if (strchr(what, 'f')) {
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=(func); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    {L->top.p++; ((void)L, ((void)0));};
  }
  if (strchr(what, 'L'))
    collectvalidlines(L, cl);
  ((void) 0);
  return status;
}
static const char *getobjname (const Proto *p, int lastpc, int reg,
                               const char **name);





static void kname (const Proto *p, int c, const char **name) {
  TValue *kvalue = &p->k[c];
  *name = ((((((((kvalue))->tt_)) & 0x0F)) == (4))) ? ((((&((((union GCUnion *)((((kvalue)->value_).gc))))->ts))))->contents) : "?";
}





static void rname (const Proto *p, int pc, int c, const char **name) {
  const char *what = getobjname(p, pc, c, name);
  if (!(what && *what == 'c'))
    *name = "?";
}





static void rkname (const Proto *p, int pc, Instruction i, const char **name) {
  int c = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
  if (((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0))))))))
    kname(p, c, name);
  else
    rname(p, pc, c, name);
}


static int filterpc (int pc, int jmptarget) {
  if (pc < jmptarget)
    return -1;
  else return pc;
}





static int findsetreg (const Proto *p, int lastpc, int reg) {
  int pc;
  int setreg = -1;
  int jmptarget = 0;
  if ((luaP_opmodes[(((OpCode)(((p->code[lastpc])>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))] & (1 << 7)))
    lastpc--;
  for (pc = 0; pc < lastpc; pc++) {
    Instruction i = p->code[pc];
    OpCode op = (((OpCode)(((i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))));
    int a = (((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))));
    int change;
    switch (op) {
      case OP_LOADNIL: {
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        change = (a <= reg && reg <= a + b);
        break;
      }
      case OP_TFORCALL: {
        change = (reg >= a + 2);
        break;
      }
      case OP_CALL:
      case OP_TAILCALL: {
        change = (reg >= a);
        break;
      }
      case OP_JMP: {
        int b = ((((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1));
        int dest = pc + 1 + b;

        if (dest <= lastpc && dest > jmptarget)
          jmptarget = dest;
        change = 0;
        break;
      }
      default:
        change = ((luaP_opmodes[op] & (1 << 3)) && reg == a);
        break;
    }
    if (change)
      setreg = filterpc(pc, jmptarget);
  }
  return setreg;
}






static const char *gxf (const Proto *p, int pc, Instruction i, int isup) {
  int t = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
  const char *name;
  if (isup)
    name = upvalname(p, t);
  else
    getobjname(p, pc, t, &name);
  return (name && strcmp(name, "_ENV") == 0) ? "global" : "field";
}


static const char *getobjname (const Proto *p, int lastpc, int reg,
                               const char **name) {
  int pc;
  *name = luaF_getlocalname(p, reg + 1, lastpc);
  if (*name)
    return "local";

  pc = findsetreg(p, lastpc, reg);
  if (pc != -1) {
    Instruction i = p->code[pc];
    OpCode op = (((OpCode)(((i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))));
    switch (op) {
      case OP_MOVE: {
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        if (b < (((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))
          return getobjname(p, pc, b, name);
        break;
      }
      case OP_GETTABUP: {
        int k = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        kname(p, k, name);
        return gxf(p, pc, i, 1);
      }
      case OP_GETTABLE: {
        int k = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        rname(p, pc, k, name);
        return gxf(p, pc, i, 0);
      }
      case OP_GETI: {
        *name = "integer index";
        return "field";
      }
      case OP_GETFIELD: {
        int k = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        kname(p, k, name);
        return gxf(p, pc, i, 0);
      }
      case OP_GETUPVAL: {
        *name = upvalname(p, ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        return "upvalue";
      }
      case OP_LOADK:
      case OP_LOADKX: {
        int b = (op == OP_LOADK) ? ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))))
                                 : ((((int)((((p->code[pc + 1])>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))));
        if ((((((((&p->k[b]))->tt_)) & 0x0F)) == (4))) {
          *name = ((((&((((union GCUnion *)((((&p->k[b])->value_).gc))))->ts))))->contents);
          return "constant";
        }
        break;
      }
      case OP_SELF: {
        rkname(p, pc, i, name);
        return "method";
      }
      default: break;
    }
  }
  return 
        ((void *)0)
            ;
}
static const char *funcnamefromcode (lua_State *L, const Proto *p,
                                     int pc, const char **name) {
  TMS tm = (TMS)0;
  Instruction i = p->code[pc];
  switch ((((OpCode)(((i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))) {
    case OP_CALL:
    case OP_TAILCALL:
      return getobjname(p, pc, (((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))), name);
    case OP_TFORCALL: {
      *name = "for iterator";
       return "for iterator";
    }

    case OP_SELF: case OP_GETTABUP: case OP_GETTABLE:
    case OP_GETI: case OP_GETFIELD:
      tm = TM_INDEX;
      break;
    case OP_SETTABUP: case OP_SETTABLE: case OP_SETI: case OP_SETFIELD:
      tm = TM_NEWINDEX;
      break;
    case OP_MMBIN: case OP_MMBINI: case OP_MMBINK: {
      tm = ((TMS)(((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))));
      break;
    }
    case OP_UNM: tm = TM_UNM; break;
    case OP_BNOT: tm = TM_BNOT; break;
    case OP_LEN: tm = TM_LEN; break;
    case OP_CONCAT: tm = TM_CONCAT; break;
    case OP_EQ: tm = TM_EQ; break;

    case OP_LT: case OP_LTI: case OP_GTI: tm = TM_LT; break;
    case OP_LE: case OP_LEI: case OP_GEI: tm = TM_LE; break;
    case OP_CLOSE: case OP_RETURN: tm = TM_CLOSE; break;
    default:
      return 
            ((void *)0)
                ;
  }
  *name = (((L->l_G)->tmname[tm])->contents) + 2;
  return "metamethod";
}





static const char *funcnamefromcall (lua_State *L, CallInfo *ci,
                                                   const char **name) {
  if (ci->callstatus & (1<<3)) {
    *name = "?";
    return "hook";
  }
  else if (ci->callstatus & (1<<7)) {
    *name = "__gc";
    return "metamethod";
  }
  else if ((!((ci)->callstatus & (1<<1))))
    return funcnamefromcode(L, (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p, currentpc(ci), name);
  else
    return 
          ((void *)0)
              ;
}
static int instack (CallInfo *ci, const TValue *o) {
  int pos;
  StkId base = ci->func.p + 1;
  for (pos = 0; base + pos < ci->top.p; pos++) {
    if (o == (&(base + pos)->val))
      return pos;
  }
  return -1;
}







static const char *getupvalname (CallInfo *ci, const TValue *o,
                                 const char **name) {
  LClosure *c = (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))));
  int i;
  for (i = 0; i < c->nupvalues; i++) {
    if (c->upvals[i]->v.p == o) {
      *name = upvalname(c->p, i);
      return "upvalue";
    }
  }
  return 
        ((void *)0)
            ;
}


static const char *formatvarinfo (lua_State *L, const char *kind,
                                                const char *name) {
  if (kind == 
             ((void *)0)
                 )
    return "";
  else
    return luaO_pushfstring(L, " (%s '%s')", kind, name);
}





static const char *varinfo (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const char *name = 
                    ((void *)0)
                        ;
  const char *kind = 
                    ((void *)0)
                        ;
  if ((!((ci)->callstatus & (1<<1)))) {
    kind = getupvalname(ci, o, &name);
    if (!kind) {
      int reg = instack(ci, o);
      if (reg >= 0)
        kind = getobjname((((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p, currentpc(ci), reg, &name);
    }
  }
  return formatvarinfo(L, kind, name);
}





static void typeerror (lua_State *L, const TValue *o, const char *op,
                          const char *extra) {
  const char *t = luaT_objtypename(L, o);
  luaG_runerror(L, "attempt to %s a %s value%s", op, t, extra);
}






void luaG_typeerror (lua_State *L, const TValue *o, const char *op) {
  typeerror(L, o, op, varinfo(L, o));
}







void luaG_callerror (lua_State *L, const TValue *o) {
  CallInfo *ci = L->ci;
  const char *name = 
                    ((void *)0)
                        ;
  const char *kind = funcnamefromcall(L, ci, &name);
  const char *extra = kind ? formatvarinfo(L, kind, name) : varinfo(L, o);
  typeerror(L, o, "call", extra);
}


void luaG_forerror (lua_State *L, const TValue *o, const char *what) {
  luaG_runerror(L, "bad 'for' %s (number expected, got %s)",
                   what, luaT_objtypename(L, o));
}


void luaG_concaterror (lua_State *L, const TValue *p1, const TValue *p2) {
  if ((((((((p1))->tt_)) & 0x0F)) == (4)) || (((((((p1))->tt_)) & 0x0F)) == (3))) p1 = p2;
  luaG_typeerror(L, p1, "concatenate");
}


void luaG_opinterror (lua_State *L, const TValue *p1,
                         const TValue *p2, const char *msg) {
  if (!(((((((p1))->tt_)) & 0x0F)) == (3)))
    p2 = p1;
  luaG_typeerror(L, p2, msg);
}





void luaG_tointerror (lua_State *L, const TValue *p1, const TValue *p2) {
  lua_Integer temp;
  if (!luaV_tointegerns(p1, &temp, F2Ieq))
    p2 = p1;
  luaG_runerror(L, "number%s has no integer representation", varinfo(L, p2));
}


void luaG_ordererror (lua_State *L, const TValue *p1, const TValue *p2) {
  const char *t1 = luaT_objtypename(L, p1);
  const char *t2 = luaT_objtypename(L, p2);
  if (strcmp(t1, t2) == 0)
    luaG_runerror(L, "attempt to compare two %s values", t1);
  else
    luaG_runerror(L, "attempt to compare %s with %s", t1, t2);
}



const char *luaG_addinfo (lua_State *L, const char *msg, TString *src,
                                        int line) {
  char buff[60];
  if (src)
    luaO_chunkid(buff, ((src)->contents), ((src)->tt == ((4) | ((0) << 4)) ? (src)->shrlen : (src)->u.lnglen));
  else {
    buff[0] = '?'; buff[1] = '\0';
  }
  return luaO_pushfstring(L, "%s:%d: %s", buff, line, msg);
}


void luaG_errormsg (lua_State *L) {
  if (L->errfunc != 0) {
    StkId errfunc = ((StkId)(((char *)((L->stack.p))) + (L->errfunc)));
    ((void)0);
    { TValue *io1=((&(L->top.p)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    { TValue *io1=((&(L->top.p - 1)->val)); const TValue *io2=((&(errfunc)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    L->top.p++;
    luaD_callnoyield(L, L->top.p - 2, 1);
  }
  luaD_throw(L, 2);
}


void luaG_runerror (lua_State *L, const char *fmt, ...) {
  CallInfo *ci = L->ci;
  const char *msg;
  va_list argp;
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
  
 __builtin_va_start(
 argp
 ,
 fmt
 )
                    ;
  msg = luaO_pushvfstring(L, fmt, argp);
  
 __builtin_va_end(
 argp
 )
             ;
  if ((!((ci)->callstatus & (1<<1)))) {
    luaG_addinfo(L, msg, (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p->source, getcurrentline(ci));
    { TValue *io1=((&(L->top.p - 2)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    L->top.p--;
  }
  luaG_errormsg(L);
}
static int changedline (const Proto *p, int oldpc, int newpc) {
  if (p->lineinfo == 
                    ((void *)0)
                        )
    return 0;
  if (newpc - oldpc < 128 / 2) {
    int delta = 0;
    int pc = oldpc;
    for (;;) {
      int lineinfo = p->lineinfo[++pc];
      if (lineinfo == (-0x80))
        break;
      delta += lineinfo;
      if (pc == newpc)
        return (delta != 0);
    }
  }


  return (luaG_getfuncline(p, oldpc) != luaG_getfuncline(p, newpc));
}
int luaG_traceexec (lua_State *L, const Instruction *pc) {
  CallInfo *ci = L->ci;
  lu_byte mask = L->hookmask;
  const Proto *p = (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p;
  int counthook;
  if (!(mask & ((1 << 2) | (1 << 3)))) {
    ci->u.l.trap = 0;
    return 0;
  }
  pc++;
  ci->u.l.savedpc = pc;
  counthook = (--L->hookcount == 0 && (mask & (1 << 3)));
  if (counthook)
    (L->hookcount = L->basehookcount);
  else if (!(mask & (1 << 2)))
    return 1;
  if (ci->callstatus & (1<<6)) {
    ci->callstatus &= ~(1<<6);
    return 1;
  }
  if (!((luaP_opmodes[(((OpCode)(((*(ci->u.l.savedpc - 1))>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))] & (1 << 5)) && ((((int)((((*(ci->u.l.savedpc - 1))>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) == 0))
    L->top.p = ci->top.p;
  if (counthook)
    luaD_hook(L, 3, -1, 0, 0);
  if (mask & (1 << 2)) {

    int oldpc = (L->oldpc < p->sizecode) ? L->oldpc : 0;
    int npci = (((int)(((pc) - (p)->code))) - 1);
    if (npci <= oldpc ||
        changedline(p, oldpc, npci)) {
      int newline = luaG_getfuncline(p, npci);
      luaD_hook(L, 2, newline, 0, 0);
    }
    L->oldpc = npci;
  }
  if (L->status == 1) {
    if (counthook)
      L->hookcount = 1;
    ci->u.l.savedpc--;
    ci->callstatus |= (1<<6);
    luaD_throw(L, 1);
  }
  return 1;
}







typedef long int __jmp_buf[8];
struct __jmp_buf_tag
  {




    __jmp_buf __jmpbuf;
    int __mask_was_saved;
    __sigset_t __saved_mask;
  };

typedef struct __jmp_buf_tag jmp_buf[1];



extern int setjmp (jmp_buf __env) ;




extern int __sigsetjmp (struct __jmp_buf_tag __env[1], int __savemask) ;



extern int _setjmp (struct __jmp_buf_tag __env[1]) ;
extern void longjmp (struct __jmp_buf_tag __env[1], int __val)
     ;





extern void _longjmp (struct __jmp_buf_tag __env[1], int __val)
     ;







typedef struct __jmp_buf_tag sigjmp_buf[1];
extern void siglongjmp (sigjmp_buf __env, int __val)
     ;


struct lua_longjmp {
  struct lua_longjmp *previous;
  jmp_buf b;
   int status;
};


void luaD_seterrorobj (lua_State *L, int errcode, StkId oldtop) {
  switch (errcode) {
    case 4: {
      { TValue *io = ((&(oldtop)->val)); TString *x_ = ((L->l_G)->memerrmsg); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
      break;
    }
    case 5: {
      { TValue *io = ((&(oldtop)->val)); TString *x_ = ((luaS_newlstr(L, "" "error in error handling", (sizeof("error in error handling")/sizeof(char))-1))); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
      break;
    }
    case 0: {
      (((&(oldtop)->val))->tt_=(((0) | ((0) << 4))));
      break;
    }
    default: {
      ((void)0);
      { TValue *io1=((&(oldtop)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      break;
    }
  }
  L->top.p = oldtop + 1;
}


void luaD_throw (lua_State *L, int errcode) {
  if (L->errorJmp) {
    L->errorJmp->status = errcode;
    _longjmp((L->errorJmp)->b, 1);
  }
  else {
    global_State *g = (L->l_G);
    errcode = luaE_resetthread(L, errcode);
    if (g->mainthread->errorJmp) {
      { TValue *io1=((&(g->mainthread->top.p++)->val)); const TValue *io2=((&(L->top.p - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      luaD_throw(g->mainthread, errcode);
    }
    else {
      if (g->panic) {
        ((void) 0);
        g->panic(L);
      }
      abort();
    }
  }
}


int luaD_rawrunprotected (lua_State *L, Pfunc f, void *ud) {
  l_uint32 oldnCcalls = L->nCcalls;
  struct lua_longjmp lj;
  lj.status = 0;
  lj.previous = L->errorJmp;
  L->errorJmp = &lj;
  if (_setjmp((&lj)->b) == 0) { (*f)(L, ud); }

   ;
  L->errorJmp = lj.previous;
  L->nCcalls = oldnCcalls;
  return lj.status;
}
static void relstack (lua_State *L) {
  CallInfo *ci;
  UpVal *up;
  L->top.offset = (((char *)((L->top.p))) - ((char *)((L->stack.p))));
  L->tbclist.offset = (((char *)((L->tbclist.p))) - ((char *)((L->stack.p))));
  for (up = L->openupval; up != 
                               ((void *)0)
                                   ; up = up->u.open.next)
    up->v.offset = (((char *)(((((StkId)((up)->v.p)))))) - ((char *)((L->stack.p))));
  for (ci = L->ci; ci != 
                        ((void *)0)
                            ; ci = ci->previous) {
    ci->top.offset = (((char *)((ci->top.p))) - ((char *)((L->stack.p))));
    ci->func.offset = (((char *)((ci->func.p))) - ((char *)((L->stack.p))));
  }
}





static void correctstack (lua_State *L) {
  CallInfo *ci;
  UpVal *up;
  L->top.p = ((StkId)(((char *)((L->stack.p))) + (L->top.offset)));
  L->tbclist.p = ((StkId)(((char *)((L->stack.p))) + (L->tbclist.offset)));
  for (up = L->openupval; up != 
                               ((void *)0)
                                   ; up = up->u.open.next)
    up->v.p = (&(((StkId)(((char *)((L->stack.p))) + (up->v.offset))))->val);
  for (ci = L->ci; ci != 
                        ((void *)0)
                            ; ci = ci->previous) {
    ci->top.p = ((StkId)(((char *)((L->stack.p))) + (ci->top.offset)));
    ci->func.p = ((StkId)(((char *)((L->stack.p))) + (ci->func.offset)));
    if ((!((ci)->callstatus & (1<<1))))
      ci->u.l.trap = 1;
  }
}
int luaD_reallocstack (lua_State *L, int newsize, int raiseerror) {
  int oldsize = ((int)(((L)->stack_last.p - (L)->stack.p)));
  int i;
  StkId newstack;
  int oldgcstop = (L->l_G)->gcstopem;
  ((void)0);
  relstack(L);
  (L->l_G)->gcstopem = 1;
  newstack = (((StackValue *)(luaM_realloc_(L, L->stack.p, ((size_t)((oldsize + 5))) * sizeof(StackValue), ((size_t)((newsize + 5))) * sizeof(StackValue)))))
                                                                     ;
  (L->l_G)->gcstopem = oldgcstop;
  if ((newstack == 
     ((void *)0)
     )) {
    correctstack(L);
    if (raiseerror)
      luaD_throw(L, 4);
    else return 0;
  }
  L->stack.p = newstack;
  correctstack(L);
  L->stack_last.p = L->stack.p + newsize;
  for (i = oldsize + 5; i < newsize + 5; i++)
    (((&(newstack + i)->val))->tt_=(((0) | ((0) << 4))));
  return 1;
}






int luaD_growstack (lua_State *L, int n, int raiseerror) {
  int size = ((int)(((L)->stack_last.p - (L)->stack.p)));
  if ((size > 1000000)) {



    ((void)0);
    if (raiseerror)
      luaD_throw(L, 5);
    return 0;
  }
  else if (n < 1000000) {
    int newsize = 2 * size;
    int needed = ((int)((L->top.p - L->stack.p))) + n;
    if (newsize > 1000000)
      newsize = 1000000;
    if (newsize < needed)
      newsize = needed;
    if ((newsize <= 1000000))
      return luaD_reallocstack(L, newsize, raiseerror);
  }


  luaD_reallocstack(L, (1000000 + 200), raiseerror);
  if (raiseerror)
    luaG_runerror(L, "stack overflow");
  return 0;
}






static int stackinuse (lua_State *L) {
  CallInfo *ci;
  int res;
  StkId lim = L->top.p;
  for (ci = L->ci; ci != 
                        ((void *)0)
                            ; ci = ci->previous) {
    if (lim < ci->top.p) lim = ci->top.p;
  }
  ((void)0);
  res = ((int)((lim - L->stack.p))) + 1;
  if (res < 20)
    res = 20;
  return res;
}
void luaD_shrinkstack (lua_State *L) {
  int inuse = stackinuse(L);
  int max = (inuse > 1000000 / 3) ? 1000000 : inuse * 3;


  if (inuse <= 1000000 && ((int)(((L)->stack_last.p - (L)->stack.p))) > max) {
    int nsize = (inuse > 1000000 / 2) ? 1000000 : inuse * 2;
    luaD_reallocstack(L, nsize, 0);
  }
  else
    ((void)0);
  luaE_shrinkCI(L);
}


void luaD_inctop (lua_State *L) {
  if ((L->stack_last.p - L->top.p <= (1))) { (void)0; luaD_growstack(L, 1, 1); (void)0; } else { ((void)0); };
  L->top.p++;
}
void luaD_hook (lua_State *L, int event, int line,
                              int ftransfer, int ntransfer) {
  lua_Hook hook = L->hook;
  if (hook && L->allowhook) {
    int mask = (1<<3);
    CallInfo *ci = L->ci;
    ptrdiff_t top = (((char *)((L->top.p))) - ((char *)((L->stack.p))));
    ptrdiff_t ci_top = (((char *)((ci->top.p))) - ((char *)((L->stack.p))));
    lua_Debug ar;
    ar.event = event;
    ar.currentline = line;
    ar.i_ci = ci;
    if (ntransfer != 0) {
      mask |= (1<<8);
      ci->u2.transferinfo.ftransfer = ftransfer;
      ci->u2.transferinfo.ntransfer = ntransfer;
    }
    if ((!((ci)->callstatus & (1<<1))) && L->top.p < ci->top.p)
      L->top.p = ci->top.p;
    if ((L->stack_last.p - L->top.p <= (20))) { (void)0; luaD_growstack(L, 20, 1); (void)0; } else { ((void)0); };
    if (ci->top.p < L->top.p + 20)
      ci->top.p = L->top.p + 20;
    L->allowhook = 0;
    ci->callstatus |= mask;
    ((void) 0);
    (*hook)(L, &ar);
    ((void) 0);
    ((void)0);
    L->allowhook = 1;
    ci->top.p = ((StkId)(((char *)((L->stack.p))) + (ci_top)));
    L->top.p = ((StkId)(((char *)((L->stack.p))) + (top)));
    ci->callstatus &= ~mask;
  }
}







void luaD_hookcall (lua_State *L, CallInfo *ci) {
  L->oldpc = 0;
  if (L->hookmask & (1 << 0)) {
    int event = (ci->callstatus & (1<<5)) ? 4
                                             : 0;
    Proto *p = (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p;
    ci->u.l.savedpc++;
    luaD_hook(L, event, -1, 1, p->numparams);
    ci->u.l.savedpc--;
  }
}







static void rethook (lua_State *L, CallInfo *ci, int nres) {
  if (L->hookmask & (1 << 1)) {
    StkId firstres = L->top.p - nres;
    int delta = 0;
    int ftransfer;
    if ((!((ci)->callstatus & (1<<1)))) {
      Proto *p = (((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p;
      if (p->is_vararg)
        delta = ci->u.l.nextraargs + p->numparams + 1;
    }
    ci->func.p += delta;
    ftransfer = ((unsigned short)(firstres - ci->func.p));
    luaD_hook(L, 1, -1, ftransfer, nres);
    ci->func.p -= delta;
  }
  if ((!((ci = ci->previous)->callstatus & (1<<1))))
    L->oldpc = (((int)(((ci->u.l.savedpc) - ((((&((((union GCUnion *)(((((&((ci)->func.p)->val))->value_).gc))))->cl.l))))->p)->code))) - 1);
}







StkId luaD_tryfuncTM (lua_State *L, StkId func) {
  const TValue *tm;
  StkId p;
  if ((L->stack_last.p - L->top.p <= (1))) { ptrdiff_t t__ = (((char *)((func))) - ((char *)((L->stack.p)))); { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, 1, 1); func = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
  tm = luaT_gettmbyobj(L, (&(func)->val), TM_CALL);
  if (((((((((tm))->tt_)) & 0x0F)) == (0))))
    luaG_callerror(L, (&(func)->val));
  for (p = L->top.p; p > func; p--)
    { TValue *io1=((&(p)->val)); const TValue *io2=((&(p-1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  L->top.p++;
  { TValue *io1=((&(func)->val)); const TValue *io2=(tm); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  return func;
}
static inline void moveresults (lua_State *L, StkId res, int nres, int wanted) {
  StkId firstresult;
  int i;
  switch (wanted) {
    case 0:
      L->top.p = res;
      return;
    case 1:
      if (nres == 0)
        (((&(res)->val))->tt_=(((0) | ((0) << 4))));
      else
        { TValue *io1=((&(res)->val)); const TValue *io2=((&(L->top.p - nres)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      L->top.p = res + 1;
      return;
    case (-1):
      wanted = nres;
      break;
    default:
      if (((wanted) < (-1))) {
        L->ci->callstatus |= (1<<9);
        L->ci->u2.nres = nres;
        res = luaF_close(L, res, (-1), 1);
        L->ci->callstatus &= ~(1<<9);
        if (L->hookmask) {
          ptrdiff_t savedres = (((char *)((res))) - ((char *)((L->stack.p))));
          rethook(L, L->ci, nres);
          res = ((StkId)(((char *)((L->stack.p))) + (savedres)));
        }
        wanted = (-(wanted) - 3);
        if (wanted == (-1))
          wanted = nres;
      }
      break;
  }

  firstresult = L->top.p - nres;
  if (nres > wanted)
    nres = wanted;
  for (i = 0; i < nres; i++)
    { TValue *io1=((&(res + i)->val)); const TValue *io2=((&(firstresult + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  for (; i < wanted; i++)
    (((&(res + i)->val))->tt_=(((0) | ((0) << 4))));
  L->top.p = res + wanted;
}
void luaD_poscall (lua_State *L, CallInfo *ci, int nres) {
  int wanted = ci->nresults;
  if ((L->hookmask && !((wanted) < (-1))))
    rethook(L, ci, nres);

  moveresults(L, ci->func.p, nres, wanted);

  ((void)0)
                                                                          ;
  L->ci = ci->previous;
}






static inline CallInfo *prepCallInfo (lua_State *L, StkId func, int nret,
                                                int mask, StkId top) {
  CallInfo *ci = L->ci = (L->ci->next ? L->ci->next : luaE_extendCI(L));
  ci->func.p = func;
  ci->nresults = nret;
  ci->callstatus = mask;
  ci->top.p = top;
  return ci;
}





static inline int precallC (lua_State *L, StkId func, int nresults,
                                            lua_CFunction f) {
  int n;
  CallInfo *ci;
  if ((L->stack_last.p - L->top.p <= (20))) { ptrdiff_t t__ = (((char *)((func))) - ((char *)((L->stack.p)))); { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, 20, 1); func = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
  L->ci = ci = prepCallInfo(L, func, nresults, (1<<1),
                               L->top.p + 20);
  ((void)0);
  if ((L->hookmask & (1 << 0))) {
    int narg = ((int)((L->top.p - func))) - 1;
    luaD_hook(L, 0, -1, 1, narg);
  }
  ((void) 0);
  n = (*f)(L);
  ((void) 0);
  ((void)L, ((void)0));
  luaD_poscall(L, ci, n);
  return n;
}
int luaD_pretailcall (lua_State *L, CallInfo *ci, StkId func,
                                    int narg1, int delta) {
 retry:
  switch ((((((&(func)->val))->tt_)) & 0x3F)) {
    case ((6) | ((2) << 4)):
      return precallC(L, func, (-1), ((&((((union GCUnion *)(((((&(func)->val))->value_).gc))))->cl.c)))->f);
    case ((6) | ((1) << 4)):
      return precallC(L, func, (-1), ((((&(func)->val))->value_).f));
    case ((6) | ((0) << 4)): {
      Proto *p = ((&((((union GCUnion *)(((((&(func)->val))->value_).gc))))->cl.l)))->p;
      int fsize = p->maxstacksize;
      int nfixparams = p->numparams;
      int i;
      if ((L->stack_last.p - L->top.p <= (fsize - delta))) { ptrdiff_t t__ = (((char *)((func))) - ((char *)((L->stack.p)))); { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, fsize - delta, 1); func = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
      ci->func.p -= delta;
      for (i = 0; i < narg1; i++)
        { TValue *io1=((&(ci->func.p + i)->val)); const TValue *io2=((&(func + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      func = ci->func.p;
      for (; narg1 <= nfixparams; narg1++)
        (((&(func + narg1)->val))->tt_=(((0) | ((0) << 4))));
      ci->top.p = func + 1 + fsize;
      ((void)0);
      ci->u.l.savedpc = p->code;
      ci->callstatus |= (1<<5);
      L->top.p = func + narg1;
      return -1;
    }
    default: {
      func = luaD_tryfuncTM(L, func);

      narg1++;
      goto retry;
    }
  }
}
CallInfo *luaD_precall (lua_State *L, StkId func, int nresults) {
 retry:
  switch ((((((&(func)->val))->tt_)) & 0x3F)) {
    case ((6) | ((2) << 4)):
      precallC(L, func, nresults, ((&((((union GCUnion *)(((((&(func)->val))->value_).gc))))->cl.c)))->f);
      return 
            ((void *)0)
                ;
    case ((6) | ((1) << 4)):
      precallC(L, func, nresults, ((((&(func)->val))->value_).f));
      return 
            ((void *)0)
                ;
    case ((6) | ((0) << 4)): {
      CallInfo *ci;
      Proto *p = ((&((((union GCUnion *)(((((&(func)->val))->value_).gc))))->cl.l)))->p;
      int narg = ((int)((L->top.p - func))) - 1;
      int nfixparams = p->numparams;
      int fsize = p->maxstacksize;
      if ((L->stack_last.p - L->top.p <= (fsize))) { ptrdiff_t t__ = (((char *)((func))) - ((char *)((L->stack.p)))); { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, fsize, 1); func = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
      L->ci = ci = prepCallInfo(L, func, nresults, 0, func + 1 + fsize);
      ci->u.l.savedpc = p->code;
      for (; narg < nfixparams; narg++)
        (((&(L->top.p++)->val))->tt_=(((0) | ((0) << 4))));
      ((void)0);
      return ci;
    }
    default: {
      func = luaD_tryfuncTM(L, func);

      goto retry;
    }
  }
}
static inline void ccall (lua_State *L, StkId func, int nResults, l_uint32 inc) {
  CallInfo *ci;
  L->nCcalls += inc;
  if ((((L)->nCcalls & 0xffff) >= 200)) {
    if ((L->stack_last.p - L->top.p <= (0))) { ptrdiff_t t__ = (((char *)((func))) - ((char *)((L->stack.p)))); luaD_growstack(L, 0, 1); func = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
    luaE_checkcstack(L);
  }
  if ((ci = luaD_precall(L, func, nResults)) != 
                                               ((void *)0)
                                                   ) {
    ci->callstatus = (1<<2);
    luaV_execute(L, ci);
  }
  L->nCcalls -= inc;
}





void luaD_call (lua_State *L, StkId func, int nResults) {
  ccall(L, func, nResults, 1);
}





void luaD_callnoyield (lua_State *L, StkId func, int nResults) {
  ccall(L, func, nResults, (0x10000 | 1));
}
static int finishpcallk (lua_State *L, CallInfo *ci) {
  int status = (((ci)->callstatus >> 10) & 7);
  if ((status == 0))
    status = 1;
  else {
    StkId func = ((StkId)(((char *)((L->stack.p))) + (ci->u2.funcidx)));
    L->allowhook = ((ci->callstatus) & (1<<0));
    func = luaF_close(L, func, status, 1);
    luaD_seterrorobj(L, status, func);
    luaD_shrinkstack(L);
    (((ci)->callstatus = ((ci)->callstatus & ~(7 << 10)) | ((0) << 10)));
  }
  ci->callstatus &= ~(1<<4);
  L->errfunc = ci->u.c.old_errfunc;


  return status;
}
static void finishCcall (lua_State *L, CallInfo *ci) {
  int n;
  if (ci->callstatus & (1<<9)) {
    ((void)0);
    n = ci->u2.nres;

  }
  else {
    int status = 1;

    ((void)0);
    if (ci->callstatus & (1<<4))
      status = finishpcallk(L, ci);
    { if (((-1)) <= (-1) && L->ci->top.p < L->top.p) L->ci->top.p = L->top.p; };
    ((void) 0);
    n = (*ci->u.c.k)(L, status, ci->u.c.ctx);
    ((void) 0);
    ((void)L, ((void)0));
  }
  luaD_poscall(L, ci, n);
}







static void unroll (lua_State *L, void *ud) {
  CallInfo *ci;
  ((void)(ud));
  while ((ci = L->ci) != &L->base_ci) {
    if (!(!((ci)->callstatus & (1<<1))))
      finishCcall(L, ci);
    else {
      luaV_finishOp(L);
      luaV_execute(L, ci);
    }
  }
}






static CallInfo *findpcall (lua_State *L) {
  CallInfo *ci;
  for (ci = L->ci; ci != 
                        ((void *)0)
                            ; ci = ci->previous) {
    if (ci->callstatus & (1<<4))
      return ci;
  }
  return 
        ((void *)0)
            ;
}







static int resume_error (lua_State *L, const char *msg, int narg) {
  L->top.p -= narg;
  { TValue *io = ((&(L->top.p)->val)); TString *x_ = (luaS_new(L, msg)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  ((void) 0);
  return 2;
}
static void resume (lua_State *L, void *ud) {
  int n = *(((int*)(ud)));
  StkId firstArg = L->top.p - n;
  CallInfo *ci = L->ci;
  if (L->status == 0)
    ccall(L, firstArg - 1, (-1), 0);
  else {
    ((void)0);
    L->status = 0;
    if ((!((ci)->callstatus & (1<<1)))) {
      L->top.p = firstArg;
      luaV_execute(L, ci);
    }
    else {
      if (ci->u.c.k != 
                      ((void *)0)
                          ) {
        ((void) 0);
        n = (*ci->u.c.k)(L, 1, ci->u.c.ctx);
        ((void) 0);
        ((void)L, ((void)0));
      }
      luaD_poscall(L, ci, n);
    }
    unroll(L, 
             ((void *)0)
                 );
  }
}
static int precover (lua_State *L, int status) {
  CallInfo *ci;
  while (((status) > 1) && (ci = findpcall(L)) != 
                                                      ((void *)0)
                                                          ) {
    L->ci = ci;
    (((ci)->callstatus = ((ci)->callstatus & ~(7 << 10)) | ((status) << 10)));
    status = luaD_rawrunprotected(L, unroll, 
                                            ((void *)0)
                                                );
  }
  return status;
}


extern int lua_resume (lua_State *L, lua_State *from, int nargs,
                                      int *nresults) {
  int status;
  ((void) 0);
  if (L->status == 0) {
    if (L->ci != &L->base_ci)
      return resume_error(L, "cannot resume non-suspended coroutine", nargs);
    else if (L->top.p - (L->ci->func.p + 1) == nargs)
      return resume_error(L, "cannot resume dead coroutine", nargs);
  }
  else if (L->status != 1)
    return resume_error(L, "cannot resume dead coroutine", nargs);
  L->nCcalls = (from) ? ((from)->nCcalls & 0xffff) : 0;
  if (((L)->nCcalls & 0xffff) >= 200)
    return resume_error(L, "C stack overflow", nargs);
  L->nCcalls++;
  ((void)L);
  ((void)L, ((void)0));
  status = luaD_rawrunprotected(L, resume, &nargs);

  status = precover(L, status);
  if ((!((status) > 1)))
    ((void)0);
  else {
    L->status = ((lu_byte)((status)));
    luaD_seterrorobj(L, status, L->top.p);
    L->ci->top.p = L->top.p;
  }
  *nresults = (status == 1) ? L->ci->u2.nyield
                                    : ((int)((L->top.p - (L->ci->func.p + 1))));
  ((void) 0);
  return status;
}


extern int lua_isyieldable (lua_State *L) {
  return (((L)->nCcalls & 0xffff0000) == 0);
}


extern int lua_yieldk (lua_State *L, int nresults, lua_KContext ctx,
                        lua_KFunction k) {
  CallInfo *ci;
  ((void)L);
  ((void) 0);
  ci = L->ci;
  ((void)L, ((void)0));
  if ((!(((L)->nCcalls & 0xffff0000) == 0))) {
    if (L != (L->l_G)->mainthread)
      luaG_runerror(L, "attempt to yield across a C-call boundary");
    else
      luaG_runerror(L, "attempt to yield from outside a coroutine");
  }
  L->status = 1;
  ci->u2.nyield = nresults;
  if ((!((ci)->callstatus & (1<<1)))) {
    ((void)0);
    ((void)L, ((void)0));
    ((void)L, ((void)0));
  }
  else {
    if ((ci->u.c.k = k) != 
                          ((void *)0)
                              )
      ci->u.c.ctx = ctx;
    luaD_throw(L, 1);
  }
  ((void)0);
  ((void) 0);
  return 0;
}





struct CloseP {
  StkId level;
  int status;
};





static void closepaux (lua_State *L, void *ud) {
  struct CloseP *pcl = ((struct CloseP *)(ud));
  luaF_close(L, pcl->level, pcl->status, 0);
}






int luaD_closeprotected (lua_State *L, ptrdiff_t level, int status) {
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  for (;;) {
    struct CloseP pcl;
    pcl.level = ((StkId)(((char *)((L->stack.p))) + (level))); pcl.status = status;
    status = luaD_rawrunprotected(L, &closepaux, &pcl);
    if ((status == 0))
      return pcl.status;
    else {
      L->ci = old_ci;
      L->allowhook = old_allowhooks;
    }
  }
}







int luaD_pcall (lua_State *L, Pfunc func, void *u,
                ptrdiff_t old_top, ptrdiff_t ef) {
  int status;
  CallInfo *old_ci = L->ci;
  lu_byte old_allowhooks = L->allowhook;
  ptrdiff_t old_errfunc = L->errfunc;
  L->errfunc = ef;
  status = luaD_rawrunprotected(L, func, u);
  if ((status != 0)) {
    L->ci = old_ci;
    L->allowhook = old_allowhooks;
    status = luaD_closeprotected(L, old_top, status);
    luaD_seterrorobj(L, status, ((StkId)(((char *)((L->stack.p))) + (old_top))));
    luaD_shrinkstack(L);
  }
  L->errfunc = old_errfunc;
  return status;
}






struct SParser {
  ZIO *z;
  Mbuffer buff;
  Dyndata dyd;
  const char *mode;
  const char *name;
};


static void checkmode (lua_State *L, const char *mode, const char *x) {
  if (mode && strchr(mode, x[0]) == 
                                   ((void *)0)
                                       ) {
    luaO_pushfstring(L,
       "attempt to load a %s chunk (mode is '%s')", x, mode);
    luaD_throw(L, 3);
  }
}


static void f_parser (lua_State *L, void *ud) {
  LClosure *cl;
  struct SParser *p = ((struct SParser *)(ud));
  int c = (((p->z)->n--)>0 ? ((unsigned char)((*(p->z)->p++))) : luaZ_fill(p->z));
  if (c == "\x1bLua"[0]) {
    checkmode(L, p->mode, "binary");
    cl = luaU_undump(L, p->z, p->name);
  }
  else {
    checkmode(L, p->mode, "text");
    cl = luaY_parser(L, p->z, &p->buff, &p->dyd, p->name, c);
  }
  ((void)0);
  luaF_initupvals(L, cl);
}


int luaD_protectedparser (lua_State *L, ZIO *z, const char *name,
                                        const char *mode) {
  struct SParser p;
  int status;
  ((L)->nCcalls += 0x10000);
  p.z = z; p.name = name; p.mode = mode;
  p.dyd.actvar.arr = 
                    ((void *)0)
                        ; p.dyd.actvar.size = 0;
  p.dyd.gt.arr = 
                ((void *)0)
                    ; p.dyd.gt.size = 0;
  p.dyd.label.arr = 
                   ((void *)0)
                       ; p.dyd.label.size = 0;
  ((&p.buff)->buffer = 
 ((void *)0)
 , (&p.buff)->buffsize = 0);
  status = luaD_pcall(L, f_parser, &p, (((char *)((L->top.p))) - ((char *)((L->stack.p)))), L->errfunc);
  ((&p.buff)->buffer = ((char *)((luaM_saferealloc_(L, ((&p.buff)->buffer), ((&p.buff)->buffsize)*sizeof(char), (0)*sizeof(char))))), (&p.buff)->buffsize = 0);
  luaM_free_(L, (p.dyd.actvar.arr), (p.dyd.actvar.size)*sizeof(*(p.dyd.actvar.arr)));
  luaM_free_(L, (p.dyd.gt.arr), (p.dyd.gt.size)*sizeof(*(p.dyd.gt.arr)));
  luaM_free_(L, (p.dyd.label.arr), (p.dyd.label.size)*sizeof(*(p.dyd.label.arr)));
  ((L)->nCcalls -= 0x10000);
  return status;
}
typedef struct {
  lua_State *L;
  lua_Writer writer;
  void *data;
  int strip;
  int status;
} DumpState;
static void dumpBlock (DumpState *D, const void *b, size_t size) {
  if (D->status == 0 && size > 0) {
    ((void) 0);
    D->status = (*D->writer)(D->L, b, size, D->data);
    ((void) 0);
  }
}





static void dumpByte (DumpState *D, int y) {
  lu_byte x = (lu_byte)y;
  dumpBlock(D,&x,(1)*sizeof((&x)[0]));
}
static void dumpSize (DumpState *D, size_t x) {
  lu_byte buff[10];
  int n = 0;
  do {
    buff[((sizeof(size_t) * 8 
        + 6) / 7) - (++n)] = x & 0x7f;
    x >>= 7;
  } while (x != 0);
  buff[9] |= 0x80;
  dumpBlock(D,buff + ((sizeof(size_t) * 8 
 + 6) / 7) - n,(n)*sizeof((buff + ((sizeof(size_t) * 8 
 + 6) / 7) - n)[0]));
}


static void dumpInt (DumpState *D, int x) {
  dumpSize(D, x);
}


static void dumpNumber (DumpState *D, lua_Number x) {
  dumpBlock(D,&x,(1)*sizeof((&x)[0]));
}


static void dumpInteger (DumpState *D, lua_Integer x) {
  dumpBlock(D,&x,(1)*sizeof((&x)[0]));
}


static void dumpString (DumpState *D, const TString *s) {
  if (s == 
          ((void *)0)
              )
    dumpSize(D, 0);
  else {
    size_t size = ((s)->tt == ((4) | ((0) << 4)) ? (s)->shrlen : (s)->u.lnglen);
    const char *str = ((s)->contents);
    dumpSize(D, size + 1);
    dumpBlock(D,str,(size)*sizeof((str)[0]));
  }
}


static void dumpCode (DumpState *D, const Proto *f) {
  dumpInt(D, f->sizecode);
  dumpBlock(D,f->code,(f->sizecode)*sizeof((f->code)[0]));
}


static void dumpFunction(DumpState *D, const Proto *f, TString *psource);

static void dumpConstants (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizek;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    const TValue *o = &f->k[i];
    int tt = ((((o)->tt_)) & 0x3F);
    dumpByte(D, tt);
    switch (tt) {
      case ((3) | ((1) << 4)):
        dumpNumber(D, (((o)->value_).n));
        break;
      case ((3) | ((0) << 4)):
        dumpInteger(D, (((o)->value_).i));
        break;
      case ((4) | ((0) << 4)):
      case ((4) | ((1) << 4)):
        dumpString(D, ((&((((union GCUnion *)((((o)->value_).gc))))->ts))));
        break;
      default:
        ((void)0);
    }
  }
}


static void dumpProtos (DumpState *D, const Proto *f) {
  int i;
  int n = f->sizep;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpFunction(D, f->p[i], f->source);
}


static void dumpUpvalues (DumpState *D, const Proto *f) {
  int i, n = f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpByte(D, f->upvalues[i].instack);
    dumpByte(D, f->upvalues[i].idx);
    dumpByte(D, f->upvalues[i].kind);
  }
}


static void dumpDebug (DumpState *D, const Proto *f) {
  int i, n;
  n = (D->strip) ? 0 : f->sizelineinfo;
  dumpInt(D, n);
  dumpBlock(D,f->lineinfo,(n)*sizeof((f->lineinfo)[0]));
  n = (D->strip) ? 0 : f->sizeabslineinfo;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpInt(D, f->abslineinfo[i].pc);
    dumpInt(D, f->abslineinfo[i].line);
  }
  n = (D->strip) ? 0 : f->sizelocvars;
  dumpInt(D, n);
  for (i = 0; i < n; i++) {
    dumpString(D, f->locvars[i].varname);
    dumpInt(D, f->locvars[i].startpc);
    dumpInt(D, f->locvars[i].endpc);
  }
  n = (D->strip) ? 0 : f->sizeupvalues;
  dumpInt(D, n);
  for (i = 0; i < n; i++)
    dumpString(D, f->upvalues[i].name);
}


static void dumpFunction (DumpState *D, const Proto *f, TString *psource) {
  if (D->strip || f->source == psource)
    dumpString(D, 
                 ((void *)0)
                     );
  else
    dumpString(D, f->source);
  dumpInt(D, f->linedefined);
  dumpInt(D, f->lastlinedefined);
  dumpByte(D, f->numparams);
  dumpByte(D, f->is_vararg);
  dumpByte(D, f->maxstacksize);
  dumpCode(D, f);
  dumpConstants(D, f);
  dumpUpvalues(D, f);
  dumpProtos(D, f);
  dumpDebug(D, f);
}


static void dumpHeader (DumpState *D) {
  dumpBlock(D,"\x1bLua",sizeof("\x1bLua") - sizeof(char));
  dumpByte(D, (("5"[0]-'0')*16+("4"[0]-'0')));
  dumpByte(D, 0);
  dumpBlock(D,"\x19\x93\r\n\x1a\n",sizeof("\x19\x93\r\n\x1a\n") - sizeof(char));
  dumpByte(D, sizeof(Instruction));
  dumpByte(D, sizeof(lua_Integer));
  dumpByte(D, sizeof(lua_Number));
  dumpInteger(D, 0x5678);
  dumpNumber(D, ((lua_Number)((370.5))));
}





int luaU_dump(lua_State *L, const Proto *f, lua_Writer w, void *data,
              int strip) {
  DumpState D;
  D.L = L;
  D.writer = w;
  D.data = data;
  D.strip = strip;
  D.status = 0;
  dumpHeader(&D);
  dumpByte(&D, f->sizeupvalues);
  dumpFunction(&D, f, 
                     ((void *)0)
                         );
  return D.status;
}
CClosure *luaF_newCclosure (lua_State *L, int nupvals) {
  GCObject *o = luaC_newobj(L, ((6) | ((2) << 4)), (((int)((
                                        ((long)&((CClosure
                                        *)0)->upvalue
                                        )
                                        ))) + ((int)((sizeof(TValue)))) * (nupvals)));
  CClosure *c = (&((((union GCUnion *)((o))))->cl.c));
  c->nupvalues = ((lu_byte)((nupvals)));
  return c;
}


LClosure *luaF_newLclosure (lua_State *L, int nupvals) {
  GCObject *o = luaC_newobj(L, ((6) | ((0) << 4)), (((int)((
                                        ((long)&((LClosure
                                        *)0)->upvals
                                        )
                                        ))) + ((int)((sizeof(TValue *)))) * (nupvals)));
  LClosure *c = (&((((union GCUnion *)((o))))->cl.l));
  c->p = 
        ((void *)0)
            ;
  c->nupvalues = ((lu_byte)((nupvals)));
  while (nupvals--) c->upvals[nupvals] = 
                                        ((void *)0)
                                            ;
  return c;
}





void luaF_initupvals (lua_State *L, LClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++) {
    GCObject *o = luaC_newobj(L, ((9) | ((0) << 4)), sizeof(UpVal));
    UpVal *uv = (&((((union GCUnion *)((o))))->upv));
    uv->v.p = &uv->u.value;
    ((uv->v.p)->tt_=(((0) | ((0) << 4))));
    cl->upvals[i] = uv;
    ( ((((cl)->marked) & ((1<<(5)))) && (((uv)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((cl)))->gc)),(&(((union GCUnion *)((uv)))->gc))) : ((void)((0))));
  }
}






static UpVal *newupval (lua_State *L, StkId level, UpVal **prev) {
  GCObject *o = luaC_newobj(L, ((9) | ((0) << 4)), sizeof(UpVal));
  UpVal *uv = (&((((union GCUnion *)((o))))->upv));
  UpVal *next = *prev;
  uv->v.p = (&(level)->val);
  uv->u.open.next = next;
  uv->u.open.previous = prev;
  if (next)
    next->u.open.previous = &uv->u.open.next;
  *prev = uv;
  if (!(L->twups != L)) {
    L->twups = (L->l_G)->twups;
    (L->l_G)->twups = L;
  }
  return uv;
}






UpVal *luaF_findupval (lua_State *L, StkId level) {
  UpVal **pp = &L->openupval;
  UpVal *p;
  ((void)0);
  while ((p = *pp) != 
                     ((void *)0) 
                          && (((StkId)((p)->v.p))) >= level) {
    ((void)0);
    if ((((StkId)((p)->v.p))) == level)
      return p;
    pp = &p->u.open.next;
  }

  return newupval(L, level, pp);
}







static void callclosemethod (lua_State *L, TValue *obj, TValue *err, int yy) {
  StkId top = L->top.p;
  const TValue *tm = luaT_gettmbyobj(L, obj, TM_CLOSE);
  { TValue *io1=((&(top)->val)); const TValue *io2=(tm); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(top + 1)->val)); const TValue *io2=(obj); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(top + 2)->val)); const TValue *io2=(err); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  L->top.p = top + 3;
  if (yy)
    luaD_call(L, top, 0);
  else
    luaD_callnoyield(L, top, 0);
}






static void checkclosemth (lua_State *L, StkId level) {
  const TValue *tm = luaT_gettmbyobj(L, (&(level)->val), TM_CLOSE);
  if ((((((((tm))->tt_)) & 0x0F)) == (0))) {
    int idx = ((int)((level - L->ci->func.p)));
    const char *vname = luaG_findlocal(L, L->ci, idx, 
                                                     ((void *)0)
                                                         );
    if (vname == 
                ((void *)0)
                    ) vname = "?";
    luaG_runerror(L, "variable '%s' got a non-closable value", vname);
  }
}
static void prepcallclosemth (lua_State *L, StkId level, int status, int yy) {
  TValue *uv = (&(level)->val);
  TValue *errobj;
  if (status == (-1))
    errobj = &(L->l_G)->nilvalue;
  else {
    errobj = (&(level + 1)->val);
    luaD_seterrorobj(L, status, level + 1);
  }
  callclosemethod(L, uv, errobj, yy);
}
void luaF_newtbcupval (lua_State *L, StkId level) {
  ((void)0);
  if (((((((&(level)->val)))->tt_) == (((1) | ((0) << 4)))) || ((((((((&(level)->val)))->tt_)) & 0x0F)) == (0))))
    return;
  checkclosemth(L, level);
  while (((unsigned int)((level - L->tbclist.p))) > ((256ul << ((sizeof(L->stack.p->tbclist.delta) - 1) * 8)) - 1)) {
    L->tbclist.p += ((256ul << ((sizeof(L->stack.p->tbclist.delta) - 1) * 8)) - 1);
    L->tbclist.p->tbclist.delta = 0;
  }
  level->tbclist.delta = ((unsigned short)(level - L->tbclist.p));
  L->tbclist.p = level;
}


void luaF_unlinkupval (UpVal *uv) {
  ((void)0);
  *uv->u.open.previous = uv->u.open.next;
  if (uv->u.open.next)
    uv->u.open.next->u.open.previous = uv->u.open.previous;
}





void luaF_closeupval (lua_State *L, StkId level) {
  UpVal *uv;
  StkId upl;
  while ((uv = L->openupval) != 
                               ((void *)0) 
                                    && (upl = (((StkId)((uv)->v.p)))) >= level) {
    TValue *slot = &uv->u.value;
    ((void)0);
    luaF_unlinkupval(uv);
    { TValue *io1=(slot); const TValue *io2=(uv->v.p); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    uv->v.p = slot;
    if (!(((uv)->marked) & (((1<<(3)) | (1<<(4)))))) {
      ((((uv)->marked) |= ((1<<(5)))));
      ( (((slot)->tt_) & (1 << 6)) ? ( ((((uv)->marked) & ((1<<(5)))) && ((((((slot)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((uv)))->gc)),(&(((union GCUnion *)(((((slot)->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));
    }
  }
}





static void poptbclist (lua_State *L) {
  StkId tbc = L->tbclist.p;
  ((void)0);
  tbc -= tbc->tbclist.delta;
  while (tbc > L->stack.p && tbc->tbclist.delta == 0)
    tbc -= ((256ul << ((sizeof(L->stack.p->tbclist.delta) - 1) * 8)) - 1);
  L->tbclist.p = tbc;
}






StkId luaF_close (lua_State *L, StkId level, int status, int yy) {
  ptrdiff_t levelrel = (((char *)((level))) - ((char *)((L->stack.p))));
  luaF_closeupval(L, level);
  while (L->tbclist.p >= level) {
    StkId tbc = L->tbclist.p;
    poptbclist(L);
    prepcallclosemth(L, tbc, status, yy);
    level = ((StkId)(((char *)((L->stack.p))) + (levelrel)));
  }
  return level;
}


Proto *luaF_newproto (lua_State *L) {
  GCObject *o = luaC_newobj(L, (((9 +1)) | ((0) << 4)), sizeof(Proto));
  Proto *f = (&((((union GCUnion *)((o))))->p));
  f->k = 
        ((void *)0)
            ;
  f->sizek = 0;
  f->p = 
        ((void *)0)
            ;
  f->sizep = 0;
  f->code = 
           ((void *)0)
               ;
  f->sizecode = 0;
  f->lineinfo = 
               ((void *)0)
                   ;
  f->sizelineinfo = 0;
  f->abslineinfo = 
                  ((void *)0)
                      ;
  f->sizeabslineinfo = 0;
  f->upvalues = 
               ((void *)0)
                   ;
  f->sizeupvalues = 0;
  f->numparams = 0;
  f->is_vararg = 0;
  f->maxstacksize = 0;
  f->locvars = 
              ((void *)0)
                  ;
  f->sizelocvars = 0;
  f->linedefined = 0;
  f->lastlinedefined = 0;
  f->source = 
             ((void *)0)
                 ;
  return f;
}


void luaF_freeproto (lua_State *L, Proto *f) {
  luaM_free_(L, (f->code), (f->sizecode)*sizeof(*(f->code)));
  luaM_free_(L, (f->p), (f->sizep)*sizeof(*(f->p)));
  luaM_free_(L, (f->k), (f->sizek)*sizeof(*(f->k)));
  luaM_free_(L, (f->lineinfo), (f->sizelineinfo)*sizeof(*(f->lineinfo)));
  luaM_free_(L, (f->abslineinfo), (f->sizeabslineinfo)*sizeof(*(f->abslineinfo)));
  luaM_free_(L, (f->locvars), (f->sizelocvars)*sizeof(*(f->locvars)));
  luaM_free_(L, (f->upvalues), (f->sizeupvalues)*sizeof(*(f->upvalues)));
  luaM_free_(L, (f), sizeof(*(f)));
}






const char *luaF_getlocalname (const Proto *f, int local_number, int pc) {
  int i;
  for (i = 0; i<f->sizelocvars && f->locvars[i].startpc <= pc; i++) {
    if (pc < f->locvars[i].endpc) {
      local_number--;
      if (local_number == 0)
        return ((f->locvars[i].varname)->contents);
    }
  }
  return 
        ((void *)0)
            ;
}















typedef struct
{
  int __count;
  union
  {
    unsigned int __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;




typedef struct _G_fpos_t
{
  __off_t __pos;
  __mbstate_t __state;
} __fpos_t;
typedef struct _G_fpos64_t
{
  __off64_t __pos;
  __mbstate_t __state;
} __fpos64_t;



struct _IO_FILE;
typedef struct _IO_FILE __FILE;



struct _IO_FILE;


typedef struct _IO_FILE FILE;
struct _IO_FILE;
struct _IO_marker;
struct _IO_codecvt;
struct _IO_wide_data;




typedef void _IO_lock_t;





struct _IO_FILE
{
  int _flags;


  char *_IO_read_ptr;
  char *_IO_read_end;
  char *_IO_read_base;
  char *_IO_write_base;
  char *_IO_write_ptr;
  char *_IO_write_end;
  char *_IO_buf_base;
  char *_IO_buf_end;


  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;
  int _flags2;
  __off_t _old_offset;


  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];

  _IO_lock_t *_lock;







  __off64_t _offset;

  struct _IO_codecvt *_codecvt;
  struct _IO_wide_data *_wide_data;
  struct _IO_FILE *_freeres_list;
  void *_freeres_buf;
  size_t __pad5;
  int _mode;

  char _unused2[20];
};
typedef __fpos64_t fpos_t;


typedef __fpos64_t fpos64_t;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;






extern int remove (const char *__filename) ;

extern int rename (const char *__old, const char *__new) ;
extern int fclose (FILE *__stream) ;
extern FILE *tmpfile64 (void)
   ;



extern char *tmpnam (char[20]) ;
extern char *tempnam (const char *__dir, const char *__pfx)
   ;






extern int fflush (FILE *__stream);
extern FILE *fopen64 (const char * __filename,
        const char * __modes)
  ;
extern FILE *freopen64 (const char * __filename,
   const char * __modes,
   FILE * __stream) ;




extern FILE *fdopen (int __fd, const char *__modes)
  ;
extern void setbuf (FILE * __stream, char * __buf)
  ;



extern int setvbuf (FILE * __stream, char * __buf,
      int __modes, size_t __n) ;
extern int fprintf (FILE * __stream,
      const char * __format, ...) ;




extern int printf (const char * __format, ...);

extern int sprintf (char * __s,
      const char * __format, ...) ;





extern int vfprintf (FILE * __s, const char * __format,
       __gnuc_va_list __arg) ;




extern int vprintf (const char * __format, __gnuc_va_list __arg);

extern int vsprintf (char * __s, const char * __format,
       __gnuc_va_list __arg) ;



extern int snprintf (char * __s, size_t __maxlen,
       const char * __format, ...)
     ;

extern int vsnprintf (char * __s, size_t __maxlen,
        const char * __format, __gnuc_va_list __arg)
     ;
extern int fscanf (FILE * __stream,
     const char * __format, ...) ;




extern int scanf (const char * __format, ...) ;

extern int sscanf (const char * __s,
     const char * __format, ...) ;
extern int __isoc99_fscanf (FILE * __stream,
       const char * __format, ...)
  ;
extern int __isoc99_scanf (const char * __format, ...) ;
extern int __isoc99_sscanf (const char * __s,
       const char * __format, ...) ;
extern int vfscanf (FILE * __s, const char * __format,
      __gnuc_va_list __arg)
     ;





extern int vscanf (const char * __format, __gnuc_va_list __arg)
     ;


extern int vsscanf (const char * __s,
      const char * __format, __gnuc_va_list __arg)
     ;
extern int __isoc99_vfscanf (FILE * __s,
        const char * __format,
        __gnuc_va_list __arg) ;
extern int __isoc99_vscanf (const char * __format,
       __gnuc_va_list __arg) ;
extern int __isoc99_vsscanf (const char * __s,
        const char * __format,
        __gnuc_va_list __arg) ;
extern int fgetc (FILE *__stream) ;
extern int getc (FILE *__stream) ;





extern int getchar (void);






extern int getc_unlocked (FILE *__stream) ;
extern int getchar_unlocked (void);
extern int fputc (int __c, FILE *__stream) ;
extern int putc (int __c, FILE *__stream) ;





extern int putchar (int __c);
extern int putc_unlocked (int __c, FILE *__stream) ;
extern int putchar_unlocked (int __c);
extern char *fgets (char * __s, int __n, FILE * __stream)
     ;
extern int fputs (const char * __s, FILE * __stream)
  ;





extern int puts (const char *__s);






extern int ungetc (int __c, FILE *__stream) ;






extern size_t fread (void * __ptr, size_t __size,
       size_t __n, FILE * __stream)
  ;




extern size_t fwrite (const void * __ptr, size_t __size,
        size_t __n, FILE * __s) ;
extern int fseek (FILE *__stream, long int __off, int __whence)
  ;




extern long int ftell (FILE *__stream) ;




extern void rewind (FILE *__stream) ;
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence)
  ;
extern __off64_t ftello64 (FILE *__stream) ;
extern int fgetpos64 (FILE * __stream, fpos64_t * __pos)
  ;
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos) ;



extern void clearerr (FILE *__stream) ;

extern int feof (FILE *__stream) ;

extern int ferror (FILE *__stream) ;
extern void perror (const char *__s) ;




extern int fileno (FILE *__stream) ;
extern int pclose (FILE *__stream) ;





extern FILE *popen (const char *__command, const char *__modes)
  ;






extern char *ctermid (char *__s)
  ;
extern void flockfile (FILE *__stream) ;



extern int ftrylockfile (FILE *__stream) ;


extern void funlockfile (FILE *__stream) ;
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);


static void reallymarkobject (global_State *g, GCObject *o);
static lu_mem atomic (lua_State *L);
static void entersweep (lua_State *L);
static GCObject **getgclist (GCObject *o) {
  switch (o->tt) {
    case ((5) | ((0) << 4)): return &(&((((union GCUnion *)((o))))->h))->gclist;
    case ((6) | ((0) << 4)): return &(&((((union GCUnion *)((o))))->cl.l))->gclist;
    case ((6) | ((2) << 4)): return &(&((((union GCUnion *)((o))))->cl.c))->gclist;
    case ((8) | ((0) << 4)): return &(&((((union GCUnion *)((o))))->th))->gclist;
    case (((9 +1)) | ((0) << 4)): return &(&((((union GCUnion *)((o))))->p))->gclist;
    case ((7) | ((0) << 4)): {
      Udata *u = (&((((union GCUnion *)((o))))->u));
      ((void)0);
      return &u->gclist;
    }
    default: ((void)0); return 0;
  }
}
static void linkgclist_ (GCObject *o, GCObject **pnext, GCObject **list) {
  ((void)0);
  *pnext = *list;
  *list = o;
  ((o->marked) &= ((lu_byte)((~(((1<<(5)) | ((1<<(3)) | (1<<(4)))))))));
}
static void clearkey (Node *n) {
  ((void)0);
  if ((((n)->u.key_tt) & (1 << 6)))
    (((n)->u.key_tt) = (9 +2));
}
static int iscleared (global_State *g, const GCObject *o) {
  if (o == 
          ((void *)0)
              ) return 0;
  else if (((o->tt) & 0x0F) == 4) {
    { if ((((o)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((o)))->gc))); };
    return 0;
  }
  else return (((o)->marked) & (((1<<(3)) | (1<<(4)))));
}
void luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v) {
  global_State *g = (L->l_G);
  ((void)0);
  if (((g)->gcstate <= 2)) {
    reallymarkobject(g, v);
    if ((((o)->marked & 7) > 1)) {
      ((void)0);
      ((v)->marked = ((lu_byte)((((v)->marked & (~7)) | 2))));
    }
  }
  else {
    ((void)0);
    if (g->gckind == 0)
      (o->marked = ((lu_byte)(((o->marked & ~((1<<(5)) | ((1<<(3)) | (1<<(4))))) | ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))))))));
  }
}






void luaC_barrierback_ (lua_State *L, GCObject *o) {
  global_State *g = (L->l_G);
  ((void)0);
  ((void)0);
  if (((o)->marked & 7) == 6)
    ((o->marked) &= ((lu_byte)((~(((1<<(5)) | ((1<<(3)) | (1<<(4)))))))));
  else
    linkgclist_((&(((union GCUnion *)((o)))->gc)), getgclist(o), &(g->grayagain));
  if ((((o)->marked & 7) > 1))
    ((o)->marked = ((lu_byte)((((o)->marked & (~7)) | 5))));
}


void luaC_fix (lua_State *L, GCObject *o) {
  global_State *g = (L->l_G);
  ((void)0);
  ((o->marked) &= ((lu_byte)((~(((1<<(5)) | ((1<<(3)) | (1<<(4)))))))));
  ((o)->marked = ((lu_byte)((((o)->marked & (~7)) | 4))));
  g->allgc = o->next;
  o->next = g->fixedgc;
  g->fixedgc = o;
}






GCObject *luaC_newobjdt (lua_State *L, int tt, size_t sz, size_t offset) {
  global_State *g = (L->l_G);
  char *p = ((char *)((luaM_malloc_(L, (sz), ((tt) & 0x0F)))));
  GCObject *o = ((GCObject *)(p + offset));
  o->marked = ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))));
  o->tt = tt;
  o->next = g->allgc;
  g->allgc = o;
  return o;
}


GCObject *luaC_newobj (lua_State *L, int tt, size_t sz) {
  return luaC_newobjdt(L, tt, sz, 0);
}
static void reallymarkobject (global_State *g, GCObject *o) {
  switch (o->tt) {
    case ((4) | ((0) << 4)):
    case ((4) | ((1) << 4)): {
      (o->marked = ((lu_byte)(((o->marked & ~((1<<(3)) | (1<<(4)))) | (1<<(5))))));
      break;
    }
    case ((9) | ((0) << 4)): {
      UpVal *uv = (&((((union GCUnion *)((o))))->upv));
      if (((uv)->v.p != &(uv)->u.value))
        ((uv->marked) &= ((lu_byte)((~(((1<<(5)) | ((1<<(3)) | (1<<(4)))))))));
      else
        (uv->marked = ((lu_byte)(((uv->marked & ~((1<<(3)) | (1<<(4)))) | (1<<(5))))));
      { ((void)g->mainthread, ((void)0)); if (((((uv->v.p)->tt_) & (1 << 6)) && ((((((uv->v.p)->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((uv->v.p)->value_).gc)); };
      break;
    }
    case ((7) | ((0) << 4)): {
      Udata *u = (&((((union GCUnion *)((o))))->u));
      if (u->nuvalue == 0) {
        { if (u->metatable) { if ((((u->metatable)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((u->metatable)))->gc))); }; };
        (u->marked = ((lu_byte)(((u->marked & ~((1<<(3)) | (1<<(4)))) | (1<<(5))))));
        break;
      }

    }
    case ((6) | ((0) << 4)): case ((6) | ((2) << 4)): case ((5) | ((0) << 4)):
    case ((8) | ((0) << 4)): case (((9 +1)) | ((0) << 4)): {
      linkgclist_((&(((union GCUnion *)((o)))->gc)), getgclist(o), &(g->gray));
      break;
    }
    default: ((void)0); break;
  }
}





static void markmt (global_State *g) {
  int i;
  for (i=0; i < 9; i++)
    { if (g->mt[i]) { if ((((g->mt[i])->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((g->mt[i])))->gc))); }; };
}





static lu_mem markbeingfnz (global_State *g) {
  GCObject *o;
  lu_mem count = 0;
  for (o = g->tobefnz; o != 
                           ((void *)0)
                               ; o = o->next) {
    count++;
    { if ((((o)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((o)))->gc))); };
  }
  return count;
}
static int remarkupvals (global_State *g) {
  lua_State *thread;
  lua_State **p = &g->twups;
  int work = 0;
  while ((thread = *p) != 
                         ((void *)0)
                             ) {
    work++;
    if (!(((thread)->marked) & (((1<<(3)) | (1<<(4))))) && thread->openupval != 
                                                ((void *)0)
                                                    )
      p = &thread->twups;
    else {
      UpVal *uv;
      ((void)0);
      *p = thread->twups;
      thread->twups = thread;
      for (uv = thread->openupval; uv != 
                                        ((void *)0)
                                            ; uv = uv->u.open.next) {
        ((void)0);
        work++;
        if (!(((uv)->marked) & (((1<<(3)) | (1<<(4)))))) {
          ((void)0);
          { ((void)g->mainthread, ((void)0)); if (((((uv->v.p)->tt_) & (1 << 6)) && ((((((uv->v.p)->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((uv->v.p)->value_).gc)); };
        }
      }
    }
  }
  return work;
}


static void cleargraylists (global_State *g) {
  g->gray = g->grayagain = 
                          ((void *)0)
                              ;
  g->weak = g->allweak = g->ephemeron = 
                                       ((void *)0)
                                           ;
}





static void restartcollection (global_State *g) {
  cleargraylists(g);
  { if ((((g->mainthread)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((g->mainthread)))->gc))); };
  { ((void)g->mainthread, ((void)0)); if (((((&g->l_registry)->tt_) & (1 << 6)) && ((((((&g->l_registry)->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&g->l_registry)->value_).gc)); };
  markmt(g);
  markbeingfnz(g);
}
static void genlink (global_State *g, GCObject *o) {
  ((void)0);
  if (((o)->marked & 7) == 5) {
    linkgclist_((&(((union GCUnion *)((o)))->gc)), getgclist(o), &(g->grayagain));
  }
  else if (((o)->marked & 7) == 6)
    ((o)->marked ^= ((6)^(4)));
}
static void traverseweakvalue (global_State *g, Table *h) {
  Node *n, *limit = (&(h)->node[((size_t)((((1<<((h)->lsizenode))))))]);


  int hasclears = (h->alimit > 0);
  for (n = (&(h)->node[0]); n < limit; n++) {
    if (((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0)))
      clearkey(n);
    else {
      ((void)0);
      { if ((((n)->u.key_tt) & (1 << 6)) && ((((((n)->u.key_val).gc))->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g,(((n)->u.key_val).gc)); };
      if (!hasclears && iscleared(g, (((((&(n)->i_val))->tt_) & (1 << 6)) ? ((((&(n)->i_val))->value_).gc) : 
                                    ((void *)0)
                                    )))
        hasclears = 1;
    }
  }
  if (g->gcstate == 2 && hasclears)
    linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->weak));
  else
    linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->grayagain));
}
static int traverseephemeron (global_State *g, Table *h, int inv) {
  int marked = 0;
  int hasclears = 0;
  int hasww = 0;
  unsigned int i;
  unsigned int asize = luaH_realasize(h);
  unsigned int nsize = ((1<<((h)->lsizenode)));

  for (i = 0; i < asize; i++) {
    if (((((&h->array[i])->tt_) & (1 << 6)) && ((((((&h->array[i])->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) {
      marked = 1;
      reallymarkobject(g, (((&h->array[i])->value_).gc));
    }
  }


  for (i = 0; i < nsize; i++) {
    Node *n = inv ? (&(h)->node[nsize - 1 - i]) : (&(h)->node[i]);
    if (((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0)))
      clearkey(n);
    else if (iscleared(g, ((((n)->u.key_tt) & (1 << 6)) ? (((n)->u.key_val).gc) : 
                         ((void *)0)
                         ))) {
      hasclears = 1;
      if ((((((&(n)->i_val))->tt_) & (1 << 6)) && (((((((&(n)->i_val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))))
        hasww = 1;
    }
    else if ((((((&(n)->i_val))->tt_) & (1 << 6)) && (((((((&(n)->i_val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) {
      marked = 1;
      reallymarkobject(g, ((((&(n)->i_val))->value_).gc));
    }
  }

  if (g->gcstate == 0)
    linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->grayagain));
  else if (hasww)
    linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->ephemeron));
  else if (hasclears)
    linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->allweak));
  else
    genlink(g, (&(((union GCUnion *)((h)))->gc)));
  return marked;
}


static void traversestrongtable (global_State *g, Table *h) {
  Node *n, *limit = (&(h)->node[((size_t)((((1<<((h)->lsizenode))))))]);
  unsigned int i;
  unsigned int asize = luaH_realasize(h);
  for (i = 0; i < asize; i++)
    { ((void)g->mainthread, ((void)0)); if (((((&h->array[i])->tt_) & (1 << 6)) && ((((((&h->array[i])->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&h->array[i])->value_).gc)); };
  for (n = (&(h)->node[0]); n < limit; n++) {
    if (((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0)))
      clearkey(n);
    else {
      ((void)0);
      { if ((((n)->u.key_tt) & (1 << 6)) && ((((((n)->u.key_val).gc))->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g,(((n)->u.key_val).gc)); };
      { ((void)g->mainthread, ((void)0)); if ((((((&(n)->i_val))->tt_) & (1 << 6)) && (((((((&(n)->i_val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,((((&(n)->i_val))->value_).gc)); };
    }
  }
  genlink(g, (&(((union GCUnion *)((h)))->gc)));
}


static lu_mem traversetable (global_State *g, Table *h) {
  const char *weakkey, *weakvalue;
  const TValue *mode = ((h->metatable) == 
                      ((void *)0) 
                      ? 
                      ((void *)0) 
                      : ((h->metatable)->flags & (1u<<(TM_MODE))) ? 
                      ((void *)0) 
                      : luaT_gettm(h->metatable, TM_MODE, (g)->tmname[TM_MODE]));
  { if (h->metatable) { if ((((h->metatable)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((h->metatable)))->gc))); }; };
  if (mode && (((((((mode))->tt_)) & 0x0F)) == (4)) &&
      (((void)((weakkey = strchr(((((&((((union GCUnion *)((((mode)->value_).gc))))->ts))))->contents), 'k')))),
       ((void)((weakvalue = strchr(((((&((((union GCUnion *)((((mode)->value_).gc))))->ts))))->contents), 'v')))),
       (weakkey || weakvalue))) {
    if (!weakkey)
      traverseweakvalue(g, h);
    else if (!weakvalue)
      traverseephemeron(g, h, 0);
    else
      linkgclist_((&(((union GCUnion *)((h)))->gc)), &(h)->gclist, &(g->allweak));
  }
  else
    traversestrongtable(g, h);
  return 1 + h->alimit + 2 * (((h)->lastfree == 
                            ((void *)0)
                            ) ? 0 : ((1<<((h)->lsizenode))));
}


static int traverseudata (global_State *g, Udata *u) {
  int i;
  { if (u->metatable) { if ((((u->metatable)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((u->metatable)))->gc))); }; };
  for (i = 0; i < u->nuvalue; i++)
    { ((void)g->mainthread, ((void)0)); if (((((&u->uv[i].uv)->tt_) & (1 << 6)) && ((((((&u->uv[i].uv)->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&u->uv[i].uv)->value_).gc)); };
  genlink(g, (&(((union GCUnion *)((u)))->gc)));
  return 1 + u->nuvalue;
}







static int traverseproto (global_State *g, Proto *f) {
  int i;
  { if (f->source) { if ((((f->source)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((f->source)))->gc))); }; };
  for (i = 0; i < f->sizek; i++)
    { ((void)g->mainthread, ((void)0)); if (((((&f->k[i])->tt_) & (1 << 6)) && ((((((&f->k[i])->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&f->k[i])->value_).gc)); };
  for (i = 0; i < f->sizeupvalues; i++)
    { if (f->upvalues[i].name) { if ((((f->upvalues[i].name)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((f->upvalues[i].name)))->gc))); }; };
  for (i = 0; i < f->sizep; i++)
    { if (f->p[i]) { if ((((f->p[i])->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((f->p[i])))->gc))); }; };
  for (i = 0; i < f->sizelocvars; i++)
    { if (f->locvars[i].varname) { if ((((f->locvars[i].varname)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((f->locvars[i].varname)))->gc))); }; };
  return 1 + f->sizek + f->sizeupvalues + f->sizep + f->sizelocvars;
}


static int traverseCclosure (global_State *g, CClosure *cl) {
  int i;
  for (i = 0; i < cl->nupvalues; i++)
    { ((void)g->mainthread, ((void)0)); if (((((&cl->upvalue[i])->tt_) & (1 << 6)) && ((((((&cl->upvalue[i])->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&cl->upvalue[i])->value_).gc)); };
  return 1 + cl->nupvalues;
}





static int traverseLclosure (global_State *g, LClosure *cl) {
  int i;
  { if (cl->p) { if ((((cl->p)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((cl->p)))->gc))); }; };
  for (i = 0; i < cl->nupvalues; i++) {
    UpVal *uv = cl->upvals[i];
    { if (uv) { if ((((uv)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((uv)))->gc))); }; };
  }
  return 1 + cl->nupvalues;
}
static int traversethread (global_State *g, lua_State *th) {
  UpVal *uv;
  StkId o = th->stack.p;
  if ((((th)->marked & 7) > 1) || g->gcstate == 0)
    linkgclist_((&(((union GCUnion *)((th)))->gc)), &(th)->gclist, &(g->grayagain));
  if (o == 
          ((void *)0)
              )
    return 1;
  ((void)0)
                                                    ;
  for (; o < th->top.p; o++)
    { ((void)g->mainthread, ((void)0)); if ((((((&(o)->val))->tt_) & (1 << 6)) && (((((((&(o)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,((((&(o)->val))->value_).gc)); };
  for (uv = th->openupval; uv != 
                                ((void *)0)
                                    ; uv = uv->u.open.next)
    { if ((((uv)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((uv)))->gc))); };
  if (g->gcstate == 2) {
    for (; o < th->stack_last.p + 5; o++)
      (((&(o)->val))->tt_=(((0) | ((0) << 4))));

    if (!(th->twups != th) && th->openupval != 
                                          ((void *)0)
                                              ) {
      th->twups = g->twups;
      g->twups = th;
    }
  }
  else if (!g->gcemergency)
    luaD_shrinkstack(th);
  return 1 + ((int)(((th)->stack_last.p - (th)->stack.p)));
}





static lu_mem propagatemark (global_State *g) {
  GCObject *o = g->gray;
  ((((o)->marked) |= ((1<<(5)))));
  g->gray = *getgclist(o);
  switch (o->tt) {
    case ((5) | ((0) << 4)): return traversetable(g, (&((((union GCUnion *)((o))))->h)));
    case ((7) | ((0) << 4)): return traverseudata(g, (&((((union GCUnion *)((o))))->u)));
    case ((6) | ((0) << 4)): return traverseLclosure(g, (&((((union GCUnion *)((o))))->cl.l)));
    case ((6) | ((2) << 4)): return traverseCclosure(g, (&((((union GCUnion *)((o))))->cl.c)));
    case (((9 +1)) | ((0) << 4)): return traverseproto(g, (&((((union GCUnion *)((o))))->p)));
    case ((8) | ((0) << 4)): return traversethread(g, (&((((union GCUnion *)((o))))->th)));
    default: ((void)0); return 0;
  }
}


static lu_mem propagateall (global_State *g) {
  lu_mem tot = 0;
  while (g->gray)
    tot += propagatemark(g);
  return tot;
}
static void convergeephemerons (global_State *g) {
  int changed;
  int dir = 0;
  do {
    GCObject *w;
    GCObject *next = g->ephemeron;
    g->ephemeron = 
                  ((void *)0)
                      ;
    changed = 0;
    while ((w = next) != 
                        ((void *)0)
                            ) {
      Table *h = (&((((union GCUnion *)((w))))->h));
      next = h->gclist;
      ((((h)->marked) |= ((1<<(5)))));
      if (traverseephemeron(g, h, dir)) {
        propagateall(g);
        changed = 1;
      }
    }
    dir = !dir;
  } while (changed);
}
static void clearbykeys (global_State *g, GCObject *l) {
  for (; l; l = (&((((union GCUnion *)((l))))->h))->gclist) {
    Table *h = (&((((union GCUnion *)((l))))->h));
    Node *limit = (&(h)->node[((size_t)((((1<<((h)->lsizenode))))))]);
    Node *n;
    for (n = (&(h)->node[0]); n < limit; n++) {
      if (iscleared(g, ((((n)->u.key_tt) & (1 << 6)) ? (((n)->u.key_val).gc) : 
                      ((void *)0)
                      )))
        (((&(n)->i_val))->tt_=(((0) | ((1) << 4))));
      if (((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0)))
        clearkey(n);
    }
  }
}






static void clearbyvalues (global_State *g, GCObject *l, GCObject *f) {
  for (; l != f; l = (&((((union GCUnion *)((l))))->h))->gclist) {
    Table *h = (&((((union GCUnion *)((l))))->h));
    Node *n, *limit = (&(h)->node[((size_t)((((1<<((h)->lsizenode))))))]);
    unsigned int i;
    unsigned int asize = luaH_realasize(h);
    for (i = 0; i < asize; i++) {
      TValue *o = &h->array[i];
      if (iscleared(g, ((((o)->tt_) & (1 << 6)) ? (((o)->value_).gc) : 
                      ((void *)0)
                      )))
        ((o)->tt_=(((0) | ((1) << 4))));
    }
    for (n = (&(h)->node[0]); n < limit; n++) {
      if (iscleared(g, (((((&(n)->i_val))->tt_) & (1 << 6)) ? ((((&(n)->i_val))->value_).gc) : 
                      ((void *)0)
                      )))
        (((&(n)->i_val))->tt_=(((0) | ((1) << 4))));
      if (((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0)))
        clearkey(n);
    }
  }
}


static void freeupval (lua_State *L, UpVal *uv) {
  if (((uv)->v.p != &(uv)->u.value))
    luaF_unlinkupval(uv);
  luaM_free_(L, (uv), sizeof(*(uv)));
}


static void freeobj (lua_State *L, GCObject *o) {
  switch (o->tt) {
    case (((9 +1)) | ((0) << 4)):
      luaF_freeproto(L, (&((((union GCUnion *)((o))))->p)));
      break;
    case ((9) | ((0) << 4)):
      freeupval(L, (&((((union GCUnion *)((o))))->upv)));
      break;
    case ((6) | ((0) << 4)): {
      LClosure *cl = (&((((union GCUnion *)((o))))->cl.l));
      luaM_free_(L, (cl), ((((int)((
     ((long)&((LClosure
     *)0)->upvals
     )
     ))) + ((int)((sizeof(TValue *)))) * (cl->nupvalues))));
      break;
    }
    case ((6) | ((2) << 4)): {
      CClosure *cl = (&((((union GCUnion *)((o))))->cl.c));
      luaM_free_(L, (cl), ((((int)((
     ((long)&((CClosure
     *)0)->upvalue
     )
     ))) + ((int)((sizeof(TValue)))) * (cl->nupvalues))));
      break;
    }
    case ((5) | ((0) << 4)):
      luaH_free(L, (&((((union GCUnion *)((o))))->h)));
      break;
    case ((8) | ((0) << 4)):
      luaE_freethread(L, (&((((union GCUnion *)((o))))->th)));
      break;
    case ((7) | ((0) << 4)): {
      Udata *u = (&((((union GCUnion *)((o))))->u));
      luaM_free_(L, (o), ((((u->nuvalue) == 0 ? 
     ((long)&((Udata0
     *)0)->bindata
     ) 
     : 
     ((long)&((Udata
     *)0)->uv
     ) 
     + (sizeof(UValue) * (u->nuvalue))) + (u->len))));
      break;
    }
    case ((4) | ((0) << 4)): {
      TString *ts = (&((((union GCUnion *)((o))))->ts));
      luaS_remove(L, ts);
      luaM_free_(L, (ts), ((
     ((long)&((TString
     *)0)->contents
     ) 
     + ((ts->shrlen) + 1) * sizeof(char))));
      break;
    }
    case ((4) | ((1) << 4)): {
      TString *ts = (&((((union GCUnion *)((o))))->ts));
      luaM_free_(L, (ts), ((
     ((long)&((TString
     *)0)->contents
     ) 
     + ((ts->u.lnglen) + 1) * sizeof(char))));
      break;
    }
    default: ((void)0);
  }
}
static GCObject **sweeplist (lua_State *L, GCObject **p, int countin,
                             int *countout) {
  global_State *g = (L->l_G);
  int ow = ((g)->currentwhite ^ ((1<<(3)) | (1<<(4))));
  int i;
  int white = ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))));
  for (i = 0; *p != 
                   ((void *)0) 
                        && i < countin; i++) {
    GCObject *curr = *p;
    int marked = curr->marked;
    if (((marked) & (ow))) {
      *p = curr->next;
      freeobj(L, curr);
    }
    else {
      curr->marked = ((lu_byte)(((marked & ~(((1<<(5)) | ((1<<(3)) | (1<<(4)))) | 7)) | white)));
      p = &curr->next;
    }
  }
  if (countout)
    *countout = i;
  return (*p == 
               ((void *)0)
                   ) ? 
                       ((void *)0) 
                            : p;
}





static GCObject **sweeptolive (lua_State *L, GCObject **p) {
  GCObject **old = p;
  do {
    p = sweeplist(L, p, 1, 
                          ((void *)0)
                              );
  } while (p == old);
  return p;
}
static void checkSizes (lua_State *L, global_State *g) {
  if (!g->gcemergency) {
    if (g->strt.nuse < g->strt.size / 4) {
      l_mem olddebt = g->GCdebt;
      luaS_resize(L, g->strt.size / 2);
      g->GCestimate += g->GCdebt - olddebt;
    }
  }
}






static GCObject *udata2finalize (global_State *g) {
  GCObject *o = g->tobefnz;
  ((void)0);
  g->tobefnz = o->next;
  o->next = g->allgc;
  g->allgc = o;
  ((o->marked) &= ((lu_byte)((~((1<<(6)))))));
  if ((3 <= (g)->gcstate && (g)->gcstate <= 6))
    (o->marked = ((lu_byte)(((o->marked & ~((1<<(5)) | ((1<<(3)) | (1<<(4))))) | ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))))))));
  else if (((o)->marked & 7) == 3)
    g->firstold1 = o;
  return o;
}


static void dothecall (lua_State *L, void *ud) {
  ((void)(ud));
  luaD_callnoyield(L, L->top.p - 2, 0);
}


static void GCTM (lua_State *L) {
  global_State *g = (L->l_G);
  const TValue *tm;
  TValue v;
  ((void)0);
  { TValue *io = (&v); GCObject *i_g=(udata2finalize(g)); ((io)->value_).gc = i_g; ((io)->tt_=(((i_g->tt) | (1 << 6)))); };
  tm = luaT_gettmbyobj(L, &v, TM_GC);
  if (!(((((((tm))->tt_)) & 0x0F)) == (0))) {
    int status;
    lu_byte oldah = L->allowhook;
    int oldgcstp = g->gcstp;
    g->gcstp |= 2;
    L->allowhook = 0;
    { TValue *io1=((&(L->top.p++)->val)); const TValue *io2=(tm); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    { TValue *io1=((&(L->top.p++)->val)); const TValue *io2=(&v); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    L->ci->callstatus |= (1<<7);
    status = luaD_pcall(L, dothecall, 
                                     ((void *)0)
                                         , (((char *)((L->top.p - 2))) - ((char *)((L->stack.p)))), 0);
    L->ci->callstatus &= ~(1<<7);
    L->allowhook = oldah;
    g->gcstp = oldgcstp;
    if ((status != 0)) {
      luaE_warnerror(L, "__gc");
      L->top.p--;
    }
  }
}





static int runafewfinalizers (lua_State *L, int n) {
  global_State *g = (L->l_G);
  int i;
  for (i = 0; i < n && g->tobefnz; i++)
    GCTM(L);
  return i;
}





static void callallpendingfinalizers (lua_State *L) {
  global_State *g = (L->l_G);
  while (g->tobefnz)
    GCTM(L);
}





static GCObject **findlast (GCObject **p) {
  while (*p != 
              ((void *)0)
                  )
    p = &(*p)->next;
  return p;
}
static void separatetobefnz (global_State *g, int all) {
  GCObject *curr;
  GCObject **p = &g->finobj;
  GCObject **lastnext = findlast(&g->tobefnz);
  while ((curr = *p) != g->finobjold1) {
    ((void)0);
    if (!((((curr)->marked) & (((1<<(3)) | (1<<(4))))) || all))
      p = &curr->next;
    else {
      if (curr == g->finobjsur)
        g->finobjsur = curr->next;
      *p = curr->next;
      curr->next = *lastnext;
      *lastnext = curr;
      lastnext = &curr->next;
    }
  }
}





static void checkpointer (GCObject **p, GCObject *o) {
  if (o == *p)
    *p = o->next;
}






static void correctpointers (global_State *g, GCObject *o) {
  checkpointer(&g->survival, o);
  checkpointer(&g->old1, o);
  checkpointer(&g->reallyold, o);
  checkpointer(&g->firstold1, o);
}






void luaC_checkfinalizer (lua_State *L, GCObject *o, Table *mt) {
  global_State *g = (L->l_G);
  if ((((o)->marked) & ((1<<(6)))) ||
      ((mt) == 
     ((void *)0) 
     ? 
     ((void *)0) 
     : ((mt)->flags & (1u<<(TM_GC))) ? 
     ((void *)0) 
     : luaT_gettm(mt, TM_GC, (g)->tmname[TM_GC])) == 
                              ((void *)0) 
                                   ||
      (g->gcstp & 4))
    return;
  else {
    GCObject **p;
    if ((3 <= (g)->gcstate && (g)->gcstate <= 6)) {
      (o->marked = ((lu_byte)(((o->marked & ~((1<<(5)) | ((1<<(3)) | (1<<(4))))) | ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))))))));
      if (g->sweepgc == &o->next)
        g->sweepgc = sweeptolive(L, g->sweepgc);
    }
    else
      correctpointers(g, o);

    for (p = &g->allgc; *p != o; p = &(*p)->next) { }
    *p = o->next;
    o->next = g->finobj;
    g->finobj = o;
    ((o->marked) |= ((1<<(6))));
  }
}
static void setpause (global_State *g) {
  l_mem threshold, debt;
  int pause = ((g->gcpause) * 4);
  l_mem estimate = g->GCestimate / 100;
  ((void)0);
  threshold = (pause < ((l_mem)(((lu_mem)(~(lu_mem)0)) >> 1)) / estimate)
            ? estimate * pause
            : ((l_mem)(((lu_mem)(~(lu_mem)0)) >> 1));
  debt = ((lu_mem)((g)->totalbytes + (g)->GCdebt)) - threshold;
  if (debt > 0) debt = 0;
  luaE_setdebt(g, debt);
}
static void sweep2old (lua_State *L, GCObject **p) {
  GCObject *curr;
  global_State *g = (L->l_G);
  while ((curr = *p) != 
                       ((void *)0)
                           ) {
    if ((((curr)->marked) & (((1<<(3)) | (1<<(4)))))) {
      ((void)0);
      *p = curr->next;
      freeobj(L, curr);
    }
    else {
      ((curr)->marked = ((lu_byte)((((curr)->marked & (~7)) | 4))));
      if (curr->tt == ((8) | ((0) << 4))) {
        lua_State *th = (&((((union GCUnion *)((curr))))->th));
        linkgclist_((&(((union GCUnion *)((th)))->gc)), &(th)->gclist, &(g->grayagain));
      }
      else if (curr->tt == ((9) | ((0) << 4)) && (((&((((union GCUnion *)((curr))))->upv)))->v.p != &((&((((union GCUnion *)((curr))))->upv)))->u.value))
        ((curr->marked) &= ((lu_byte)((~(((1<<(5)) | ((1<<(3)) | (1<<(4)))))))));
      else
        ((((curr)->marked) |= ((1<<(5)))));
      p = &curr->next;
    }
  }
}
static GCObject **sweepgen (lua_State *L, global_State *g, GCObject **p,
                            GCObject *limit, GCObject **pfirstold1) {
  static const lu_byte nextage[] = {
    1,
    3,
    3,
    4,
    4,
    5,
    6
  };
  int white = ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))));
  GCObject *curr;
  while ((curr = *p) != limit) {
    if ((((curr)->marked) & (((1<<(3)) | (1<<(4)))))) {
      ((void)0);
      *p = curr->next;
      freeobj(L, curr);
    }
    else {
      if (((curr)->marked & 7) == 0) {
        int marked = curr->marked & ~(((1<<(5)) | ((1<<(3)) | (1<<(4)))) | 7);
        curr->marked = ((lu_byte)((marked | 1 | white)));
      }
      else {
        ((curr)->marked = ((lu_byte)((((curr)->marked & (~7)) | nextage[((curr)->marked & 7)]))));
        if (((curr)->marked & 7) == 3 && *pfirstold1 == 
                                                    ((void *)0)
                                                        )
          *pfirstold1 = curr;
      }
      p = &curr->next;
    }
  }
  return p;
}







static void whitelist (global_State *g, GCObject *p) {
  int white = ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))));
  for (; p != 
             ((void *)0)
                 ; p = p->next)
    p->marked = ((lu_byte)(((p->marked & ~(((1<<(5)) | ((1<<(3)) | (1<<(4)))) | 7)) | white)));
}
static GCObject **correctgraylist (GCObject **p) {
  GCObject *curr;
  while ((curr = *p) != 
                       ((void *)0)
                           ) {
    GCObject **next = getgclist(curr);
    if ((((curr)->marked) & (((1<<(3)) | (1<<(4))))))
      goto remove;
    else if (((curr)->marked & 7) == 5) {
      ((void)0);
      ((((curr)->marked) |= ((1<<(5)))));
      ((curr)->marked ^= ((5)^(6)));
      goto remain;
    }
    else if (curr->tt == ((8) | ((0) << 4))) {
      ((void)0);
      goto remain;
    }
    else {
      ((void)0);
      if (((curr)->marked & 7) == 6)
        ((curr)->marked ^= ((6)^(4)));
      ((((curr)->marked) |= ((1<<(5)))));
      goto remove;
    }
    remove: *p = *next; continue;
    remain: p = next; continue;
  }
  return p;
}





static void correctgraylists (global_State *g) {
  GCObject **list = correctgraylist(&g->grayagain);
  *list = g->weak; g->weak = 
                            ((void *)0)
                                ;
  list = correctgraylist(list);
  *list = g->allweak; g->allweak = 
                                  ((void *)0)
                                      ;
  list = correctgraylist(list);
  *list = g->ephemeron; g->ephemeron = 
                                      ((void *)0)
                                          ;
  correctgraylist(list);
}







static void markold (global_State *g, GCObject *from, GCObject *to) {
  GCObject *p;
  for (p = from; p != to; p = p->next) {
    if (((p)->marked & 7) == 3) {
      ((void)0);
      ((p)->marked ^= ((3)^(4)));
      if ((((p)->marked) & ((1<<(5)))))
        reallymarkobject(g, p);
    }
  }
}





static void finishgencycle (lua_State *L, global_State *g) {
  correctgraylists(g);
  checkSizes(L, g);
  g->gcstate = 0;
  if (!g->gcemergency)
    callallpendingfinalizers(L);
}







static void youngcollection (lua_State *L, global_State *g) {
  GCObject **psurvival;
  GCObject *dummy;
  ((void)0);
  if (g->firstold1) {
    markold(g, g->firstold1, g->reallyold);
    g->firstold1 = 
                  ((void *)0)
                      ;
  }
  markold(g, g->finobj, g->finobjrold);
  markold(g, g->tobefnz, 
                        ((void *)0)
                            );
  atomic(L);


  g->gcstate = 3;
  psurvival = sweepgen(L, g, &g->allgc, g->survival, &g->firstold1);

  sweepgen(L, g, psurvival, g->old1, &g->firstold1);
  g->reallyold = g->old1;
  g->old1 = *psurvival;
  g->survival = g->allgc;


  dummy = 
         ((void *)0)
             ;
  psurvival = sweepgen(L, g, &g->finobj, g->finobjsur, &dummy);

  sweepgen(L, g, psurvival, g->finobjold1, &dummy);
  g->finobjrold = g->finobjold1;
  g->finobjold1 = *psurvival;
  g->finobjsur = g->finobj;

  sweepgen(L, g, &g->tobefnz, 
                             ((void *)0)
                                 , &dummy);
  finishgencycle(L, g);
}
static void atomic2gen (lua_State *L, global_State *g) {
  cleargraylists(g);

  g->gcstate = 3;
  sweep2old(L, &g->allgc);

  g->reallyold = g->old1 = g->survival = g->allgc;
  g->firstold1 = 
                ((void *)0)
                    ;


  sweep2old(L, &g->finobj);
  g->finobjrold = g->finobjold1 = g->finobjsur = g->finobj;

  sweep2old(L, &g->tobefnz);

  g->gckind = 1;
  g->lastatomic = 0;
  g->GCestimate = ((lu_mem)((g)->totalbytes + (g)->GCdebt));
  finishgencycle(L, g);
}






static void setminordebt (global_State *g) {
  luaE_setdebt(g, -(((l_mem)((((lu_mem)((g)->totalbytes + (g)->GCdebt)) / 100))) * g->genminormul));
}
static lu_mem entergen (lua_State *L, global_State *g) {
  lu_mem numobjs;
  luaC_runtilstate(L, (1<<(8)));
  luaC_runtilstate(L, (1<<(0)));
  numobjs = atomic(L);
  atomic2gen(L, g);
  setminordebt(g);
  return numobjs;
}







static void enterinc (global_State *g) {
  whitelist(g, g->allgc);
  g->reallyold = g->old1 = g->survival = 
                                        ((void *)0)
                                            ;
  whitelist(g, g->finobj);
  whitelist(g, g->tobefnz);
  g->finobjrold = g->finobjold1 = g->finobjsur = 
                                                ((void *)0)
                                                    ;
  g->gcstate = 8;
  g->gckind = 0;
  g->lastatomic = 0;
}





void luaC_changemode (lua_State *L, int newmode) {
  global_State *g = (L->l_G);
  if (newmode != g->gckind) {
    if (newmode == 1)
      entergen(L, g);
    else
      enterinc(g);
  }
  g->lastatomic = 0;
}





static lu_mem fullgen (lua_State *L, global_State *g) {
  enterinc(g);
  return entergen(L, g);
}
static void stepgenfull (lua_State *L, global_State *g) {
  lu_mem newatomic;
  lu_mem lastatomic = g->lastatomic;
  if (g->gckind == 1)
    enterinc(g);
  luaC_runtilstate(L, (1<<(0)));
  newatomic = atomic(L);
  if (newatomic < lastatomic + (lastatomic >> 3)) {
    atomic2gen(L, g);
    setminordebt(g);
  }
  else {
    g->GCestimate = ((lu_mem)((g)->totalbytes + (g)->GCdebt)); ;
    entersweep(L);
    luaC_runtilstate(L, (1<<(8)));
    setpause(g);
    g->lastatomic = newatomic;
  }
}
static void genstep (lua_State *L, global_State *g) {
  if (g->lastatomic != 0)
    stepgenfull(L, g);
  else {
    lu_mem majorbase = g->GCestimate;
    lu_mem majorinc = (majorbase / 100) * ((g->genmajormul) * 4);
    if (g->GCdebt > 0 && ((lu_mem)((g)->totalbytes + (g)->GCdebt)) > majorbase + majorinc) {
      lu_mem numobjs = fullgen(L, g);
      if (((lu_mem)((g)->totalbytes + (g)->GCdebt)) < majorbase + (majorinc / 2)) {


        ((void)0);
      }
      else {
        g->lastatomic = numobjs;
        setpause(g);
      }
    }
    else {
      youngcollection(L, g);
      setminordebt(g);
      g->GCestimate = majorbase;
    }
  }
  ((void)0);
}
static void entersweep (lua_State *L) {
  global_State *g = (L->l_G);
  g->gcstate = 3;
  ((void)0);
  g->sweepgc = sweeptolive(L, &g->allgc);
}






static void deletelist (lua_State *L, GCObject *p, GCObject *limit) {
  while (p != limit) {
    GCObject *next = p->next;
    freeobj(L, p);
    p = next;
  }
}






void luaC_freeallobjects (lua_State *L) {
  global_State *g = (L->l_G);
  g->gcstp = 4;
  luaC_changemode(L, 0);
  separatetobefnz(g, 1);
  ((void)0);
  callallpendingfinalizers(L);
  deletelist(L, g->allgc, (&(((union GCUnion *)((g->mainthread)))->gc)));
  ((void)0);
  deletelist(L, g->fixedgc, 
                           ((void *)0)
                               );
  ((void)0);
}


static lu_mem atomic (lua_State *L) {
  global_State *g = (L->l_G);
  lu_mem work = 0;
  GCObject *origweak, *origall;
  GCObject *grayagain = g->grayagain;
  g->grayagain = 
                ((void *)0)
                    ;
  ((void)0);
  ((void)0);
  g->gcstate = 2;
  { if ((((L)->marked) & (((1<<(3)) | (1<<(4)))))) reallymarkobject(g, (&(((union GCUnion *)((L)))->gc))); };

  { ((void)g->mainthread, ((void)0)); if (((((&g->l_registry)->tt_) & (1 << 6)) && ((((((&g->l_registry)->value_).gc))->marked) & (((1<<(3)) | (1<<(4))))))) reallymarkobject(g,(((&g->l_registry)->value_).gc)); };
  markmt(g);
  work += propagateall(g);

  work += remarkupvals(g);
  work += propagateall(g);
  g->gray = grayagain;
  work += propagateall(g);
  convergeephemerons(g);


  clearbyvalues(g, g->weak, 
                           ((void *)0)
                               );
  clearbyvalues(g, g->allweak, 
                              ((void *)0)
                                  );
  origweak = g->weak; origall = g->allweak;
  separatetobefnz(g, 0);
  work += markbeingfnz(g);
  work += propagateall(g);
  convergeephemerons(g);


  clearbykeys(g, g->ephemeron);
  clearbykeys(g, g->allweak);

  clearbyvalues(g, g->weak, origweak);
  clearbyvalues(g, g->allweak, origall);
  luaS_clearcache(g);
  g->currentwhite = ((lu_byte)((((g)->currentwhite ^ ((1<<(3)) | (1<<(4)))))));
  ((void)0);
  return work;
}


static int sweepstep (lua_State *L, global_State *g,
                      int nextstate, GCObject **nextlist) {
  if (g->sweepgc) {
    l_mem olddebt = g->GCdebt;
    int count;
    g->sweepgc = sweeplist(L, g->sweepgc, 100, &count);
    g->GCestimate += g->GCdebt - olddebt;
    return count;
  }
  else {
    g->gcstate = nextstate;
    g->sweepgc = nextlist;
    return 0;
  }
}


static lu_mem singlestep (lua_State *L) {
  global_State *g = (L->l_G);
  lu_mem work;
  ((void)0);
  g->gcstopem = 1;
  switch (g->gcstate) {
    case 8: {
      restartcollection(g);
      g->gcstate = 0;
      work = 1;
      break;
    }
    case 0: {
      if (g->gray == 
                    ((void *)0)
                        ) {
        g->gcstate = 1;
        work = 0;
      }
      else
        work = propagatemark(g);
      break;
    }
    case 1: {
      work = atomic(L);
      entersweep(L);
      g->GCestimate = ((lu_mem)((g)->totalbytes + (g)->GCdebt)); ;
      break;
    }
    case 3: {
      work = sweepstep(L, g, 4, &g->finobj);
      break;
    }
    case 4: {
      work = sweepstep(L, g, 5, &g->tobefnz);
      break;
    }
    case 5: {
      work = sweepstep(L, g, 6, 
                                       ((void *)0)
                                           );
      break;
    }
    case 6: {
      checkSizes(L, g);
      g->gcstate = 7;
      work = 0;
      break;
    }
    case 7: {
      if (g->tobefnz && !g->gcemergency) {
        g->gcstopem = 0;
        work = runafewfinalizers(L, 10) * 50;
      }
      else {
        g->gcstate = 8;
        work = 0;
      }
      break;
    }
    default: ((void)0); return 0;
  }
  g->gcstopem = 0;
  return work;
}






void luaC_runtilstate (lua_State *L, int statesmask) {
  global_State *g = (L->l_G);
  while (!((statesmask) & ((1<<(g->gcstate)))))
    singlestep(L);
}
static void incstep (lua_State *L, global_State *g) {
  int stepmul = (((g->gcstepmul) * 4) | 1);
  l_mem debt = (g->GCdebt / sizeof(TValue)) * stepmul;
  l_mem stepsize = (g->gcstepsize <= (sizeof(l_mem) * 8 - 2))
                 ? ((((l_mem)(1)) << g->gcstepsize) / sizeof(TValue)) * stepmul
                 : ((l_mem)(((lu_mem)(~(lu_mem)0)) >> 1));
  do {
    lu_mem work = singlestep(L);
    debt -= work;
  } while (debt > -stepsize && g->gcstate != 8);
  if (g->gcstate == 8)
    setpause(g);
  else {
    debt = (debt / stepmul) * sizeof(TValue);
    luaE_setdebt(g, debt);
  }
}






void luaC_step (lua_State *L) {
  global_State *g = (L->l_G);
  if (!((g)->gcstp == 0))
    luaE_setdebt(g, -2000);
  else {
    if((g->gckind == 1 || g->lastatomic != 0))
      genstep(L, g);
    else
      incstep(L, g);
  }
}
static void fullinc (lua_State *L, global_State *g) {
  if (((g)->gcstate <= 2))
    entersweep(L);

  luaC_runtilstate(L, (1<<(8)));
  luaC_runtilstate(L, (1<<(7)));

  ((void)0);
  luaC_runtilstate(L, (1<<(8)));
  setpause(g);
}







void luaC_fullgc (lua_State *L, int isemergency) {
  global_State *g = (L->l_G);
  ((void)0);
  g->gcemergency = isemergency;
  if (g->gckind == 0)
    fullinc(L, g);
  else
    fullgen(L, g);
  g->gcemergency = 0;
}



struct lconv
{


  char *decimal_point;
  char *thousands_sep;





  char *grouping;





  char *int_curr_symbol;
  char *currency_symbol;
  char *mon_decimal_point;
  char *mon_thousands_sep;
  char *mon_grouping;
  char *positive_sign;
  char *negative_sign;
  char int_frac_digits;
  char frac_digits;

  char p_cs_precedes;

  char p_sep_by_space;

  char n_cs_precedes;

  char n_sep_by_space;






  char p_sign_posn;
  char n_sign_posn;


  char int_p_cs_precedes;

  char int_p_sep_by_space;

  char int_n_cs_precedes;

  char int_n_sep_by_space;






  char int_p_sign_posn;
  char int_n_sign_posn;
};



extern char *setlocale (int __category, const char *__locale) ;


extern struct lconv *localeconv (void) ;


static const char *const luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};





static void lexerror (LexState *ls, const char *msg, int token);


static void save (LexState *ls, int c) {
  Mbuffer *b = ls->buff;
  if (((b)->n) + 1 > ((b)->buffsize)) {
    size_t newsize;
    if (((b)->buffsize) >= (sizeof(size_t) < sizeof(lua_Integer) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffffffffffffLL
                             ))/2)
      lexerror(ls, "lexical element too long", 0);
    newsize = ((b)->buffsize) * 2;
    ((b)->buffer = ((char *)((luaM_saferealloc_(ls->L, ((b)->buffer), ((b)->buffsize)*sizeof(char), (newsize)*sizeof(char))))), (b)->buffsize = newsize);
  }
  b->buffer[((b)->n)++] = ((char)((c)));
}


void luaX_init (lua_State *L) {
  int i;
  TString *e = (luaS_newlstr(L, "" "_ENV", (sizeof("_ENV")/sizeof(char))-1));
  luaC_fix(L, (&(((union GCUnion *)((e)))->gc)));
  for (i=0; i<(((int)((TK_WHILE-(
             (0x7f * 2 + 1) 
             + 1) + 1)))); i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, (&(((union GCUnion *)((ts)))->gc)));
    ts->extra = ((lu_byte)((i+1)));
  }
}


const char *luaX_token2str (LexState *ls, int token) {
  if (token < (
             (0x7f * 2 + 1) 
             + 1)) {
    if ((luai_ctype_[(token)+1] & ((1 << (2)))))
      return luaO_pushfstring(ls->L, "'%c'", token);
    else
      return luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  else {
    const char *s = luaX_tokens[token - (
                                       (0x7f * 2 + 1) 
                                       + 1)];
    if (token < TK_EOS)
      return luaO_pushfstring(ls->L, "'%s'", s);
    else
      return s;
  }
}


static const char *txtToken (LexState *ls, int token) {
  switch (token) {
    case TK_NAME: case TK_STRING:
    case TK_FLT: case TK_INT:
      save(ls, '\0');
      return luaO_pushfstring(ls->L, "'%s'", ((ls->buff)->buffer));
    default:
      return luaX_token2str(ls, token);
  }
}


static void lexerror (LexState *ls, const char *msg, int token) {
  msg = luaG_addinfo(ls->L, msg, ls->source, ls->linenumber);
  if (token)
    luaO_pushfstring(ls->L, "%s near %s", msg, txtToken(ls, token));
  luaD_throw(ls->L, 3);
}


void luaX_syntaxerror (LexState *ls, const char *msg) {
  lexerror(ls, msg, ls->t.token);
}
TString *luaX_newstring (LexState *ls, const char *str, size_t l) {
  lua_State *L = ls->L;
  TString *ts = luaS_newlstr(L, str, l);
  const TValue *o = luaH_getstr(ls->h, ts);
  if (!(((((((o))->tt_)) & 0x0F)) == (0)))
    ts = ((&((((union GCUnion *)((((((Node *)((o))))->u.key_val).gc))))->ts)));
  else {
    TValue *stv = (&(L->top.p++)->val);
    { TValue *io = (stv); TString *x_ = (ts); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    luaH_finishset(L, ls->h, stv, o, stv);

    { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
    L->top.p--;
  }
  return ts;
}






static void inclinenumber (LexState *ls) {
  int old = ls->current;
  ((void)0);
  (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
  if ((ls->current == '\n' || ls->current == '\r') && ls->current != old)
    (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
  if (++ls->linenumber >= 0x7fffffff
                                )
    lexerror(ls, "chunk has too many lines", 0);
}


void luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    int firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;
  ls->z = z;
  ls->fs = 
          ((void *)0)
              ;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  ls->envn = (luaS_newlstr(L, "" "_ENV", (sizeof("_ENV")/sizeof(char))-1));
  ((ls->buff)->buffer = ((char *)((luaM_saferealloc_(ls->L, ((ls->buff)->buffer), ((ls->buff)->buffsize)*sizeof(char), (32)*sizeof(char))))), (ls->buff)->buffsize = 32);
}
static int check_next1 (LexState *ls, int c) {
  if (ls->current == c) {
    (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
    return 1;
  }
  else return 0;
}






static int check_next2 (LexState *ls, const char *set) {
  ((void)0);
  if (ls->current == set[0] || ls->current == set[1]) {
    (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
    return 1;
  }
  else return 0;
}
static int read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  const char *expo = "Ee";
  int first = ls->current;
  ((void)0);
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  if (first == '0' && check_next2(ls, "xX"))
    expo = "Pp";
  for (;;) {
    if (check_next2(ls, expo))
      check_next2(ls, "-+");
    else if ((luai_ctype_[(ls->current)+1] & ((1 << (4)))) || ls->current == '.')
      (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
    else break;
  }
  if ((luai_ctype_[(ls->current)+1] & ((1 << (0)))))
    (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  save(ls, '\0');
  if (luaO_str2num(((ls->buff)->buffer), &obj) == 0)
    lexerror(ls, "malformed number", TK_FLT);
  if (((((&obj))->tt_) == (((3) | ((0) << 4))))) {
    seminfo->i = (((&obj)->value_).i);
    return TK_INT;
  }
  else {
    ((void)0);
    seminfo->r = (((&obj)->value_).n);
    return TK_FLT;
  }
}
static size_t skip_sep (LexState *ls) {
  size_t count = 0;
  int s = ls->current;
  ((void)0);
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  while (ls->current == '=') {
    (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
    count++;
  }
  return (ls->current == s) ? count + 2
         : (count == 0) ? 1
         : 0;
}


static void read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  int line = ls->linenumber;
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  if ((ls->current == '\n' || ls->current == '\r'))
    inclinenumber(ls);
  for (;;) {
    switch (ls->current) {
      case (-1): {
        const char *what = (seminfo ? "string" : "comment");
        const char *msg = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, line);
        lexerror(ls, msg, TK_EOS);
        break;
      }
      case ']': {
        if (skip_sep(ls) == sep) {
          (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
          goto endloop;
        }
        break;
      }
      case '\n': case '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        if (!seminfo) ((ls->buff)->n = 0);
        break;
      }
      default: {
        if (seminfo) (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
        else (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
      }
    }
  } endloop:
  if (seminfo)
    seminfo->ts = luaX_newstring(ls, ((ls->buff)->buffer) + sep,
                                     ((ls->buff)->n) - 2 * sep);
}


static void esccheck (LexState *ls, int c, const char *msg) {
  if (!c) {
    if (ls->current != (-1))
      (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
    lexerror(ls, msg, TK_STRING);
  }
}


static int gethexa (LexState *ls) {
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  esccheck (ls, (luai_ctype_[(ls->current)+1] & ((1 << (4)))), "hexadecimal digit expected");
  return luaO_hexavalue(ls->current);
}


static int readhexaesc (LexState *ls) {
  int r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  ((ls->buff)->n -= (2));
  return r;
}


static unsigned long readutf8esc (LexState *ls) {
  unsigned long r;
  int i = 4;
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  esccheck(ls, ls->current == '{', "missing '{'");
  r = gethexa(ls);
  while (((void)(((save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))))))), (luai_ctype_[(ls->current)+1] & ((1 << (4))))) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
  ((ls->buff)->n -= (i));
  return r;
}


static void utf8esc (LexState *ls) {
  char buff[8];
  int n = luaO_utf8esc(buff, readutf8esc(ls));
  for (; n > 0; n--)
    save(ls, buff[8 - n]);
}


static int readdecesc (LexState *ls) {
  int i;
  int r = 0;
  for (i = 0; i < 3 && (luai_ctype_[(ls->current)+1] & ((1 << (1)))); i++) {
    r = 10*r + ls->current - '0';
    (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  }
  esccheck(ls, r <= 
                   (0x7f * 2 + 1)
                            , "decimal escape too large");
  ((ls->buff)->n -= (i));
  return r;
}


static void read_string (LexState *ls, int del, SemInfo *seminfo) {
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  while (ls->current != del) {
    switch (ls->current) {
      case (-1):
        lexerror(ls, "unfinished string", TK_EOS);
        break;
      case '\n':
      case '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        break;
      case '\\': {
        int c;
        (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
        switch (ls->current) {
          case 'a': c = '\a'; goto read_save;
          case 'b': c = '\b'; goto read_save;
          case 'f': c = '\f'; goto read_save;
          case 'n': c = '\n'; goto read_save;
          case 'r': c = '\r'; goto read_save;
          case 't': c = '\t'; goto read_save;
          case 'v': c = '\v'; goto read_save;
          case 'x': c = readhexaesc(ls); goto read_save;
          case 'u': utf8esc(ls); goto no_save;
          case '\n': case '\r':
            inclinenumber(ls); c = '\n'; goto only_save;
          case '\\': case '\"': case '\'':
            c = ls->current; goto read_save;
          case (-1): goto no_save;
          case 'z': {
            ((ls->buff)->n -= (1));
            (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
            while ((luai_ctype_[(ls->current)+1] & ((1 << (3))))) {
              if ((ls->current == '\n' || ls->current == '\r')) inclinenumber(ls);
              else (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
            }
            goto no_save;
          }
          default: {
            esccheck(ls, (luai_ctype_[(ls->current)+1] & ((1 << (1)))), "invalid escape sequence");
            c = readdecesc(ls);
            goto only_save;
          }
        }
       read_save:
         (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));

       only_save:
         ((ls->buff)->n -= (1));
         save(ls, c);

       no_save: break;
      }
      default:
        (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
    }
  }
  (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
  seminfo->ts = luaX_newstring(ls, ((ls->buff)->buffer) + 1,
                                   ((ls->buff)->n) - 2);
}


static int llex (LexState *ls, SemInfo *seminfo) {
  ((ls->buff)->n = 0);
  for (;;) {
    switch (ls->current) {
      case '\n': case '\r': {
        inclinenumber(ls);
        break;
      }
      case ' ': case '\f': case '\t': case '\v': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        break;
      }
      case '-': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (ls->current != '-') return '-';

        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (ls->current == '[') {
          size_t sep = skip_sep(ls);
          ((ls->buff)->n = 0);
          if (sep >= 2) {
            read_long_string(ls, 
                                ((void *)0)
                                    , sep);
            ((ls->buff)->n = 0);
            break;
          }
        }

        while (!(ls->current == '\n' || ls->current == '\r') && ls->current != (-1))
          (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        break;
      }
      case '[': {
        size_t sep = skip_sep(ls);
        if (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          return TK_STRING;
        }
        else if (sep == 0)
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        return '[';
      }
      case '=': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, '=')) return TK_EQ;
        else return '=';
      }
      case '<': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, '=')) return TK_LE;
        else if (check_next1(ls, '<')) return TK_SHL;
        else return '<';
      }
      case '>': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, '=')) return TK_GE;
        else if (check_next1(ls, '>')) return TK_SHR;
        else return '>';
      }
      case '/': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, '/')) return TK_IDIV;
        else return '/';
      }
      case '~': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, '=')) return TK_NE;
        else return '~';
      }
      case ':': {
        (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
        if (check_next1(ls, ':')) return TK_DBCOLON;
        else return ':';
      }
      case '"': case '\'': {
        read_string(ls, ls->current, seminfo);
        return TK_STRING;
      }
      case '.': {
        (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
        if (check_next1(ls, '.')) {
          if (check_next1(ls, '.'))
            return TK_DOTS;
          else return TK_CONCAT;
        }
        else if (!(luai_ctype_[(ls->current)+1] & ((1 << (1))))) return '.';
        else return read_numeral(ls, seminfo);
      }
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9': {
        return read_numeral(ls, seminfo);
      }
      case (-1): {
        return TK_EOS;
      }
      default: {
        if ((luai_ctype_[(ls->current)+1] & ((1 << (0))))) {
          TString *ts;
          do {
            (save(ls, ls->current), (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z))));
          } while ((luai_ctype_[(ls->current)+1] & (((1 << (0)) | (1 << (1))))));
          ts = luaX_newstring(ls, ((ls->buff)->buffer),
                                  ((ls->buff)->n));
          seminfo->ts = ts;
          if (((ts)->tt == ((4) | ((0) << 4)) && (ts)->extra > 0))
            return ts->extra - 1 + (
                                  (0x7f * 2 + 1) 
                                  + 1);
          else {
            return TK_NAME;
          }
        }
        else {
          int c = ls->current;
          (ls->current = (((ls->z)->n--)>0 ? ((unsigned char)((*(ls->z)->p++))) : luaZ_fill(ls->z)));
          return c;
        }
      }
    }
  }
}


void luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  if (ls->lookahead.token != TK_EOS) {
    ls->t = ls->lookahead;
    ls->lookahead.token = TK_EOS;
  }
  else
    ls->t.token = llex(ls, &ls->t.seminfo);
}


int luaX_lookahead (LexState *ls) {
  ((void)0);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  return ls->lookahead.token;
}
void *luaM_growaux_ (lua_State *L, void *block, int nelems, int *psize,
                     int size_elems, int limit, const char *what) {
  void *newblock;
  int size = *psize;
  if (nelems + 1 <= size)
    return block;
  if (size >= limit / 2) {
    if ((size >= limit))
      luaG_runerror(L, "too many %s (limit is %d)", what, limit);
    size = limit;
  }
  else {
    size *= 2;
    if (size < 4)
      size = 4;
  }
  ((void)0);

  newblock = luaM_saferealloc_(L, block, ((size_t)((*psize))) * size_elems,
                                         ((size_t)((size))) * size_elems);
  *psize = size;
  return newblock;
}
void *luaM_shrinkvector_ (lua_State *L, void *block, int *size,
                          int final_n, int size_elem) {
  void *newblock;
  size_t oldsize = ((size_t)(((*size) * size_elem)));
  size_t newsize = ((size_t)((final_n * size_elem)));
  ((void)0);
  newblock = luaM_saferealloc_(L, block, oldsize, newsize);
  *size = final_n;
  return newblock;
}




void luaM_toobig (lua_State *L) {
  luaG_runerror(L, "memory allocation error: block too big");
}





void luaM_free_ (lua_State *L, void *block, size_t osize) {
  global_State *g = (L->l_G);
  ((void)0);
  ((*g->frealloc)(g->ud, block, osize, 0));
  g->GCdebt -= osize;
}






static void *tryagain (lua_State *L, void *block,
                       size_t osize, size_t nsize) {
  global_State *g = (L->l_G);
  if (((((((((&g->nilvalue))->tt_)) & 0x0F)) == (0)) && !g->gcstopem)) {
    luaC_fullgc(L, 1);
    return ((*g->frealloc)(g->ud, block, osize, nsize));
  }
  else return 
             ((void *)0)
                 ;
}





void *luaM_realloc_ (lua_State *L, void *block, size_t osize, size_t nsize) {
  void *newblock;
  global_State *g = (L->l_G);
  ((void)0);
  newblock = ((*g->frealloc)(g->ud, block, osize, nsize));
  if ((newblock == 
     ((void *)0) 
     && nsize > 0)) {
    newblock = tryagain(L, block, osize, nsize);
    if (newblock == 
                   ((void *)0)
                       )
      return 
            ((void *)0)
                ;
  }
  ((void)0);
  g->GCdebt = (g->GCdebt + nsize) - osize;
  return newblock;
}


void *luaM_saferealloc_ (lua_State *L, void *block, size_t osize,
                                                    size_t nsize) {
  void *newblock = luaM_realloc_(L, block, osize, nsize);
  if ((newblock == 
     ((void *)0) 
     && nsize > 0))
    luaD_throw(L, 4);
  return newblock;
}


void *luaM_malloc_ (lua_State *L, size_t size, int tag) {
  if (size == 0)
    return 
          ((void *)0)
              ;
  else {
    global_State *g = (L->l_G);
    void *newblock = ((*g->frealloc)(g->ud, 
                    ((void *)0)
                    , tag, size));
    if ((newblock == 
       ((void *)0)
       )) {
      newblock = tryagain(L, 
                            ((void *)0)
                                , tag, size);
      if (newblock == 
                     ((void *)0)
                         )
        luaD_throw(L, 4);
    }
    g->GCdebt += size;
    return newblock;
  }
}
int luaO_ceillog2 (unsigned int x) {
  static const lu_byte log_2[256] = {
    0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  };
  int l = 0;
  x--;
  while (x >= 256) { l += 8; x >>= 8; }
  return l + log_2[x];
}


static lua_Integer intarith (lua_State *L, int op, lua_Integer v1,
                                                   lua_Integer v2) {
  switch (op) {
    case 0: return ((lua_Integer)(((lua_Unsigned)(v1)) + ((lua_Unsigned)(v2))));
    case 1:return ((lua_Integer)(((lua_Unsigned)(v1)) - ((lua_Unsigned)(v2))));
    case 2:return ((lua_Integer)(((lua_Unsigned)(v1)) * ((lua_Unsigned)(v2))));
    case 3: return luaV_mod(L, v1, v2);
    case 6: return luaV_idiv(L, v1, v2);
    case 7: return ((lua_Integer)(((lua_Unsigned)(v1)) & ((lua_Unsigned)(v2))));
    case 8: return ((lua_Integer)(((lua_Unsigned)(v1)) | ((lua_Unsigned)(v2))));
    case 9: return ((lua_Integer)(((lua_Unsigned)(v1)) ^ ((lua_Unsigned)(v2))));
    case 10: return luaV_shiftl(v1, v2);
    case 11: return luaV_shiftl(v1,((lua_Integer)(((lua_Unsigned)(0)) - ((lua_Unsigned)(v2)))));
    case 12: return ((lua_Integer)(((lua_Unsigned)(0)) - ((lua_Unsigned)(v1))));
    case 13: return ((lua_Integer)(((lua_Unsigned)(~((lua_Unsigned)(0)))) ^ ((lua_Unsigned)(v1))));
    default: ((void)0); return 0;
  }
}


static lua_Number numarith (lua_State *L, int op, lua_Number v1,
                                                  lua_Number v2) {
  switch (op) {
    case 0: return ((v1)+(v2));
    case 1: return ((v1)-(v2));
    case 2: return ((v1)*(v2));
    case 5: return ((v1)/(v2));
    case 4: return ((void)L, (v2 == 2) ? (v1)*(v1) : pow(v1,v2));
    case 6: return ((void)L, (floor(((v1)/(v2)))));
    case 12: return (-(v1));
    case 3: return luaV_modf(L, v1, v2);
    default: ((void)0); return 0;
  }
}


int luaO_rawarith (lua_State *L, int op, const TValue *p1, const TValue *p2,
                   TValue *res) {
  switch (op) {
    case 7: case 8: case 9:
    case 10: case 11:
    case 13: {
      lua_Integer i1; lua_Integer i2;
      if (((((((p1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((p1)->value_).i), 1) : luaV_tointegerns(p1,&i1,F2Ieq)) && ((((((p2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((p2)->value_).i), 1) : luaV_tointegerns(p2,&i2,F2Ieq))) {
        { TValue *io=(res); ((io)->value_).i=(intarith(L, op, i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); };
        return 1;
      }
      else return 0;
    }
    case 5: case 4: {
      lua_Number n1; lua_Number n2;
      if ((((((p1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((p1)->value_).n), 1) : (((((p1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((p1)->value_).i)))), 1) : 0)) && (((((p2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((p2)->value_).n), 1) : (((((p2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((p2)->value_).i)))), 1) : 0))) {
        { TValue *io=(res); ((io)->value_).n=(numarith(L, op, n1, n2)); ((io)->tt_=(((3) | ((1) << 4)))); };
        return 1;
      }
      else return 0;
    }
    default: {
      lua_Number n1; lua_Number n2;
      if (((((p1))->tt_) == (((3) | ((0) << 4)))) && ((((p2))->tt_) == (((3) | ((0) << 4))))) {
        { TValue *io=(res); ((io)->value_).i=(intarith(L, op, (((p1)->value_).i), (((p2)->value_).i))); ((io)->tt_=(((3) | ((0) << 4)))); };
        return 1;
      }
      else if ((((((p1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((p1)->value_).n), 1) : (((((p1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((p1)->value_).i)))), 1) : 0)) && (((((p2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((p2)->value_).n), 1) : (((((p2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((p2)->value_).i)))), 1) : 0))) {
        { TValue *io=(res); ((io)->value_).n=(numarith(L, op, n1, n2)); ((io)->tt_=(((3) | ((1) << 4)))); };
        return 1;
      }
      else return 0;
    }
  }
}


void luaO_arith (lua_State *L, int op, const TValue *p1, const TValue *p2,
                 StkId res) {
  if (!luaO_rawarith(L, op, p1, p2, (&(res)->val))) {

    luaT_trybinTM(L, p1, p2, res, ((TMS)((op - 0) + TM_ADD)));
  }
}


int luaO_hexavalue (int c) {
  if ((luai_ctype_[(c)+1] & ((1 << (1))))) return c - '0';
  else return (((c) | ('A' ^ 'a')) - 'a') + 10;
}


static int isneg (const char **s) {
  if (**s == '-') { (*s)++; return 1; }
  else if (**s == '+') (*s)++;
  return 0;
}
static const char *l_str2dloc (const char *s, lua_Number *result, int mode) {
  char *endptr;
  *result = (mode == 'x') ? strtod((s), (&endptr))
                          : strtod((s), (&endptr));
  if (endptr == s) return 
                         ((void *)0)
                             ;
  while ((luai_ctype_[(((unsigned char)((*endptr))))+1] & ((1 << (3))))) endptr++;
  return (*endptr == '\0') ? endptr : 
                                     ((void *)0)
                                         ;
}
static const char *l_str2d (const char *s, lua_Number *result) {
  const char *endptr;
  const char *pmode = strpbrk(s, ".xXnN");
  int mode = pmode ? ((((unsigned char)((*pmode)))) | ('A' ^ 'a')) : 0;
  if (mode == 'n')
    return 
          ((void *)0)
              ;
  endptr = l_str2dloc(s, result, mode);
  if (endptr == 
               ((void *)0)
                   ) {
    char buff[201];
    const char *pdot = strchr(s, '.');
    if (pdot == 
               ((void *)0) 
                    || strlen(s) > 200)
      return 
            ((void *)0)
                ;
    strcpy(buff, s);
    buff[pdot - s] = (localeconv()->decimal_point[0]);
    endptr = l_str2dloc(buff, result, mode);
    if (endptr != 
                 ((void *)0)
                     )
      endptr = s + (endptr - buff);
  }
  return endptr;
}





static const char *l_str2int (const char *s, lua_Integer *result) {
  lua_Unsigned a = 0;
  int empty = 1;
  int neg;
  while ((luai_ctype_[(((unsigned char)((*s))))+1] & ((1 << (3))))) s++;
  neg = isneg(&s);
  if (s[0] == '0' &&
      (s[1] == 'x' || s[1] == 'X')) {
    s += 2;
    for (; (luai_ctype_[(((unsigned char)((*s))))+1] & ((1 << (4)))); s++) {
      a = a * 16 + luaO_hexavalue(*s);
      empty = 0;
    }
  }
  else {
    for (; (luai_ctype_[(((unsigned char)((*s))))+1] & ((1 << (1)))); s++) {
      int d = *s - '0';
      if (a >= ((lua_Unsigned)(0x7fffffffffffffffLL 
              / 10)) && (a > ((lua_Unsigned)(0x7fffffffffffffffLL 
                              / 10)) || d > ((int)((0x7fffffffffffffffLL 
                                             % 10))) + neg))
        return 
              ((void *)0)
                  ;
      a = a * 10 + d;
      empty = 0;
    }
  }
  while ((luai_ctype_[(((unsigned char)((*s))))+1] & ((1 << (3))))) s++;
  if (empty || *s != '\0') return 
                                 ((void *)0)
                                     ;
  else {
    *result = ((lua_Integer)((neg) ? 0u - a : a));
    return s;
  }
}


size_t luaO_str2num (const char *s, TValue *o) {
  lua_Integer i; lua_Number n;
  const char *e;
  if ((e = l_str2int(s, &i)) != 
                               ((void *)0)
                                   ) {
    { TValue *io=(o); ((io)->value_).i=(i); ((io)->tt_=(((3) | ((0) << 4)))); };
  }
  else if ((e = l_str2d(s, &n)) != 
                                  ((void *)0)
                                      ) {
    { TValue *io=(o); ((io)->value_).n=(n); ((io)->tt_=(((3) | ((1) << 4)))); };
  }
  else
    return 0;
  return (e - s) + 1;
}


int luaO_utf8esc (char *buff, unsigned long x) {
  int n = 1;
  ((void)0);
  if (x < 0x80)
    buff[7] = ((char)((x)));
  else {
    unsigned int mfb = 0x3f;
    do {
      buff[8 - (n++)] = ((char)((0x80 | (x & 0x3f))));
      x >>= 6;
      mfb >>= 1;
    } while (x > mfb);
    buff[8 - n] = ((char)(((~mfb << 1) | x)));
  }
  return n;
}
static int tostringbuff (TValue *obj, char *buff) {
  int len;
  ((void)0);
  if (((((obj))->tt_) == (((3) | ((0) << 4)))))
    len = snprintf((buff),44,"%" "ll" "d",(long long)((((obj)->value_).i)));
  else {
    len = snprintf((buff),44,"%.14g",(double)((((obj)->value_).n)));
    if (buff[strspn(buff, "-0123456789")] == '\0') {
      buff[len++] = (localeconv()->decimal_point[0]);
      buff[len++] = '0';
    }
  }
  return len;
}





void luaO_tostring (lua_State *L, TValue *obj) {
  char buff[44];
  int len = tostringbuff(obj, buff);
  { TValue *io = (obj); TString *x_ = (luaS_newlstr(L, buff, len)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
}
typedef struct BuffFS {
  lua_State *L;
  int pushed;
  int blen;
  char space[199];
} BuffFS;
static void pushstr (BuffFS *buff, const char *str, size_t lstr) {
  lua_State *L = buff->L;
  { TValue *io = ((&(L->top.p)->val)); TString *x_ = (luaS_newlstr(L, str, lstr)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
  L->top.p++;
  if (!buff->pushed)
    buff->pushed = 1;
  else
    luaV_concat(L, 2);
}





static void clearbuff (BuffFS *buff) {
  pushstr(buff, buff->space, buff->blen);
  buff->blen = 0;
}






static char *getbuff (BuffFS *buff, int sz) {
  ((void)0); ((void)0);
  if (sz > (60 + 44 + 95) - buff->blen)
    clearbuff(buff);
  return buff->space + buff->blen;
}
static void addstr2buff (BuffFS *buff, const char *str, size_t slen) {
  if (slen <= (60 + 44 + 95)) {
    char *bf = getbuff(buff, ((int)((slen))));
    memcpy(bf, str, slen);
    ((buff)->blen += (((int)((slen)))));
  }
  else {
    clearbuff(buff);
    pushstr(buff, str, slen);
  }
}





static void addnum2buff (BuffFS *buff, TValue *num) {
  char *numbuff = getbuff(buff, 44);
  int len = tostringbuff(num, numbuff);
  ((buff)->blen += (len));
}






const char *luaO_pushvfstring (lua_State *L, const char *fmt, va_list argp) {
  BuffFS buff;
  const char *e;
  buff.pushed = buff.blen = 0;
  buff.L = L;
  while ((e = strchr(fmt, '%')) != 
                                  ((void *)0)
                                      ) {
    addstr2buff(&buff, fmt, e - fmt);
    switch (*(e + 1)) {
      case 's': {
        const char *s = 
                       __builtin_va_arg(
                       argp
                       ,
                       char *
                       )
                                           ;
        if (s == 
                ((void *)0)
                    ) s = "(null)";
        addstr2buff(&buff, s, strlen(s));
        break;
      }
      case 'c': {
        char c = ((unsigned char)((
                __builtin_va_arg(
                argp
                ,
                int
                )
                )));
        addstr2buff(&buff, &c, sizeof(char));
        break;
      }
      case 'd': {
        TValue num;
        { TValue *io=(&num); ((io)->value_).i=(
       __builtin_va_arg(
       argp
       ,
       int
       )
       ); ((io)->tt_=(((3) | ((0) << 4)))); };
        addnum2buff(&buff, &num);
        break;
      }
      case 'I': {
        TValue num;
        { TValue *io=(&num); ((io)->value_).i=(((lua_Integer)(
       __builtin_va_arg(
       argp
       ,
       l_uacInt
       )
       ))); ((io)->tt_=(((3) | ((0) << 4)))); };
        addnum2buff(&buff, &num);
        break;
      }
      case 'f': {
        TValue num;
        { TValue *io=(&num); ((io)->value_).n=(((lua_Number)((
       __builtin_va_arg(
       argp
       ,
       l_uacNumber
       )
       )))); ((io)->tt_=(((3) | ((1) << 4)))); };
        addnum2buff(&buff, &num);
        break;
      }
      case 'p': {
        const int sz = 3 * sizeof(void*) + 8;
        char *bf = getbuff(&buff, sz);
        void *p = 
                 __builtin_va_arg(
                 argp
                 ,
                 void *
                 )
                                     ;
        int len = snprintf(bf,sz,"%p",p);
        ((&buff)->blen += (len));
        break;
      }
      case 'U': {
        char bf[8];
        int len = luaO_utf8esc(bf, 
                                  __builtin_va_arg(
                                  argp
                                  ,
                                  long
                                  )
                                                    );
        addstr2buff(&buff, bf + 8 - len, len);
        break;
      }
      case '%': {
        addstr2buff(&buff, "%", 1);
        break;
      }
      default: {
        luaG_runerror(L, "invalid option '%%%c' to 'lua_pushfstring'",
                         *(e + 1));
      }
    }
    fmt = e + 2;
  }
  addstr2buff(&buff, fmt, strlen(fmt));
  clearbuff(&buff);
  ((void)0);
  return ((((&((((union GCUnion *)(((((&(L->top.p - 1)->val))->value_).gc))))->ts))))->contents);
}


const char *luaO_pushfstring (lua_State *L, const char *fmt, ...) {
  const char *msg;
  va_list argp;
  
 __builtin_va_start(
 argp
 ,
 fmt
 )
                    ;
  msg = luaO_pushvfstring(L, fmt, argp);
  
 __builtin_va_end(
 argp
 )
             ;
  return msg;
}
void luaO_chunkid (char *out, const char *source, size_t srclen) {
  size_t bufflen = 60;
  if (*source == '=') {
    if (srclen <= bufflen)
      memcpy(out, source + 1, srclen * sizeof(char));
    else {
      ( memcpy(out,source + 1,(bufflen - 1) * sizeof(char)), out += (bufflen - 1) );
      *out = '\0';
    }
  }
  else if (*source == '@') {
    if (srclen <= bufflen)
      memcpy(out, source + 1, srclen * sizeof(char));
    else {
      ( memcpy(out,"...",((sizeof("...")/sizeof(char) - 1)) * sizeof(char)), out += ((sizeof("...")/sizeof(char) - 1)) );
      bufflen -= (sizeof("...")/sizeof(char) - 1);
      memcpy(out, source + 1 + srclen - bufflen, bufflen * sizeof(char));
    }
  }
  else {
    const char *nl = strchr(source, '\n');
    ( memcpy(out,"[string \"",((sizeof("[string \"")/sizeof(char) - 1)) * sizeof(char)), out += ((sizeof("[string \"")/sizeof(char) - 1)) );
    bufflen -= (sizeof("[string \"" "..." "\"]")/sizeof(char) - 1) + 1;
    if (srclen < bufflen && nl == 
                                 ((void *)0)
                                     ) {
      ( memcpy(out,source,(srclen) * sizeof(char)), out += (srclen) );
    }
    else {
      if (nl != 
               ((void *)0)
                   ) srclen = nl - source;
      if (srclen > bufflen) srclen = bufflen;
      ( memcpy(out,source,(srclen) * sizeof(char)), out += (srclen) );
      ( memcpy(out,"...",((sizeof("...")/sizeof(char) - 1)) * sizeof(char)), out += ((sizeof("...")/sizeof(char) - 1)) );
    }
    memcpy(out, "\"]", ((sizeof("\"]")/sizeof(char) - 1) + 1) * sizeof(char));
  }
}
 const lu_byte luaP_opmodes[84] = {

  (((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iAsBx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iAsBx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((1) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((1) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((1) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (isJ))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((1) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((1) << 6) | ((1) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((1) << 6) | ((1) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((1) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((0) << 6) | ((1) << 5) | ((0) << 4) | ((0) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABx))
 ,(((0) << 7) | ((1) << 6) | ((0) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((1) << 5) | ((0) << 4) | ((1) << 3) | (iABC))
 ,(((0) << 7) | ((0) << 6) | ((0) << 5) | ((0) << 4) | ((0) << 3) | (iAx))
};
typedef struct BlockCnt {
  struct BlockCnt *previous;
  int firstlabel;
  int firstgoto;
  lu_byte nactvar;
  lu_byte upval;
  lu_byte isloop;
  lu_byte insidetbc;
} BlockCnt;






static void statement (LexState *ls);
static void expr (LexState *ls, expdesc *v);


static void error_expected (LexState *ls, int token) {
  luaX_syntaxerror(ls,
      luaO_pushfstring(ls->L, "%s expected", luaX_token2str(ls, token)));
}


static void errorlimit (FuncState *fs, int limit, const char *what) {
  lua_State *L = fs->ls->L;
  const char *msg;
  int line = fs->f->linedefined;
  const char *where = (line == 0)
                      ? "main function"
                      : luaO_pushfstring(L, "function at line %d", line);
  msg = luaO_pushfstring(L, "too many %s (limit is %d) in %s",
                             what, limit, where);
  luaX_syntaxerror(fs->ls, msg);
}


static void checklimit (FuncState *fs, int v, int l, const char *what) {
  if (v > l) errorlimit(fs, l, what);
}





static int testnext (LexState *ls, int c) {
  if (ls->t.token == c) {
    luaX_next(ls);
    return 1;
  }
  else return 0;
}





static void check (LexState *ls, int c) {
  if (ls->t.token != c)
    error_expected(ls, c);
}





static void checknext (LexState *ls, int c) {
  check(ls, c);
  luaX_next(ls);
}
static void check_match (LexState *ls, int what, int who, int where) {
  if ((!testnext(ls, what))) {
    if (where == ls->linenumber)
      error_expected(ls, what);
    else {
      luaX_syntaxerror(ls, luaO_pushfstring(ls->L,
             "%s expected (to close %s at line %d)",
              luaX_token2str(ls, what), luaX_token2str(ls, who), where));
    }
  }
}


static TString *str_checkname (LexState *ls) {
  TString *ts;
  check(ls, TK_NAME);
  ts = ls->t.seminfo.ts;
  luaX_next(ls);
  return ts;
}


static void init_exp (expdesc *e, expkind k, int i) {
  e->f = e->t = (-1);
  e->k = k;
  e->u.info = i;
}


static void codestring (expdesc *e, TString *s) {
  e->f = e->t = (-1);
  e->k = VKSTR;
  e->u.strval = s;
}


static void codename (LexState *ls, expdesc *e) {
  codestring(e, str_checkname(ls));
}






static int registerlocalvar (LexState *ls, FuncState *fs, TString *varname) {
  Proto *f = fs->f;
  int oldsize = f->sizelocvars;
  ((f->locvars)=((LocVar *)(luaM_growaux_(ls->L,f->locvars,fs->ndebugvars,&(f->sizelocvars),sizeof(LocVar), ((((size_t)((0x7fff
 ))) <= ((size_t)(~(size_t)0))/sizeof(LocVar)) ? (0x7fff
 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(LocVar)))))),"local variables"))))
                                                      ;
  while (oldsize < f->sizelocvars)
    f->locvars[oldsize++].varname = 
                                   ((void *)0)
                                       ;
  f->locvars[fs->ndebugvars].varname = varname;
  f->locvars[fs->ndebugvars].startpc = fs->pc;
  ( ((((f)->marked) & ((1<<(5)))) && (((varname)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(ls->L,(&(((union GCUnion *)((f)))->gc)),(&(((union GCUnion *)((varname)))->gc))) : ((void)((0))));
  return fs->ndebugvars++;
}






static int new_localvar (LexState *ls, TString *name) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Dyndata *dyd = ls->dyd;
  Vardesc *var;
  checklimit(fs, dyd->actvar.n + 1 - fs->firstlocal,
                 200, "local variables");
  ((dyd->actvar.arr)=((Vardesc *)(luaM_growaux_(L,dyd->actvar.arr,dyd->actvar.n + 1,&(dyd->actvar.size),sizeof(Vardesc), ((((size_t)((
 (0x7fff * 2 + 1)
 ))) <= ((size_t)(~(size_t)0))/sizeof(Vardesc)) ? (
 (0x7fff * 2 + 1)
 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Vardesc)))))),"local variables"))))
                                                                          ;
  var = &dyd->actvar.arr[dyd->actvar.n++];
  var->vd.kind = 0;
  var->vd.name = name;
  return dyd->actvar.n - 1 - fs->firstlocal;
}
static Vardesc *getlocalvardesc (FuncState *fs, int vidx) {
  return &fs->ls->dyd->actvar.arr[fs->firstlocal + vidx];
}







static int reglevel (FuncState *fs, int nvar) {
  while (nvar-- > 0) {
    Vardesc *vd = getlocalvardesc(fs, nvar);
    if (vd->vd.kind != 3)
      return vd->vd.ridx + 1;
  }
  return 0;
}






int luaY_nvarstack (FuncState *fs) {
  return reglevel(fs, fs->nactvar);
}





static LocVar *localdebuginfo (FuncState *fs, int vidx) {
  Vardesc *vd = getlocalvardesc(fs, vidx);
  if (vd->vd.kind == 3)
    return 
          ((void *)0)
              ;
  else {
    int idx = vd->vd.pidx;
    ((void)0);
    return &fs->f->locvars[idx];
  }
}





static void init_var (FuncState *fs, expdesc *e, int vidx) {
  e->f = e->t = (-1);
  e->k = VLOCAL;
  e->u.var.vidx = vidx;
  e->u.var.ridx = getlocalvardesc(fs, vidx)->vd.ridx;
}





static void check_readonly (LexState *ls, expdesc *e) {
  FuncState *fs = ls->fs;
  TString *varname = 
                    ((void *)0)
                        ;
  switch (e->k) {
    case VCONST: {
      varname = ls->dyd->actvar.arr[e->u.info].vd.name;
      break;
    }
    case VLOCAL: {
      Vardesc *vardesc = getlocalvardesc(fs, e->u.var.vidx);
      if (vardesc->vd.kind != 0)
        varname = vardesc->vd.name;
      break;
    }
    case VUPVAL: {
      Upvaldesc *up = &fs->f->upvalues[e->u.info];
      if (up->kind != 0)
        varname = up->name;
      break;
    }
    default:
      return;
  }
  if (varname) {
    const char *msg = luaO_pushfstring(ls->L,
       "attempt to assign to const variable '%s'", ((varname)->contents));
    luaK_semerror(ls, msg);
  }
}





static void adjustlocalvars (LexState *ls, int nvars) {
  FuncState *fs = ls->fs;
  int reglevel = luaY_nvarstack(fs);
  int i;
  for (i = 0; i < nvars; i++) {
    int vidx = fs->nactvar++;
    Vardesc *var = getlocalvardesc(fs, vidx);
    var->vd.ridx = reglevel++;
    var->vd.pidx = registerlocalvar(ls, fs, var->vd.name);
  }
}






static void removevars (FuncState *fs, int tolevel) {
  fs->ls->dyd->actvar.n -= (fs->nactvar - tolevel);
  while (fs->nactvar > tolevel) {
    LocVar *var = localdebuginfo(fs, --fs->nactvar);
    if (var)
      var->endpc = fs->pc;
  }
}






static int searchupvalue (FuncState *fs, TString *name) {
  int i;
  Upvaldesc *up = fs->f->upvalues;
  for (i = 0; i < fs->nups; i++) {
    if (((up[i].name) == (name))) return i;
  }
  return -1;
}


static Upvaldesc *allocupvalue (FuncState *fs) {
  Proto *f = fs->f;
  int oldsize = f->sizeupvalues;
  checklimit(fs, fs->nups + 1, 255, "upvalues");
  ((f->upvalues)=((Upvaldesc *)(luaM_growaux_(fs->ls->L,f->upvalues,fs->nups,&(f->sizeupvalues),sizeof(Upvaldesc), ((((size_t)((255))) <= ((size_t)(~(size_t)0))/sizeof(Upvaldesc)) ? (255) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Upvaldesc)))))),"upvalues"))))
                                                  ;
  while (oldsize < f->sizeupvalues)
    f->upvalues[oldsize++].name = 
                                 ((void *)0)
                                     ;
  return &f->upvalues[fs->nups++];
}


static int newupvalue (FuncState *fs, TString *name, expdesc *v) {
  Upvaldesc *up = allocupvalue(fs);
  FuncState *prev = fs->prev;
  if (v->k == VLOCAL) {
    up->instack = 1;
    up->idx = v->u.var.ridx;
    up->kind = getlocalvardesc(prev, v->u.var.vidx)->vd.kind;
    ((void)0);
  }
  else {
    up->instack = 0;
    up->idx = ((lu_byte)((v->u.info)));
    up->kind = prev->f->upvalues[v->u.info].kind;
    ((void)0);
  }
  up->name = name;
  ( ((((fs->f)->marked) & ((1<<(5)))) && (((name)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(fs->ls->L,(&(((union GCUnion *)((fs->f)))->gc)),(&(((union GCUnion *)((name)))->gc))) : ((void)((0))));
  return fs->nups - 1;
}







static int searchvar (FuncState *fs, TString *n, expdesc *var) {
  int i;
  for (i = ((int)((fs->nactvar))) - 1; i >= 0; i--) {
    Vardesc *vd = getlocalvardesc(fs, i);
    if (((n) == (vd->vd.name))) {
      if (vd->vd.kind == 3)
        init_exp(var, VCONST, fs->firstlocal + i);
      else
        init_var(fs, var, i);
      return var->k;
    }
  }
  return -1;
}






static void markupval (FuncState *fs, int level) {
  BlockCnt *bl = fs->bl;
  while (bl->nactvar > level)
    bl = bl->previous;
  bl->upval = 1;
  fs->needclose = 1;
}





static void marktobeclosed (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  bl->upval = 1;
  bl->insidetbc = 1;
  fs->needclose = 1;
}







static void singlevaraux (FuncState *fs, TString *n, expdesc *var, int base) {
  if (fs == 
           ((void *)0)
               )
    init_exp(var, VVOID, 0);
  else {
    int v = searchvar(fs, n, var);
    if (v >= 0) {
      if (v == VLOCAL && !base)
        markupval(fs, var->u.var.vidx);
    }
    else {
      int idx = searchupvalue(fs, n);
      if (idx < 0) {
        singlevaraux(fs->prev, n, var, 0);
        if (var->k == VLOCAL || var->k == VUPVAL)
          idx = newupvalue(fs, n, var);
        else
          return;
      }
      init_exp(var, VUPVAL, idx);
    }
  }
}






static void singlevar (LexState *ls, expdesc *var) {
  TString *varname = str_checkname(ls);
  FuncState *fs = ls->fs;
  singlevaraux(fs, varname, var, 1);
  if (var->k == VVOID) {
    expdesc key;
    singlevaraux(fs, ls->envn, var, 1);
    ((void)0);
    luaK_exp2anyregup(fs, var);
    codestring(&key, varname);
    luaK_indexed(fs, var, &key);
  }
}






static void adjust_assign (LexState *ls, int nvars, int nexps, expdesc *e) {
  FuncState *fs = ls->fs;
  int needed = nvars - nexps;
  if (((e->k) == VCALL || (e->k) == VVARARG)) {
    int extra = needed + 1;
    if (extra < 0)
      extra = 0;
    luaK_setreturns(fs, e, extra);
  }
  else {
    if (e->k != VVOID)
      luaK_exp2nextreg(fs, e);
    if (needed > 0)
      luaK_nil(fs, fs->freereg, needed);
  }
  if (needed > 0)
    luaK_reserveregs(fs, needed);
  else
    fs->freereg += needed;
}
static void jumpscopeerror (LexState *ls, Labeldesc *gt) {
  const char *varname = ((getlocalvardesc(ls->fs, gt->nactvar)->vd.name)->contents);
  const char *msg = "<goto %s> at line %d jumps into the scope of local '%s'";
  msg = luaO_pushfstring(ls->L, msg, ((gt->name)->contents), gt->line, varname);
  luaK_semerror(ls, msg);
}







static void solvegoto (LexState *ls, int g, Labeldesc *label) {
  int i;
  Labellist *gl = &ls->dyd->gt;
  Labeldesc *gt = &gl->arr[g];
  ((void)0);
  if ((gt->nactvar < label->nactvar))
    jumpscopeerror(ls, gt);
  luaK_patchlist(ls->fs, gt->pc, label->pc);
  for (i = g; i < gl->n - 1; i++)
    gl->arr[i] = gl->arr[i + 1];
  gl->n--;
}





static Labeldesc *findlabel (LexState *ls, TString *name) {
  int i;
  Dyndata *dyd = ls->dyd;

  for (i = ls->fs->firstlabel; i < dyd->label.n; i++) {
    Labeldesc *lb = &dyd->label.arr[i];
    if (((lb->name) == (name)))
      return lb;
  }
  return 
        ((void *)0)
            ;
}





static int newlabelentry (LexState *ls, Labellist *l, TString *name,
                          int line, int pc) {
  int n = l->n;
  ((l->arr)=((Labeldesc *)(luaM_growaux_(ls->L,l->arr,n,&(l->size),sizeof(Labeldesc), ((((size_t)((0x7fff
 ))) <= ((size_t)(~(size_t)0))/sizeof(Labeldesc)) ? (0x7fff
 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Labeldesc)))))),"labels/gotos"))))
                                                      ;
  l->arr[n].name = name;
  l->arr[n].line = line;
  l->arr[n].nactvar = ls->fs->nactvar;
  l->arr[n].close = 0;
  l->arr[n].pc = pc;
  l->n = n + 1;
  return n;
}


static int newgotoentry (LexState *ls, TString *name, int line, int pc) {
  return newlabelentry(ls, &ls->dyd->gt, name, line, pc);
}







static int solvegotos (LexState *ls, Labeldesc *lb) {
  Labellist *gl = &ls->dyd->gt;
  int i = ls->fs->bl->firstgoto;
  int needsclose = 0;
  while (i < gl->n) {
    if (((gl->arr[i].name) == (lb->name))) {
      needsclose |= gl->arr[i].close;
      solvegoto(ls, i, lb);
    }
    else
      i++;
  }
  return needsclose;
}
static int createlabel (LexState *ls, TString *name, int line,
                        int last) {
  FuncState *fs = ls->fs;
  Labellist *ll = &ls->dyd->label;
  int l = newlabelentry(ls, ll, name, line, luaK_getlabel(fs));
  if (last) {

    ll->arr[l].nactvar = fs->bl->nactvar;
  }
  if (solvegotos(ls, &ll->arr[l])) {
    luaK_codeABCk(fs,OP_CLOSE,luaY_nvarstack(fs),0,0,0);
    return 1;
  }
  return 0;
}





static void movegotosout (FuncState *fs, BlockCnt *bl) {
  int i;
  Labellist *gl = &fs->ls->dyd->gt;

  for (i = bl->firstgoto; i < gl->n; i++) {
    Labeldesc *gt = &gl->arr[i];

    if (reglevel(fs, gt->nactvar) > reglevel(fs, bl->nactvar))
      gt->close |= bl->upval;
    gt->nactvar = bl->nactvar;
  }
}


static void enterblock (FuncState *fs, BlockCnt *bl, lu_byte isloop) {
  bl->isloop = isloop;
  bl->nactvar = fs->nactvar;
  bl->firstlabel = fs->ls->dyd->label.n;
  bl->firstgoto = fs->ls->dyd->gt.n;
  bl->upval = 0;
  bl->insidetbc = (fs->bl != 
                            ((void *)0) 
                                 && fs->bl->insidetbc);
  bl->previous = fs->bl;
  fs->bl = bl;
  ((void)0);
}





static void undefgoto (LexState *ls, Labeldesc *gt) {
  const char *msg;
  if (((gt->name) == ((luaS_newlstr(ls->L, "" "break", (sizeof("break")/sizeof(char))-1))))) {
    msg = "break outside loop at line %d";
    msg = luaO_pushfstring(ls->L, msg, gt->line);
  }
  else {
    msg = "no visible label '%s' for <goto> at line %d";
    msg = luaO_pushfstring(ls->L, msg, ((gt->name)->contents), gt->line);
  }
  luaK_semerror(ls, msg);
}


static void leaveblock (FuncState *fs) {
  BlockCnt *bl = fs->bl;
  LexState *ls = fs->ls;
  int hasclose = 0;
  int stklevel = reglevel(fs, bl->nactvar);
  removevars(fs, bl->nactvar);
  ((void)0);
  if (bl->isloop)
    hasclose = createlabel(ls, (luaS_newlstr(ls->L, "" "break", (sizeof("break")/sizeof(char))-1)), 0, 0);
  if (!hasclose && bl->previous && bl->upval)
    luaK_codeABCk(fs,OP_CLOSE,stklevel,0,0,0);
  fs->freereg = stklevel;
  ls->dyd->label.n = bl->firstlabel;
  fs->bl = bl->previous;
  if (bl->previous)
    movegotosout(fs, bl);
  else {
    if (bl->firstgoto < ls->dyd->gt.n)
      undefgoto(ls, &ls->dyd->gt.arr[bl->firstgoto]);
  }
}





static Proto *addprototype (LexState *ls) {
  Proto *clp;
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  if (fs->np >= f->sizep) {
    int oldsize = f->sizep;
    ((f->p)=((Proto * *)(luaM_growaux_(L,f->p,fs->np,&(f->sizep),sizeof(Proto *), ((((size_t)((((1<<(8 + 8 + 1))-1)))) <= ((size_t)(~(size_t)0))/sizeof(Proto *)) ? (((1<<(8 + 8 + 1))-1)) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Proto *)))))),"functions"))));
    while (oldsize < f->sizep)
      f->p[oldsize++] = 
                       ((void *)0)
                           ;
  }
  f->p[fs->np++] = clp = luaF_newproto(L);
  ( ((((f)->marked) & ((1<<(5)))) && (((clp)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((f)))->gc)),(&(((union GCUnion *)((clp)))->gc))) : ((void)((0))));
  return clp;
}
static void codeclosure (LexState *ls, expdesc *v) {
  FuncState *fs = ls->fs->prev;
  init_exp(v, VRELOC, luaK_codeABx(fs, OP_CLOSURE, 0, fs->np - 1));
  luaK_exp2nextreg(fs, v);
}


static void open_func (LexState *ls, FuncState *fs, BlockCnt *bl) {
  Proto *f = fs->f;
  fs->prev = ls->fs;
  fs->ls = ls;
  ls->fs = fs;
  fs->pc = 0;
  fs->previousline = f->linedefined;
  fs->iwthabs = 0;
  fs->lasttarget = 0;
  fs->freereg = 0;
  fs->nk = 0;
  fs->nabslineinfo = 0;
  fs->np = 0;
  fs->nups = 0;
  fs->ndebugvars = 0;
  fs->nactvar = 0;
  fs->needclose = 0;
  fs->firstlocal = ls->dyd->actvar.n;
  fs->firstlabel = ls->dyd->label.n;
  fs->bl = 
          ((void *)0)
              ;
  f->source = ls->source;
  ( ((((f)->marked) & ((1<<(5)))) && (((f->source)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(ls->L,(&(((union GCUnion *)((f)))->gc)),(&(((union GCUnion *)((f->source)))->gc))) : ((void)((0))));
  f->maxstacksize = 2;
  enterblock(fs, bl, 0);
}


static void close_func (LexState *ls) {
  lua_State *L = ls->L;
  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  luaK_ret(fs, luaY_nvarstack(fs), 0);
  leaveblock(fs);
  ((void)0);
  luaK_finish(fs);
  ((f->code)=((Instruction *)(luaM_shrinkvector_(L, f->code, &(f->sizecode), fs->pc, sizeof(Instruction)))));
  ((f->lineinfo)=((ls_byte *)(luaM_shrinkvector_(L, f->lineinfo, &(f->sizelineinfo), fs->pc, sizeof(ls_byte)))));
  ((f->abslineinfo)=((AbsLineInfo *)(luaM_shrinkvector_(L, f->abslineinfo, &(f->sizeabslineinfo), fs->nabslineinfo, sizeof(AbsLineInfo)))))
                                                     ;
  ((f->k)=((TValue *)(luaM_shrinkvector_(L, f->k, &(f->sizek), fs->nk, sizeof(TValue)))));
  ((f->p)=((Proto * *)(luaM_shrinkvector_(L, f->p, &(f->sizep), fs->np, sizeof(Proto *)))));
  ((f->locvars)=((LocVar *)(luaM_shrinkvector_(L, f->locvars, &(f->sizelocvars), fs->ndebugvars, sizeof(LocVar)))));
  ((f->upvalues)=((Upvaldesc *)(luaM_shrinkvector_(L, f->upvalues, &(f->sizeupvalues), fs->nups, sizeof(Upvaldesc)))));
  ls->fs = fs->prev;
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };
}
static int block_follow (LexState *ls, int withuntil) {
  switch (ls->t.token) {
    case TK_ELSE: case TK_ELSEIF:
    case TK_END: case TK_EOS:
      return 1;
    case TK_UNTIL: return withuntil;
    default: return 0;
  }
}


static void statlist (LexState *ls) {

  while (!block_follow(ls, 1)) {
    if (ls->t.token == TK_RETURN) {
      statement(ls);
      return;
    }
    statement(ls);
  }
}


static void fieldsel (LexState *ls, expdesc *v) {

  FuncState *fs = ls->fs;
  expdesc key;
  luaK_exp2anyregup(fs, v);
  luaX_next(ls);
  codename(ls, &key);
  luaK_indexed(fs, v, &key);
}


static void yindex (LexState *ls, expdesc *v) {

  luaX_next(ls);
  expr(ls, v);
  luaK_exp2val(ls->fs, v);
  checknext(ls, ']');
}
typedef struct ConsControl {
  expdesc v;
  expdesc *t;
  int nh;
  int na;
  int tostore;
} ConsControl;


static void recfield (LexState *ls, ConsControl *cc) {

  FuncState *fs = ls->fs;
  int reg = ls->fs->freereg;
  expdesc tab, key, val;
  if (ls->t.token == TK_NAME) {
    checklimit(fs, cc->nh, 0x7fffffff
                                 , "items in a constructor");
    codename(ls, &key);
  }
  else
    yindex(ls, &key);
  cc->nh++;
  checknext(ls, '=');
  tab = *cc->t;
  luaK_indexed(fs, &tab, &key);
  expr(ls, &val);
  luaK_storevar(fs, &tab, &val);
  fs->freereg = reg;
}


static void closelistfield (FuncState *fs, ConsControl *cc) {
  if (cc->v.k == VVOID) return;
  luaK_exp2nextreg(fs, &cc->v);
  cc->v.k = VVOID;
  if (cc->tostore == 50) {
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
    cc->na += cc->tostore;
    cc->tostore = 0;
  }
}


static void lastlistfield (FuncState *fs, ConsControl *cc) {
  if (cc->tostore == 0) return;
  if (((cc->v.k) == VCALL || (cc->v.k) == VVARARG)) {
    luaK_setreturns(fs, &cc->v, (-1));
    luaK_setlist(fs, cc->t->u.info, cc->na, (-1));
    cc->na--;
  }
  else {
    if (cc->v.k != VVOID)
      luaK_exp2nextreg(fs, &cc->v);
    luaK_setlist(fs, cc->t->u.info, cc->na, cc->tostore);
  }
  cc->na += cc->tostore;
}


static void listfield (LexState *ls, ConsControl *cc) {

  expr(ls, &cc->v);
  cc->tostore++;
}


static void field (LexState *ls, ConsControl *cc) {

  switch(ls->t.token) {
    case TK_NAME: {
      if (luaX_lookahead(ls) != '=')
        listfield(ls, cc);
      else
        recfield(ls, cc);
      break;
    }
    case '[': {
      recfield(ls, cc);
      break;
    }
    default: {
      listfield(ls, cc);
      break;
    }
  }
}


static void constructor (LexState *ls, expdesc *t) {


  FuncState *fs = ls->fs;
  int line = ls->linenumber;
  int pc = luaK_codeABCk(fs,OP_NEWTABLE,0,0,0,0);
  ConsControl cc;
  luaK_code(fs, 0);
  cc.na = cc.nh = cc.tostore = 0;
  cc.t = t;
  init_exp(t, VNONRELOC, fs->freereg);
  luaK_reserveregs(fs, 1);
  init_exp(&cc.v, VVOID, 0);
  checknext(ls, '{');
  do {
    ((void)0);
    if (ls->t.token == '}') break;
    closelistfield(fs, &cc);
    field(ls, &cc);
  } while (testnext(ls, ',') || testnext(ls, ';'));
  check_match(ls, '}', '{', line);
  lastlistfield(fs, &cc);
  luaK_settablesize(fs, pc, t->u.info, cc.na, cc.nh);
}




static void setvararg (FuncState *fs, int nparams) {
  fs->f->is_vararg = 1;
  luaK_codeABCk(fs,OP_VARARGPREP,nparams,0,0,0);
}


static void parlist (LexState *ls) {

  FuncState *fs = ls->fs;
  Proto *f = fs->f;
  int nparams = 0;
  int isvararg = 0;
  if (ls->t.token != ')') {
    do {
      switch (ls->t.token) {
        case TK_NAME: {
          new_localvar(ls, str_checkname(ls));
          nparams++;
          break;
        }
        case TK_DOTS: {
          luaX_next(ls);
          isvararg = 1;
          break;
        }
        default: luaX_syntaxerror(ls, "<name> or '...' expected");
      }
    } while (!isvararg && testnext(ls, ','));
  }
  adjustlocalvars(ls, nparams);
  f->numparams = ((lu_byte)((fs->nactvar)));
  if (isvararg)
    setvararg(fs, f->numparams);
  luaK_reserveregs(fs, fs->nactvar);
}


static void body (LexState *ls, expdesc *e, int ismethod, int line) {

  FuncState new_fs;
  BlockCnt bl;
  new_fs.f = addprototype(ls);
  new_fs.f->linedefined = line;
  open_func(ls, &new_fs, &bl);
  checknext(ls, '(');
  if (ismethod) {
    new_localvar(ls, luaX_newstring(ls, "" "self", (sizeof("self")/sizeof(char)) - 1));;
    adjustlocalvars(ls, 1);
  }
  parlist(ls);
  checknext(ls, ')');
  statlist(ls);
  new_fs.f->lastlinedefined = ls->linenumber;
  check_match(ls, TK_END, TK_FUNCTION, line);
  codeclosure(ls, e);
  close_func(ls);
}


static int explist (LexState *ls, expdesc *v) {

  int n = 1;
  expr(ls, v);
  while (testnext(ls, ',')) {
    luaK_exp2nextreg(ls->fs, v);
    expr(ls, v);
    n++;
  }
  return n;
}


static void funcargs (LexState *ls, expdesc *f, int line) {
  FuncState *fs = ls->fs;
  expdesc args;
  int base, nparams;
  switch (ls->t.token) {
    case '(': {
      luaX_next(ls);
      if (ls->t.token == ')')
        args.k = VVOID;
      else {
        explist(ls, &args);
        if (((args.k) == VCALL || (args.k) == VVARARG))
          luaK_setreturns(fs, &args, (-1));
      }
      check_match(ls, ')', '(', line);
      break;
    }
    case '{': {
      constructor(ls, &args);
      break;
    }
    case TK_STRING: {
      codestring(&args, ls->t.seminfo.ts);
      luaX_next(ls);
      break;
    }
    default: {
      luaX_syntaxerror(ls, "function arguments expected");
    }
  }
  ((void)0);
  base = f->u.info;
  if (((args.k) == VCALL || (args.k) == VVARARG))
    nparams = (-1);
  else {
    if (args.k != VVOID)
      luaK_exp2nextreg(fs, &args);
    nparams = fs->freereg - (base+1);
  }
  init_exp(f, VCALL, luaK_codeABCk(fs,OP_CALL,base,nparams+1,2,0));
  luaK_fixline(fs, line);
  fs->freereg = base+1;

}
static void primaryexp (LexState *ls, expdesc *v) {

  switch (ls->t.token) {
    case '(': {
      int line = ls->linenumber;
      luaX_next(ls);
      expr(ls, v);
      check_match(ls, ')', '(', line);
      luaK_dischargevars(ls->fs, v);
      return;
    }
    case TK_NAME: {
      singlevar(ls, v);
      return;
    }
    default: {
      luaX_syntaxerror(ls, "unexpected symbol");
    }
  }
}


static void suffixedexp (LexState *ls, expdesc *v) {


  FuncState *fs = ls->fs;
  int line = ls->linenumber;
  primaryexp(ls, v);
  for (;;) {
    switch (ls->t.token) {
      case '.': {
        fieldsel(ls, v);
        break;
      }
      case '[': {
        expdesc key;
        luaK_exp2anyregup(fs, v);
        yindex(ls, &key);
        luaK_indexed(fs, v, &key);
        break;
      }
      case ':': {
        expdesc key;
        luaX_next(ls);
        codename(ls, &key);
        luaK_self(fs, v, &key);
        funcargs(ls, v, line);
        break;
      }
      case '(': case TK_STRING: case '{': {
        luaK_exp2nextreg(fs, v);
        funcargs(ls, v, line);
        break;
      }
      default: return;
    }
  }
}


static void simpleexp (LexState *ls, expdesc *v) {


  switch (ls->t.token) {
    case TK_FLT: {
      init_exp(v, VKFLT, 0);
      v->u.nval = ls->t.seminfo.r;
      break;
    }
    case TK_INT: {
      init_exp(v, VKINT, 0);
      v->u.ival = ls->t.seminfo.i;
      break;
    }
    case TK_STRING: {
      codestring(v, ls->t.seminfo.ts);
      break;
    }
    case TK_NIL: {
      init_exp(v, VNIL, 0);
      break;
    }
    case TK_TRUE: {
      init_exp(v, VTRUE, 0);
      break;
    }
    case TK_FALSE: {
      init_exp(v, VFALSE, 0);
      break;
    }
    case TK_DOTS: {
      FuncState *fs = ls->fs;
      { if (!(fs->f->is_vararg)) luaX_syntaxerror(ls, "cannot use '...' outside a vararg function"); }
                                                                   ;
      init_exp(v, VVARARG, luaK_codeABCk(fs,OP_VARARG,0,0,1,0));
      break;
    }
    case '{': {
      constructor(ls, v);
      return;
    }
    case TK_FUNCTION: {
      luaX_next(ls);
      body(ls, v, 0, ls->linenumber);
      return;
    }
    default: {
      suffixedexp(ls, v);
      return;
    }
  }
  luaX_next(ls);
}


static UnOpr getunopr (int op) {
  switch (op) {
    case TK_NOT: return OPR_NOT;
    case '-': return OPR_MINUS;
    case '~': return OPR_BNOT;
    case '#': return OPR_LEN;
    default: return OPR_NOUNOPR;
  }
}


static BinOpr getbinopr (int op) {
  switch (op) {
    case '+': return OPR_ADD;
    case '-': return OPR_SUB;
    case '*': return OPR_MUL;
    case '%': return OPR_MOD;
    case '^': return OPR_POW;
    case '/': return OPR_DIV;
    case TK_IDIV: return OPR_IDIV;
    case '&': return OPR_BAND;
    case '|': return OPR_BOR;
    case '~': return OPR_BXOR;
    case TK_SHL: return OPR_SHL;
    case TK_SHR: return OPR_SHR;
    case TK_CONCAT: return OPR_CONCAT;
    case TK_NE: return OPR_NE;
    case TK_EQ: return OPR_EQ;
    case '<': return OPR_LT;
    case TK_LE: return OPR_LE;
    case '>': return OPR_GT;
    case TK_GE: return OPR_GE;
    case TK_AND: return OPR_AND;
    case TK_OR: return OPR_OR;
    default: return OPR_NOBINOPR;
  }
}





static const struct {
  lu_byte left;
  lu_byte right;
} priority[] = {
   {10, 10}, {10, 10},
   {11, 11}, {11, 11},
   {14, 13},
   {11, 11}, {11, 11},
   {6, 6}, {4, 4}, {5, 5},
   {7, 7}, {7, 7},
   {9, 8},
   {3, 3}, {3, 3}, {3, 3},
   {3, 3}, {3, 3}, {3, 3},
   {2, 2}, {1, 1}
};
static BinOpr subexpr (LexState *ls, expdesc *v, int limit) {
  BinOpr op;
  UnOpr uop;
  luaE_incCstack(ls->L);
  uop = getunopr(ls->t.token);
  if (uop != OPR_NOUNOPR) {
    int line = ls->linenumber;
    luaX_next(ls);
    subexpr(ls, v, 12);
    luaK_prefix(ls->fs, uop, v, line);
  }
  else simpleexp(ls, v);

  op = getbinopr(ls->t.token);
  while (op != OPR_NOBINOPR && priority[op].left > limit) {
    expdesc v2;
    BinOpr nextop;
    int line = ls->linenumber;
    luaX_next(ls);
    luaK_infix(ls->fs, op, v);

    nextop = subexpr(ls, &v2, priority[op].right);
    luaK_posfix(ls->fs, op, v, &v2, line);
    op = nextop;
  }
  ((ls)->L->nCcalls--);
  return op;
}


static void expr (LexState *ls, expdesc *v) {
  subexpr(ls, v, 0);
}
static void block (LexState *ls) {

  FuncState *fs = ls->fs;
  BlockCnt bl;
  enterblock(fs, &bl, 0);
  statlist(ls);
  leaveblock(fs);
}






struct LHS_assign {
  struct LHS_assign *prev;
  expdesc v;
};
static void check_conflict (LexState *ls, struct LHS_assign *lh, expdesc *v) {
  FuncState *fs = ls->fs;
  int extra = fs->freereg;
  int conflict = 0;
  for (; lh; lh = lh->prev) {
    if ((VINDEXED <= (lh->v.k) && (lh->v.k) <= VINDEXSTR)) {
      if (lh->v.k == VINDEXUP) {
        if (v->k == VUPVAL && lh->v.u.ind.t == v->u.info) {
          conflict = 1;
          lh->v.k = VINDEXSTR;
          lh->v.u.ind.t = extra;
        }
      }
      else {
        if (v->k == VLOCAL && lh->v.u.ind.t == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.t = extra;
        }

        if (lh->v.k == VINDEXED && v->k == VLOCAL &&
            lh->v.u.ind.idx == v->u.var.ridx) {
          conflict = 1;
          lh->v.u.ind.idx = extra;
        }
      }
    }
  }
  if (conflict) {

    if (v->k == VLOCAL)
      luaK_codeABCk(fs,OP_MOVE,extra,v->u.var.ridx,0,0);
    else
      luaK_codeABCk(fs,OP_GETUPVAL,extra,v->u.info,0,0);
    luaK_reserveregs(fs, 1);
  }
}
static void restassign (LexState *ls, struct LHS_assign *lh, int nvars) {
  expdesc e;
  { if (!((VLOCAL <= (lh->v.k) && (lh->v.k) <= VINDEXSTR))) luaX_syntaxerror(ls, "syntax error"); };
  check_readonly(ls, &lh->v);
  if (testnext(ls, ',')) {
    struct LHS_assign nv;
    nv.prev = lh;
    suffixedexp(ls, &nv.v);
    if (!(VINDEXED <= (nv.v.k) && (nv.v.k) <= VINDEXSTR))
      check_conflict(ls, lh, &nv.v);
    luaE_incCstack(ls->L);
    restassign(ls, &nv, nvars+1);
    ((ls)->L->nCcalls--);
  }
  else {
    int nexps;
    checknext(ls, '=');
    nexps = explist(ls, &e);
    if (nexps != nvars)
      adjust_assign(ls, nvars, nexps, &e);
    else {
      luaK_setoneret(ls->fs, &e);
      luaK_storevar(ls->fs, &lh->v, &e);
      return;
    }
  }
  init_exp(&e, VNONRELOC, ls->fs->freereg-1);
  luaK_storevar(ls->fs, &lh->v, &e);
}


static int cond (LexState *ls) {

  expdesc v;
  expr(ls, &v);
  if (v.k == VNIL) v.k = VFALSE;
  luaK_goiftrue(ls->fs, &v);
  return v.f;
}


static void gotostat (LexState *ls) {
  FuncState *fs = ls->fs;
  int line = ls->linenumber;
  TString *name = str_checkname(ls);
  Labeldesc *lb = findlabel(ls, name);
  if (lb == 
           ((void *)0)
               )

    newgotoentry(ls, name, line, luaK_jump(fs));
  else {

    int lblevel = reglevel(fs, lb->nactvar);
    if (luaY_nvarstack(fs) > lblevel)
      luaK_codeABCk(fs,OP_CLOSE,lblevel,0,0,0);

    luaK_patchlist(fs, luaK_jump(fs), lb->pc);
  }
}





static void breakstat (LexState *ls) {
  int line = ls->linenumber;
  luaX_next(ls);
  newgotoentry(ls, (luaS_newlstr(ls->L, "" "break", (sizeof("break")/sizeof(char))-1)), line, luaK_jump(ls->fs));
}





static void checkrepeated (LexState *ls, TString *name) {
  Labeldesc *lb = findlabel(ls, name);
  if ((lb != 
     ((void *)0)
     )) {
    const char *msg = "label '%s' already defined on line %d";
    msg = luaO_pushfstring(ls->L, msg, ((name)->contents), lb->line);
    luaK_semerror(ls, msg);
  }
}


static void labelstat (LexState *ls, TString *name, int line) {

  checknext(ls, TK_DBCOLON);
  while (ls->t.token == ';' || ls->t.token == TK_DBCOLON)
    statement(ls);
  checkrepeated(ls, name);
  createlabel(ls, name, line, block_follow(ls, 0));
}


static void whilestat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  int whileinit;
  int condexit;
  BlockCnt bl;
  luaX_next(ls);
  whileinit = luaK_getlabel(fs);
  condexit = cond(ls);
  enterblock(fs, &bl, 1);
  checknext(ls, TK_DO);
  block(ls);
  luaK_patchlist(fs, luaK_jump(fs), whileinit);
  check_match(ls, TK_END, TK_WHILE, line);
  leaveblock(fs);
  luaK_patchtohere(fs, condexit);
}


static void repeatstat (LexState *ls, int line) {

  int condexit;
  FuncState *fs = ls->fs;
  int repeat_init = luaK_getlabel(fs);
  BlockCnt bl1, bl2;
  enterblock(fs, &bl1, 1);
  enterblock(fs, &bl2, 0);
  luaX_next(ls);
  statlist(ls);
  check_match(ls, TK_UNTIL, TK_REPEAT, line);
  condexit = cond(ls);
  leaveblock(fs);
  if (bl2.upval) {
    int exit = luaK_jump(fs);
    luaK_patchtohere(fs, condexit);
    luaK_codeABCk(fs,OP_CLOSE,reglevel(fs, bl2.nactvar),0,0,0);
    condexit = luaK_jump(fs);
    luaK_patchtohere(fs, exit);
  }
  luaK_patchlist(fs, condexit, repeat_init);
  leaveblock(fs);
}







static void exp1 (LexState *ls) {
  expdesc e;
  expr(ls, &e);
  luaK_exp2nextreg(ls->fs, &e);
  ((void)0);
}







static void fixforjump (FuncState *fs, int pc, int dest, int back) {
  Instruction *jmp = &fs->f->code[pc];
  int offset = dest - (pc + 1);
  if (back)
    offset = -offset;
  if ((offset > ((1<<(8 + 8 + 1))-1)))
    luaX_syntaxerror(fs->ls, "control structure too long");
  ((*jmp) = (((*jmp)&(~((~((~(Instruction)0)<<((8 + 8 + 1))))<<(((0 + 7) + 8))))) | ((((Instruction)(offset))<<((0 + 7) + 8))&((~((~(Instruction)0)<<((8 + 8 + 1))))<<(((0 + 7) + 8))))));
}





static void forbody (LexState *ls, int base, int line, int nvars, int isgen) {

  static const OpCode forprep[2] = {OP_FORPREP, OP_TFORPREP};
  static const OpCode forloop[2] = {OP_FORLOOP, OP_TFORLOOP};
  BlockCnt bl;
  FuncState *fs = ls->fs;
  int prep, endfor;
  checknext(ls, TK_DO);
  prep = luaK_codeABx(fs, forprep[isgen], base, 0);
  enterblock(fs, &bl, 0);
  adjustlocalvars(ls, nvars);
  luaK_reserveregs(fs, nvars);
  block(ls);
  leaveblock(fs);
  fixforjump(fs, prep, luaK_getlabel(fs), 0);
  if (isgen) {
    luaK_codeABCk(fs,OP_TFORCALL,base,0,nvars,0);
    luaK_fixline(fs, line);
  }
  endfor = luaK_codeABx(fs, forloop[isgen], base, 0);
  fixforjump(fs, endfor, prep + 1, 1);
  luaK_fixline(fs, line);
}


static void fornum (LexState *ls, TString *varname, int line) {

  FuncState *fs = ls->fs;
  int base = fs->freereg;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, varname);
  checknext(ls, '=');
  exp1(ls);
  checknext(ls, ',');
  exp1(ls);
  if (testnext(ls, ','))
    exp1(ls);
  else {
    luaK_int(fs, fs->freereg, 1);
    luaK_reserveregs(fs, 1);
  }
  adjustlocalvars(ls, 3);
  forbody(ls, base, line, 1, 0);
}


static void forlist (LexState *ls, TString *indexname) {

  FuncState *fs = ls->fs;
  expdesc e;
  int nvars = 5;
  int line;
  int base = fs->freereg;

  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;
  new_localvar(ls, luaX_newstring(ls, "" "(for state)", (sizeof("(for state)")/sizeof(char)) - 1));;

  new_localvar(ls, indexname);
  while (testnext(ls, ',')) {
    new_localvar(ls, str_checkname(ls));
    nvars++;
  }
  checknext(ls, TK_IN);
  line = ls->linenumber;
  adjust_assign(ls, 4, explist(ls, &e), &e);
  adjustlocalvars(ls, 4);
  marktobeclosed(fs);
  luaK_checkstack(fs, 3);
  forbody(ls, base, line, nvars - 4, 1);
}


static void forstat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  TString *varname;
  BlockCnt bl;
  enterblock(fs, &bl, 1);
  luaX_next(ls);
  varname = str_checkname(ls);
  switch (ls->t.token) {
    case '=': fornum(ls, varname, line); break;
    case ',': case TK_IN: forlist(ls, varname); break;
    default: luaX_syntaxerror(ls, "'=' or 'in' expected");
  }
  check_match(ls, TK_END, TK_FOR, line);
  leaveblock(fs);
}


static void test_then_block (LexState *ls, int *escapelist) {

  BlockCnt bl;
  FuncState *fs = ls->fs;
  expdesc v;
  int jf;
  luaX_next(ls);
  expr(ls, &v);
  checknext(ls, TK_THEN);
  if (ls->t.token == TK_BREAK) {
    int line = ls->linenumber;
    luaK_goiffalse(ls->fs, &v);
    luaX_next(ls);
    enterblock(fs, &bl, 0);
    newgotoentry(ls, (luaS_newlstr(ls->L, "" "break", (sizeof("break")/sizeof(char))-1)), line, v.t);
    while (testnext(ls, ';')) {}
    if (block_follow(ls, 0)) {
      leaveblock(fs);
      return;
    }
    else
      jf = luaK_jump(fs);
  }
  else {
    luaK_goiftrue(ls->fs, &v);
    enterblock(fs, &bl, 0);
    jf = v.f;
  }
  statlist(ls);
  leaveblock(fs);
  if (ls->t.token == TK_ELSE ||
      ls->t.token == TK_ELSEIF)
    luaK_concat(fs, escapelist, luaK_jump(fs));
  luaK_patchtohere(fs, jf);
}


static void ifstat (LexState *ls, int line) {

  FuncState *fs = ls->fs;
  int escapelist = (-1);
  test_then_block(ls, &escapelist);
  while (ls->t.token == TK_ELSEIF)
    test_then_block(ls, &escapelist);
  if (testnext(ls, TK_ELSE))
    block(ls);
  check_match(ls, TK_END, TK_IF, line);
  luaK_patchtohere(fs, escapelist);
}


static void localfunc (LexState *ls) {
  expdesc b;
  FuncState *fs = ls->fs;
  int fvar = fs->nactvar;
  new_localvar(ls, str_checkname(ls));
  adjustlocalvars(ls, 1);
  body(ls, &b, 0, ls->linenumber);

  localdebuginfo(fs, fvar)->startpc = fs->pc;
}


static int getlocalattribute (LexState *ls) {

  if (testnext(ls, '<')) {
    const char *attr = ((str_checkname(ls))->contents);
    checknext(ls, '>');
    if (strcmp(attr, "const") == 0)
      return 1;
    else if (strcmp(attr, "close") == 0)
      return 2;
    else
      luaK_semerror(ls,
        luaO_pushfstring(ls->L, "unknown attribute '%s'", attr));
  }
  return 0;
}


static void checktoclose (FuncState *fs, int level) {
  if (level != -1) {
    marktobeclosed(fs);
    luaK_codeABCk(fs,OP_TBC,reglevel(fs, level),0,0,0);
  }
}


static void localstat (LexState *ls) {

  FuncState *fs = ls->fs;
  int toclose = -1;
  Vardesc *var;
  int vidx, kind;
  int nvars = 0;
  int nexps;
  expdesc e;
  do {
    vidx = new_localvar(ls, str_checkname(ls));
    kind = getlocalattribute(ls);
    getlocalvardesc(fs, vidx)->vd.kind = kind;
    if (kind == 2) {
      if (toclose != -1)
        luaK_semerror(ls, "multiple to-be-closed variables in local list");
      toclose = fs->nactvar + nvars;
    }
    nvars++;
  } while (testnext(ls, ','));
  if (testnext(ls, '='))
    nexps = explist(ls, &e);
  else {
    e.k = VVOID;
    nexps = 0;
  }
  var = getlocalvardesc(fs, vidx);
  if (nvars == nexps &&
      var->vd.kind == 1 &&
      luaK_exp2const(fs, &e, &var->k)) {
    var->vd.kind = 3;
    adjustlocalvars(ls, nvars - 1);
    fs->nactvar++;
  }
  else {
    adjust_assign(ls, nvars, nexps, &e);
    adjustlocalvars(ls, nvars);
  }
  checktoclose(fs, toclose);
}


static int funcname (LexState *ls, expdesc *v) {

  int ismethod = 0;
  singlevar(ls, v);
  while (ls->t.token == '.')
    fieldsel(ls, v);
  if (ls->t.token == ':') {
    ismethod = 1;
    fieldsel(ls, v);
  }
  return ismethod;
}


static void funcstat (LexState *ls, int line) {

  int ismethod;
  expdesc v, b;
  luaX_next(ls);
  ismethod = funcname(ls, &v);
  body(ls, &b, ismethod, line);
  check_readonly(ls, &v);
  luaK_storevar(ls->fs, &v, &b);
  luaK_fixline(ls->fs, line);
}


static void exprstat (LexState *ls) {

  FuncState *fs = ls->fs;
  struct LHS_assign v;
  suffixedexp(ls, &v.v);
  if (ls->t.token == '=' || ls->t.token == ',') {
    v.prev = 
            ((void *)0)
                ;
    restassign(ls, &v, 1);
  }
  else {
    Instruction *inst;
    { if (!(v.v.k == VCALL)) luaX_syntaxerror(ls, "syntax error"); };
    inst = &((fs)->f->code[(&v.v)->u.info]);
    ((*inst) = (((*inst)&(~((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))) | ((((Instruction)(1))<<((((0 + 7) + 8) + 1) + 8))&((~((~(Instruction)0)<<(8)))<<(((((0 + 7) + 8) + 1) + 8))))));
  }
}


static void retstat (LexState *ls) {

  FuncState *fs = ls->fs;
  expdesc e;
  int nret;
  int first = luaY_nvarstack(fs);
  if (block_follow(ls, 1) || ls->t.token == ';')
    nret = 0;
  else {
    nret = explist(ls, &e);
    if (((e.k) == VCALL || (e.k) == VVARARG)) {
      luaK_setreturns(fs, &e, (-1));
      if (e.k == VCALL && nret == 1 && !fs->bl->insidetbc) {
        ((((fs)->f->code[(&e)->u.info])) = (((((fs)->f->code[(&e)->u.info]))&(~((~((~(Instruction)0)<<(7)))<<(0)))) | ((((Instruction)(OP_TAILCALL))<<0)&((~((~(Instruction)0)<<(7)))<<(0)))));
        ((void)0);
      }
      nret = (-1);
    }
    else {
      if (nret == 1)
        first = luaK_exp2anyreg(fs, &e);
      else {
        luaK_exp2nextreg(fs, &e);
        ((void)0);
      }
    }
  }
  luaK_ret(fs, first, nret);
  testnext(ls, ';');
}


static void statement (LexState *ls) {
  int line = ls->linenumber;
  luaE_incCstack(ls->L);
  switch (ls->t.token) {
    case ';': {
      luaX_next(ls);
      break;
    }
    case TK_IF: {
      ifstat(ls, line);
      break;
    }
    case TK_WHILE: {
      whilestat(ls, line);
      break;
    }
    case TK_DO: {
      luaX_next(ls);
      block(ls);
      check_match(ls, TK_END, TK_DO, line);
      break;
    }
    case TK_FOR: {
      forstat(ls, line);
      break;
    }
    case TK_REPEAT: {
      repeatstat(ls, line);
      break;
    }
    case TK_FUNCTION: {
      funcstat(ls, line);
      break;
    }
    case TK_LOCAL: {
      luaX_next(ls);
      if (testnext(ls, TK_FUNCTION))
        localfunc(ls);
      else
        localstat(ls);
      break;
    }
    case TK_DBCOLON: {
      luaX_next(ls);
      labelstat(ls, str_checkname(ls), line);
      break;
    }
    case TK_RETURN: {
      luaX_next(ls);
      retstat(ls);
      break;
    }
    case TK_BREAK: {
      breakstat(ls);
      break;
    }
    case TK_GOTO: {
      luaX_next(ls);
      gotostat(ls);
      break;
    }
    default: {
      exprstat(ls);
      break;
    }
  }
  ((void)0)
                                                       ;
  ls->fs->freereg = luaY_nvarstack(ls->fs);
  ((ls)->L->nCcalls--);
}
static void mainfunc (LexState *ls, FuncState *fs) {
  BlockCnt bl;
  Upvaldesc *env;
  open_func(ls, fs, &bl);
  setvararg(fs, 0);
  env = allocupvalue(fs);
  env->instack = 1;
  env->idx = 0;
  env->kind = 0;
  env->name = ls->envn;
  ( ((((fs->f)->marked) & ((1<<(5)))) && (((env->name)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(ls->L,(&(((union GCUnion *)((fs->f)))->gc)),(&(((union GCUnion *)((env->name)))->gc))) : ((void)((0))));
  luaX_next(ls);
  statlist(ls);
  check(ls, TK_EOS);
  close_func(ls);
}


LClosure *luaY_parser (lua_State *L, ZIO *z, Mbuffer *buff,
                       Dyndata *dyd, const char *name, int firstchar) {
  LexState lexstate;
  FuncState funcstate;
  LClosure *cl = luaF_newLclosure(L, 1);
  { TValue *io = ((&(L->top.p)->val)); LClosure *x_ = (cl); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((6) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  luaD_inctop(L);
  lexstate.h = luaH_new(L);
  { TValue *io = ((&(L->top.p)->val)); Table *x_ = (lexstate.h); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  luaD_inctop(L);
  funcstate.f = cl->p = luaF_newproto(L);
  ( ((((cl)->marked) & ((1<<(5)))) && (((cl->p)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((cl)))->gc)),(&(((union GCUnion *)((cl->p)))->gc))) : ((void)((0))));
  funcstate.f->source = luaS_new(L, name);
  ( ((((funcstate.f)->marked) & ((1<<(5)))) && (((funcstate.f->source)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((funcstate.f)))->gc)),(&(((union GCUnion *)((funcstate.f->source)))->gc))) : ((void)((0))));
  lexstate.buff = buff;
  lexstate.dyd = dyd;
  dyd->actvar.n = dyd->gt.n = dyd->label.n = 0;
  luaX_setinput(L, &lexstate, z, funcstate.f->source, firstchar);
  mainfunc(&lexstate, &funcstate);
  ((void)0);

  ((void)0);
  L->top.p--;
  return cl;
}
typedef struct LX {
  lu_byte extra_[8];
  lua_State l;
} LX;





typedef struct LG {
  LX l;
  global_State g;
} LG;















struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;





  long int __tm_gmtoff;
  const char *__tm_zone;

};







struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };
struct sigevent;




extern clock_t clock (void) ;



extern time_t time (time_t *__timer) ;


extern double difftime (time_t __time1, time_t __time0)
     ;


extern time_t mktime (struct tm *__tp) ;
extern size_t strftime (char * __s, size_t __maxsize,
   const char * __format,
   const struct tm * __tp)
   ;




extern char *strptime (const char * __s,
         const char * __fmt, struct tm *__tp)
     ;
extern struct tm *gmtime (const time_t *__timer) ;



extern struct tm *localtime (const time_t *__timer) ;
extern struct tm *gmtime_r (const time_t * __timer,
       struct tm * __tp) ;



extern struct tm *localtime_r (const time_t * __timer,
          struct tm * __tp) ;
extern char *asctime (const struct tm *__tp) ;



extern char *ctime (const time_t *__timer) ;
extern char *asctime_r (const struct tm * __tp,
   char * __buf) ;



extern char *ctime_r (const time_t * __timer,
        char * __buf) ;
extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) ;



extern int daylight;
extern long int timezone;
extern int nanosleep (const struct timespec *__requested_time,
        struct timespec *__remaining);


extern int clock_getres (clockid_t __clock_id, struct timespec *__res) ;


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp)
     ;


extern int clock_settime (clockid_t __clock_id, const struct timespec *__tp)
     ;
extern int clock_nanosleep (clockid_t __clock_id, int __flags,
       const struct timespec *__req,
       struct timespec *__rem);
extern int clock_getcpuclockid (pid_t __pid, clockid_t *__clock_id) ;




extern int timer_create (clockid_t __clock_id,
    struct sigevent * __evp,
    timer_t * __timerid) ;


extern int timer_delete (timer_t __timerid) ;



extern int timer_settime (timer_t __timerid, int __flags,
     const struct itimerspec * __value,
     struct itimerspec * __ovalue) ;


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     ;
extern int timer_getoverrun (timer_t __timerid) ;






extern int timespec_get (struct timespec *__ts, int __base)
     ;
extern int getdate_err;
extern struct tm *getdate (const char *__string);


static unsigned int luai_makeseed (lua_State *L) {
  char buff[24];
  unsigned int h = ((unsigned int)((time(
                  ((void *)0)
                  ))));
  int p = 0;
  { size_t t = ((size_t)((L))); memcpy(buff + p, &t, sizeof(t)); p += sizeof(t); };
  { size_t t = ((size_t)((&h))); memcpy(buff + p, &t, sizeof(t)); p += sizeof(t); };
  { size_t t = ((size_t)((&lua_newstate))); memcpy(buff + p, &t, sizeof(t)); p += sizeof(t); };
  ((void)0);
  return luaS_hash(buff, p, h);
}
void luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = ((lu_mem)((g)->totalbytes + (g)->GCdebt));
  ((void)0);
  if (debt < tb - ((l_mem)(((lu_mem)(~(lu_mem)0)) >> 1)))
    debt = tb - ((l_mem)(((lu_mem)(~(lu_mem)0)) >> 1));
  g->totalbytes = tb - debt;
  g->GCdebt = debt;
}


extern int lua_setcstacklimit (lua_State *L, unsigned int limit) {
  ((void)(L)); ((void)(limit));
  return 200;
}


CallInfo *luaE_extendCI (lua_State *L) {
  CallInfo *ci;
  ((void)0);
  ci = ((CallInfo*)(luaM_malloc_(L, sizeof(CallInfo), 0)));
  ((void)0);
  L->ci->next = ci;
  ci->previous = L->ci;
  ci->next = 
            ((void *)0)
                ;
  ci->u.l.trap = 0;
  L->nci++;
  return ci;
}





void luaE_freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = 
            ((void *)0)
                ;
  while ((ci = next) != 
                       ((void *)0)
                           ) {
    next = ci->next;
    luaM_free_(L, (ci), sizeof(*(ci)));
    L->nci--;
  }
}






void luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci->next;
  CallInfo *next;
  if (ci == 
           ((void *)0)
               )
    return;
  while ((next = ci->next) != 
                             ((void *)0)
                                 ) {
    CallInfo *next2 = next->next;
    ci->next = next2;
    L->nci--;
    luaM_free_(L, (next), sizeof(*(next)));
    if (next2 == 
                ((void *)0)
                    )
      break;
    else {
      next2->previous = ci;
      ci = next2;
    }
  }
}
void luaE_checkcstack (lua_State *L) {
  if (((L)->nCcalls & 0xffff) == 200)
    luaG_runerror(L, "C stack overflow");
  else if (((L)->nCcalls & 0xffff) >= (200 / 10 * 11))
    luaD_throw(L, 5);
}


extern void luaE_incCstack (lua_State *L) {
  L->nCcalls++;
  if ((((L)->nCcalls & 0xffff) >= 200))
    luaE_checkcstack(L);
}


static void stack_init (lua_State *L1, lua_State *L) {
  int i; CallInfo *ci;

  L1->stack.p = ((StackValue*)(luaM_malloc_(L, ((2*20) + 5)*sizeof(StackValue), 0)));
  L1->tbclist.p = L1->stack.p;
  for (i = 0; i < (2*20) + 5; i++)
    (((&(L1->stack.p + i)->val))->tt_=(((0) | ((0) << 4))));
  L1->top.p = L1->stack.p;
  L1->stack_last.p = L1->stack.p + (2*20);

  ci = &L1->base_ci;
  ci->next = ci->previous = 
                           ((void *)0)
                               ;
  ci->callstatus = (1<<1);
  ci->func.p = L1->top.p;
  ci->u.c.k = 
             ((void *)0)
                 ;
  ci->nresults = 0;
  (((&(L1->top.p)->val))->tt_=(((0) | ((0) << 4))));
  L1->top.p++;
  ci->top.p = L1->top.p + 20;
  L1->ci = ci;
}


static void freestack (lua_State *L) {
  if (L->stack.p == 
                   ((void *)0)
                       )
    return;
  L->ci = &L->base_ci;
  luaE_freeCI(L);
  ((void)0);
  luaM_free_(L, (L->stack.p), (((int)(((L)->stack_last.p - (L)->stack.p))) + 5)*sizeof(*(L->stack.p)));
}





static void init_registry (lua_State *L, global_State *g) {

  Table *registry = luaH_new(L);
  { TValue *io = (&g->l_registry); Table *x_ = (registry); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  luaH_resize(L, registry, 2, 0);

  { TValue *io = (&registry->array[1 - 1]); lua_State *x_ = (L); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((8) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };

  { TValue *io = (&registry->array[1]); Table *x_ = (luaH_new(L)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
}





static void f_luaopen (lua_State *L, void *ud) {
  global_State *g = (L->l_G);
  ((void)(ud));
  stack_init(L, L);
  init_registry(L, g);
  luaS_init(L);
  luaT_init(L);
  luaX_init(L);
  g->gcstp = 0;
  ((&g->nilvalue)->tt_=(((0) | ((0) << 4))));
  ((void)L);
}






static void preinit_thread (lua_State *L, global_State *g) {
  (L->l_G) = g;
  L->stack.p = 
              ((void *)0)
                  ;
  L->ci = 
         ((void *)0)
             ;
  L->nci = 0;
  L->twups = L;
  L->nCcalls = 0;
  L->errorJmp = 
               ((void *)0)
                   ;
  L->hook = 
           ((void *)0)
               ;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  (L->hookcount = L->basehookcount);
  L->openupval = 
                ((void *)0)
                    ;
  L->status = 0;
  L->errfunc = 0;
  L->oldpc = 0;
}


static void close_state (lua_State *L) {
  global_State *g = (L->l_G);
  if (!(((((((&g->nilvalue))->tt_)) & 0x0F)) == (0)))
    luaC_freeallobjects(L);
  else {
    L->ci = &L->base_ci;
    luaD_closeprotected(L, 1, 0);
    luaC_freeallobjects(L);
    ((void)L);
  }
  luaM_free_(L, ((L->l_G)->strt.hash), ((L->l_G)->strt.size)*sizeof(*((L->l_G)->strt.hash)));
  freestack(L);
  ((void)0);
  (*g->frealloc)(g->ud, (((LX *)(((lu_byte *)((L))) - 
                       ((long)&((LX
                       *)0)->l
                       )
                       ))), sizeof(LG), 0);
}


extern lua_State *lua_newthread (lua_State *L) {
  global_State *g = (L->l_G);
  GCObject *o;
  lua_State *L1;
  ((void) 0);
  { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); };

  o = luaC_newobjdt(L, 8, sizeof(LX), 
                                               ((long)&((LX
                                               *)0)->l
                                               )
                                                              );
  L1 = (&((((union GCUnion *)((o))))->th));

  { TValue *io = ((&(L->top.p)->val)); lua_State *x_ = (L1); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((8) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  {L->top.p++; ((void)L, ((void)0));};
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  (L1->hookcount = L1->basehookcount);

  memcpy(((void *)((char *)(L1) - (sizeof(void *)))), ((void *)((char *)(g->mainthread) - (sizeof(void *)))),
         (sizeof(void *)));
  ((void)L);
  stack_init(L1, L);
  ((void) 0);
  return L1;
}


void luaE_freethread (lua_State *L, lua_State *L1) {
  LX *l = (((LX *)(((lu_byte *)((L1))) - 
         ((long)&((LX
         *)0)->l
         )
         )));
  luaF_closeupval(L1, L1->stack.p);
  ((void)0);
  ((void)L);
  freestack(L1);
  luaM_free_(L, (l), sizeof(*(l)));
}


int luaE_resetthread (lua_State *L, int status) {
  CallInfo *ci = L->ci = &L->base_ci;
  (((&(L->stack.p)->val))->tt_=(((0) | ((0) << 4))));
  ci->func.p = L->stack.p;
  ci->callstatus = (1<<1);
  if (status == 1)
    status = 0;
  L->status = 0;
  status = luaD_closeprotected(L, 1, status);
  if (status != 0)
    luaD_seterrorobj(L, status, L->stack.p + 1);
  else
    L->top.p = L->stack.p + 1;
  ci->top.p = L->top.p + 20;
  luaD_reallocstack(L, ((int)((ci->top.p - L->stack.p))), 0);
  return status;
}


extern int lua_closethread (lua_State *L, lua_State *from) {
  int status;
  ((void) 0);
  L->nCcalls = (from) ? ((from)->nCcalls & 0xffff) : 0;
  status = luaE_resetthread(L, L->status);
  ((void) 0);
  return status;
}





extern int lua_resetthread (lua_State *L) {
  return lua_closethread(L, 
                           ((void *)0)
                               );
}


extern lua_State *lua_newstate (lua_Alloc f, void *ud) {
  int i;
  lua_State *L;
  global_State *g;
  LG *l = ((LG *)((*f)(ud, 
         ((void *)0)
         , 8, sizeof(LG))));
  if (l == 
          ((void *)0)
              ) return 
                       ((void *)0)
                           ;
  L = &l->l.l;
  g = &l->g;
  L->tt = ((8) | ((0) << 4));
  g->currentwhite = (1<<(3));
  L->marked = ((lu_byte)(((g)->currentwhite & ((1<<(3)) | (1<<(4))))));
  preinit_thread(L, g);
  g->allgc = (&(((union GCUnion *)((L)))->gc));
  L->next = 
           ((void *)0)
               ;
  ((L)->nCcalls += 0x10000);
  g->frealloc = f;
  g->ud = ud;
  g->warnf = 
            ((void *)0)
                ;
  g->ud_warn = 
              ((void *)0)
                  ;
  g->mainthread = L;
  g->seed = luai_makeseed(L);
  g->gcstp = 2;
  g->strt.size = g->strt.nuse = 0;
  g->strt.hash = 
                ((void *)0)
                    ;
  ((&g->l_registry)->tt_=(((0) | ((0) << 4))));
  g->panic = 
            ((void *)0)
                ;
  g->gcstate = 8;
  g->gckind = 0;
  g->gcstopem = 0;
  g->gcemergency = 0;
  g->finobj = g->tobefnz = g->fixedgc = 
                                       ((void *)0)
                                           ;
  g->firstold1 = g->survival = g->old1 = g->reallyold = 
                                                       ((void *)0)
                                                           ;
  g->finobjsur = g->finobjold1 = g->finobjrold = 
                                                ((void *)0)
                                                    ;
  g->sweepgc = 
              ((void *)0)
                  ;
  g->gray = g->grayagain = 
                          ((void *)0)
                              ;
  g->weak = g->ephemeron = g->allweak = 
                                       ((void *)0)
                                           ;
  g->twups = 
            ((void *)0)
                ;
  g->totalbytes = sizeof(LG);
  g->GCdebt = 0;
  g->lastatomic = 0;
  { TValue *io=(&g->nilvalue); ((io)->value_).i=(0); ((io)->tt_=(((3) | ((0) << 4)))); };
  ((g->gcpause) = (200) / 4);
  ((g->gcstepmul) = (100) / 4);
  g->gcstepsize = 13;
  ((g->genmajormul) = (100) / 4);
  g->genminormul = 20;
  for (i=0; i < 9; i++) g->mt[i] = 
                                            ((void *)0)
                                                ;
  if (luaD_rawrunprotected(L, f_luaopen, 
                                        ((void *)0)
                                            ) != 0) {

    close_state(L);
    L = 
       ((void *)0)
           ;
  }
  return L;
}


extern void lua_close (lua_State *L) {
  ((void) 0);
  L = (L->l_G)->mainthread;
  close_state(L);
}


void luaE_warning (lua_State *L, const char *msg, int tocont) {
  lua_WarnFunction wf = (L->l_G)->warnf;
  if (wf != 
           ((void *)0)
               )
    wf((L->l_G)->ud_warn, msg, tocont);
}





void luaE_warnerror (lua_State *L, const char *where) {
  TValue *errobj = (&(L->top.p - 1)->val);
  const char *msg = ((((((((errobj))->tt_)) & 0x0F)) == (4)))
                  ? ((((&((((union GCUnion *)((((errobj)->value_).gc))))->ts))))->contents)
                  : "error object is not a string";

  luaE_warning(L, "error in ", 1);
  luaE_warning(L, where, 1);
  luaE_warning(L, " (", 1);
  luaE_warning(L, msg, 1);
  luaE_warning(L, ")", 0);
}
int luaS_eqlngstr (TString *a, TString *b) {
  size_t len = a->u.lnglen;
  ((void)0);
  return (a == b) ||
    ((len == b->u.lnglen) &&
     (memcmp(((a)->contents), ((b)->contents), len) == 0));
}


unsigned int luaS_hash (const char *str, size_t l, unsigned int seed) {
  unsigned int h = seed ^ ((unsigned int)((l)));
  for (; l > 0; l--)
    h ^= ((h<<5) + (h>>2) + ((lu_byte)((str[l - 1]))));
  return h;
}


unsigned int luaS_hashlongstr (TString *ts) {
  ((void)0);
  if (ts->extra == 0) {
    size_t len = ts->u.lnglen;
    ts->hash = luaS_hash(((ts)->contents), len, ts->hash);
    ts->extra = 1;
  }
  return ts->hash;
}


static void tablerehash (TString **vect, int osize, int nsize) {
  int i;
  for (i = osize; i < nsize; i++)
    vect[i] = 
             ((void *)0)
                 ;
  for (i = 0; i < osize; i++) {
    TString *p = vect[i];
    vect[i] = 
             ((void *)0)
                 ;
    while (p) {
      TString *hnext = p->u.hnext;
      unsigned int h = (((((int)(((p->hash) & ((nsize)-1)))))));
      p->u.hnext = vect[h];
      vect[h] = p;
      p = hnext;
    }
  }
}







void luaS_resize (lua_State *L, int nsize) {
  stringtable *tb = &(L->l_G)->strt;
  int osize = tb->size;
  TString **newvect;
  if (nsize < osize)
    tablerehash(tb->hash, osize, nsize);
  newvect = (((TString* *)(luaM_realloc_(L, tb->hash, ((size_t)((osize))) * sizeof(TString*), ((size_t)((nsize))) * sizeof(TString*)))));
  if ((newvect == 
     ((void *)0)
     )) {
    if (nsize < osize)
      tablerehash(tb->hash, nsize, osize);

  }
  else {
    tb->hash = newvect;
    tb->size = nsize;
    if (nsize > osize)
      tablerehash(newvect, osize, nsize);
  }
}






void luaS_clearcache (global_State *g) {
  int i, j;
  for (i = 0; i < 53; i++)
    for (j = 0; j < 2; j++) {
      if ((((g->strcache[i][j])->marked) & (((1<<(3)) | (1<<(4))))))
        g->strcache[i][j] = g->memerrmsg;
    }
}





void luaS_init (lua_State *L) {
  global_State *g = (L->l_G);
  int i, j;
  stringtable *tb = &(L->l_G)->strt;
  tb->hash = ((TString**)(luaM_malloc_(L, (128)*sizeof(TString*), 0)));
  tablerehash(tb->hash, 0, 128);
  tb->size = 128;

  g->memerrmsg = (luaS_newlstr(L, "" "not enough memory", (sizeof("not enough memory")/sizeof(char))-1));
  luaC_fix(L, (&(((union GCUnion *)((g->memerrmsg)))->gc)));
  for (i = 0; i < 53; i++)
    for (j = 0; j < 2; j++)
      g->strcache[i][j] = g->memerrmsg;
}






static TString *createstrobj (lua_State *L, size_t l, int tag, unsigned int h) {
  TString *ts;
  GCObject *o;
  size_t totalsize;
  totalsize = (
             ((long)&((TString
             *)0)->contents
             ) 
             + ((l) + 1) * sizeof(char));
  o = luaC_newobj(L, tag, totalsize);
  ts = (&((((union GCUnion *)((o))))->ts));
  ts->hash = h;
  ts->extra = 0;
  ((ts)->contents)[l] = '\0';
  return ts;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  TString *ts = createstrobj(L, l, ((4) | ((1) << 4)), (L->l_G)->seed);
  ts->u.lnglen = l;
  return ts;
}


void luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &(L->l_G)->strt;
  TString **p = &tb->hash[(((((int)(((ts->hash) & ((tb->size)-1)))))))];
  while (*p != ts)
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;
  tb->nuse--;
}


static void growstrtab (lua_State *L, stringtable *tb) {
  if ((tb->nuse == 0x7fffffff
     )) {
    luaC_fullgc(L, 1);
    if (tb->nuse == 0x7fffffff
                          )
      luaD_throw(L, 4);
  }
  if (tb->size <= ((int)((((((size_t)((0x7fffffff
                 ))) <= ((size_t)(~(size_t)0))/sizeof(TString*)) ? (0x7fffffff
                 ) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(TString*))))))))) / 2)
    luaS_resize(L, tb->size * 2);
}





static TString *internshrstr (lua_State *L, const char *str, size_t l) {
  TString *ts;
  global_State *g = (L->l_G);
  stringtable *tb = &g->strt;
  unsigned int h = luaS_hash(str, l, g->seed);
  TString **list = &tb->hash[(((((int)(((h) & ((tb->size)-1)))))))];
  ((void)0);
  for (ts = *list; ts != 
                        ((void *)0)
                            ; ts = ts->u.hnext) {
    if (l == ts->shrlen && (memcmp(str, ((ts)->contents), l * sizeof(char)) == 0)) {

      if ((((ts)->marked) & (((g)->currentwhite ^ ((1<<(3)) | (1<<(4)))))))
        ((ts)->marked ^= ((1<<(3)) | (1<<(4))));
      return ts;
    }
  }

  if (tb->nuse >= tb->size) {
    growstrtab(L, tb);
    list = &tb->hash[(((((int)(((h) & ((tb->size)-1)))))))];
  }
  ts = createstrobj(L, l, ((4) | ((0) << 4)), h);
  memcpy(((ts)->contents), str, l * sizeof(char));
  ts->shrlen = ((lu_byte)((l)));
  ts->u.hnext = *list;
  *list = ts;
  tb->nuse++;
  return ts;
}





TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  if (l <= 40)
    return internshrstr(L, str, l);
  else {
    TString *ts;
    if ((l >= ((sizeof(size_t) < sizeof(lua_Integer) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffffffffffffLL
       )) - sizeof(TString))/sizeof(char)))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(((ts)->contents), str, l * sizeof(char));
    return ts;
  }
}
TString *luaS_new (lua_State *L, const char *str) {
  unsigned int i = ((unsigned int)((uintptr_t)(str) & 
                  (0x7fffffff * 2U + 1U)
                  )) % 53;
  int j;
  TString **p = (L->l_G)->strcache[i];
  for (j = 0; j < 2; j++) {
    if (strcmp(str, ((p[j])->contents)) == 0)
      return p[j];
  }

  for (j = 2 - 1; j > 0; j--)
    p[j] = p[j - 1];

  p[0] = luaS_newlstr(L, str, strlen(str));
  return p[0];
}


Udata *luaS_newudata (lua_State *L, size_t s, int nuvalue) {
  Udata *u;
  int i;
  GCObject *o;
  if ((s > (sizeof(size_t) < sizeof(lua_Integer) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffffffffffffLL
     )) - ((nuvalue) == 0 ? 
     ((long)&((Udata0
     *)0)->bindata
     ) 
     : 
     ((long)&((Udata
     *)0)->uv
     ) 
     + (sizeof(UValue) * (nuvalue)))))
    luaM_toobig(L);
  o = luaC_newobj(L, ((7) | ((0) << 4)), (((nuvalue) == 0 ? 
                                   ((long)&((Udata0
                                   *)0)->bindata
                                   ) 
                                   : 
                                   ((long)&((Udata
                                   *)0)->uv
                                   ) 
                                   + (sizeof(UValue) * (nuvalue))) + (s)));
  u = (&((((union GCUnion *)((o))))->u));
  u->len = s;
  u->nuvalue = nuvalue;
  u->metatable = 
                ((void *)0)
                    ;
  for (i = 0; i < nuvalue; i++)
    ((&u->uv[i].uv)->tt_=(((0) | ((0) << 4))));
  return u;
}
static const Node dummynode_ = {
  {{
   ((void *)0)
       }, ((0) | ((1) << 4)),
   ((0) | ((0) << 4)), 0, {
                ((void *)0)
                    }}
};


static const TValue absentkey = {{
                                ((void *)0)
                                }, ((0) | ((2) << 4))};
static Node *hashint (const Table *t, lua_Integer i) {
  lua_Unsigned ui = ((lua_Unsigned)(i));
  if (ui <= ((unsigned int)((0x7fffffff
           ))))
    return ((&(t)->node[((((int)((ui)))) % ((((1<<((t)->lsizenode)))-1)|1))]));
  else
    return ((&(t)->node[((ui) % ((((1<<((t)->lsizenode)))-1)|1))]));
}
static int l_hashfloat (lua_Number n) {
  int i;
  lua_Integer ni;
  n = frexp(n, &i) * -((lua_Number)((
                               (-0x7fffffff - 1)
                               )));
  if (!((n) >= (double)(
      (-0x7fffffffffffffffLL - 1LL)
      ) && (n) < -(double)(
      (-0x7fffffffffffffffLL - 1LL)
      ) && (*(&ni) = (long long)(n), 1))) {
    ((void)0);
    return 0;
  }
  else {
    unsigned int u = ((unsigned int)((i))) + ((unsigned int)((ni)));
    return ((int)((u <= ((unsigned int)((0x7fffffff
          ))) ? u : ~u)));
  }
}







static Node *mainpositionTV (const Table *t, const TValue *key) {
  switch (((((key)->tt_)) & 0x3F)) {
    case ((3) | ((0) << 4)): {
      lua_Integer i = (((key)->value_).i);
      return hashint(t, i);
    }
    case ((3) | ((1) << 4)): {
      lua_Number n = (((key)->value_).n);
      return ((&(t)->node[((l_hashfloat(n)) % ((((1<<((t)->lsizenode)))-1)|1))]));
    }
    case ((4) | ((0) << 4)): {
      TString *ts = ((&((((union GCUnion *)((((key)->value_).gc))))->ts)));
      return ((&(t)->node[(((((int)(((((ts)->hash)) & ((((1<<((t)->lsizenode))))-1)))))))]));
    }
    case ((4) | ((1) << 4)): {
      TString *ts = ((&((((union GCUnion *)((((key)->value_).gc))))->ts)));
      return ((&(t)->node[(((((int)((((luaS_hashlongstr(ts))) & ((((1<<((t)->lsizenode))))-1)))))))]));
    }
    case ((1) | ((0) << 4)):
      return ((&(t)->node[(((((int)((((0)) & ((((1<<((t)->lsizenode))))-1)))))))]));
    case ((1) | ((1) << 4)):
      return ((&(t)->node[(((((int)((((1)) & ((((1<<((t)->lsizenode))))-1)))))))]));
    case ((2) | ((0) << 4)): {
      void *p = (((key)->value_).p);
      return ((&(t)->node[((((unsigned int)((uintptr_t)(p) & 
            (0x7fffffff * 2U + 1U)
            ))) % ((((1<<((t)->lsizenode)))-1)|1))]));
    }
    case ((6) | ((1) << 4)): {
      lua_CFunction f = (((key)->value_).f);
      return ((&(t)->node[((((unsigned int)((uintptr_t)(f) & 
            (0x7fffffff * 2U + 1U)
            ))) % ((((1<<((t)->lsizenode)))-1)|1))]));
    }
    default: {
      GCObject *o = (((key)->value_).gc);
      return ((&(t)->node[((((unsigned int)((uintptr_t)(o) & 
            (0x7fffffff * 2U + 1U)
            ))) % ((((1<<((t)->lsizenode)))-1)|1))]));
    }
  }
}


static inline Node *mainpositionfromnode (const Table *t, Node *nd) {
  TValue key;
  { TValue *io_=(&key); const Node *n_=(nd); io_->value_ = n_->u.key_val; io_->tt_ = n_->u.key_tt; ((void)((lua_State *)(
 ((void *)0)
 )), ((void)0)); };
  return mainpositionTV(t, &key);
}
static int equalkey (const TValue *k1, const Node *n2, int deadok) {
  if ((((k1)->tt_) != ((n2)->u.key_tt)) &&
       !(deadok && (((n2)->u.key_tt) == (9 +2)) && (((k1)->tt_) & (1 << 6))))
   return 0;
  switch (((n2)->u.key_tt)) {
    case ((0) | ((0) << 4)): case ((1) | ((0) << 4)): case ((1) | ((1) << 4)):
      return 1;
    case ((3) | ((0) << 4)):
      return ((((k1)->value_).i) == (((n2)->u.key_val).i));
    case ((3) | ((1) << 4)):
      return (((((k1)->value_).n))==(((((n2)->u.key_val)).n)));
    case ((2) | ((0) << 4)):
      return (((k1)->value_).p) == ((((n2)->u.key_val)).p);
    case ((6) | ((1) << 4)):
      return (((k1)->value_).f) == ((((n2)->u.key_val)).f);
    case ((((4) | ((1) << 4))) | (1 << 6)):
      return luaS_eqlngstr(((&((((union GCUnion *)((((k1)->value_).gc))))->ts))), ((&((((union GCUnion *)((((n2)->u.key_val).gc))))->ts))));
    default:
      return (((k1)->value_).gc) == ((((n2)->u.key_val)).gc);
  }
}
extern unsigned int luaH_realasize (const Table *t) {
  if (((!((t)->flags & (1 << 7))) || ((((t)->alimit) & (((t)->alimit) - 1)) == 0)))
    return t->alimit;
  else {
    unsigned int size = t->alimit;

    size |= (size >> 1);
    size |= (size >> 2);
    size |= (size >> 4);
    size |= (size >> 8);

    size |= (size >> 16);




    size++;
    ((void)0);
    return size;
  }
}







static int ispow2realasize (const Table *t) {
  return (!(!((t)->flags & (1 << 7))) || (((t->alimit) & ((t->alimit) - 1)) == 0));
}


static unsigned int setlimittosize (Table *t) {
  t->alimit = luaH_realasize(t);
  ((t)->flags &= ((lu_byte)((~(1 << 7)))));
  return t->alimit;
}
static const TValue *getgeneric (Table *t, const TValue *key, int deadok) {
  Node *n = mainpositionTV(t, key);
  for (;;) {
    if (equalkey(key, n, deadok))
      return (&(n)->i_val);
    else {
      int nx = ((n)->u.next);
      if (nx == 0)
        return &absentkey;
      n += nx;
    }
  }
}






static unsigned int arrayindex (lua_Integer k) {
  if (((lua_Unsigned)(k)) - 1u < ((((size_t)((1u << ((int)((sizeof(int) * 8 
                         - 1)))))) <= ((size_t)(~(size_t)0))/sizeof(TValue)) ? (1u << ((int)((sizeof(int) * 8 
                         - 1)))) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(TValue)))))))
    return ((unsigned int)((k)));
  else
    return 0;
}







static unsigned int findindex (lua_State *L, Table *t, TValue *key,
                               unsigned int asize) {
  unsigned int i;
  if ((((((((key))->tt_)) & 0x0F)) == (0))) return 0;
  i = ((((key))->tt_) == (((3) | ((0) << 4)))) ? arrayindex((((key)->value_).i)) : 0;
  if (i - 1u < asize)
    return i;
  else {
    const TValue *n = getgeneric(t, key, 1);
    if ((((((n))->tt_) == (((0) | ((2) << 4))))))
      luaG_runerror(L, "invalid key to 'next'");
    i = ((int)((((Node *)((n))) - (&(t)->node[0]))));

    return (i + 1) + asize;
  }
}


int luaH_next (lua_State *L, Table *t, StkId key) {
  unsigned int asize = luaH_realasize(t);
  unsigned int i = findindex(L, t, (&(key)->val), asize);
  for (; i < asize; i++) {
    if (!(((((((&t->array[i]))->tt_)) & 0x0F)) == (0))) {
      { TValue *io=((&(key)->val)); ((io)->value_).i=(i + 1); ((io)->tt_=(((3) | ((0) << 4)))); };
      { TValue *io1=((&(key + 1)->val)); const TValue *io2=(&t->array[i]); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      return 1;
    }
  }
  for (i -= asize; ((int)((i))) < ((1<<((t)->lsizenode))); i++) {
    if (!((((((((&((&(t)->node[i]))->i_val)))->tt_)) & 0x0F)) == (0))) {
      Node *n = (&(t)->node[i]);
      { TValue *io_=((&(key)->val)); const Node *n_=(n); io_->value_ = n_->u.key_val; io_->tt_ = n_->u.key_tt; ((void)L, ((void)0)); };
      { TValue *io1=((&(key + 1)->val)); const TValue *io2=((&(n)->i_val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      return 1;
    }
  }
  return 0;
}


static void freehash (lua_State *L, Table *t) {
  if (!((t)->lastfree == 
      ((void *)0)
      ))
    luaM_free_(L, (t->node), (((size_t)((((1<<((t)->lsizenode)))))))*sizeof(*(t->node)));
}
static unsigned int computesizes (unsigned int nums[], unsigned int *pna) {
  int i;
  unsigned int twotoi;
  unsigned int a = 0;
  unsigned int na = 0;
  unsigned int optimal = 0;

  for (i = 0, twotoi = 1;
       twotoi > 0 && *pna > twotoi / 2;
       i++, twotoi *= 2) {
    a += nums[i];
    if (a > twotoi/2) {
      optimal = twotoi;
      na = a;
    }
  }
  ((void)0);
  *pna = na;
  return optimal;
}


static int countint (lua_Integer key, unsigned int *nums) {
  unsigned int k = arrayindex(key);
  if (k != 0) {
    nums[luaO_ceillog2(k)]++;
    return 1;
  }
  else
    return 0;
}







static unsigned int numusearray (const Table *t, unsigned int *nums) {
  int lg;
  unsigned int ttlg;
  unsigned int ause = 0;
  unsigned int i = 1;
  unsigned int asize = (t->alimit);

  for (lg = 0, ttlg = 1; lg <= ((int)((sizeof(int) * 8 
                              - 1))); lg++, ttlg *= 2) {
    unsigned int lc = 0;
    unsigned int lim = ttlg;
    if (lim > asize) {
      lim = asize;
      if (i > lim)
        break;
    }

    for (; i <= lim; i++) {
      if (!(((((((&t->array[i-1]))->tt_)) & 0x0F)) == (0)))
        lc++;
    }
    nums[lg] += lc;
    ause += lc;
  }
  return ause;
}


static int numusehash (const Table *t, unsigned int *nums, unsigned int *pna) {
  int totaluse = 0;
  int ause = 0;
  int i = ((1<<((t)->lsizenode)));
  while (i--) {
    Node *n = &t->node[i];
    if (!((((((((&(n)->i_val)))->tt_)) & 0x0F)) == (0))) {
      if ((((n)->u.key_tt) == ((3) | ((0) << 4))))
        ause += countint((((n)->u.key_val).i), nums);
      totaluse++;
    }
  }
  *pna += ause;
  return totaluse;
}
static void setnodevector (lua_State *L, Table *t, unsigned int size) {
  if (size == 0) {
    t->node = ((Node *)((&dummynode_)));
    t->lsizenode = 0;
    t->lastfree = 
                 ((void *)0)
                     ;
  }
  else {
    int i;
    int lsize = luaO_ceillog2(size);
    if (lsize > (((int)((sizeof(int) * 8 
               - 1))) - 1) || (1u << lsize) > ((((size_t)((1u << (((int)((sizeof(int) * 8 
                                           - 1))) - 1)))) <= ((size_t)(~(size_t)0))/sizeof(Node)) ? (1u << (((int)((sizeof(int) * 8 
                                           - 1))) - 1)) : ((unsigned int)(((((size_t)(~(size_t)0))/sizeof(Node)))))))
      luaG_runerror(L, "table overflow");
    size = (1<<(lsize));
    t->node = ((Node*)(luaM_malloc_(L, (size)*sizeof(Node), 0)));
    for (i = 0; i < ((int)((size))); i++) {
      Node *n = (&(t)->node[i]);
      ((n)->u.next) = 0;
      (((n)->u.key_tt) = 0);
      (((&(n)->i_val))->tt_=(((0) | ((1) << 4))));
    }
    t->lsizenode = ((lu_byte)((lsize)));
    t->lastfree = (&(t)->node[size]);
  }
}





static void reinsert (lua_State *L, Table *ot, Table *t) {
  int j;
  int size = ((1<<((ot)->lsizenode)));
  for (j = 0; j < size; j++) {
    Node *old = (&(ot)->node[j]);
    if (!((((((((&(old)->i_val)))->tt_)) & 0x0F)) == (0))) {


      TValue k;
      { TValue *io_=(&k); const Node *n_=(old); io_->value_ = n_->u.key_val; io_->tt_ = n_->u.key_tt; ((void)L, ((void)0)); };
      luaH_set(L, t, &k, (&(old)->i_val));
    }
  }
}





static void exchangehashpart (Table *t1, Table *t2) {
  lu_byte lsizenode = t1->lsizenode;
  Node *node = t1->node;
  Node *lastfree = t1->lastfree;
  t1->lsizenode = t2->lsizenode;
  t1->node = t2->node;
  t1->lastfree = t2->lastfree;
  t2->lsizenode = lsizenode;
  t2->node = node;
  t2->lastfree = lastfree;
}
void luaH_resize (lua_State *L, Table *t, unsigned int newasize,
                                          unsigned int nhsize) {
  unsigned int i;
  Table newt;
  unsigned int oldasize = setlimittosize(t);
  TValue *newarray;

  setnodevector(L, &newt, nhsize);
  if (newasize < oldasize) {
    t->alimit = newasize;
    exchangehashpart(t, &newt);

    for (i = newasize; i < oldasize; i++) {
      if (!(((((((&t->array[i]))->tt_)) & 0x0F)) == (0)))
        luaH_setint(L, t, i + 1, &t->array[i]);
    }
    t->alimit = oldasize;
    exchangehashpart(t, &newt);
  }

  newarray = (((TValue *)(luaM_realloc_(L, t->array, ((size_t)((oldasize))) * sizeof(TValue), ((size_t)((newasize))) * sizeof(TValue)))));
  if ((newarray == 
     ((void *)0) 
     && newasize > 0)) {
    freehash(L, &newt);
    luaD_throw(L, 4);
  }

  exchangehashpart(t, &newt);
  t->array = newarray;
  t->alimit = newasize;
  for (i = oldasize; i < newasize; i++)
     ((&t->array[i])->tt_=(((0) | ((1) << 4))));

  reinsert(L, &newt, t);
  freehash(L, &newt);
}


void luaH_resizearray (lua_State *L, Table *t, unsigned int nasize) {
  int nsize = (((t)->lastfree == 
             ((void *)0)
             ) ? 0 : ((1<<((t)->lsizenode))));
  luaH_resize(L, t, nasize, nsize);
}




static void rehash (lua_State *L, Table *t, const TValue *ek) {
  unsigned int asize;
  unsigned int na;
  unsigned int nums[32];
  int i;
  int totaluse;
  for (i = 0; i <= ((int)((sizeof(int) * 8 
                  - 1))); i++) nums[i] = 0;
  setlimittosize(t);
  na = numusearray(t, nums);
  totaluse = na;
  totaluse += numusehash(t, nums, &na);

  if (((((ek))->tt_) == (((3) | ((0) << 4)))))
    na += countint((((ek)->value_).i), nums);
  totaluse++;

  asize = computesizes(nums, &na);

  luaH_resize(L, t, asize, totaluse - na);
}
Table *luaH_new (lua_State *L) {
  GCObject *o = luaC_newobj(L, ((5) | ((0) << 4)), sizeof(Table));
  Table *t = (&((((union GCUnion *)((o))))->h));
  t->metatable = 
                ((void *)0)
                    ;
  t->flags = ((lu_byte)(((~(~0u << (TM_EQ + 1))))));
  t->array = 
            ((void *)0)
                ;
  t->alimit = 0;
  setnodevector(L, t, 0);
  return t;
}


void luaH_free (lua_State *L, Table *t) {
  freehash(L, t);
  luaM_free_(L, (t->array), (luaH_realasize(t))*sizeof(*(t->array)));
  luaM_free_(L, (t), sizeof(*(t)));
}


static Node *getfreepos (Table *t) {
  if (!((t)->lastfree == 
      ((void *)0)
      )) {
    while (t->lastfree > t->node) {
      t->lastfree--;
      if ((((t->lastfree)->u.key_tt) == 0))
        return t->lastfree;
    }
  }
  return 
        ((void *)0)
            ;
}
void luaH_newkey (lua_State *L, Table *t, const TValue *key, TValue *value) {
  Node *mp;
  TValue aux;
  if (((((((((key))->tt_)) & 0x0F)) == (0))))
    luaG_runerror(L, "table index is nil");
  else if (((((key))->tt_) == (((3) | ((1) << 4))))) {
    lua_Number f = (((key)->value_).n);
    lua_Integer k;
    if (luaV_flttointeger(f, &k, F2Ieq)) {
      { TValue *io=(&aux); ((io)->value_).i=(k); ((io)->tt_=(((3) | ((0) << 4)))); };
      key = &aux;
    }
    else if (((!(((f))==((f))))))
      luaG_runerror(L, "table index is NaN");
  }
  if ((((((((value))->tt_)) & 0x0F)) == (0)))
    return;
  mp = mainpositionTV(t, key);
  if (!((((((((&(mp)->i_val)))->tt_)) & 0x0F)) == (0)) || ((t)->lastfree == 
                           ((void *)0)
                           )) {
    Node *othern;
    Node *f = getfreepos(t);
    if (f == 
            ((void *)0)
                ) {
      rehash(L, t, key);

      luaH_set(L, t, key, value);
      return;
    }
    ((void)0);
    othern = mainpositionfromnode(t, mp);
    if (othern != mp) {

      while (othern + ((othern)->u.next) != mp)
        othern += ((othern)->u.next);
      ((othern)->u.next) = ((int)((f - othern)));
      *f = *mp;
      if (((mp)->u.next) != 0) {
        ((f)->u.next) += ((int)((mp - f)));
        ((mp)->u.next) = 0;
      }
      (((&(mp)->i_val))->tt_=(((0) | ((1) << 4))));
    }
    else {

      if (((mp)->u.next) != 0)
        ((f)->u.next) = ((int)(((mp + ((mp)->u.next)) - f)));
      else ((void)0);
      ((mp)->u.next) = ((int)((f - mp)));
      mp = f;
    }
  }
  { Node *n_=(mp); const TValue *io_=(key); n_->u.key_val = io_->value_; n_->u.key_tt = io_->tt_; ((void)L, ((void)0)); };
  ( (((key)->tt_) & (1 << 6)) ? ( (((((&(((union GCUnion *)((t)))->gc)))->marked) & ((1<<(5)))) && ((((((key)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(&(((union GCUnion *)((t)))->gc))) : ((void)((0)))) : ((void)((0))));
  ((void)0);
  { TValue *io1=((&(mp)->i_val)); const TValue *io2=(value); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
}
const TValue *luaH_getint (Table *t, lua_Integer key) {
  if (((lua_Unsigned)(key)) - 1u < t->alimit)
    return &t->array[key - 1];
  else if (!((!((t)->flags & (1 << 7))) || ((((t)->alimit) & (((t)->alimit) - 1)) == 0)) &&
           (((lua_Unsigned)(key)) == t->alimit + 1 ||
            ((lua_Unsigned)(key)) - 1u < luaH_realasize(t))) {
    t->alimit = ((unsigned int)((key)));
    return &t->array[key - 1];
  }
  else {
    Node *n = hashint(t, key);
    for (;;) {
      if ((((n)->u.key_tt) == ((3) | ((0) << 4))) && (((n)->u.key_val).i) == key)
        return (&(n)->i_val);
      else {
        int nx = ((n)->u.next);
        if (nx == 0) break;
        n += nx;
      }
    }
    return &absentkey;
  }
}





const TValue *luaH_getshortstr (Table *t, TString *key) {
  Node *n = ((&(t)->node[(((((int)(((((key)->hash)) & ((((1<<((t)->lsizenode))))-1)))))))]));
  ((void)0);
  for (;;) {
    if ((((n)->u.key_tt) == ((((4) | ((0) << 4))) | (1 << 6))) && ((((&((((union GCUnion *)((((n)->u.key_val).gc))))->ts)))) == (key)))
      return (&(n)->i_val);
    else {
      int nx = ((n)->u.next);
      if (nx == 0)
        return &absentkey;
      n += nx;
    }
  }
}


const TValue *luaH_getstr (Table *t, TString *key) {
  if (key->tt == ((4) | ((0) << 4)))
    return luaH_getshortstr(t, key);
  else {
    TValue ko;
    { TValue *io = (&ko); TString *x_ = (key); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)((lua_State *)(
   ((void *)0)
   )), ((void)0)); };
    return getgeneric(t, &ko, 0);
  }
}





const TValue *luaH_get (Table *t, const TValue *key) {
  switch (((((key)->tt_)) & 0x3F)) {
    case ((4) | ((0) << 4)): return luaH_getshortstr(t, ((&((((union GCUnion *)((((key)->value_).gc))))->ts))));
    case ((3) | ((0) << 4)): return luaH_getint(t, (((key)->value_).i));
    case ((0) | ((0) << 4)): return &absentkey;
    case ((3) | ((1) << 4)): {
      lua_Integer k;
      if (luaV_flttointeger((((key)->value_).n), &k, F2Ieq))
        return luaH_getint(t, k);

    }
    default:
      return getgeneric(t, key, 0);
  }
}
void luaH_finishset (lua_State *L, Table *t, const TValue *key,
                                   const TValue *slot, TValue *value) {
  if (((((slot))->tt_) == (((0) | ((2) << 4)))))
    luaH_newkey(L, t, key, value);
  else
    { TValue *io1=(((TValue *)(slot))); const TValue *io2=(value); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
}






void luaH_set (lua_State *L, Table *t, const TValue *key, TValue *value) {
  const TValue *slot = luaH_get(t, key);
  luaH_finishset(L, t, key, slot, value);
}


void luaH_setint (lua_State *L, Table *t, lua_Integer key, TValue *value) {
  const TValue *p = luaH_getint(t, key);
  if (((((p))->tt_) == (((0) | ((2) << 4))))) {
    TValue k;
    { TValue *io=(&k); ((io)->value_).i=(key); ((io)->tt_=(((3) | ((0) << 4)))); };
    luaH_newkey(L, t, &k, value);
  }
  else
    { TValue *io1=(((TValue *)(p))); const TValue *io2=(value); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
}
static lua_Unsigned hash_search (Table *t, lua_Unsigned j) {
  lua_Unsigned i;
  if (j == 0) j++;
  do {
    i = j;
    if (j <= ((lua_Unsigned)(0x7fffffffffffffffLL
            )) / 2)
      j *= 2;
    else {
      j = 0x7fffffffffffffffLL
                       ;
      if ((((((((luaH_getint(t, j)))->tt_)) & 0x0F)) == (0)))
        break;
      else
        return j;
    }
  } while (!(((((((luaH_getint(t, j)))->tt_)) & 0x0F)) == (0)));

  while (j - i > 1u) {
    lua_Unsigned m = (i + j) / 2;
    if ((((((((luaH_getint(t, m)))->tt_)) & 0x0F)) == (0))) j = m;
    else i = m;
  }
  return i;
}


static unsigned int binsearch (const TValue *array, unsigned int i,
                                                    unsigned int j) {
  while (j - i > 1u) {
    unsigned int m = (i + j) / 2;
    if ((((((((&array[m - 1]))->tt_)) & 0x0F)) == (0))) j = m;
    else i = m;
  }
  return i;
}
lua_Unsigned luaH_getn (Table *t) {
  unsigned int limit = t->alimit;
  if (limit > 0 && (((((((&t->array[limit - 1]))->tt_)) & 0x0F)) == (0))) {

    if (limit >= 2 && !(((((((&t->array[limit - 2]))->tt_)) & 0x0F)) == (0))) {

      if (ispow2realasize(t) && !(((limit - 1) & ((limit - 1) - 1)) == 0)) {
        t->alimit = limit - 1;
        ((t)->flags |= (1 << 7));
      }
      return limit - 1;
    }
    else {
      unsigned int boundary = binsearch(t->array, 0, limit);

      if (ispow2realasize(t) && boundary > luaH_realasize(t) / 2) {
        t->alimit = boundary;
        ((t)->flags |= (1 << 7));
      }
      return boundary;
    }
  }

  if (!((!((t)->flags & (1 << 7))) || ((((t)->alimit) & (((t)->alimit) - 1)) == 0))) {

    if ((((((((&t->array[limit]))->tt_)) & 0x0F)) == (0)))
      return limit;

    limit = luaH_realasize(t);
    if ((((((((&t->array[limit - 1]))->tt_)) & 0x0F)) == (0))) {


      unsigned int boundary = binsearch(t->array, t->alimit, limit);
      t->alimit = boundary;
      return boundary;
    }

  }

  ((void)0)
                                                            ;
  if (((t)->lastfree == 
     ((void *)0)
     ) || (((((((luaH_getint(t, ((lua_Integer)(limit + 1)))))->tt_)) & 0x0F)) == (0)))
    return limit;
  else
    return hash_search(t, limit);
}
static const char udatatypename[] = "userdata";

 const char *const luaT_typenames_[12] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "upvalue", "proto"
};


void luaT_init (lua_State *L) {
  static const char *const luaT_eventname[] = {
    "__index", "__newindex",
    "__gc", "__mode", "__len", "__eq",
    "__add", "__sub", "__mul", "__mod", "__pow",
    "__div", "__idiv",
    "__band", "__bor", "__bxor", "__shl", "__shr",
    "__unm", "__bnot", "__lt", "__le",
    "__concat", "__call", "__close"
  };
  int i;
  for (i=0; i<TM_N; i++) {
    (L->l_G)->tmname[i] = luaS_new(L, luaT_eventname[i]);
    luaC_fix(L, (&(((union GCUnion *)(((L->l_G)->tmname[i])))->gc)));
  }
}






const TValue *luaT_gettm (Table *events, TMS event, TString *ename) {
  const TValue *tm = luaH_getshortstr(events, ename);
  ((void)0);
  if ((((((((tm))->tt_)) & 0x0F)) == (0))) {
    events->flags |= ((lu_byte)((1u<<event)));
    return 
          ((void *)0)
              ;
  }
  else return tm;
}


const TValue *luaT_gettmbyobj (lua_State *L, const TValue *o, TMS event) {
  Table *mt;
  switch ((((((o)->tt_)) & 0x0F))) {
    case 5:
      mt = ((&((((union GCUnion *)((((o)->value_).gc))))->h)))->metatable;
      break;
    case 7:
      mt = ((&((((union GCUnion *)((((o)->value_).gc))))->u)))->metatable;
      break;
    default:
      mt = (L->l_G)->mt[(((((o)->tt_)) & 0x0F))];
  }
  return (mt ? luaH_getshortstr(mt, (L->l_G)->tmname[event]) : &(L->l_G)->nilvalue);
}






const char *luaT_objtypename (lua_State *L, const TValue *o) {
  Table *mt;
  if ((((((o))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) && (mt = ((&((((union GCUnion *)((((o)->value_).gc))))->h)))->metatable) != 
                                                     ((void *)0)
                                                         ) ||
      (((((o))->tt_) == (((((7) | ((0) << 4))) | (1 << 6)))) && (mt = ((&((((union GCUnion *)((((o)->value_).gc))))->u)))->metatable) != 
                                                            ((void *)0)
                                                                )) {
    const TValue *name = luaH_getshortstr(mt, luaS_new(L, "__name"));
    if ((((((((name))->tt_)) & 0x0F)) == (4)))
      return ((((&((((union GCUnion *)((((name)->value_).gc))))->ts))))->contents);
  }
  return luaT_typenames_[((((((o)->tt_)) & 0x0F))) + 1];
}


void luaT_callTM (lua_State *L, const TValue *f, const TValue *p1,
                  const TValue *p2, const TValue *p3) {
  StkId func = L->top.p;
  { TValue *io1=((&(func)->val)); const TValue *io2=(f); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(func + 1)->val)); const TValue *io2=(p1); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(func + 2)->val)); const TValue *io2=(p2); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(func + 3)->val)); const TValue *io2=(p3); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  L->top.p = func + 4;

  if ((!((L->ci)->callstatus & ((1<<1) | (1<<3)))))
    luaD_call(L, func, 0);
  else
    luaD_callnoyield(L, func, 0);
}


void luaT_callTMres (lua_State *L, const TValue *f, const TValue *p1,
                     const TValue *p2, StkId res) {
  ptrdiff_t result = (((char *)((res))) - ((char *)((L->stack.p))));
  StkId func = L->top.p;
  { TValue *io1=((&(func)->val)); const TValue *io2=(f); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(func + 1)->val)); const TValue *io2=(p1); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  { TValue *io1=((&(func + 2)->val)); const TValue *io2=(p2); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  L->top.p += 3;

  if ((!((L->ci)->callstatus & ((1<<1) | (1<<3)))))
    luaD_call(L, func, 1);
  else
    luaD_callnoyield(L, func, 1);
  res = ((StkId)(((char *)((L->stack.p))) + (result)));
  { TValue *io1=((&(res)->val)); const TValue *io2=((&(--L->top.p)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
}


static int callbinTM (lua_State *L, const TValue *p1, const TValue *p2,
                      StkId res, TMS event) {
  const TValue *tm = luaT_gettmbyobj(L, p1, event);
  if ((((((((tm))->tt_)) & 0x0F)) == (0)))
    tm = luaT_gettmbyobj(L, p2, event);
  if ((((((((tm))->tt_)) & 0x0F)) == (0))) return 0;
  luaT_callTMres(L, tm, p1, p2, res);
  return 1;
}


void luaT_trybinTM (lua_State *L, const TValue *p1, const TValue *p2,
                    StkId res, TMS event) {
  if ((!callbinTM(L, p1, p2, res, event))) {
    switch (event) {
      case TM_BAND: case TM_BOR: case TM_BXOR:
      case TM_SHL: case TM_SHR: case TM_BNOT: {
        if ((((((((p1))->tt_)) & 0x0F)) == (3)) && (((((((p2))->tt_)) & 0x0F)) == (3)))
          luaG_tointerror(L, p1, p2);
        else
          luaG_opinterror(L, p1, p2, "perform bitwise operation on");
      }

      default:
        luaG_opinterror(L, p1, p2, "perform arithmetic on");
    }
  }
}


void luaT_tryconcatTM (lua_State *L) {
  StkId top = L->top.p;
  if ((!callbinTM(L, (&(top - 2)->val), (&(top - 1)->val), top - 2, TM_CONCAT))
                                          )
    luaG_concaterror(L, (&(top - 2)->val), (&(top - 1)->val));
}


void luaT_trybinassocTM (lua_State *L, const TValue *p1, const TValue *p2,
                                       int flip, StkId res, TMS event) {
  if (flip)
    luaT_trybinTM(L, p2, p1, res, event);
  else
    luaT_trybinTM(L, p1, p2, res, event);
}


void luaT_trybiniTM (lua_State *L, const TValue *p1, lua_Integer i2,
                                   int flip, StkId res, TMS event) {
  TValue aux;
  { TValue *io=(&aux); ((io)->value_).i=(i2); ((io)->tt_=(((3) | ((0) << 4)))); };
  luaT_trybinassocTM(L, p1, &aux, flip, res, event);
}
int luaT_callorderTM (lua_State *L, const TValue *p1, const TValue *p2,
                      TMS event) {
  if (callbinTM(L, p1, p2, L->top.p, event))
    return !((((((&(L->top.p)->val)))->tt_) == (((1) | ((0) << 4)))) || ((((((((&(L->top.p)->val)))->tt_)) & 0x0F)) == (0)));
  luaG_ordererror(L, p1, p2);
  return 0;
}


int luaT_callorderiTM (lua_State *L, const TValue *p1, int v2,
                       int flip, int isfloat, TMS event) {
  TValue aux; const TValue *p2;
  if (isfloat) {
    { TValue *io=(&aux); ((io)->value_).n=(((lua_Number)((v2)))); ((io)->tt_=(((3) | ((1) << 4)))); };
  }
  else
    { TValue *io=(&aux); ((io)->value_).i=(v2); ((io)->tt_=(((3) | ((0) << 4)))); };
  if (flip) {
    p2 = p1; p1 = &aux;
  }
  else
    p2 = &aux;
  return luaT_callorderTM(L, p1, p2, event);
}


void luaT_adjustvarargs (lua_State *L, int nfixparams, CallInfo *ci,
                         const Proto *p) {
  int i;
  int actual = ((int)((L->top.p - ci->func.p))) - 1;
  int nextra = actual - nfixparams;
  ci->u.l.nextraargs = nextra;
  if ((L->stack_last.p - L->top.p <= (p->maxstacksize + 1))) { (void)0; luaD_growstack(L, p->maxstacksize + 1, 1); (void)0; } else { ((void)0); };

  { TValue *io1=((&(L->top.p++)->val)); const TValue *io2=((&(ci->func.p)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };

  for (i = 1; i <= nfixparams; i++) {
    { TValue *io1=((&(L->top.p++)->val)); const TValue *io2=((&(ci->func.p + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    (((&(ci->func.p + i)->val))->tt_=(((0) | ((0) << 4))));
  }
  ci->func.p += actual + 1;
  ci->top.p += actual + 1;
  ((void)0);
}


void luaT_getvarargs (lua_State *L, CallInfo *ci, StkId where, int wanted) {
  int i;
  int nextra = ci->u.l.nextraargs;
  if (wanted < 0) {
    wanted = nextra;
    if ((L->stack_last.p - L->top.p <= (nextra))) { ptrdiff_t t__ = (((char *)((where))) - ((char *)((L->stack.p)))); { if ((L->l_G)->GCdebt > 0) { (void)0; luaC_step(L); (void)0;}; ((void)0); }; luaD_growstack(L, nextra, 1); where = ((StkId)(((char *)((L->stack.p))) + (t__))); } else { ((void)0); };
    L->top.p = where + nextra;
  }
  for (i = 0; i < wanted && i < nextra; i++)
    { TValue *io1=((&(where + i)->val)); const TValue *io2=((&(ci->func.p - nextra + i)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
  for (; i < wanted; i++)
    (((&(where + i)->val))->tt_=(((0) | ((0) << 4))));
}
typedef struct {
  lua_State *L;
  ZIO *Z;
  const char *name;
} LoadState;


static void error (LoadState *S, const char *why) {
  luaO_pushfstring(S->L, "%s: bad binary format (%s)", S->name, why);
  luaD_throw(S->L, 3);
}
static void loadBlock (LoadState *S, void *b, size_t size) {
  if (luaZ_read(S->Z, b, size) != 0)
    error(S, "truncated chunk");
}





static lu_byte loadByte (LoadState *S) {
  int b = (((S->Z)->n--)>0 ? ((unsigned char)((*(S->Z)->p++))) : luaZ_fill(S->Z));
  if (b == (-1))
    error(S, "truncated chunk");
  return ((lu_byte)((b)));
}


static size_t loadUnsigned (LoadState *S, size_t limit) {
  size_t x = 0;
  int b;
  limit >>= 7;
  do {
    b = loadByte(S);
    if (x >= limit)
      error(S, "integer overflow");
    x = (x << 7) | (b & 0x7f);
  } while ((b & 0x80) == 0);
  return x;
}


static size_t loadSize (LoadState *S) {
  return loadUnsigned(S, ~(size_t)0);
}


static int loadInt (LoadState *S) {
  return ((int)((loadUnsigned(S, 0x7fffffff
        ))));
}


static lua_Number loadNumber (LoadState *S) {
  lua_Number x;
  loadBlock(S,&x,(1)*sizeof((&x)[0]));
  return x;
}


static lua_Integer loadInteger (LoadState *S) {
  lua_Integer x;
  loadBlock(S,&x,(1)*sizeof((&x)[0]));
  return x;
}





static TString *loadStringN (LoadState *S, Proto *p) {
  lua_State *L = S->L;
  TString *ts;
  size_t size = loadSize(S);
  if (size == 0)
    return 
          ((void *)0)
              ;
  else if (--size <= 40) {
    char buff[40];
    loadBlock(S,buff,(size)*sizeof((buff)[0]));
    ts = luaS_newlstr(L, buff, size);
  }
  else {
    ts = luaS_createlngstrobj(L, size);
    { TValue *io = ((&(L->top.p)->val)); TString *x_ = (ts); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    luaD_inctop(L);
    loadBlock(S,((ts)->contents),(size)*sizeof((((ts)->contents))[0]));
    L->top.p--;
  }
  ( ((((p)->marked) & ((1<<(5)))) && (((ts)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((p)))->gc)),(&(((union GCUnion *)((ts)))->gc))) : ((void)((0))));
  return ts;
}





static TString *loadString (LoadState *S, Proto *p) {
  TString *st = loadStringN(S, p);
  if (st == 
           ((void *)0)
               )
    error(S, "bad format for constant string");
  return st;
}


static void loadCode (LoadState *S, Proto *f) {
  int n = loadInt(S);
  f->code = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(Instruction))) ? luaM_toobig(S->L) : ((void)((0)))), ((Instruction*)(luaM_malloc_(S->L, (n)*sizeof(Instruction), 0))));
  f->sizecode = n;
  loadBlock(S,f->code,(n)*sizeof((f->code)[0]));
}


static void loadFunction(LoadState *S, Proto *f, TString *psource);


static void loadConstants (LoadState *S, Proto *f) {
  int i;
  int n = loadInt(S);
  f->k = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(TValue))) ? luaM_toobig(S->L) : ((void)((0)))), ((TValue*)(luaM_malloc_(S->L, (n)*sizeof(TValue), 0))));
  f->sizek = n;
  for (i = 0; i < n; i++)
    ((&f->k[i])->tt_=(((0) | ((0) << 4))));
  for (i = 0; i < n; i++) {
    TValue *o = &f->k[i];
    int t = loadByte(S);
    switch (t) {
      case ((0) | ((0) << 4)):
        ((o)->tt_=(((0) | ((0) << 4))));
        break;
      case ((1) | ((0) << 4)):
        ((o)->tt_=(((1) | ((0) << 4))));
        break;
      case ((1) | ((1) << 4)):
        ((o)->tt_=(((1) | ((1) << 4))));
        break;
      case ((3) | ((1) << 4)):
        { TValue *io=(o); ((io)->value_).n=(loadNumber(S)); ((io)->tt_=(((3) | ((1) << 4)))); };
        break;
      case ((3) | ((0) << 4)):
        { TValue *io=(o); ((io)->value_).i=(loadInteger(S)); ((io)->tt_=(((3) | ((0) << 4)))); };
        break;
      case ((4) | ((0) << 4)):
      case ((4) | ((1) << 4)):
        { TValue *io = (o); TString *x_ = (loadString(S, f)); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)S->L, ((void)0)); };
        break;
      default: ((void)0);
    }
  }
}


static void loadProtos (LoadState *S, Proto *f) {
  int i;
  int n = loadInt(S);
  f->p = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(Proto *))) ? luaM_toobig(S->L) : ((void)((0)))), ((Proto **)(luaM_malloc_(S->L, (n)*sizeof(Proto *), 0))));
  f->sizep = n;
  for (i = 0; i < n; i++)
    f->p[i] = 
             ((void *)0)
                 ;
  for (i = 0; i < n; i++) {
    f->p[i] = luaF_newproto(S->L);
    ( ((((f)->marked) & ((1<<(5)))) && (((f->p[i])->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(S->L,(&(((union GCUnion *)((f)))->gc)),(&(((union GCUnion *)((f->p[i])))->gc))) : ((void)((0))));
    loadFunction(S, f->p[i], f->source);
  }
}
static void loadUpvalues (LoadState *S, Proto *f) {
  int i, n;
  n = loadInt(S);
  f->upvalues = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(Upvaldesc))) ? luaM_toobig(S->L) : ((void)((0)))), ((Upvaldesc*)(luaM_malloc_(S->L, (n)*sizeof(Upvaldesc), 0))));
  f->sizeupvalues = n;
  for (i = 0; i < n; i++)
    f->upvalues[i].name = 
                         ((void *)0)
                             ;
  for (i = 0; i < n; i++) {
    f->upvalues[i].instack = loadByte(S);
    f->upvalues[i].idx = loadByte(S);
    f->upvalues[i].kind = loadByte(S);
  }
}


static void loadDebug (LoadState *S, Proto *f) {
  int i, n;
  n = loadInt(S);
  f->lineinfo = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(ls_byte))) ? luaM_toobig(S->L) : ((void)((0)))), ((ls_byte*)(luaM_malloc_(S->L, (n)*sizeof(ls_byte), 0))));
  f->sizelineinfo = n;
  loadBlock(S,f->lineinfo,(n)*sizeof((f->lineinfo)[0]));
  n = loadInt(S);
  f->abslineinfo = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(AbsLineInfo))) ? luaM_toobig(S->L) : ((void)((0)))), ((AbsLineInfo*)(luaM_malloc_(S->L, (n)*sizeof(AbsLineInfo), 0))));
  f->sizeabslineinfo = n;
  for (i = 0; i < n; i++) {
    f->abslineinfo[i].pc = loadInt(S);
    f->abslineinfo[i].line = loadInt(S);
  }
  n = loadInt(S);
  f->locvars = (((sizeof(n) >= sizeof(size_t) && ((size_t)(((n)))) + 1 > ((size_t)(~(size_t)0))/(sizeof(LocVar))) ? luaM_toobig(S->L) : ((void)((0)))), ((LocVar*)(luaM_malloc_(S->L, (n)*sizeof(LocVar), 0))));
  f->sizelocvars = n;
  for (i = 0; i < n; i++)
    f->locvars[i].varname = 
                           ((void *)0)
                               ;
  for (i = 0; i < n; i++) {
    f->locvars[i].varname = loadStringN(S, f);
    f->locvars[i].startpc = loadInt(S);
    f->locvars[i].endpc = loadInt(S);
  }
  n = loadInt(S);
  if (n != 0)
    n = f->sizeupvalues;
  for (i = 0; i < n; i++)
    f->upvalues[i].name = loadStringN(S, f);
}


static void loadFunction (LoadState *S, Proto *f, TString *psource) {
  f->source = loadStringN(S, f);
  if (f->source == 
                  ((void *)0)
                      )
    f->source = psource;
  f->linedefined = loadInt(S);
  f->lastlinedefined = loadInt(S);
  f->numparams = loadByte(S);
  f->is_vararg = loadByte(S);
  f->maxstacksize = loadByte(S);
  loadCode(S, f);
  loadConstants(S, f);
  loadUpvalues(S, f);
  loadProtos(S, f);
  loadDebug(S, f);
}


static void checkliteral (LoadState *S, const char *s, const char *msg) {
  char buff[sizeof("\x1bLua") + sizeof("\x19\x93\r\n\x1a\n")];
  size_t len = strlen(s);
  loadBlock(S,buff,(len)*sizeof((buff)[0]));
  if (memcmp(s, buff, len) != 0)
    error(S, msg);
}


static void fchecksize (LoadState *S, size_t size, const char *tname) {
  if (loadByte(S) != size)
    error(S, luaO_pushfstring(S->L, "%s size mismatch", tname));
}




static void checkHeader (LoadState *S) {

  checkliteral(S, &"\x1bLua"[1], "not a binary chunk");
  if (loadByte(S) != (("5"[0]-'0')*16+("4"[0]-'0')))
    error(S, "version mismatch");
  if (loadByte(S) != 0)
    error(S, "format mismatch");
  checkliteral(S, "\x19\x93\r\n\x1a\n", "corrupted chunk");
  fchecksize(S,sizeof(Instruction),"Instruction");
  fchecksize(S,sizeof(lua_Integer),"lua_Integer");
  fchecksize(S,sizeof(lua_Number),"lua_Number");
  if (loadInteger(S) != 0x5678)
    error(S, "integer format mismatch");
  if (loadNumber(S) != ((lua_Number)((370.5))))
    error(S, "float format mismatch");
}





LClosure *luaU_undump(lua_State *L, ZIO *Z, const char *name) {
  LoadState S;
  LClosure *cl;
  if (*name == '@' || *name == '=')
    S.name = name + 1;
  else if (*name == "\x1bLua"[0])
    S.name = "binary string";
  else
    S.name = name;
  S.L = L;
  S.Z = Z;
  checkHeader(&S);
  cl = luaF_newLclosure(L, loadByte(&S));
  { TValue *io = ((&(L->top.p)->val)); LClosure *x_ = (cl); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((6) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  luaD_inctop(L);
  cl->p = luaF_newproto(L);
  ( ((((cl)->marked) & ((1<<(5)))) && (((cl->p)->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((cl)))->gc)),(&(((union GCUnion *)((cl->p)))->gc))) : ((void)((0))));
  loadFunction(&S, cl->p, 
                         ((void *)0)
                             );
  ((void)0);
  ;
  return cl;
}
static int l_strton (const TValue *obj, TValue *result) {
  ((void)0);
  if (!(((((((obj))->tt_)) & 0x0F)) == (4)))
    return 0;
  else
    return (luaO_str2num(((((&((((union GCUnion *)((((obj)->value_).gc))))->ts))))->contents), result) == ((((&((((union GCUnion *)((((obj)->value_).gc))))->ts))))->tt == ((4) | ((0) << 4)) ? (((&((((union GCUnion *)((((obj)->value_).gc))))->ts))))->shrlen : (((&((((union GCUnion *)((((obj)->value_).gc))))->ts))))->u.lnglen) + 1);
}






int luaV_tonumber_ (const TValue *obj, lua_Number *n) {
  TValue v;
  if (((((obj))->tt_) == (((3) | ((0) << 4))))) {
    *n = ((lua_Number)(((((obj)->value_).i))));
    return 1;
  }
  else if (l_strton(obj, &v)) {
    *n = ((((((&v))->tt_) == (((3) | ((0) << 4)))) ? ((lua_Number)(((((&v)->value_).i)))) : (((&v)->value_).n)));
    return 1;
  }
  else
    return 0;
}





int luaV_flttointeger (lua_Number n, lua_Integer *p, F2Imod mode) {
  lua_Number f = (floor(n));
  if (n != f) {
    if (mode == F2Ieq) return 0;
    else if (mode == F2Iceil)
      f += 1;
  }
  return ((f) >= (double)(
        (-0x7fffffffffffffffLL - 1LL)
        ) && (f) < -(double)(
        (-0x7fffffffffffffffLL - 1LL)
        ) && (*(p) = (long long)(f), 1));
}







int luaV_tointegerns (const TValue *obj, lua_Integer *p, F2Imod mode) {
  if (((((obj))->tt_) == (((3) | ((1) << 4)))))
    return luaV_flttointeger((((obj)->value_).n), p, mode);
  else if (((((obj))->tt_) == (((3) | ((0) << 4))))) {
    *p = (((obj)->value_).i);
    return 1;
  }
  else
    return 0;
}





int luaV_tointeger (const TValue *obj, lua_Integer *p, F2Imod mode) {
  TValue v;
  if (l_strton(obj, &v))
    obj = &v;
  return luaV_tointegerns(obj, p, mode);
}
static int forlimit (lua_State *L, lua_Integer init, const TValue *lim,
                                   lua_Integer *p, lua_Integer step) {
  if (!luaV_tointeger(lim, p, (step < 0 ? F2Iceil : F2Ifloor))) {

    lua_Number flim;
    if (!(((((lim))->tt_) == (((3) | ((1) << 4)))) ? (*(&flim) = (((lim)->value_).n), 1) : luaV_tonumber_(lim,&flim)))
      luaG_forerror(L, lim, "limit");

    if (((0)<(flim))) {
      if (step < 0) return 1;
      *p = 0x7fffffffffffffffLL
                        ;
    }
    else {
      if (step > 0) return 1;
      *p = 
          (-0x7fffffffffffffffLL - 1LL)
                        ;
    }
  }
  return (step > 0 ? init > *p : init < *p);
}
static int forprep (lua_State *L, StkId ra) {
  TValue *pinit = (&(ra)->val);
  TValue *plimit = (&(ra + 1)->val);
  TValue *pstep = (&(ra + 2)->val);
  if (((((pinit))->tt_) == (((3) | ((0) << 4)))) && ((((pstep))->tt_) == (((3) | ((0) << 4))))) {
    lua_Integer init = (((pinit)->value_).i);
    lua_Integer step = (((pstep)->value_).i);
    lua_Integer limit;
    if (step == 0)
      luaG_runerror(L, "'for' step is zero");
    { TValue *io=((&(ra + 3)->val)); ((io)->value_).i=(init); ((io)->tt_=(((3) | ((0) << 4)))); };
    if (forlimit(L, init, plimit, &limit, step))
      return 1;
    else {
      lua_Unsigned count;
      if (step > 0) {
        count = ((lua_Unsigned)(limit)) - ((lua_Unsigned)(init));
        if (step != 1)
          count /= ((lua_Unsigned)(step));
      }
      else {
        count = ((lua_Unsigned)(init)) - ((lua_Unsigned)(limit));

        count /= ((lua_Unsigned)(-(step + 1))) + 1u;
      }


      { TValue *io=(plimit); ((io)->value_).i=(((lua_Integer)(count))); ((io)->tt_=(((3) | ((0) << 4)))); };
    }
  }
  else {
    lua_Number init; lua_Number limit; lua_Number step;
    if ((!(((((plimit))->tt_) == (((3) | ((1) << 4)))) ? (*(&limit) = (((plimit)->value_).n), 1) : luaV_tonumber_(plimit,&limit))))
      luaG_forerror(L, plimit, "limit");
    if ((!(((((pstep))->tt_) == (((3) | ((1) << 4)))) ? (*(&step) = (((pstep)->value_).n), 1) : luaV_tonumber_(pstep,&step))))
      luaG_forerror(L, pstep, "step");
    if ((!(((((pinit))->tt_) == (((3) | ((1) << 4)))) ? (*(&init) = (((pinit)->value_).n), 1) : luaV_tonumber_(pinit,&init))))
      luaG_forerror(L, pinit, "initial value");
    if (step == 0)
      luaG_runerror(L, "'for' step is zero");
    if (((0)<(step)) ? ((limit)<(init))
                            : ((init)<(limit)))
      return 1;
    else {

      { TValue *io=(plimit); ((io)->value_).n=(limit); ((io)->tt_=(((3) | ((1) << 4)))); };
      { TValue *io=(pstep); ((io)->value_).n=(step); ((io)->tt_=(((3) | ((1) << 4)))); };
      { TValue *io=((&(ra)->val)); ((io)->value_).n=(init); ((io)->tt_=(((3) | ((1) << 4)))); };
      { TValue *io=((&(ra + 3)->val)); ((io)->value_).n=(init); ((io)->tt_=(((3) | ((1) << 4)))); };
    }
  }
  return 0;
}







static int floatforloop (StkId ra) {
  lua_Number step = ((((&(ra + 2)->val))->value_).n);
  lua_Number limit = ((((&(ra + 1)->val))->value_).n);
  lua_Number idx = ((((&(ra)->val))->value_).n);
  idx = ((idx)+(step));
  if (((0)<(step)) ? ((idx)<=(limit))
                          : ((limit)<=(idx))) {
    { TValue *io=((&(ra)->val)); ((void)0); ((io)->value_).n=(idx); };
    { TValue *io=((&(ra + 3)->val)); ((io)->value_).n=(idx); ((io)->tt_=(((3) | ((1) << 4)))); };
    return 1;
  }
  else
    return 0;
}







void luaV_finishget (lua_State *L, const TValue *t, TValue *key, StkId val,
                      const TValue *slot) {
  int loop;
  const TValue *tm;
  for (loop = 0; loop < 2000; loop++) {
    if (slot == 
               ((void *)0)
                   ) {
      ((void)0);
      tm = luaT_gettmbyobj(L, t, TM_INDEX);
      if (((((((((tm))->tt_)) & 0x0F)) == (0))))
        luaG_typeerror(L, t, "index");

    }
    else {
      ((void)0);
      tm = ((((&((((union GCUnion *)((((t)->value_).gc))))->h)))->metatable) == 
          ((void *)0) 
          ? 
          ((void *)0) 
          : ((((&((((union GCUnion *)((((t)->value_).gc))))->h)))->metatable)->flags & (1u<<(TM_INDEX))) ? 
          ((void *)0) 
          : luaT_gettm(((&((((union GCUnion *)((((t)->value_).gc))))->h)))->metatable, TM_INDEX, ((L->l_G))->tmname[TM_INDEX]));
      if (tm == 
               ((void *)0)
                   ) {
        (((&(val)->val))->tt_=(((0) | ((0) << 4))));
        return;
      }

    }
    if (((((((tm)->tt_)) & 0x0F)) == (6))) {
      luaT_callTMres(L, tm, t, key, val);
      return;
    }
    t = tm;
    if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
       ((void *)0)
       , 0) : (slot = luaH_get(((&((((union GCUnion *)((((t)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
      { TValue *io1=((&(val)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      return;
    }

  }
  luaG_runerror(L, "'__index' chain too long; possible loop");
}
void luaV_finishset (lua_State *L, const TValue *t, TValue *key,
                     TValue *val, const TValue *slot) {
  int loop;
  for (loop = 0; loop < 2000; loop++) {
    const TValue *tm;
    if (slot != 
               ((void *)0)
                   ) {
      Table *h = ((&((((union GCUnion *)((((t)->value_).gc))))->h)));
      ((void)0);
      tm = ((h->metatable) == 
          ((void *)0) 
          ? 
          ((void *)0) 
          : ((h->metatable)->flags & (1u<<(TM_NEWINDEX))) ? 
          ((void *)0) 
          : luaT_gettm(h->metatable, TM_NEWINDEX, ((L->l_G))->tmname[TM_NEWINDEX]));
      if (tm == 
               ((void *)0)
                   ) {
        luaH_finishset(L, h, key, slot, val);
        ((h)->flags &= ~(~(~0u << (TM_EQ + 1))));
        ( (((val)->tt_) & (1 << 6)) ? ( (((((&(((union GCUnion *)((h)))->gc)))->marked) & ((1<<(5)))) && ((((((val)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(&(((union GCUnion *)((h)))->gc))) : ((void)((0)))) : ((void)((0))));
        return;
      }

    }
    else {
      tm = luaT_gettmbyobj(L, t, TM_NEWINDEX);
      if (((((((((tm))->tt_)) & 0x0F)) == (0))))
        luaG_typeerror(L, t, "index");
    }

    if (((((((tm)->tt_)) & 0x0F)) == (6))) {
      luaT_callTM(L, tm, t, key, val);
      return;
    }
    t = tm;
    if ((!((((t))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
       ((void *)0)
       , 0) : (slot = luaH_get(((&((((union GCUnion *)((((t)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
      { { TValue *io1=(((TValue *)(slot))); const TValue *io2=(val); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( (((val)->tt_) & (1 << 6)) ? ( (((((((t)->value_).gc))->marked) & ((1<<(5)))) && ((((((val)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((t)->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
      return;
    }

  }
  luaG_runerror(L, "'__newindex' chain too long; possible loop");
}
static int l_strcmp (const TString *ls, const TString *rs) {
  const char *l = ((ls)->contents);
  size_t ll = ((ls)->tt == ((4) | ((0) << 4)) ? (ls)->shrlen : (ls)->u.lnglen);
  const char *r = ((rs)->contents);
  size_t lr = ((rs)->tt == ((4) | ((0) << 4)) ? (rs)->shrlen : (rs)->u.lnglen);
  for (;;) {
    int temp = strcoll(l, r);
    if (temp != 0)
      return temp;
    else {
      size_t len = strlen(l);
      if (len == lr)
        return (len == ll) ? 0 : 1;
      else if (len == ll)
        return -1;

      len++;
      l += len; ll -= len; r += len; lr -= len;
    }
  }
}
static inline int LTintfloat (lua_Integer i, lua_Number f) {
  if (((((lua_Unsigned)1 << ((53
     ))) + ((lua_Unsigned)(i))) <= (2 * ((lua_Unsigned)1 << ((53
     ))))))
    return ((((lua_Number)((i))))<(f));
  else {
    lua_Integer fi;
    if (luaV_flttointeger(f, &fi, F2Iceil))
      return i < fi;
    else
      return f > 0;
  }
}






static inline int LEintfloat (lua_Integer i, lua_Number f) {
  if (((((lua_Unsigned)1 << ((53
     ))) + ((lua_Unsigned)(i))) <= (2 * ((lua_Unsigned)1 << ((53
     ))))))
    return ((((lua_Number)((i))))<=(f));
  else {
    lua_Integer fi;
    if (luaV_flttointeger(f, &fi, F2Ifloor))
      return i <= fi;
    else
      return f > 0;
  }
}






static inline int LTfloatint (lua_Number f, lua_Integer i) {
  if (((((lua_Unsigned)1 << ((53
     ))) + ((lua_Unsigned)(i))) <= (2 * ((lua_Unsigned)1 << ((53
     ))))))
    return ((f)<(((lua_Number)((i)))));
  else {
    lua_Integer fi;
    if (luaV_flttointeger(f, &fi, F2Ifloor))
      return fi < i;
    else
      return f < 0;
  }
}






static inline int LEfloatint (lua_Number f, lua_Integer i) {
  if (((((lua_Unsigned)1 << ((53
     ))) + ((lua_Unsigned)(i))) <= (2 * ((lua_Unsigned)1 << ((53
     ))))))
    return ((f)<=(((lua_Number)((i)))));
  else {
    lua_Integer fi;
    if (luaV_flttointeger(f, &fi, F2Iceil))
      return fi <= i;
    else
      return f < 0;
  }
}





static inline int LTnum (const TValue *l, const TValue *r) {
  ((void)0);
  if (((((l))->tt_) == (((3) | ((0) << 4))))) {
    lua_Integer li = (((l)->value_).i);
    if (((((r))->tt_) == (((3) | ((0) << 4)))))
      return li < (((r)->value_).i);
    else
      return LTintfloat(li, (((r)->value_).n));
  }
  else {
    lua_Number lf = (((l)->value_).n);
    if (((((r))->tt_) == (((3) | ((1) << 4)))))
      return ((lf)<((((r)->value_).n)));
    else
      return LTfloatint(lf, (((r)->value_).i));
  }
}





static inline int LEnum (const TValue *l, const TValue *r) {
  ((void)0);
  if (((((l))->tt_) == (((3) | ((0) << 4))))) {
    lua_Integer li = (((l)->value_).i);
    if (((((r))->tt_) == (((3) | ((0) << 4)))))
      return li <= (((r)->value_).i);
    else
      return LEintfloat(li, (((r)->value_).n));
  }
  else {
    lua_Number lf = (((l)->value_).n);
    if (((((r))->tt_) == (((3) | ((1) << 4)))))
      return ((lf)<=((((r)->value_).n)));
    else
      return LEfloatint(lf, (((r)->value_).i));
  }
}





static int lessthanothers (lua_State *L, const TValue *l, const TValue *r) {
  ((void)0);
  if ((((((((l))->tt_)) & 0x0F)) == (4)) && (((((((r))->tt_)) & 0x0F)) == (4)))
    return l_strcmp(((&((((union GCUnion *)((((l)->value_).gc))))->ts))), ((&((((union GCUnion *)((((r)->value_).gc))))->ts)))) < 0;
  else
    return luaT_callorderTM(L, l, r, TM_LT);
}





int luaV_lessthan (lua_State *L, const TValue *l, const TValue *r) {
  if ((((((((l))->tt_)) & 0x0F)) == (3)) && (((((((r))->tt_)) & 0x0F)) == (3)))
    return LTnum(l, r);
  else return lessthanothers(L, l, r);
}





static int lessequalothers (lua_State *L, const TValue *l, const TValue *r) {
  ((void)0);
  if ((((((((l))->tt_)) & 0x0F)) == (4)) && (((((((r))->tt_)) & 0x0F)) == (4)))
    return l_strcmp(((&((((union GCUnion *)((((l)->value_).gc))))->ts))), ((&((((union GCUnion *)((((r)->value_).gc))))->ts)))) <= 0;
  else
    return luaT_callorderTM(L, l, r, TM_LE);
}





int luaV_lessequal (lua_State *L, const TValue *l, const TValue *r) {
  if ((((((((l))->tt_)) & 0x0F)) == (3)) && (((((((r))->tt_)) & 0x0F)) == (3)))
    return LEnum(l, r);
  else return lessequalothers(L, l, r);
}






int luaV_equalobj (lua_State *L, const TValue *t1, const TValue *t2) {
  const TValue *tm;
  if (((((t1)->tt_)) & 0x3F) != ((((t2)->tt_)) & 0x3F)) {
    if ((((((t1)->tt_)) & 0x0F)) != (((((t2)->tt_)) & 0x0F)) || (((((t1)->tt_)) & 0x0F)) != 3)
      return 0;
    else {



      lua_Integer i1, i2;
      return (luaV_tointegerns(t1, &i1, F2Ieq) &&
              luaV_tointegerns(t2, &i2, F2Ieq) &&
              i1 == i2);
    }
  }

  switch (((((t1)->tt_)) & 0x3F)) {
    case ((0) | ((0) << 4)): case ((1) | ((0) << 4)): case ((1) | ((1) << 4)): return 1;
    case ((3) | ((0) << 4)): return ((((t1)->value_).i) == (((t2)->value_).i));
    case ((3) | ((1) << 4)): return (((((t1)->value_).n))==((((t2)->value_).n)));
    case ((2) | ((0) << 4)): return (((t1)->value_).p) == (((t2)->value_).p);
    case ((6) | ((1) << 4)): return (((t1)->value_).f) == (((t2)->value_).f);
    case ((4) | ((0) << 4)): return ((((&((((union GCUnion *)((((t1)->value_).gc))))->ts)))) == (((&((((union GCUnion *)((((t2)->value_).gc))))->ts)))));
    case ((4) | ((1) << 4)): return luaS_eqlngstr(((&((((union GCUnion *)((((t1)->value_).gc))))->ts))), ((&((((union GCUnion *)((((t2)->value_).gc))))->ts))));
    case ((7) | ((0) << 4)): {
      if (((&((((union GCUnion *)((((t1)->value_).gc))))->u))) == ((&((((union GCUnion *)((((t2)->value_).gc))))->u)))) return 1;
      else if (L == 
                   ((void *)0)
                       ) return 0;
      tm = ((((&((((union GCUnion *)((((t1)->value_).gc))))->u)))->metatable) == 
          ((void *)0) 
          ? 
          ((void *)0) 
          : ((((&((((union GCUnion *)((((t1)->value_).gc))))->u)))->metatable)->flags & (1u<<(TM_EQ))) ? 
          ((void *)0) 
          : luaT_gettm(((&((((union GCUnion *)((((t1)->value_).gc))))->u)))->metatable, TM_EQ, ((L->l_G))->tmname[TM_EQ]));
      if (tm == 
               ((void *)0)
                   )
        tm = ((((&((((union GCUnion *)((((t2)->value_).gc))))->u)))->metatable) == 
            ((void *)0) 
            ? 
            ((void *)0) 
            : ((((&((((union GCUnion *)((((t2)->value_).gc))))->u)))->metatable)->flags & (1u<<(TM_EQ))) ? 
            ((void *)0) 
            : luaT_gettm(((&((((union GCUnion *)((((t2)->value_).gc))))->u)))->metatable, TM_EQ, ((L->l_G))->tmname[TM_EQ]));
      break;
    }
    case ((5) | ((0) << 4)): {
      if (((&((((union GCUnion *)((((t1)->value_).gc))))->h))) == ((&((((union GCUnion *)((((t2)->value_).gc))))->h)))) return 1;
      else if (L == 
                   ((void *)0)
                       ) return 0;
      tm = ((((&((((union GCUnion *)((((t1)->value_).gc))))->h)))->metatable) == 
          ((void *)0) 
          ? 
          ((void *)0) 
          : ((((&((((union GCUnion *)((((t1)->value_).gc))))->h)))->metatable)->flags & (1u<<(TM_EQ))) ? 
          ((void *)0) 
          : luaT_gettm(((&((((union GCUnion *)((((t1)->value_).gc))))->h)))->metatable, TM_EQ, ((L->l_G))->tmname[TM_EQ]));
      if (tm == 
               ((void *)0)
                   )
        tm = ((((&((((union GCUnion *)((((t2)->value_).gc))))->h)))->metatable) == 
            ((void *)0) 
            ? 
            ((void *)0) 
            : ((((&((((union GCUnion *)((((t2)->value_).gc))))->h)))->metatable)->flags & (1u<<(TM_EQ))) ? 
            ((void *)0) 
            : luaT_gettm(((&((((union GCUnion *)((((t2)->value_).gc))))->h)))->metatable, TM_EQ, ((L->l_G))->tmname[TM_EQ]));
      break;
    }
    default:
      return (((t1)->value_).gc) == (((t2)->value_).gc);
  }
  if (tm == 
           ((void *)0)
               )
    return 0;
  else {
    luaT_callTMres(L, tm, t1, t2, L->top.p);
    return !((((((&(L->top.p)->val)))->tt_) == (((1) | ((0) << 4)))) || ((((((((&(L->top.p)->val)))->tt_)) & 0x0F)) == (0)));
  }
}
static void copy2buff (StkId top, int n, char *buff) {
  size_t tl = 0;
  do {
    size_t l = ((((&((((union GCUnion *)(((((&(top - n)->val))->value_).gc))))->ts))))->tt == ((4) | ((0) << 4)) ? (((&((((union GCUnion *)(((((&(top - n)->val))->value_).gc))))->ts))))->shrlen : (((&((((union GCUnion *)(((((&(top - n)->val))->value_).gc))))->ts))))->u.lnglen);
    memcpy(buff + tl, ((((&((((union GCUnion *)(((((&(top - n)->val))->value_).gc))))->ts))))->contents), l * sizeof(char));
    tl += l;
  } while (--n > 0);
}






void luaV_concat (lua_State *L, int total) {
  if (total == 1)
    return;
  do {
    StkId top = L->top.p;
    int n = 2;
    if (!(((((((((&(top - 2)->val)))->tt_)) & 0x0F)) == (4)) || ((((((((&(top - 2)->val)))->tt_)) & 0x0F)) == (3))) ||
        !(((((((((&(top - 1)->val)))->tt_)) & 0x0F)) == (4)) || (((((((((&(top - 1)->val)))->tt_)) & 0x0F)) == (3)) && (luaO_tostring(L, (&(top - 1)->val)), 1))))
      luaT_tryconcatTM(L);
    else if (((((((&(top - 1)->val)))->tt_) == (((((4) | ((0) << 4))) | (1 << 6)))) && ((&((((union GCUnion *)(((((&(top - 1)->val))->value_).gc))))->ts)))->shrlen == 0))
      ((void)(((((((((((&(top - 2)->val)))->tt_)) & 0x0F)) == (4)) || (((((((((&(top - 2)->val)))->tt_)) & 0x0F)) == (3)) && (luaO_tostring(L, (&(top - 2)->val)), 1))))));
    else if (((((((&(top - 2)->val)))->tt_) == (((((4) | ((0) << 4))) | (1 << 6)))) && ((&((((union GCUnion *)(((((&(top - 2)->val))->value_).gc))))->ts)))->shrlen == 0)) {
      { TValue *io1=((&(top - 2)->val)); const TValue *io2=((&(top - 1)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
    }
    else {

      size_t tl = ((((&((((union GCUnion *)(((((&(top - 1)->val))->value_).gc))))->ts))))->tt == ((4) | ((0) << 4)) ? (((&((((union GCUnion *)(((((&(top - 1)->val))->value_).gc))))->ts))))->shrlen : (((&((((union GCUnion *)(((((&(top - 1)->val))->value_).gc))))->ts))))->u.lnglen);
      TString *ts;

      for (n = 1; n < total && (((((((((&(top - n - 1)->val)))->tt_)) & 0x0F)) == (4)) || (((((((((&(top - n - 1)->val)))->tt_)) & 0x0F)) == (3)) && (luaO_tostring(L, (&(top - n - 1)->val)), 1))); n++) {
        size_t l = ((((&((((union GCUnion *)(((((&(top - n - 1)->val))->value_).gc))))->ts))))->tt == ((4) | ((0) << 4)) ? (((&((((union GCUnion *)(((((&(top - n - 1)->val))->value_).gc))))->ts))))->shrlen : (((&((((union GCUnion *)(((((&(top - n - 1)->val))->value_).gc))))->ts))))->u.lnglen);
        if ((l >= ((sizeof(size_t) < sizeof(lua_Integer) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffffffffffffLL
           ))/sizeof(char)) - tl)) {
          L->top.p = top - total;
          luaG_runerror(L, "string length overflow");
        }
        tl += l;
      }
      if (tl <= 40) {
        char buff[40];
        copy2buff(top, n, buff);
        ts = luaS_newlstr(L, buff, tl);
      }
      else {
        ts = luaS_createlngstrobj(L, tl);
        copy2buff(top, n, ((ts)->contents));
      }
      { TValue *io = ((&(top - n)->val)); TString *x_ = (ts); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((x_->tt) | (1 << 6)))); ((void)L, ((void)0)); };
    }
    total -= n - 1;
    L->top.p -= n - 1;
  } while (total > 1);
}





void luaV_objlen (lua_State *L, StkId ra, const TValue *rb) {
  const TValue *tm;
  switch (((((rb)->tt_)) & 0x3F)) {
    case ((5) | ((0) << 4)): {
      Table *h = ((&((((union GCUnion *)((((rb)->value_).gc))))->h)));
      tm = ((h->metatable) == 
          ((void *)0) 
          ? 
          ((void *)0) 
          : ((h->metatable)->flags & (1u<<(TM_LEN))) ? 
          ((void *)0) 
          : luaT_gettm(h->metatable, TM_LEN, ((L->l_G))->tmname[TM_LEN]));
      if (tm) break;
      { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaH_getn(h)); ((io)->tt_=(((3) | ((0) << 4)))); };
      return;
    }
    case ((4) | ((0) << 4)): {
      { TValue *io=((&(ra)->val)); ((io)->value_).i=(((&((((union GCUnion *)((((rb)->value_).gc))))->ts)))->shrlen); ((io)->tt_=(((3) | ((0) << 4)))); };
      return;
    }
    case ((4) | ((1) << 4)): {
      { TValue *io=((&(ra)->val)); ((io)->value_).i=(((&((((union GCUnion *)((((rb)->value_).gc))))->ts)))->u.lnglen); ((io)->tt_=(((3) | ((0) << 4)))); };
      return;
    }
    default: {
      tm = luaT_gettmbyobj(L, rb, TM_LEN);
      if (((((((((tm))->tt_)) & 0x0F)) == (0))))
        luaG_typeerror(L, rb, "get length of");
      break;
    }
  }
  luaT_callTMres(L, tm, rb, rb, ra);
}
lua_Integer luaV_idiv (lua_State *L, lua_Integer m, lua_Integer n) {
  if ((((lua_Unsigned)(n)) + 1u <= 1u)) {
    if (n == 0)
      luaG_runerror(L, "attempt to divide by zero");
    return ((lua_Integer)(((lua_Unsigned)(0)) - ((lua_Unsigned)(m))));
  }
  else {
    lua_Integer q = m / n;
    if ((m ^ n) < 0 && m % n != 0)
      q -= 1;
    return q;
  }
}







lua_Integer luaV_mod (lua_State *L, lua_Integer m, lua_Integer n) {
  if ((((lua_Unsigned)(n)) + 1u <= 1u)) {
    if (n == 0)
      luaG_runerror(L, "attempt to perform 'n%%0'");
    return 0;
  }
  else {
    lua_Integer r = m % n;
    if (r != 0 && (r ^ n) < 0)
      r += n;
    return r;
  }
}





lua_Number luaV_modf (lua_State *L, lua_Number m, lua_Number n) {
  lua_Number r;
  { (void)L; (r) = fmod(m,n); if (((r) > 0) ? (n) < 0 : ((r) < 0 && (n) > 0)) (r) += (n); };
  return r;
}
lua_Integer luaV_shiftl (lua_Integer x, lua_Integer y) {
  if (y < 0) {
    if (y <= -((int)((sizeof(lua_Integer) * 8
             )))) return 0;
    else return ((lua_Integer)(((lua_Unsigned)(x)) >> ((lua_Unsigned)(-y))));
  }
  else {
    if (y >= ((int)((sizeof(lua_Integer) * 8
            )))) return 0;
    else return ((lua_Integer)(((lua_Unsigned)(x)) << ((lua_Unsigned)(y))));
  }
}






static void pushclosure (lua_State *L, Proto *p, UpVal **encup, StkId base,
                         StkId ra) {
  int nup = p->sizeupvalues;
  Upvaldesc *uv = p->upvalues;
  int i;
  LClosure *ncl = luaF_newLclosure(L, nup);
  ncl->p = p;
  { TValue *io = ((&(ra)->val)); LClosure *x_ = (ncl); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((6) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
  for (i = 0; i < nup; i++) {
    if (uv[i].instack)
      ncl->upvals[i] = luaF_findupval(L, base + uv[i].idx);
    else
      ncl->upvals[i] = encup[uv[i].idx];
    ( ((((ncl)->marked) & ((1<<(5)))) && (((ncl->upvals[i])->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((ncl)))->gc)),(&(((union GCUnion *)((ncl->upvals[i])))->gc))) : ((void)((0))));
  }
}





void luaV_finishOp (lua_State *L) {
  CallInfo *ci = L->ci;
  StkId base = ci->func.p + 1;
  Instruction inst = *(ci->u.l.savedpc - 1);
  OpCode op = (((OpCode)(((inst)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))));
  switch (op) {
    case OP_MMBIN: case OP_MMBINI: case OP_MMBINK: {
      { TValue *io1=((&(base + (((int)((((*(ci->u.l.savedpc - 2))>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))->val)); const TValue *io2=((&(--L->top.p)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      break;
    }
    case OP_UNM: case OP_BNOT: case OP_LEN:
    case OP_GETTABUP: case OP_GETTABLE: case OP_GETI:
    case OP_GETFIELD: case OP_SELF: {
      { TValue *io1=((&(base + (((int)((((inst)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))->val)); const TValue *io2=((&(--L->top.p)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      break;
    }
    case OP_LT: case OP_LE:
    case OP_LTI: case OP_LEI:
    case OP_GTI: case OP_GEI:
    case OP_EQ: {
      int res = !((((((&(L->top.p - 1)->val)))->tt_) == (((1) | ((0) << 4)))) || ((((((((&(L->top.p - 1)->val)))->tt_)) & 0x0F)) == (0)));
      L->top.p--;






      ((void)0);
      if (res != ((((int)((((inst)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0))))))))
        ci->u.l.savedpc++;
      break;
    }
    case OP_CONCAT: {
      StkId top = L->top.p - 1;
      int a = (((int)((((inst)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))));
      int total = ((int)((top - 1 - (base + a))));
      { TValue *io1=((&(top - 2)->val)); const TValue *io2=((&(top)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
      L->top.p = top - 1;
      luaV_concat(L, total);
      break;
    }
    case OP_CLOSE: {
      ci->u.l.savedpc--;
      break;
    }
    case OP_RETURN: {
      StkId ra = base + (((int)((((inst)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))));


      L->top.p = ra + ci->u2.nres;

      ci->u.l.savedpc--;
      break;
    }
    default: {

      ((void)0)

                                              ;
      break;
    }
  }
}
void luaV_execute (lua_State *L, CallInfo *ci) {
  LClosure *cl;
  TValue *k;
  StkId base;
  const Instruction *pc;
  int trap;



 startfunc:
  trap = L->hookmask;
 returning:
  cl = ((&((((union GCUnion *)(((((&(ci->func.p)->val))->value_).gc))))->cl.l)));
  k = cl->p->k;
  pc = ci->u.l.savedpc;
  if ((trap)) {
    if (pc == cl->p->code) {
      if (cl->p->is_vararg)
        trap = 0;
      else
        luaD_hookcall(L, ci);
    }
    ci->u.l.trap = 1;
  }
  base = ci->func.p + 1;

  for (;;) {
    Instruction i;
    { if ((trap)) { trap = luaG_traceexec(L, pc); (base = ci->func.p + 1); } i = *(pc++); };




    ((void)0);
    ((void)0);

    ((void)0);
    switch((((OpCode)(((i)>>0) & ((~((~(Instruction)0)<<(7)))<<(0)))))) {
      case OP_MOVE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        { TValue *io1=((&(ra)->val)); const TValue *io2=((&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        break;
      }
      case OP_LOADI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        lua_Integer b = ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))) - (((1<<(8 + 8 + 1))-1)>>1));
        { TValue *io=((&(ra)->val)); ((io)->value_).i=(b); ((io)->tt_=(((3) | ((0) << 4)))); };
        break;
      }
      case OP_LOADF: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int b = ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))) - (((1<<(8 + 8 + 1))-1)>>1));
        { TValue *io=((&(ra)->val)); ((io)->value_).n=(((lua_Number)((b)))); ((io)->tt_=(((3) | ((1) << 4)))); };
        break;
      }
      case OP_LOADK: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = k + ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))));
        { TValue *io1=((&(ra)->val)); const TValue *io2=(rb); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        break;
      }
      case OP_LOADKX: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb;
        rb = k + ((((int)((((*pc)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0))))))); pc++;
        { TValue *io1=((&(ra)->val)); const TValue *io2=(rb); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        break;
      }
      case OP_LOADFALSE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((&(ra)->val))->tt_=(((1) | ((0) << 4))));
        break;
      }
      case OP_LFALSESKIP: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((&(ra)->val))->tt_=(((1) | ((0) << 4))));
        pc++;
        break;
      }
      case OP_LOADTRUE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((&(ra)->val))->tt_=(((1) | ((1) << 4))));
        break;
      }
      case OP_LOADNIL: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        do {
          (((&(ra++)->val))->tt_=(((0) | ((0) << 4))));
        } while (b--);
        break;
      }
      case OP_GETUPVAL: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        { TValue *io1=((&(ra)->val)); const TValue *io2=(cl->upvals[b]->v.p); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        break;
      }
      case OP_SETUPVAL: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        UpVal *uv = cl->upvals[((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))];
        { TValue *io1=(uv->v.p); const TValue *io2=((&(ra)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        ( ((((&(ra)->val))->tt_) & (1 << 6)) ? ( ((((uv)->marked) & ((1<<(5)))) && (((((((&(ra)->val))->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrier_(L,(&(((union GCUnion *)((uv)))->gc)),(&(((union GCUnion *)((((((&(ra)->val))->value_).gc))))->gc))) : ((void)((0)))) : ((void)((0))));
        break;
      }
      case OP_GETTABUP: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *upval = cl->upvals[((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))]->v.p;
        TValue *rc = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        TString *key = ((&((((union GCUnion *)((((rc)->value_).gc))))->ts)));
        if ((!((((upval))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = luaH_getshortstr(((&((((union GCUnion *)((((upval)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishget(L, upval, rc, ra, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_GETTABLE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        TValue *rc = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        lua_Unsigned n;
        if (((((rc))->tt_) == (((3) | ((0) << 4))))
            ? (((void)((n = (((rc)->value_).i)))), (!((((rb))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
                                         ((void *)0)
                                         , 0) : (slot = (((lua_Unsigned)(n)) - 1u < ((&((((union GCUnion *)((((rb)->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)((((rb)->value_).gc))))->h)))->array[n - 1] : luaH_getint(((&((((union GCUnion *)((((rb)->value_).gc))))->h))), n), !(((((((slot))->tt_)) & 0x0F)) == (0)))))
            : (!((((rb))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
             ((void *)0)
             , 0) : (slot = luaH_get(((&((((union GCUnion *)((((rb)->value_).gc))))->h))), rc), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishget(L, rb, rc, ra, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_GETI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        int c = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        if ((!((((rb))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = (((lua_Unsigned)(c)) - 1u < ((&((((union GCUnion *)((((rb)->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)((((rb)->value_).gc))))->h)))->array[c - 1] : luaH_getint(((&((((union GCUnion *)((((rb)->value_).gc))))->h))), c), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        }
        else {
          TValue key;
          { TValue *io=(&key); ((io)->value_).i=(c); ((io)->tt_=(((3) | ((0) << 4)))); };
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishget(L, rb, &key, ra, slot)), (trap = ci->u.l.trap));
        }
        break;
      }
      case OP_GETFIELD: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        TValue *rc = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        TString *key = ((&((((union GCUnion *)((((rc)->value_).gc))))->ts)));
        if ((!((((rb))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = luaH_getshortstr(((&((((union GCUnion *)((((rb)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishget(L, rb, rc, ra, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_SETTABUP: {
        const TValue *slot;
        TValue *upval = cl->upvals[(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))]->v.p;
        TValue *rb = (k+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        TValue *rc = ((((((int)((((i) & (1u << ((0 + 7) + 8))))))))) ? k + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) : (&(base + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))->val));
        TString *key = ((&((((union GCUnion *)((((rb)->value_).gc))))->ts)));
        if ((!((((upval))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = luaH_getshortstr(((&((((union GCUnion *)((((upval)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { { TValue *io1=(((TValue *)(slot))); const TValue *io2=(rc); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( (((rc)->tt_) & (1 << 6)) ? ( (((((((upval)->value_).gc))->marked) & ((1<<(5)))) && ((((((rc)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(((upval)->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishset(L, upval, rb, rc, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_SETTABLE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        TValue *rc = ((((((int)((((i) & (1u << ((0 + 7) + 8))))))))) ? k + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) : (&(base + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))->val));
        lua_Unsigned n;
        if (((((rb))->tt_) == (((3) | ((0) << 4))))
            ? (((void)((n = (((rb)->value_).i)))), (!(((((&(ra)->val)))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
                                         ((void *)0)
                                         , 0) : (slot = (((lua_Unsigned)(n)) - 1u < ((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h)))->array[n - 1] : luaH_getint(((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h))), n), !(((((((slot))->tt_)) & 0x0F)) == (0)))))
            : (!(((((&(ra)->val)))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
             ((void *)0)
             , 0) : (slot = luaH_get(((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h))), rb), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { { TValue *io1=(((TValue *)(slot))); const TValue *io2=(rc); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( (((rc)->tt_) & (1 << 6)) ? ( ((((((((&(ra)->val))->value_).gc))->marked) & ((1<<(5)))) && ((((((rc)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,((((&(ra)->val))->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishset(L, (&(ra)->val), rb, rc, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_SETI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        int c = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rc = ((((((int)((((i) & (1u << ((0 + 7) + 8))))))))) ? k + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) : (&(base + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))->val));
        if ((!(((((&(ra)->val)))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = (((lua_Unsigned)(c)) - 1u < ((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h)))->alimit) ? &((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h)))->array[c - 1] : luaH_getint(((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h))), c), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { { TValue *io1=(((TValue *)(slot))); const TValue *io2=(rc); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( (((rc)->tt_) & (1 << 6)) ? ( ((((((((&(ra)->val))->value_).gc))->marked) & ((1<<(5)))) && ((((((rc)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,((((&(ra)->val))->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
        }
        else {
          TValue key;
          { TValue *io=(&key); ((io)->value_).i=(c); ((io)->tt_=(((3) | ((0) << 4)))); };
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishset(L, (&(ra)->val), &key, rc, slot)), (trap = ci->u.l.trap));
        }
        break;
      }
      case OP_SETFIELD: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (k+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        TValue *rc = ((((((int)((((i) & (1u << ((0 + 7) + 8))))))))) ? k + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) : (&(base + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))->val));
        TString *key = ((&((((union GCUnion *)((((rb)->value_).gc))))->ts)));
        if ((!(((((&(ra)->val)))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = luaH_getshortstr(((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { { TValue *io1=(((TValue *)(slot))); const TValue *io2=(rc); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); }; ( (((rc)->tt_) & (1 << 6)) ? ( ((((((((&(ra)->val))->value_).gc))->marked) & ((1<<(5)))) && ((((((rc)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,((((&(ra)->val))->value_).gc)) : ((void)((0)))) : ((void)((0)))); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishset(L, (&(ra)->val), rb, rc, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_NEWTABLE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int c = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Table *t;
        if (b > 0)
          b = 1 << (b - 1);
        ((void)0);
        if (((((int)((((i) & (1u << ((0 + 7) + 8)))))))))
          c += ((((int)((((*pc)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0))))))) * (((1<<8)-1) + 1);
        pc++;
        L->top.p = ra + 1;
        t = luaH_new(L);
        { TValue *io = ((&(ra)->val)); Table *x_ = (t); ((io)->value_).gc = (&(((union GCUnion *)((x_)))->gc)); ((io)->tt_=(((((5) | ((0) << 4))) | (1 << 6)))); ((void)L, ((void)0)); };
        if (b != 0 || c != 0)
          luaH_resize(L, t, c, b);
        { { if ((L->l_G)->GCdebt > 0) { ((ci->u.l.savedpc = pc), L->top.p = (ra + 1)); luaC_step(L); (trap = ci->u.l.trap);}; ((void)0); }; {((void) 0); ((void) 0);}; };
        break;
      }
      case OP_SELF: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        const TValue *slot;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        TValue *rc = ((((((int)((((i) & (1u << ((0 + 7) + 8))))))))) ? k + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) : (&(base + ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))->val));
        TString *key = ((&((((union GCUnion *)((((rc)->value_).gc))))->ts)));
        { TValue *io1=((&(ra + 1)->val)); const TValue *io2=(rb); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        if ((!((((rb))->tt_) == (((((5) | ((0) << 4))) | (1 << 6)))) ? (slot = 
           ((void *)0)
           , 0) : (slot = luaH_getstr(((&((((union GCUnion *)((((rb)->value_).gc))))->h))), key), !(((((((slot))->tt_)) & 0x0F)) == (0))))) {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(slot); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_finishget(L, rb, rc, ra, slot)), (trap = ci->u.l.trap));
        break;
      }
      case OP_ADDI: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); int imm = ((((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1)); if (((((v1))->tt_) == (((3) | ((0) << 4))))) { lua_Integer iv1 = (((v1)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(iv1)) + ((lua_Unsigned)(imm))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else if (((((v1))->tt_) == (((3) | ((1) << 4))))) { lua_Number nb = (((v1)->value_).n); lua_Number fimm = ((lua_Number)((imm))); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((nb)+(fimm))); ((io)->tt_=(((3) | ((1) << 4)))); }; }};
        break;
      }
      case OP_ADDK: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) + ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)+(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_SUBK: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) - ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)-(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_MULK: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) * ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)*(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_MODK: {
        ((ci->u.l.savedpc = pc), L->top.p = ci->top.p);
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_mod(L, i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(luaV_modf(L, n1, n2)); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_POWK: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((void)L, (n2 == 2) ? (n1)*(n1) : pow(n1,n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; };
        break;
      }
      case OP_DIVK: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)/(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; };
        break;
      }
      case OP_IDIVK: {
        ((ci->u.l.savedpc = pc), L->top.p = ci->top.p);
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); ((void)0); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_idiv(L, i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((void)L, (floor(((n1)/(n2)))))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_BANDK: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); lua_Integer i1; lua_Integer i2 = (((v2)->value_).i); if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) & ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_BORK: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); lua_Integer i1; lua_Integer i2 = (((v2)->value_).i); if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) | ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_BXORK: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (k+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))); lua_Integer i1; lua_Integer i2 = (((v2)->value_).i); if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) ^ ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_SHRI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        int ic = ((((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1));
        lua_Integer ib;
        if (((((((rb))->tt_) == (((3) | ((0) << 4))))) ? (*(&ib) = (((rb)->value_).i), 1) : luaV_tointegerns(rb,&ib,F2Ieq))) {
          pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_shiftl(ib, -ic)); ((io)->tt_=(((3) | ((0) << 4)))); };
        }
        break;
      }
      case OP_SHLI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        int ic = ((((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1));
        lua_Integer ib;
        if (((((((rb))->tt_) == (((3) | ((0) << 4))))) ? (*(&ib) = (((rb)->value_).i), 1) : luaV_tointegerns(rb,&ib,F2Ieq))) {
          pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_shiftl(ic, ib)); ((io)->tt_=(((3) | ((0) << 4)))); };
        }
        break;
      }
      case OP_ADD: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) + ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)+(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_SUB: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) - ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)-(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_MUL: {
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) * ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)*(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_MOD: {
        ((ci->u.l.savedpc = pc), L->top.p = ci->top.p);
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_mod(L, i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(luaV_modf(L, n1, n2)); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_POW: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((void)L, (n2 == 2) ? (n1)*(n1) : pow(n1,n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; };
        break;
      }
      case OP_DIV: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((n1)/(n2))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; };
        break;
      }
      case OP_IDIV: {
        ((ci->u.l.savedpc = pc), L->top.p = ci->top.p);
        { TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); if (((((v1))->tt_) == (((3) | ((0) << 4)))) && ((((v2))->tt_) == (((3) | ((0) << 4))))) { lua_Integer i1 = (((v1)->value_).i); lua_Integer i2 = (((v2)->value_).i); pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_idiv(L, i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); }; } else { lua_Number n1; lua_Number n2; if ((((((v1))->tt_) == (((3) | ((1) << 4)))) ? ((n1) = (((v1)->value_).n), 1) : (((((v1))->tt_) == (((3) | ((0) << 4)))) ? ((n1) = ((lua_Number)(((((v1)->value_).i)))), 1) : 0)) && (((((v2))->tt_) == (((3) | ((1) << 4)))) ? ((n2) = (((v2)->value_).n), 1) : (((((v2))->tt_) == (((3) | ((0) << 4)))) ? ((n2) = ((lua_Number)(((((v2)->value_).i)))), 1) : 0))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).n=(((void)L, (floor(((n1)/(n2)))))); ((io)->tt_=(((3) | ((1) << 4)))); }; }}; }; };
        break;
      }
      case OP_BAND: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); lua_Integer i1; lua_Integer i2; if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq)) && ((((((v2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((v2)->value_).i), 1) : luaV_tointegerns(v2,&i2,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) & ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_BOR: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); lua_Integer i1; lua_Integer i2; if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq)) && ((((((v2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((v2)->value_).i), 1) : luaV_tointegerns(v2,&i2,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) | ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_BXOR: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); lua_Integer i1; lua_Integer i2; if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq)) && ((((((v2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((v2)->value_).i), 1) : luaV_tointegerns(v2,&i2,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(i1)) ^ ((lua_Unsigned)(i2))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_SHR: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); lua_Integer i1; lua_Integer i2; if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq)) && ((((((v2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((v2)->value_).i), 1) : luaV_tointegerns(v2,&i2,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_shiftl(i1,((lua_Integer)(((lua_Unsigned)(0)) - ((lua_Unsigned)(i2)))))); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_SHL: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); TValue *v1 = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); TValue *v2 = (&((base+((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); lua_Integer i1; lua_Integer i2; if (((((((v1))->tt_) == (((3) | ((0) << 4))))) ? (*(&i1) = (((v1)->value_).i), 1) : luaV_tointegerns(v1,&i1,F2Ieq)) && ((((((v2))->tt_) == (((3) | ((0) << 4))))) ? (*(&i2) = (((v2)->value_).i), 1) : luaV_tointegerns(v2,&i2,F2Ieq))) { pc++; { TValue *io=((&(ra)->val)); ((io)->value_).i=(luaV_shiftl(i1, i2)); ((io)->tt_=(((3) | ((0) << 4)))); }; }};
        break;
      }
      case OP_MMBIN: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Instruction pi = *(pc - 2);
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        TMS tm = (TMS)((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        StkId result = (base+(((int)((((pi)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        ((void)0);
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_trybinTM(L, (&(ra)->val), rb, result, tm)), (trap = ci->u.l.trap));
        break;
      }
      case OP_MMBINI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Instruction pi = *(pc - 2);
        int imm = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1));
        TMS tm = (TMS)((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int flip = ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))));
        StkId result = (base+(((int)((((pi)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_trybiniTM(L, (&(ra)->val), imm, flip, result, tm)), (trap = ci->u.l.trap));
        break;
      }
      case OP_MMBINK: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Instruction pi = *(pc - 2);
        TValue *imm = (k+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));
        TMS tm = (TMS)((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int flip = ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))));
        StkId result = (base+(((int)((((pi)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_trybinassocTM(L, (&(ra)->val), imm, flip, result, tm)), (trap = ci->u.l.trap));
        break;
      }
      case OP_UNM: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        lua_Number nb;
        if (((((rb))->tt_) == (((3) | ((0) << 4))))) {
          lua_Integer ib = (((rb)->value_).i);
          { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(0)) - ((lua_Unsigned)(ib))))); ((io)->tt_=(((3) | ((0) << 4)))); };
        }
        else if ((((((rb))->tt_) == (((3) | ((1) << 4)))) ? ((nb) = (((rb)->value_).n), 1) : (((((rb))->tt_) == (((3) | ((0) << 4)))) ? ((nb) = ((lua_Number)(((((rb)->value_).i)))), 1) : 0))) {
          { TValue *io=((&(ra)->val)); ((io)->value_).n=((-(nb))); ((io)->tt_=(((3) | ((1) << 4)))); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_trybinTM(L, rb, rb, ra, TM_UNM)), (trap = ci->u.l.trap));
        break;
      }
      case OP_BNOT: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        lua_Integer ib;
        if (((((((rb))->tt_) == (((3) | ((0) << 4))))) ? (*(&ib) = (((rb)->value_).i), 1) : luaV_tointegerns(rb,&ib,F2Ieq))) {
          { TValue *io=((&(ra)->val)); ((io)->value_).i=(((lua_Integer)(((lua_Unsigned)(~((lua_Unsigned)(0)))) ^ ((lua_Unsigned)(ib))))); ((io)->tt_=(((3) | ((0) << 4)))); };
        }
        else
          (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_trybinTM(L, rb, rb, ra, TM_BNOT)), (trap = ci->u.l.trap));
        break;
      }
      case OP_NOT: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        if ((((((rb))->tt_) == (((1) | ((0) << 4)))) || (((((((rb))->tt_)) & 0x0F)) == (0))))
          (((&(ra)->val))->tt_=(((1) | ((1) << 4))));
        else
          (((&(ra)->val))->tt_=(((1) | ((0) << 4))));
        break;
      }
      case OP_LEN: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaV_objlen(L, ra, (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val))), (trap = ci->u.l.trap));
        break;
      }
      case OP_CONCAT: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int n = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        L->top.p = ra + n;
        ((ci->u.l.savedpc = pc), (luaV_concat(L, n)), (trap = ci->u.l.trap));
        { { if ((L->l_G)->GCdebt > 0) { ((ci->u.l.savedpc = pc), L->top.p = (L->top.p)); luaC_step(L); (trap = ci->u.l.trap);}; ((void)0); }; {((void) 0); ((void) 0);}; };
        break;
      }
      case OP_CLOSE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaF_close(L, ra, 0, 1)), (trap = ci->u.l.trap));
        break;
      }
      case OP_TBC: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));

        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaF_newtbcupval(L, ra)));
        break;
      }
      case OP_JMP: {
        { pc += ((((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 0; (trap = ci->u.l.trap); };
        break;
      }
      case OP_EQ: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int cond;
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = luaV_equalobj(L, (&(ra)->val), rb)), (trap = ci->u.l.trap));
        if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };;
        break;
      }
      case OP_LT: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4)))) && ((((rb))->tt_) == (((3) | ((0) << 4))))) { lua_Integer ia = ((((&(ra)->val))->value_).i); lua_Integer ib = (((rb)->value_).i); cond = (ia < ib); } else if (((((((((&(ra)->val)))->tt_)) & 0x0F)) == (3)) && (((((((rb))->tt_)) & 0x0F)) == (3))) cond = LTnum((&(ra)->val), rb); else (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = lessthanothers(L, (&(ra)->val), rb)), (trap = ci->u.l.trap)); if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_LE: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4)))) && ((((rb))->tt_) == (((3) | ((0) << 4))))) { lua_Integer ia = ((((&(ra)->val))->value_).i); lua_Integer ib = (((rb)->value_).i); cond = (ia <= ib); } else if (((((((((&(ra)->val)))->tt_)) & 0x0F)) == (3)) && (((((((rb))->tt_)) & 0x0F)) == (3))) cond = LEnum((&(ra)->val), rb); else (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = lessequalothers(L, (&(ra)->val), rb)), (trap = ci->u.l.trap)); if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_EQK: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (k+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))));

        int cond = luaV_equalobj(
                  ((void *)0)
                  ,(&(ra)->val),rb);
        if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };;
        break;
      }
      case OP_EQI: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int cond;
        int im = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1));
        if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4)))))
          cond = (((((&(ra)->val))->value_).i) == im);
        else if ((((((&(ra)->val)))->tt_) == (((3) | ((1) << 4)))))
          cond = ((((((&(ra)->val))->value_).n))==(((lua_Number)((im)))));
        else
          cond = 0;
        if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };;
        break;
      }
      case OP_LTI: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; int im = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1)); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4))))) cond = (((((&(ra)->val))->value_).i) < im); else if ((((((&(ra)->val)))->tt_) == (((3) | ((1) << 4))))) { lua_Number fa = ((((&(ra)->val))->value_).n); lua_Number fim = ((lua_Number)((im))); cond = ((fa)<(fim)); } else { int isf = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = luaT_callorderiTM(L, (&(ra)->val), im, 0, isf, TM_LT)), (trap = ci->u.l.trap)); } if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_LEI: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; int im = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1)); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4))))) cond = (((((&(ra)->val))->value_).i) <= im); else if ((((((&(ra)->val)))->tt_) == (((3) | ((1) << 4))))) { lua_Number fa = ((((&(ra)->val))->value_).n); lua_Number fim = ((lua_Number)((im))); cond = ((fa)<=(fim)); } else { int isf = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = luaT_callorderiTM(L, (&(ra)->val), im, 0, isf, TM_LE)), (trap = ci->u.l.trap)); } if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_GTI: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; int im = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1)); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4))))) cond = (((((&(ra)->val))->value_).i) > im); else if ((((((&(ra)->val)))->tt_) == (((3) | ((1) << 4))))) { lua_Number fa = ((((&(ra)->val))->value_).n); lua_Number fim = ((lua_Number)((im))); cond = ((fa)>(fim)); } else { int isf = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = luaT_callorderiTM(L, (&(ra)->val), im, 1, isf, TM_LT)), (trap = ci->u.l.trap)); } if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_GEI: {
        { StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); int cond; int im = ((((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))) - (((1<<8)-1) >> 1)); if ((((((&(ra)->val)))->tt_) == (((3) | ((0) << 4))))) cond = (((((&(ra)->val))->value_).i) >= im); else if ((((((&(ra)->val)))->tt_) == (((3) | ((1) << 4))))) { lua_Number fa = ((((&(ra)->val))->value_).n); lua_Number fim = ((lua_Number)((im))); cond = ((fa)>=(fim)); } else { int isf = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (cond = luaT_callorderiTM(L, (&(ra)->val), im, 1, isf, TM_LE)), (trap = ci->u.l.trap)); } if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };; };
        break;
      }
      case OP_TEST: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int cond = !((((((&(ra)->val)))->tt_) == (((1) | ((0) << 4)))) || ((((((((&(ra)->val)))->tt_)) & 0x0F)) == (0)));
        if (cond != ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0)))))))) pc++; else { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };;
        break;
      }
      case OP_TESTSET: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        TValue *rb = (&((base+((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))))))->val);
        if ((((((rb))->tt_) == (((1) | ((0) << 4)))) || (((((((rb))->tt_)) & 0x0F)) == (0))) == ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<(1)))<<(0))))))))
          pc++;
        else {
          { TValue *io1=((&(ra)->val)); const TValue *io2=(rb); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
          { Instruction ni = *pc; { pc += ((((int)((((ni)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0)))))) - (((1 << ((8 + 8 + 1) + 8)) - 1) >> 1)) + 1; (trap = ci->u.l.trap); }; };
        }
        break;
      }
      case OP_CALL: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        CallInfo *newci;
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int nresults = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) - 1;
        if (b != 0)
          L->top.p = ra + b;

        (ci->u.l.savedpc = pc);
        if ((newci = luaD_precall(L, ra, nresults)) == 
                                                      ((void *)0)
                                                          )
          (trap = ci->u.l.trap);
        else {
          ci = newci;
          goto startfunc;
        }
        break;
      }
      case OP_TAILCALL: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int b = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int n;
        int nparams1 = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));

        int delta = (nparams1) ? ci->u.l.nextraargs + nparams1 : 0;
        if (b != 0)
          L->top.p = ra + b;
        else
          b = ((int)((L->top.p - ra)));
        (ci->u.l.savedpc = pc);
        if (((((int)((((i) & (1u << ((0 + 7) + 8))))))))) {
          luaF_closeupval(L, base);
          ((void)0);
          ((void)0);
        }
        if ((n = luaD_pretailcall(L, ci, ra, b, delta)) < 0)
          goto startfunc;
        else {
          ci->func.p -= delta;
          luaD_poscall(L, ci, n);
          (trap = ci->u.l.trap);
          goto ret;
        }
      }
      case OP_RETURN: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int n = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) - 1;
        int nparams1 = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        if (n < 0)
          n = ((int)((L->top.p - ra)));
        (ci->u.l.savedpc = pc);
        if (((((int)((((i) & (1u << ((0 + 7) + 8))))))))) {
          ci->u2.nres = n;
          if (L->top.p < ci->top.p)
            L->top.p = ci->top.p;
          luaF_close(L, base, (-1), 1);
          (trap = ci->u.l.trap);
          { if ((trap)) { (base = ci->func.p + 1); ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); } };
        }
        if (nparams1)
          ci->func.p -= ci->u.l.nextraargs + nparams1;
        L->top.p = ra + n;
        luaD_poscall(L, ci, n);
        (trap = ci->u.l.trap);
        goto ret;
      }
      case OP_RETURN0: {
        if ((L->hookmask)) {
          StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
          L->top.p = ra;
          (ci->u.l.savedpc = pc);
          luaD_poscall(L, ci, 0);
          trap = 1;
        }
        else {
          int nres;
          L->ci = ci->previous;
          L->top.p = base - 1;
          for (nres = ci->nresults; (nres > 0); nres--)
            (((&(L->top.p++)->val))->tt_=(((0) | ((0) << 4))));
        }
        goto ret;
      }
      case OP_RETURN1: {
        if ((L->hookmask)) {
          StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
          L->top.p = ra + 1;
          (ci->u.l.savedpc = pc);
          luaD_poscall(L, ci, 1);
          trap = 1;
        }
        else {
          int nres = ci->nresults;
          L->ci = ci->previous;
          if (nres == 0)
            L->top.p = base - 1;
          else {
            StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
            { TValue *io1=((&(base - 1)->val)); const TValue *io2=((&(ra)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
            L->top.p = base;
            for (; (nres > 1); nres--)
              (((&(L->top.p++)->val))->tt_=(((0) | ((0) << 4))));
          }
        }
       ret:
        if (ci->callstatus & (1<<2))
          return;
        else {
          ci = ci->previous;
          goto returning;
        }
      }
      case OP_FORLOOP: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        if ((((((&(ra + 2)->val)))->tt_) == (((3) | ((0) << 4))))) {
          lua_Unsigned count = ((lua_Unsigned)(((((&(ra + 1)->val))->value_).i)));
          if (count > 0) {
            lua_Integer step = ((((&(ra + 2)->val))->value_).i);
            lua_Integer idx = ((((&(ra)->val))->value_).i);
            { TValue *io=((&(ra + 1)->val)); ((void)0); ((io)->value_).i=(count - 1); };
            idx = ((lua_Integer)(((lua_Unsigned)(idx)) + ((lua_Unsigned)(step))));
            { TValue *io=((&(ra)->val)); ((void)0); ((io)->value_).i=(idx); };
            { TValue *io=((&(ra + 3)->val)); ((io)->value_).i=(idx); ((io)->tt_=(((3) | ((0) << 4)))); };
            pc -= ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))));
          }
        }
        else if (floatforloop(ra))
          pc -= ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))));
        (trap = ci->u.l.trap);
        break;
      }
      case OP_FORPREP: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        ((ci->u.l.savedpc = pc), L->top.p = ci->top.p);
        if (forprep(L, ra))
          pc += ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0))))))) + 1;
        break;
      }
      case OP_TFORPREP: {
       StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));

        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaF_newtbcupval(L, ra + 3)));
        pc += ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))));
        i = *(pc++);
        ((void)0);
        goto l_tforcall;
      }
      case OP_TFORCALL: {
       l_tforcall: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));






        memcpy(ra + 4, ra, 3 * sizeof(*ra));
        L->top.p = ra + 4 + 3;
        ((ci->u.l.savedpc = pc), (luaD_call(L, ra + 4, ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))))), (trap = ci->u.l.trap));
        { if ((trap)) { (base = ci->func.p + 1); ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0))))))); } };
        i = *(pc++);
        ((void)0);
        goto l_tforloop;
      }}
      case OP_TFORLOOP: {
       l_tforloop: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        if (!((((((((&(ra + 4)->val)))->tt_)) & 0x0F)) == (0))) {
          { TValue *io1=((&(ra + 2)->val)); const TValue *io2=((&(ra + 4)->val)); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
          pc -= ((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))));
        }
        break;
      }}
      case OP_SETLIST: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int n = ((((int)((((i)>>((((0 + 7) + 8) + 1))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        unsigned int last = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Table *h = ((&((((union GCUnion *)(((((&(ra)->val))->value_).gc))))->h)));
        if (n == 0)
          n = ((int)((L->top.p - ra))) - 1;
        else
          L->top.p = ci->top.p;
        last += n;
        if (((((int)((((i) & (1u << ((0 + 7) + 8))))))))) {
          last += ((((int)((((*pc)>>((0 + 7))) & ((~((~(Instruction)0)<<(((8 + 8 + 1) + 8))))<<(0))))))) * (((1<<8)-1) + 1);
          pc++;
        }
        if (last > luaH_realasize(h))
          luaH_resizearray(L, h, last);
        for (; n > 0; n--) {
          TValue *val = (&(ra + n)->val);
          { TValue *io1=(&h->array[last - 1]); const TValue *io2=(val); io1->value_ = io2->value_; ((io1)->tt_=(io2->tt_)); ((void)L, ((void)0)); ((void)0); };
          last--;
          ( (((val)->tt_) & (1 << 6)) ? ( (((((&(((union GCUnion *)((h)))->gc)))->marked) & ((1<<(5)))) && ((((((val)->value_).gc))->marked) & (((1<<(3)) | (1<<(4)))))) ? luaC_barrierback_(L,(&(((union GCUnion *)((h)))->gc))) : ((void)((0)))) : ((void)((0))));
        }
        break;
      }
      case OP_CLOSURE: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        Proto *p = cl->p->p[((((int)((((i)>>(((0 + 7) + 8))) & ((~((~(Instruction)0)<<((8 + 8 + 1))))<<(0)))))))];
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (pushclosure(L, p, cl->upvals, base, ra)));
        { { if ((L->l_G)->GCdebt > 0) { ((ci->u.l.savedpc = pc), L->top.p = (ra + 1)); luaC_step(L); (trap = ci->u.l.trap);}; ((void)0); }; {((void) 0); ((void) 0);}; };
        break;
      }
      case OP_VARARG: {
        StkId ra = (base+(((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))));
        int n = ((((int)((((i)>>(((((0 + 7) + 8) + 1) + 8))) & ((~((~(Instruction)0)<<(8)))<<(0))))))) - 1;
        (((ci->u.l.savedpc = pc), L->top.p = ci->top.p), (luaT_getvarargs(L, ci, ra, n)), (trap = ci->u.l.trap));
        break;
      }
      case OP_VARARGPREP: {
        ((ci->u.l.savedpc = pc), (luaT_adjustvarargs(L, (((int)((((i)>>((0 + 7))) & ((~((~(Instruction)0)<<(8)))<<(0)))))), ci, cl->p)), (trap = ci->u.l.trap));
        if ((trap)) {
          luaD_hookcall(L, ci);
          L->oldpc = 1;
        }
        (base = ci->func.p + 1);
        break;
      }
      case OP_EXTRAARG: {
        ((void)0);
        break;
      }
    }
  }
}
int luaZ_fill (ZIO *z) {
  size_t size;
  lua_State *L = z->L;
  const char *buff;
  ((void) 0);
  buff = z->reader(L, z->data, &size);
  ((void) 0);
  if (buff == 
             ((void *)0) 
                  || size == 0)
    return (-1);
  z->n = size - 1;
  z->p = buff;
  return ((unsigned char)((*(z->p++))));
}


void luaZ_init (lua_State *L, ZIO *z, lua_Reader reader, void *data) {
  z->L = L;
  z->reader = reader;
  z->data = data;
  z->n = 0;
  z->p = 
        ((void *)0)
            ;
}



size_t luaZ_read (ZIO *z, void *b, size_t n) {
  while (n) {
    size_t m;
    if (z->n == 0) {
      if (luaZ_fill(z) == (-1))
        return n;
      else {
        z->n++;
        z->p--;
      }
    }
    m = (n <= z->n) ? n : z->n;
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}















extern int *__errno_location (void) ;


typedef struct luaL_Buffer luaL_Buffer;
typedef struct luaL_Reg {
  const char *name;
  lua_CFunction func;
} luaL_Reg;




extern void luaL_checkversion_(lua_State *L, lua_Number ver, size_t sz);



extern int luaL_getmetafield(lua_State *L, int obj, const char *e);
extern int luaL_callmeta(lua_State *L, int obj, const char *e);
extern const char * luaL_tolstring(lua_State *L, int idx, size_t *len);
extern int luaL_argerror(lua_State *L, int arg, const char *extramsg);
extern int luaL_typeerror(lua_State *L, int arg, const char *tname);
extern const char * luaL_checklstring(lua_State *L, int arg,
                                                          size_t *l);
extern const char * luaL_optlstring(lua_State *L, int arg,
                                          const char *def, size_t *l);
extern lua_Number luaL_checknumber(lua_State *L, int arg);
extern lua_Number luaL_optnumber(lua_State *L, int arg, lua_Number def);

extern lua_Integer luaL_checkinteger(lua_State *L, int arg);
extern lua_Integer luaL_optinteger(lua_State *L, int arg,
                                          lua_Integer def);

extern void luaL_checkstack(lua_State *L, int sz, const char *msg);
extern void luaL_checktype(lua_State *L, int arg, int t);
extern void luaL_checkany(lua_State *L, int arg);

extern int luaL_newmetatable(lua_State *L, const char *tname);
extern void luaL_setmetatable(lua_State *L, const char *tname);
extern void * luaL_testudata(lua_State *L, int ud, const char *tname);
extern void * luaL_checkudata(lua_State *L, int ud, const char *tname);

extern void luaL_where(lua_State *L, int lvl);
extern int luaL_error(lua_State *L, const char *fmt, ...);

extern int luaL_checkoption(lua_State *L, int arg, const char *def,
                                   const char *const lst[]);

extern int luaL_fileresult(lua_State *L, int stat, const char *fname);
extern int luaL_execresult(lua_State *L, int stat);






extern int luaL_ref(lua_State *L, int t);
extern void luaL_unref(lua_State *L, int t, int ref);

extern int luaL_loadfilex(lua_State *L, const char *filename,
                                               const char *mode);



extern int luaL_loadbufferx(lua_State *L, const char *buff, size_t sz,
                                   const char *name, const char *mode);
extern int luaL_loadstring(lua_State *L, const char *s);

extern lua_State * luaL_newstate(void);

extern lua_Integer luaL_len(lua_State *L, int idx);

extern void luaL_addgsub(luaL_Buffer *b, const char *s,
                                     const char *p, const char *r);
extern const char * luaL_gsub(lua_State *L, const char *s,
                                    const char *p, const char *r);

extern void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);

extern int luaL_getsubtable(lua_State *L, int idx, const char *fname);

extern void luaL_traceback(lua_State *L, lua_State *L1,
                                  const char *msg, int level);

extern void luaL_requiref(lua_State *L, const char *modname,
                                 lua_CFunction openf, int glb);
struct luaL_Buffer {
  char *b;
  size_t size;
  size_t n;
  lua_State *L;
  union {
    lua_Number n; double u; void *s; lua_Integer i; long l;
    char b[((int)(16 * sizeof(void*) * sizeof(lua_Number)))];
  } init;
};
extern void luaL_buffinit(lua_State *L, luaL_Buffer *B);
extern char * luaL_prepbuffsize(luaL_Buffer *B, size_t sz);
extern void luaL_addlstring(luaL_Buffer *B, const char *s, size_t l);
extern void luaL_addstring(luaL_Buffer *B, const char *s);
extern void luaL_addvalue(luaL_Buffer *B);
extern void luaL_pushresult(luaL_Buffer *B);
extern void luaL_pushresultsize(luaL_Buffer *B, size_t sz);
extern char * luaL_buffinitsize(lua_State *L, luaL_Buffer *B, size_t sz);
typedef struct luaL_Stream {
  FILE *f;
  lua_CFunction closef;
} luaL_Stream;
static int findfield (lua_State *L, int objidx, int level) {
  if (level == 0 || !(lua_type(L, (-1)) == 5))
    return 0;
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    if (lua_type(L, -2) == 4) {
      if (lua_rawequal(L, objidx, -1)) {
        lua_settop(L, -(1)-1);
        return 1;
      }
      else if (findfield(L, objidx, level - 1)) {

        lua_pushstring(L, "" ".");
        (lua_copy(L, -1, (-3)), lua_settop(L, -(1)-1));
        lua_concat(L, 3);
        return 1;
      }
    }
    lua_settop(L, -(1)-1);
  }
  return 0;
}





static int pushglobalfuncname (lua_State *L, lua_Debug *ar) {
  int top = lua_gettop(L);
  lua_getinfo(L, "f", ar);
  lua_getfield(L, (-1000000 - 1000), "_LOADED");
  if (findfield(L, top + 1, 2)) {
    const char *name = lua_tolstring(L, (-1), 
                      ((void *)0)
                      );
    if (strncmp(name, "_G" ".", 3) == 0) {
      lua_pushstring(L, name + 3);
      (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
    }
    lua_copy(L, -1, top + 1);
    lua_settop(L, top + 1);
    return 1;
  }
  else {
    lua_settop(L, top);
    return 0;
  }
}


static void pushfuncname (lua_State *L, lua_Debug *ar) {
  if (pushglobalfuncname(L, ar)) {
    lua_pushfstring(L, "function '%s'", lua_tolstring(L, (-1), 
                                       ((void *)0)
                                       ));
    (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
  }
  else if (*ar->namewhat != '\0')
    lua_pushfstring(L, "%s '%s'", ar->namewhat, ar->name);
  else if (*ar->what == 'm')
      lua_pushstring(L, "" "main chunk");
  else if (*ar->what != 'C')
    lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
  else
    lua_pushstring(L, "" "?");
}


static int lastlevel (lua_State *L) {
  lua_Debug ar;
  int li = 1, le = 1;

  while (lua_getstack(L, le, &ar)) { li = le; le *= 2; }

  while (li < le) {
    int m = (li + le)/2;
    if (lua_getstack(L, m, &ar)) li = m + 1;
    else le = m;
  }
  return le - 1;
}


extern void luaL_traceback (lua_State *L, lua_State *L1,
                                const char *msg, int level) {
  luaL_Buffer b;
  lua_Debug ar;
  int last = lastlevel(L1);
  int limit2show = (last - level > 10 + 11) ? 10 : -1;
  luaL_buffinit(L, &b);
  if (msg) {
    luaL_addstring(&b, msg);
    ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = ('\n')));
  }
  luaL_addstring(&b, "stack traceback:");
  while (lua_getstack(L1, level++, &ar)) {
    if (limit2show-- == 0) {
      int n = last - level - 11 + 1;
      lua_pushfstring(L, "\n\t...\t(skipping %d levels)", n);
      luaL_addvalue(&b);
      level += n;
    }
    else {
      lua_getinfo(L1, "Slnt", &ar);
      if (ar.currentline <= 0)
        lua_pushfstring(L, "\n\t%s: in ", ar.short_src);
      else
        lua_pushfstring(L, "\n\t%s:%d: in ", ar.short_src, ar.currentline);
      luaL_addvalue(&b);
      pushfuncname(L, &ar);
      luaL_addvalue(&b);
      if (ar.istailcall)
        luaL_addstring(&b, "\n\t(...tail calls...)");
    }
  }
  luaL_pushresult(&b);
}
extern int luaL_argerror (lua_State *L, int arg, const char *extramsg) {
  lua_Debug ar;
  if (!lua_getstack(L, 0, &ar))
    return luaL_error(L, "bad argument #%d (%s)", arg, extramsg);
  lua_getinfo(L, "n", &ar);
  if (strcmp(ar.namewhat, "method") == 0) {
    arg--;
    if (arg == 0)
      return luaL_error(L, "calling '%s' on bad self (%s)",
                           ar.name, extramsg);
  }
  if (ar.name == 
                ((void *)0)
                    )
    ar.name = (pushglobalfuncname(L, &ar)) ? lua_tolstring(L, (-1), 
                                            ((void *)0)
                                            ) : "?";
  return luaL_error(L, "bad argument #%d to '%s' (%s)",
                        arg, ar.name, extramsg);
}


extern int luaL_typeerror (lua_State *L, int arg, const char *tname) {
  const char *msg;
  const char *typearg;
  if (luaL_getmetafield(L, arg, "__name") == 4)
    typearg = lua_tolstring(L, (-1), 
             ((void *)0)
             );
  else if (lua_type(L, arg) == 2)
    typearg = "light userdata";
  else
    typearg = lua_typename(L, lua_type(L,(arg)));
  msg = lua_pushfstring(L, "%s expected, got %s", tname, typearg);
  return luaL_argerror(L, arg, msg);
}


static void tag_error (lua_State *L, int arg, int tag) {
  luaL_typeerror(L, arg, lua_typename(L, tag));
}






extern void luaL_where (lua_State *L, int level) {
  lua_Debug ar;
  if (lua_getstack(L, level, &ar)) {
    lua_getinfo(L, "Sl", &ar);
    if (ar.currentline > 0) {
      lua_pushfstring(L, "%s:%d: ", ar.short_src, ar.currentline);
      return;
    }
  }
  lua_pushfstring(L, "");
}







extern int luaL_error (lua_State *L, const char *fmt, ...) {
  va_list argp;
  
 __builtin_va_start(
 argp
 ,
 fmt
 )
                    ;
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  
 __builtin_va_end(
 argp
 )
             ;
  lua_concat(L, 2);
  return lua_error(L);
}


extern int luaL_fileresult (lua_State *L, int stat, const char *fname) {
  int en = 
          (*__errno_location ())
               ;
  if (stat) {
    lua_pushboolean(L, 1);
    return 1;
  }
  else {
    lua_pushnil(L);
    if (fname)
      lua_pushfstring(L, "%s: %s", fname, strerror(en));
    else
      lua_pushstring(L, strerror(en));
    lua_pushinteger(L, en);
    return 3;
  }
}















struct timeval
{




  __time_t tv_sec;
  __suseconds_t tv_usec;

};
struct rusage
  {

    struct timeval ru_utime;

    struct timeval ru_stime;

    union
      {
 long int ru_maxrss;
 __syscall_slong_t __ru_maxrss_word;
      };


    union
      {
 long int ru_ixrss;
 __syscall_slong_t __ru_ixrss_word;
      };

    union
      {
 long int ru_idrss;
 __syscall_slong_t __ru_idrss_word;
      };

    union
      {
 long int ru_isrss;
  __syscall_slong_t __ru_isrss_word;
      };


    union
      {
 long int ru_minflt;
 __syscall_slong_t __ru_minflt_word;
      };

    union
      {
 long int ru_majflt;
 __syscall_slong_t __ru_majflt_word;
      };

    union
      {
 long int ru_nswap;
 __syscall_slong_t __ru_nswap_word;
      };


    union
      {
 long int ru_inblock;
 __syscall_slong_t __ru_inblock_word;
      };

    union
      {
 long int ru_oublock;
 __syscall_slong_t __ru_oublock_word;
      };

    union
      {
 long int ru_msgsnd;
 __syscall_slong_t __ru_msgsnd_word;
      };

    union
      {
 long int ru_msgrcv;
 __syscall_slong_t __ru_msgrcv_word;
      };

    union
      {
 long int ru_nsignals;
 __syscall_slong_t __ru_nsignals_word;
      };



    union
      {
 long int ru_nvcsw;
 __syscall_slong_t __ru_nvcsw_word;
      };


    union
      {
 long int ru_nivcsw;
 __syscall_slong_t __ru_nivcsw_word;
      };
  };




typedef enum
{
  P_ALL,
  P_PID,
  P_PGID,
  P_PIDFD,

} idtype_t;
extern __pid_t wait (int *__stat_loc);
extern __pid_t waitpid (__pid_t __pid, int *__stat_loc, int __options);
extern int waitid (idtype_t __idtype, __id_t __id, siginfo_t *__infop,
     int __options);


extern int luaL_execresult (lua_State *L, int stat) {
  if (stat != 0 && 
                  (*__errno_location ()) 
                        != 0)
    return luaL_fileresult(L, 0, 
                                ((void *)0)
                                    );
  else {
    const char *what = "exit";
    if (
   (((
   stat
   ) & 0x7f) == 0)
   ) { stat = 
   (((
   stat
   ) & 0xff00) >> 8)
   ; } else if (
   (((signed char) (((
   stat
   ) & 0x7f) + 1) >> 1) > 0)
   ) { stat = 
   ((
   stat
   ) & 0x7f)
   ; what = "signal"; };
    if (*what == 'e' && stat == 0)
      lua_pushboolean(L, 1);
    else
      lua_pushnil(L);
    lua_pushstring(L, what);
    lua_pushinteger(L, stat);
    return 3;
  }
}
extern int luaL_newmetatable (lua_State *L, const char *tname) {
  if ((lua_getfield(L, (-1000000 - 1000), (tname))) != 0)
    return 0;
  lua_settop(L, -(1)-1);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, tname);
  lua_setfield(L, -2, "__name");
  lua_pushvalue(L, -1);
  lua_setfield(L, (-1000000 - 1000), tname);
  return 1;
}


extern void luaL_setmetatable (lua_State *L, const char *tname) {
  (lua_getfield(L, (-1000000 - 1000), (tname)));
  lua_setmetatable(L, -2);
}


extern void *luaL_testudata (lua_State *L, int ud, const char *tname) {
  void *p = lua_touserdata(L, ud);
  if (p != 
          ((void *)0)
              ) {
    if (lua_getmetatable(L, ud)) {
      (lua_getfield(L, (-1000000 - 1000), (tname)));
      if (!lua_rawequal(L, -1, -2))
        p = 
           ((void *)0)
               ;
      lua_settop(L, -(2)-1);
      return p;
    }
  }
  return 
        ((void *)0)
            ;
}


extern void *luaL_checkudata (lua_State *L, int ud, const char *tname) {
  void *p = luaL_testudata(L, ud, tname);
  ((void)((p != 
 ((void *)0)
 ) || luaL_typeerror(L, (ud), (tname))));
  return p;
}
extern int luaL_checkoption (lua_State *L, int arg, const char *def,
                                 const char *const lst[]) {
  const char *name = (def) ? (luaL_optlstring(L, (arg), (def), 
                            ((void *)0)
                            )) :
                             (luaL_checklstring(L, (arg), 
                            ((void *)0)
                            ));
  int i;
  for (i=0; lst[i]; i++)
    if (strcmp(lst[i], name) == 0)
      return i;
  return luaL_argerror(L, arg,
                       lua_pushfstring(L, "invalid option '%s'", name));
}
extern void luaL_checkstack (lua_State *L, int space, const char *msg) {
  if ((!lua_checkstack(L, space))) {
    if (msg)
      luaL_error(L, "stack overflow (%s)", msg);
    else
      luaL_error(L, "stack overflow");
  }
}


extern void luaL_checktype (lua_State *L, int arg, int t) {
  if ((lua_type(L, arg) != t))
    tag_error(L, arg, t);
}


extern void luaL_checkany (lua_State *L, int arg) {
  if ((lua_type(L, arg) == (-1)))
    luaL_argerror(L, arg, "value expected");
}


extern const char *luaL_checklstring (lua_State *L, int arg, size_t *len) {
  const char *s = lua_tolstring(L, arg, len);
  if ((!s)) tag_error(L, arg, 4);
  return s;
}


extern const char *luaL_optlstring (lua_State *L, int arg,
                                        const char *def, size_t *len) {
  if ((lua_type(L, (arg)) <= 0)) {
    if (len)
      *len = (def ? strlen(def) : 0);
    return def;
  }
  else return luaL_checklstring(L, arg, len);
}


extern lua_Number luaL_checknumber (lua_State *L, int arg) {
  int isnum;
  lua_Number d = lua_tonumberx(L, arg, &isnum);
  if ((!isnum))
    tag_error(L, arg, 3);
  return d;
}


extern lua_Number luaL_optnumber (lua_State *L, int arg, lua_Number def) {
  return ((lua_type(L, ((arg))) <= 0) ? (def) : luaL_checknumber(L,(arg)));
}


static void interror (lua_State *L, int arg) {
  if (lua_isnumber(L, arg))
    luaL_argerror(L, arg, "number has no integer representation");
  else
    tag_error(L, arg, 3);
}


extern lua_Integer luaL_checkinteger (lua_State *L, int arg) {
  int isnum;
  lua_Integer d = lua_tointegerx(L, arg, &isnum);
  if ((!isnum)) {
    interror(L, arg);
  }
  return d;
}


extern lua_Integer luaL_optinteger (lua_State *L, int arg,
                                                      lua_Integer def) {
  return ((lua_type(L, ((arg))) <= 0) ? (def) : luaL_checkinteger(L,(arg)));
}
typedef struct UBox {
  void *box;
  size_t bsize;
} UBox;


static void *resizebox (lua_State *L, int idx, size_t newsize) {
  void *ud;
  lua_Alloc allocf = lua_getallocf(L, &ud);
  UBox *box = (UBox *)lua_touserdata(L, idx);
  void *temp = allocf(ud, box->box, box->bsize, newsize);
  if ((temp == 
     ((void *)0) 
     && newsize > 0)) {
    lua_pushstring(L, "" "not enough memory");
    lua_error(L);
  }
  box->box = temp;
  box->bsize = newsize;
  return temp;
}


static int boxgc (lua_State *L) {
  resizebox(L, 1, 0);
  return 0;
}


static const luaL_Reg boxmt[] = {
  {"__gc", boxgc},
  {"__close", boxgc},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


static void newbox (lua_State *L) {
  UBox *box = (UBox *)lua_newuserdatauv(L, sizeof(UBox), 0);
  box->box = 
            ((void *)0)
                ;
  box->bsize = 0;
  if (luaL_newmetatable(L, "_UBOX*"))
    luaL_setfuncs(L, boxmt, 0);
  lua_setmetatable(L, -2);
}
static size_t newbuffsize (luaL_Buffer *B, size_t sz) {
  size_t newsize = (B->size / 2) * 3;
  if ((((size_t)(~(size_t)0)) - sz < B->n))
    return luaL_error(B->L, "buffer too large");
  if (newsize < B->n + sz)
    newsize = B->n + sz;
  return newsize;
}







static char *prepbuffsize (luaL_Buffer *B, size_t sz, int boxidx) {
  ((void)0);
  if (B->size - B->n >= sz)
    return B->b + B->n;
  else {
    lua_State *L = B->L;
    char *newbuff;
    size_t newsize = newbuffsize(B, sz);

    if (((B)->b != (B)->init.b))
      newbuff = (char *)resizebox(L, boxidx, newsize);
    else {
      (lua_rotate(L, (boxidx), -1), lua_settop(L, -(1)-1));
      newbox(L);
      lua_rotate(L, (boxidx), 1);
      lua_toclose(L, boxidx);
      newbuff = (char *)resizebox(L, boxidx, newsize);
      memcpy(newbuff, B->b, B->n * sizeof(char));
    }
    B->b = newbuff;
    B->size = newsize;
    return newbuff + B->n;
  }
}




extern char *luaL_prepbuffsize (luaL_Buffer *B, size_t sz) {
  return prepbuffsize(B, sz, -1);
}


extern void luaL_addlstring (luaL_Buffer *B, const char *s, size_t l) {
  if (l > 0) {
    char *b = prepbuffsize(B, l, -1);
    memcpy(b, s, l * sizeof(char));
    ((B)->n += (l));
  }
}


extern void luaL_addstring (luaL_Buffer *B, const char *s) {
  luaL_addlstring(B, s, strlen(s));
}


extern void luaL_pushresult (luaL_Buffer *B) {
  lua_State *L = B->L;
  ((void)0);
  lua_pushlstring(L, B->b, B->n);
  if (((B)->b != (B)->init.b))
    lua_closeslot(L, -2);
  (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
}


extern void luaL_pushresultsize (luaL_Buffer *B, size_t sz) {
  ((B)->n += (sz));
  luaL_pushresult(B);
}
extern void luaL_addvalue (luaL_Buffer *B) {
  lua_State *L = B->L;
  size_t len;
  const char *s = lua_tolstring(L, -1, &len);
  char *b = prepbuffsize(B, len, -2);
  memcpy(b, s, len * sizeof(char));
  ((B)->n += (len));
  lua_settop(L, -(1)-1);
}


extern void luaL_buffinit (lua_State *L, luaL_Buffer *B) {
  B->L = L;
  B->b = B->init.b;
  B->n = 0;
  B->size = ((int)(16 * sizeof(void*) * sizeof(lua_Number)));
  lua_pushlightuserdata(L, (void*)B);
}


extern char *luaL_buffinitsize (lua_State *L, luaL_Buffer *B, size_t sz) {
  luaL_buffinit(L, B);
  return prepbuffsize(B, sz, -1);
}
extern int luaL_ref (lua_State *L, int t) {
  int ref;
  if ((lua_type(L, (-1)) == 0)) {
    lua_settop(L, -(1)-1);
    return (-1);
  }
  t = lua_absindex(L, t);
  if (lua_rawgeti(L, t, (2 + 1)) == 0) {
    ref = 0;
    lua_pushinteger(L, 0);
    lua_rawseti(L, t, (2 + 1));
  }
  else {
    ((void)0);
    ref = (int)lua_tointegerx(L,(-1),
              ((void *)0)
              );
  }
  lua_settop(L, -(1)-1);
  if (ref != 0) {
    lua_rawgeti(L, t, ref);
    lua_rawseti(L, t, (2 + 1));
  }
  else
    ref = (int)lua_rawlen(L, t) + 1;
  lua_rawseti(L, t, ref);
  return ref;
}


extern void luaL_unref (lua_State *L, int t, int ref) {
  if (ref >= 0) {
    t = lua_absindex(L, t);
    lua_rawgeti(L, t, (2 + 1));
    ((void)0);
    lua_rawseti(L, t, ref);
    lua_pushinteger(L, ref);
    lua_rawseti(L, t, (2 + 1));
  }
}
typedef struct LoadF {
  int n;
  FILE *f;
  char buff[8192];
} LoadF;


static const char *getF (lua_State *L, void *ud, size_t *size) {
  LoadF *lf = (LoadF *)ud;
  (void)L;
  if (lf->n > 0) {
    *size = lf->n;
    lf->n = 0;
  }
  else {



    if (feof(lf->f)) return 
                           ((void *)0)
                               ;
    *size = fread(lf->buff, 1, sizeof(lf->buff), lf->f);
  }
  return lf->buff;
}


static int errfile (lua_State *L, const char *what, int fnameindex) {
  const char *serr = strerror(
                             (*__errno_location ())
                                  );
  const char *filename = lua_tolstring(L, (fnameindex), 
                        ((void *)0)
                        ) + 1;
  lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
  (lua_rotate(L, (fnameindex), -1), lua_settop(L, -(1)-1));
  return (5 +1);
}
static int skipBOM (FILE *f) {
  int c = getc(f);
  if (c == 0xEF && getc(f) == 0xBB && getc(f) == 0xBF)
    return getc(f);
  else
    return c;
}
static int skipcomment (FILE *f, int *cp) {
  int c = *cp = skipBOM(f);
  if (c == '#') {
    do {
      c = getc(f);
    } while (c != 
                 (-1) 
                     && c != '\n');
    *cp = getc(f);
    return 1;
  }
  else return 0;
}


extern int luaL_loadfilex (lua_State *L, const char *filename,
                                             const char *mode) {
  LoadF lf;
  int status, readstatus;
  int c;
  int fnameindex = lua_gettop(L) + 1;
  if (filename == 
                 ((void *)0)
                     ) {
    lua_pushstring(L, "" "=stdin");
    lf.f = 
          stdin
               ;
  }
  else {
    lua_pushfstring(L, "@%s", filename);
    lf.f = 
          fopen64
               (filename, "r");
    if (lf.f == 
               ((void *)0)
                   ) return errfile(L, "open", fnameindex);
  }
  lf.n = 0;
  if (skipcomment(lf.f, &c))
    lf.buff[lf.n++] = '\n';
  if (c == "\x1bLua"[0]) {
    lf.n = 0;
    if (filename) {
      lf.f = 
            freopen64
                   (filename, "rb", lf.f);
      if (lf.f == 
                 ((void *)0)
                     ) return errfile(L, "reopen", fnameindex);
      skipcomment(lf.f, &c);
    }
  }
  if (c != 
          (-1)
             )
    lf.buff[lf.n++] = c;
  status = lua_load(L, getF, &lf, lua_tolstring(L, (-1), 
                                 ((void *)0)
                                 ), mode);
  readstatus = ferror(lf.f);
  if (filename) fclose(lf.f);
  if (readstatus) {
    lua_settop(L, fnameindex);
    return errfile(L, "read", fnameindex);
  }
  (lua_rotate(L, (fnameindex), -1), lua_settop(L, -(1)-1));
  return status;
}


typedef struct LoadS {
  const char *s;
  size_t size;
} LoadS;


static const char *getS (lua_State *L, void *ud, size_t *size) {
  LoadS *ls = (LoadS *)ud;
  (void)L;
  if (ls->size == 0) return 
                           ((void *)0)
                               ;
  *size = ls->size;
  ls->size = 0;
  return ls->s;
}


extern int luaL_loadbufferx (lua_State *L, const char *buff, size_t size,
                                 const char *name, const char *mode) {
  LoadS ls;
  ls.s = buff;
  ls.size = size;
  return lua_load(L, getS, &ls, name, mode);
}


extern int luaL_loadstring (lua_State *L, const char *s) {
  return luaL_loadbufferx(L,s,strlen(s),s,
        ((void *)0)
        );
}





extern int luaL_getmetafield (lua_State *L, int obj, const char *event) {
  if (!lua_getmetatable(L, obj))
    return 0;
  else {
    int tt;
    lua_pushstring(L, event);
    tt = lua_rawget(L, -2);
    if (tt == 0)
      lua_settop(L, -(2)-1);
    else
      (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
    return tt;
  }
}


extern int luaL_callmeta (lua_State *L, int obj, const char *event) {
  obj = lua_absindex(L, obj);
  if (luaL_getmetafield(L, obj, event) == 0)
    return 0;
  lua_pushvalue(L, obj);
  lua_callk(L, (1), (1), 0, 
 ((void *)0)
 );
  return 1;
}


extern lua_Integer luaL_len (lua_State *L, int idx) {
  lua_Integer l;
  int isnum;
  lua_len(L, idx);
  l = lua_tointegerx(L, -1, &isnum);
  if ((!isnum))
    luaL_error(L, "object length is not an integer");
  lua_settop(L, -(1)-1);
  return l;
}


extern const char *luaL_tolstring (lua_State *L, int idx, size_t *len) {
  idx = lua_absindex(L,idx);
  if (luaL_callmeta(L, idx, "__tostring")) {
    if (!lua_isstring(L, -1))
      luaL_error(L, "'__tostring' must return a string");
  }
  else {
    switch (lua_type(L, idx)) {
      case 3: {
        if (lua_isinteger(L, idx))
          lua_pushfstring(L, "%I", (long long)lua_tointegerx(L,(idx),
                                               ((void *)0)
                                               ));
        else
          lua_pushfstring(L, "%f", (double)lua_tonumberx(L,(idx),
                                                  ((void *)0)
                                                  ));
        break;
      }
      case 4:
        lua_pushvalue(L, idx);
        break;
      case 1:
        lua_pushstring(L, (lua_toboolean(L, idx) ? "true" : "false"));
        break;
      case 0:
        lua_pushstring(L, "" "nil");
        break;
      default: {
        int tt = luaL_getmetafield(L, idx, "__name");
        const char *kind = (tt == 4) ? lua_tolstring(L, (-1), 
                                                ((void *)0)
                                                ) :
                                                 lua_typename(L, lua_type(L,(idx)));
        lua_pushfstring(L, "%s: %p", kind, lua_topointer(L, idx));
        if (tt != 0)
          (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
        break;
      }
    }
  }
  return lua_tolstring(L, -1, len);
}







extern void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup) {
  luaL_checkstack(L, nup, "too many upvalues");
  for (; l->name != 
                   ((void *)0)
                       ; l++) {
    if (l->func == 
                  ((void *)0)
                      )
      lua_pushboolean(L, 0);
    else {
      int i;
      for (i = 0; i < nup; i++)
        lua_pushvalue(L, -nup);
      lua_pushcclosure(L, l->func, nup);
    }
    lua_setfield(L, -(nup + 2), l->name);
  }
  lua_settop(L, -(nup)-1);
}






extern int luaL_getsubtable (lua_State *L, int idx, const char *fname) {
  if (lua_getfield(L, idx, fname) == 5)
    return 1;
  else {
    lua_settop(L, -(1)-1);
    idx = lua_absindex(L, idx);
    lua_createtable(L, 0, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, idx, fname);
    return 0;
  }
}
extern void luaL_requiref (lua_State *L, const char *modname,
                               lua_CFunction openf, int glb) {
  luaL_getsubtable(L, (-1000000 - 1000), "_LOADED");
  lua_getfield(L, -1, modname);
  if (!lua_toboolean(L, -1)) {
    lua_settop(L, -(1)-1);
    lua_pushcclosure(L, (openf), 0);
    lua_pushstring(L, modname);
    lua_callk(L, (1), (1), 0, 
   ((void *)0)
   );
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, modname);
  }
  (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
  if (glb) {
    lua_pushvalue(L, -1);
    lua_setglobal(L, modname);
  }
}


extern void luaL_addgsub (luaL_Buffer *b, const char *s,
                                     const char *p, const char *r) {
  const char *wild;
  size_t l = strlen(p);
  while ((wild = strstr(s, p)) != 
                                 ((void *)0)
                                     ) {
    luaL_addlstring(b, s, wild - s);
    luaL_addstring(b, r);
    s = wild + l;
  }
  luaL_addstring(b, s);
}


extern const char *luaL_gsub (lua_State *L, const char *s,
                                  const char *p, const char *r) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addgsub(&b, s, p, r);
  luaL_pushresult(&b);
  return lua_tolstring(L, (-1), 
        ((void *)0)
        );
}


static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud; (void)osize;
  if (nsize == 0) {
    free(ptr);
    return 
          ((void *)0)
              ;
  }
  else
    return realloc(ptr, nsize);
}


static int panic (lua_State *L) {
  const char *msg = lua_tolstring(L, (-1), 
                   ((void *)0)
                   );
  if (msg == 
            ((void *)0)
                ) msg = "error object is not a string";
  (fprintf(
 stderr
 , ("PANIC: unprotected error in call to Lua API (%s)\n"), (msg)), fflush(
 stderr
 ))
                            ;
  return 0;
}
static void warnfoff (void *ud, const char *message, int tocont);
static void warnfon (void *ud, const char *message, int tocont);
static void warnfcont (void *ud, const char *message, int tocont);






static int checkcontrol (lua_State *L, const char *message, int tocont) {
  if (tocont || *(message++) != '@')
    return 0;
  else {
    if (strcmp(message, "off") == 0)
      lua_setwarnf(L, warnfoff, L);
    else if (strcmp(message, "on") == 0)
      lua_setwarnf(L, warnfon, L);
    return 1;
  }
}


static void warnfoff (void *ud, const char *message, int tocont) {
  checkcontrol((lua_State *)ud, message, tocont);
}






static void warnfcont (void *ud, const char *message, int tocont) {
  lua_State *L = (lua_State *)ud;
  (fprintf(
 stderr
 , ("%s"), (message)), fflush(
 stderr
 ));
  if (tocont)
    lua_setwarnf(L, warnfcont, L);
  else {
    (fprintf(
   stderr
   , ("%s"), ("\n")), fflush(
   stderr
   ));
    lua_setwarnf(L, warnfon, L);
  }
}


static void warnfon (void *ud, const char *message, int tocont) {
  if (checkcontrol((lua_State *)ud, message, tocont))
    return;
  (fprintf(
 stderr
 , ("%s"), ("Lua warning: ")), fflush(
 stderr
 ));
  warnfcont(ud, message, tocont);
}


extern lua_State *luaL_newstate (void) {
  lua_State *L = lua_newstate(l_alloc, 
                                      ((void *)0)
                                          );
  if ((L)) {
    lua_atpanic(L, &panic);
    lua_setwarnf(L, warnfoff, L);
  }
  return L;
}


extern void luaL_checkversion_ (lua_State *L, lua_Number ver, size_t sz) {
  lua_Number v = lua_version(L);
  if (sz != (sizeof(lua_Integer)*16 + sizeof(lua_Number)))
    luaL_error(L, "core and library have incompatible numeric types");
  else if (v != ver)
    luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
                  (double)ver, (double)v);
}


enum
{
  _ISupper = ((0) < 8 ? ((1 << (0)) << 8) : ((1 << (0)) >> 8)),
  _ISlower = ((1) < 8 ? ((1 << (1)) << 8) : ((1 << (1)) >> 8)),
  _ISalpha = ((2) < 8 ? ((1 << (2)) << 8) : ((1 << (2)) >> 8)),
  _ISdigit = ((3) < 8 ? ((1 << (3)) << 8) : ((1 << (3)) >> 8)),
  _ISxdigit = ((4) < 8 ? ((1 << (4)) << 8) : ((1 << (4)) >> 8)),
  _ISspace = ((5) < 8 ? ((1 << (5)) << 8) : ((1 << (5)) >> 8)),
  _ISprint = ((6) < 8 ? ((1 << (6)) << 8) : ((1 << (6)) >> 8)),
  _ISgraph = ((7) < 8 ? ((1 << (7)) << 8) : ((1 << (7)) >> 8)),
  _ISblank = ((8) < 8 ? ((1 << (8)) << 8) : ((1 << (8)) >> 8)),
  _IScntrl = ((9) < 8 ? ((1 << (9)) << 8) : ((1 << (9)) >> 8)),
  _ISpunct = ((10) < 8 ? ((1 << (10)) << 8) : ((1 << (10)) >> 8)),
  _ISalnum = ((11) < 8 ? ((1 << (11)) << 8) : ((1 << (11)) >> 8))
};
extern const unsigned short int **__ctype_b_loc (void)
     ;
extern const __int32_t **__ctype_tolower_loc (void)
     ;
extern const __int32_t **__ctype_toupper_loc (void)
     ;
extern int isalnum (int) ;
extern int isalpha (int) ;
extern int iscntrl (int) ;
extern int isdigit (int) ;
extern int islower (int) ;
extern int isgraph (int) ;
extern int isprint (int) ;
extern int ispunct (int) ;
extern int isspace (int) ;
extern int isupper (int) ;
extern int isxdigit (int) ;



extern int tolower (int __c) ;


extern int toupper (int __c) ;




extern int isblank (int) ;
extern int isascii (int __c) ;



extern int toascii (int __c) ;



extern int _toupper (int) ;
extern int _tolower (int) ;









extern int luaopen_base(lua_State *L);


extern int luaopen_coroutine(lua_State *L);


extern int luaopen_table(lua_State *L);


extern int luaopen_io(lua_State *L);


extern int luaopen_os(lua_State *L);


extern int luaopen_string(lua_State *L);


extern int luaopen_utf8(lua_State *L);


extern int luaopen_math(lua_State *L);


extern int luaopen_debug(lua_State *L);


extern int luaopen_package(lua_State *L);



extern void luaL_openlibs(lua_State *L);


static int luaB_print (lua_State *L) {
  int n = lua_gettop(L);
  int i;
  for (i = 1; i <= n; i++) {
    size_t l;
    const char *s = luaL_tolstring(L, i, &l);
    if (i > 1)
      fwrite(("\t"), sizeof(char), (1), 
     stdout
     );
    fwrite((s), sizeof(char), (l), 
   stdout
   );
    lua_settop(L, -(1)-1);
  }
  (fwrite(("\n"), sizeof(char), (1), 
 stdout
 ), fflush(
 stdout
 ));
  return 0;
}







static int luaB_warn (lua_State *L) {
  int n = lua_gettop(L);
  int i;
  (luaL_checklstring(L, (1), 
 ((void *)0)
 ));
  for (i = 2; i <= n; i++)
    (luaL_checklstring(L, (i), 
   ((void *)0)
   ));
  for (i = 1; i < n; i++)
    lua_warning(L, lua_tolstring(L, (i), 
                  ((void *)0)
                  ), 1);
  lua_warning(L, lua_tolstring(L, (n), 
                ((void *)0)
                ), 0);
  return 0;
}




static const char *b_str2int (const char *s, int base, lua_Integer *pn) {
  lua_Unsigned n = 0;
  int neg = 0;
  s += strspn(s, " \f\n\r\t\v");
  if (*s == '-') { s++; neg = 1; }
  else if (*s == '+') s++;
  if (!
      ((*__ctype_b_loc ())[(int) ((
      (unsigned char)*s
      ))] & (unsigned short int) _ISalnum)
                                )
    return 
          ((void *)0)
              ;
  do {
    int digit = (
                ((*__ctype_b_loc ())[(int) ((
                (unsigned char)*s
                ))] & (unsigned short int) _ISdigit)
                                          ) ? *s - '0'
                   : (toupper((unsigned char)*s) - 'A') + 10;
    if (digit >= base) return 
                             ((void *)0)
                                 ;
    n = n * base + digit;
    s++;
  } while (
          ((*__ctype_b_loc ())[(int) ((
          (unsigned char)*s
          ))] & (unsigned short int) _ISalnum)
                                    );
  s += strspn(s, " \f\n\r\t\v");
  *pn = (lua_Integer)((neg) ? (0u - n) : n);
  return s;
}


static int luaB_tonumber (lua_State *L) {
  if ((lua_type(L, (2)) <= 0)) {
    if (lua_type(L, 1) == 3) {
      lua_settop(L, 1);
      return 1;
    }
    else {
      size_t l;
      const char *s = lua_tolstring(L, 1, &l);
      if (s != 
              ((void *)0) 
                   && lua_stringtonumber(L, s) == l + 1)
        return 1;

      luaL_checkany(L, 1);
    }
  }
  else {
    size_t l;
    const char *s;
    lua_Integer n = 0;
    lua_Integer base = luaL_checkinteger(L, 2);
    luaL_checktype(L, 1, 4);
    s = lua_tolstring(L, 1, &l);
    ((void)((2 <= base && base <= 36) || luaL_argerror(L, (2), ("base out of range"))));
    if (b_str2int(s, (int)base, &n) == s + l) {
      lua_pushinteger(L, n);
      return 1;
    }
  }
  lua_pushnil(L);
  return 1;
}


static int luaB_error (lua_State *L) {
  int level = (int)luaL_optinteger(L, 2, 1);
  lua_settop(L, 1);
  if (lua_type(L, 1) == 4 && level > 0) {
    luaL_where(L, level);
    lua_pushvalue(L, 1);
    lua_concat(L, 2);
  }
  return lua_error(L);
}


static int luaB_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
    return 1;
  }
  luaL_getmetafield(L, 1, "__metatable");
  return 1;
}


static int luaB_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  luaL_checktype(L, 1, 5);
  ((void)((t == 0 || t == 5) || luaL_typeerror(L, (2), ("nil or table"))));
  if ((luaL_getmetafield(L, 1, "__metatable") != 0))
    return luaL_error(L, "cannot change a protected metatable");
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}


static int luaB_rawequal (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_checkany(L, 2);
  lua_pushboolean(L, lua_rawequal(L, 1, 2));
  return 1;
}


static int luaB_rawlen (lua_State *L) {
  int t = lua_type(L, 1);
  ((void)((t == 5 || t == 4) || luaL_typeerror(L, (1), ("table or string"))))
                                        ;
  lua_pushinteger(L, lua_rawlen(L, 1));
  return 1;
}


static int luaB_rawget (lua_State *L) {
  luaL_checktype(L, 1, 5);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  lua_rawget(L, 1);
  return 1;
}

static int luaB_rawset (lua_State *L) {
  luaL_checktype(L, 1, 5);
  luaL_checkany(L, 2);
  luaL_checkany(L, 3);
  lua_settop(L, 3);
  lua_rawset(L, 1);
  return 1;
}


static int pushmode (lua_State *L, int oldmode) {
  if (oldmode == -1)
    lua_pushnil(L);
  else
    lua_pushstring(L, (oldmode == 11) ? "incremental"
                                             : "generational");
  return 1;
}







static int luaB_collectgarbage (lua_State *L) {
  static const char *const opts[] = {"stop", "restart", "collect",
    "count", "step", "setpause", "setstepmul",
    "isrunning", "generational", "incremental", 
                                               ((void *)0)
                                                   };
  static const int optsnum[] = {0, 1, 2,
    3, 5, 6, 7,
    9, 10, 11};
  int o = optsnum[luaL_checkoption(L, 1, "collect", opts)];
  switch (o) {
    case 3: {
      int k = lua_gc(L, o);
      int b = lua_gc(L, 4);
      { if (k == -1) break; };
      lua_pushnumber(L, (lua_Number)k + ((lua_Number)b/1024));
      return 1;
    }
    case 5: {
      int step = (int)luaL_optinteger(L, 2, 0);
      int res = lua_gc(L, o, step);
      { if (res == -1) break; };
      lua_pushboolean(L, res);
      return 1;
    }
    case 6:
    case 7: {
      int p = (int)luaL_optinteger(L, 2, 0);
      int previous = lua_gc(L, o, p);
      { if (previous == -1) break; };
      lua_pushinteger(L, previous);
      return 1;
    }
    case 9: {
      int res = lua_gc(L, o);
      { if (res == -1) break; };
      lua_pushboolean(L, res);
      return 1;
    }
    case 10: {
      int minormul = (int)luaL_optinteger(L, 2, 0);
      int majormul = (int)luaL_optinteger(L, 3, 0);
      return pushmode(L, lua_gc(L, o, minormul, majormul));
    }
    case 11: {
      int pause = (int)luaL_optinteger(L, 2, 0);
      int stepmul = (int)luaL_optinteger(L, 3, 0);
      int stepsize = (int)luaL_optinteger(L, 4, 0);
      return pushmode(L, lua_gc(L, o, pause, stepmul, stepsize));
    }
    default: {
      int res = lua_gc(L, o);
      { if (res == -1) break; };
      lua_pushinteger(L, res);
      return 1;
    }
  }
  lua_pushnil(L);
  return 1;
}


static int luaB_type (lua_State *L) {
  int t = lua_type(L, 1);
  ((void)((t != (-1)) || luaL_argerror(L, (1), ("value expected"))));
  lua_pushstring(L, lua_typename(L, t));
  return 1;
}


static int luaB_next (lua_State *L) {
  luaL_checktype(L, 1, 5);
  lua_settop(L, 2);
  if (lua_next(L, 1))
    return 2;
  else {
    lua_pushnil(L);
    return 1;
  }
}


static int pairscont (lua_State *L, int status, lua_KContext k) {
  (void)L; (void)status; (void)k;
  return 3;
}

static int luaB_pairs (lua_State *L) {
  luaL_checkany(L, 1);
  if (luaL_getmetafield(L, 1, "__pairs") == 0) {
    lua_pushcclosure(L, (luaB_next), 0);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
  }
  else {
    lua_pushvalue(L, 1);
    lua_callk(L, 1, 3, 0, pairscont);
  }
  return 3;
}





static int ipairsaux (lua_State *L) {
  lua_Integer i = luaL_checkinteger(L, 2);
  i = ((lua_Integer)((lua_Unsigned)(i) + (lua_Unsigned)(1)));
  lua_pushinteger(L, i);
  return (lua_geti(L, 1, i) == 0) ? 1 : 2;
}






static int luaB_ipairs (lua_State *L) {
  luaL_checkany(L, 1);
  lua_pushcclosure(L, (ipairsaux), 0);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  return 3;
}


static int load_aux (lua_State *L, int status, int envidx) {
  if ((status == 0)) {
    if (envidx != 0) {
      lua_pushvalue(L, envidx);
      if (!lua_setupvalue(L, -2, 1))
        lua_settop(L, -(1)-1);
    }
    return 1;
  }
  else {
    lua_pushnil(L);
    lua_rotate(L, (-2), 1);
    return 2;
  }
}


static int luaB_loadfile (lua_State *L) {
  const char *fname = (luaL_optlstring(L, (1), (
                     ((void *)0)
                     ), 
                     ((void *)0)
                     ));
  const char *mode = (luaL_optlstring(L, (2), (
                    ((void *)0)
                    ), 
                    ((void *)0)
                    ));
  int env = (!(lua_type(L, (3)) == (-1)) ? 3 : 0);
  int status = luaL_loadfilex(L, fname, mode);
  return load_aux(L, status, env);
}
static const char *generic_reader (lua_State *L, void *ud, size_t *size) {
  (void)(ud);
  luaL_checkstack(L, 2, "too many nested functions");
  lua_pushvalue(L, 1);
  lua_callk(L, (0), (1), 0, 
 ((void *)0)
 );
  if ((lua_type(L, (-1)) == 0)) {
    lua_settop(L, -(1)-1);
    *size = 0;
    return 
          ((void *)0)
              ;
  }
  else if ((!lua_isstring(L, -1)))
    luaL_error(L, "reader function must return a string");
  (lua_copy(L, -1, (5)), lua_settop(L, -(1)-1));
  return lua_tolstring(L, 5, size);
}


static int luaB_load (lua_State *L) {
  int status;
  size_t l;
  const char *s = lua_tolstring(L, 1, &l);
  const char *mode = (luaL_optlstring(L, (3), ("bt"), 
                    ((void *)0)
                    ));
  int env = (!(lua_type(L, (4)) == (-1)) ? 4 : 0);
  if (s != 
          ((void *)0)
              ) {
    const char *chunkname = (luaL_optlstring(L, (2), (s), 
                           ((void *)0)
                           ));
    status = luaL_loadbufferx(L, s, l, chunkname, mode);
  }
  else {
    const char *chunkname = (luaL_optlstring(L, (2), ("=(load)"), 
                           ((void *)0)
                           ));
    luaL_checktype(L, 1, 6);
    lua_settop(L, 5);
    status = lua_load(L, generic_reader, 
                                        ((void *)0)
                                            , chunkname, mode);
  }
  return load_aux(L, status, env);
}




static int dofilecont (lua_State *L, int d1, lua_KContext d2) {
  (void)d1; (void)d2;
  return lua_gettop(L) - 1;
}


static int luaB_dofile (lua_State *L) {
  const char *fname = (luaL_optlstring(L, (1), (
                     ((void *)0)
                     ), 
                     ((void *)0)
                     ));
  lua_settop(L, 1);
  if ((luaL_loadfilex(L,fname,
     ((void *)0)
     ) != 0))
    return lua_error(L);
  lua_callk(L, 0, (-1), 0, dofilecont);
  return dofilecont(L, 0, 0);
}


static int luaB_assert (lua_State *L) {
  if ((lua_toboolean(L, 1)))
    return lua_gettop(L);
  else {
    luaL_checkany(L, 1);
    (lua_rotate(L, (1), -1), lua_settop(L, -(1)-1));
    lua_pushstring(L, "" "assertion failed!");
    lua_settop(L, 1);
    return luaB_error(L);
  }
}


static int luaB_select (lua_State *L) {
  int n = lua_gettop(L);
  if (lua_type(L, 1) == 4 && *lua_tolstring(L, (1), 
                                       ((void *)0)
                                       ) == '#') {
    lua_pushinteger(L, n-1);
    return 1;
  }
  else {
    lua_Integer i = luaL_checkinteger(L, 1);
    if (i < 0) i = n + i;
    else if (i > n) i = n;
    ((void)((1 <= i) || luaL_argerror(L, (1), ("index out of range"))));
    return n - (int)i;
  }
}
static int finishpcall (lua_State *L, int status, lua_KContext extra) {
  if ((status != 0 && status != 1)) {
    lua_pushboolean(L, 0);
    lua_pushvalue(L, -2);
    return 2;
  }
  else
    return lua_gettop(L) - (int)extra;
}


static int luaB_pcall (lua_State *L) {
  int status;
  luaL_checkany(L, 1);
  lua_pushboolean(L, 1);
  lua_rotate(L, (1), 1);
  status = lua_pcallk(L, lua_gettop(L) - 2, (-1), 0, 0, finishpcall);
  return finishpcall(L, status, 0);
}







static int luaB_xpcall (lua_State *L) {
  int status;
  int n = lua_gettop(L);
  luaL_checktype(L, 2, 6);
  lua_pushboolean(L, 1);
  lua_pushvalue(L, 1);
  lua_rotate(L, 3, 2);
  status = lua_pcallk(L, n - 2, (-1), 2, 2, finishpcall);
  return finishpcall(L, status, 2);
}


static int luaB_tostring (lua_State *L) {
  luaL_checkany(L, 1);
  luaL_tolstring(L, 1, 
                      ((void *)0)
                          );
  return 1;
}


static const luaL_Reg base_funcs[] = {
  {"assert", luaB_assert},
  {"collectgarbage", luaB_collectgarbage},
  {"dofile", luaB_dofile},
  {"error", luaB_error},
  {"getmetatable", luaB_getmetatable},
  {"ipairs", luaB_ipairs},
  {"loadfile", luaB_loadfile},
  {"load", luaB_load},
  {"next", luaB_next},
  {"pairs", luaB_pairs},
  {"pcall", luaB_pcall},
  {"print", luaB_print},
  {"warn", luaB_warn},
  {"rawequal", luaB_rawequal},
  {"rawlen", luaB_rawlen},
  {"rawget", luaB_rawget},
  {"rawset", luaB_rawset},
  {"select", luaB_select},
  {"setmetatable", luaB_setmetatable},
  {"tonumber", luaB_tonumber},
  {"tostring", luaB_tostring},
  {"type", luaB_type},
  {"xpcall", luaB_xpcall},

  {"_G", 
             ((void *)0)
                 },
  {"_VERSION", 
              ((void *)0)
                  },
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


extern int luaopen_base (lua_State *L) {

  ((void)lua_rawgeti(L, (-1000000 - 1000), 2));
  luaL_setfuncs(L, base_funcs, 0);

  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "_G");

  lua_pushstring(L, "" "Lua " "5" "." "4");
  lua_setfield(L, -2, "_VERSION");
  return 1;
}
static lua_State *getco (lua_State *L) {
  lua_State *co = lua_tothread(L, 1);
  ((void)((co) || luaL_typeerror(L, (1), ("thread"))));
  return co;
}






static int auxresume (lua_State *L, lua_State *co, int narg) {
  int status, nres;
  if ((!lua_checkstack(co, narg))) {
    lua_pushstring(L, "" "too many arguments to resume");
    return -1;
  }
  lua_xmove(L, co, narg);
  status = lua_resume(co, L, narg, &nres);
  if ((status == 0 || status == 1)) {
    if ((!lua_checkstack(L, nres + 1))) {
      lua_settop(co, -(nres)-1);
      lua_pushstring(L, "" "too many results to resume");
      return -1;
    }
    lua_xmove(co, L, nres);
    return nres;
  }
  else {
    lua_xmove(co, L, 1);
    return -1;
  }
}


static int luaB_coresume (lua_State *L) {
  lua_State *co = getco(L);
  int r;
  r = auxresume(L, co, lua_gettop(L) - 1);
  if ((r < 0)) {
    lua_pushboolean(L, 0);
    lua_rotate(L, (-2), 1);
    return 2;
  }
  else {
    lua_pushboolean(L, 1);
    lua_rotate(L, (-(r + 1)), 1);
    return r + 1;
  }
}


static int luaB_auxwrap (lua_State *L) {
  lua_State *co = lua_tothread(L, ((-1000000 - 1000) - (1)));
  int r = auxresume(L, co, lua_gettop(L));
  if ((r < 0)) {
    int stat = lua_status(co);
    if (stat != 0 && stat != 1) {
      stat = lua_closethread(co, L);
      ((void)0);
      lua_xmove(co, L, 1);
    }
    if (stat != 4 &&
        lua_type(L, -1) == 4) {
      luaL_where(L, 1);
      lua_rotate(L, (-2), 1);
      lua_concat(L, 2);
    }
    return lua_error(L);
  }
  return r;
}


static int luaB_cocreate (lua_State *L) {
  lua_State *NL;
  luaL_checktype(L, 1, 6);
  NL = lua_newthread(L);
  lua_pushvalue(L, 1);
  lua_xmove(L, NL, 1);
  return 1;
}


static int luaB_cowrap (lua_State *L) {
  luaB_cocreate(L);
  lua_pushcclosure(L, luaB_auxwrap, 1);
  return 1;
}


static int luaB_yield (lua_State *L) {
  return lua_yieldk(L, (lua_gettop(L)), 0, 
        ((void *)0)
        );
}
static const char *const statname[] =
  {"running", "dead", "suspended", "normal"};


static int auxstatus (lua_State *L, lua_State *co) {
  if (L == co) return 0;
  else {
    switch (lua_status(co)) {
      case 1:
        return 2;
      case 0: {
        lua_Debug ar;
        if (lua_getstack(co, 0, &ar))
          return 3;
        else if (lua_gettop(co) == 0)
            return 1;
        else
          return 2;
      }
      default:
        return 1;
    }
  }
}


static int luaB_costatus (lua_State *L) {
  lua_State *co = getco(L);
  lua_pushstring(L, statname[auxstatus(L, co)]);
  return 1;
}


static int luaB_yieldable (lua_State *L) {
  lua_State *co = (lua_type(L, (1)) == (-1)) ? L : getco(L);
  lua_pushboolean(L, lua_isyieldable(co));
  return 1;
}


static int luaB_corunning (lua_State *L) {
  int ismain = lua_pushthread(L);
  lua_pushboolean(L, ismain);
  return 2;
}


static int luaB_close (lua_State *L) {
  lua_State *co = getco(L);
  int status = auxstatus(L, co);
  switch (status) {
    case 1: case 2: {
      status = lua_closethread(co, L);
      if (status == 0) {
        lua_pushboolean(L, 1);
        return 1;
      }
      else {
        lua_pushboolean(L, 0);
        lua_xmove(co, L, 1);
        return 2;
      }
    }
    default:
      return luaL_error(L, "cannot close a %s coroutine", statname[status]);
  }
}


static const luaL_Reg co_funcs[] = {
  {"create", luaB_cocreate},
  {"resume", luaB_coresume},
  {"running", luaB_corunning},
  {"status", luaB_costatus},
  {"wrap", luaB_cowrap},
  {"yield", luaB_yield},
  {"isyieldable", luaB_yieldable},
  {"close", luaB_close},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};



extern int luaopen_coroutine (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(co_funcs)/sizeof((co_funcs)[0]) - 1), luaL_setfuncs(L,co_funcs,0));
  return 1;
}
static const char *const HOOKKEY = "_HOOKKEY";







static void checkstack (lua_State *L, lua_State *L1, int n) {
  if ((L != L1 && !lua_checkstack(L1, n)))
    luaL_error(L, "stack overflow");
}


static int db_getregistry (lua_State *L) {
  lua_pushvalue(L, (-1000000 - 1000));
  return 1;
}


static int db_getmetatable (lua_State *L) {
  luaL_checkany(L, 1);
  if (!lua_getmetatable(L, 1)) {
    lua_pushnil(L);
  }
  return 1;
}


static int db_setmetatable (lua_State *L) {
  int t = lua_type(L, 2);
  ((void)((t == 0 || t == 5) || luaL_typeerror(L, (2), ("nil or table"))));
  lua_settop(L, 2);
  lua_setmetatable(L, 1);
  return 1;
}


static int db_getuservalue (lua_State *L) {
  int n = (int)luaL_optinteger(L, 2, 1);
  if (lua_type(L, 1) != 7)
    lua_pushnil(L);
  else if (lua_getiuservalue(L, 1, n) != (-1)) {
    lua_pushboolean(L, 1);
    return 2;
  }
  return 1;
}


static int db_setuservalue (lua_State *L) {
  int n = (int)luaL_optinteger(L, 3, 1);
  luaL_checktype(L, 1, 7);
  luaL_checkany(L, 2);
  lua_settop(L, 2);
  if (!lua_setiuservalue(L, 1, n))
    lua_pushnil(L);
  return 1;
}
static lua_State *getthread (lua_State *L, int *arg) {
  if ((lua_type(L, (1)) == 8)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;
  }
}







static void settabss (lua_State *L, const char *k, const char *v) {
  lua_pushstring(L, v);
  lua_setfield(L, -2, k);
}

static void settabsi (lua_State *L, const char *k, int v) {
  lua_pushinteger(L, v);
  lua_setfield(L, -2, k);
}

static void settabsb (lua_State *L, const char *k, int v) {
  lua_pushboolean(L, v);
  lua_setfield(L, -2, k);
}
static void treatstackoption (lua_State *L, lua_State *L1, const char *fname) {
  if (L == L1)
    lua_rotate(L, -2, 1);
  else
    lua_xmove(L1, L, 1);
  lua_setfield(L, -2, fname);
}
static int db_getinfo (lua_State *L) {
  lua_Debug ar;
  int arg;
  lua_State *L1 = getthread(L, &arg);
  const char *options = (luaL_optlstring(L, (arg+2), ("flnSrtu"), 
                       ((void *)0)
                       ));
  checkstack(L, L1, 3);
  ((void)((options[0] != '>') || luaL_argerror(L, (arg + 2), ("invalid option '>'"))));
  if ((lua_type(L, (arg + 1)) == 6)) {
    options = lua_pushfstring(L, ">%s", options);
    lua_pushvalue(L, arg + 1);
    lua_xmove(L, L1, 1);
  }
  else {
    if (!lua_getstack(L1, (int)luaL_checkinteger(L, arg + 1), &ar)) {
      lua_pushnil(L);
      return 1;
    }
  }
  if (!lua_getinfo(L1, options, &ar))
    return luaL_argerror(L, arg+2, "invalid option");
  lua_createtable(L, 0, 0);
  if (strchr(options, 'S')) {
    lua_pushlstring(L, ar.source, ar.srclen);
    lua_setfield(L, -2, "source");
    settabss(L, "short_src", ar.short_src);
    settabsi(L, "linedefined", ar.linedefined);
    settabsi(L, "lastlinedefined", ar.lastlinedefined);
    settabss(L, "what", ar.what);
  }
  if (strchr(options, 'l'))
    settabsi(L, "currentline", ar.currentline);
  if (strchr(options, 'u')) {
    settabsi(L, "nups", ar.nups);
    settabsi(L, "nparams", ar.nparams);
    settabsb(L, "isvararg", ar.isvararg);
  }
  if (strchr(options, 'n')) {
    settabss(L, "name", ar.name);
    settabss(L, "namewhat", ar.namewhat);
  }
  if (strchr(options, 'r')) {
    settabsi(L, "ftransfer", ar.ftransfer);
    settabsi(L, "ntransfer", ar.ntransfer);
  }
  if (strchr(options, 't'))
    settabsb(L, "istailcall", ar.istailcall);
  if (strchr(options, 'L'))
    treatstackoption(L, L1, "activelines");
  if (strchr(options, 'f'))
    treatstackoption(L, L1, "func");
  return 1;
}


static int db_getlocal (lua_State *L) {
  int arg;
  lua_State *L1 = getthread(L, &arg);
  int nvar = (int)luaL_checkinteger(L, arg + 2);
  if ((lua_type(L, (arg + 1)) == 6)) {
    lua_pushvalue(L, arg + 1);
    lua_pushstring(L, lua_getlocal(L, 
                                     ((void *)0)
                                         , nvar));
    return 1;
  }
  else {
    lua_Debug ar;
    const char *name;
    int level = (int)luaL_checkinteger(L, arg + 1);
    if ((!lua_getstack(L1, level, &ar)))
      return luaL_argerror(L, arg+1, "level out of range");
    checkstack(L, L1, 1);
    name = lua_getlocal(L1, &ar, nvar);
    if (name) {
      lua_xmove(L1, L, 1);
      lua_pushstring(L, name);
      lua_rotate(L, -2, 1);
      return 2;
    }
    else {
      lua_pushnil(L);
      return 1;
    }
  }
}


static int db_setlocal (lua_State *L) {
  int arg;
  const char *name;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;
  int level = (int)luaL_checkinteger(L, arg + 1);
  int nvar = (int)luaL_checkinteger(L, arg + 2);
  if ((!lua_getstack(L1, level, &ar)))
    return luaL_argerror(L, arg+1, "level out of range");
  luaL_checkany(L, arg+3);
  lua_settop(L, arg+3);
  checkstack(L, L1, 1);
  lua_xmove(L, L1, 1);
  name = lua_setlocal(L1, &ar, nvar);
  if (name == 
             ((void *)0)
                 )
    lua_settop(L1, -(1)-1);
  lua_pushstring(L, name);
  return 1;
}





static int auxupvalue (lua_State *L, int get) {
  const char *name;
  int n = (int)luaL_checkinteger(L, 2);
  luaL_checktype(L, 1, 6);
  name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
  if (name == 
             ((void *)0)
                 ) return 0;
  lua_pushstring(L, name);
  lua_rotate(L, (-(get+1)), 1);
  return get + 1;
}


static int db_getupvalue (lua_State *L) {
  return auxupvalue(L, 1);
}


static int db_setupvalue (lua_State *L) {
  luaL_checkany(L, 3);
  return auxupvalue(L, 0);
}






static void *checkupval (lua_State *L, int argf, int argnup, int *pnup) {
  void *id;
  int nup = (int)luaL_checkinteger(L, argnup);
  luaL_checktype(L, argf, 6);
  id = lua_upvalueid(L, argf, nup);
  if (pnup) {
    ((void)((id != 
   ((void *)0)
   ) || luaL_argerror(L, (argnup), ("invalid upvalue index"))));
    *pnup = nup;
  }
  return id;
}


static int db_upvalueid (lua_State *L) {
  void *id = checkupval(L, 1, 2, 
                                ((void *)0)
                                    );
  if (id != 
           ((void *)0)
               )
    lua_pushlightuserdata(L, id);
  else
    lua_pushnil(L);
  return 1;
}


static int db_upvaluejoin (lua_State *L) {
  int n1, n2;
  checkupval(L, 1, 2, &n1);
  checkupval(L, 3, 4, &n2);
  ((void)((!lua_iscfunction(L, 1)) || luaL_argerror(L, (1), ("Lua function expected"))));
  ((void)((!lua_iscfunction(L, 3)) || luaL_argerror(L, (3), ("Lua function expected"))));
  lua_upvaluejoin(L, 1, n1, 3, n2);
  return 0;
}






static void hookf (lua_State *L, lua_Debug *ar) {
  static const char *const hooknames[] =
    {"call", "return", "line", "count", "tail call"};
  lua_getfield(L, (-1000000 - 1000), HOOKKEY);
  lua_pushthread(L);
  if (lua_rawget(L, -2) == 6) {
    lua_pushstring(L, hooknames[(int)ar->event]);
    if (ar->currentline >= 0)
      lua_pushinteger(L, ar->currentline);
    else lua_pushnil(L);
    ((void)0);
    lua_callk(L, (2), (0), 0, 
   ((void *)0)
   );
  }
}





static int makemask (const char *smask, int count) {
  int mask = 0;
  if (strchr(smask, 'c')) mask |= (1 << 0);
  if (strchr(smask, 'r')) mask |= (1 << 1);
  if (strchr(smask, 'l')) mask |= (1 << 2);
  if (count > 0) mask |= (1 << 3);
  return mask;
}





static char *unmakemask (int mask, char *smask) {
  int i = 0;
  if (mask & (1 << 0)) smask[i++] = 'c';
  if (mask & (1 << 1)) smask[i++] = 'r';
  if (mask & (1 << 2)) smask[i++] = 'l';
  smask[i] = '\0';
  return smask;
}


static int db_sethook (lua_State *L) {
  int arg, mask, count;
  lua_Hook func;
  lua_State *L1 = getthread(L, &arg);
  if ((lua_type(L, (arg+1)) <= 0)) {
    lua_settop(L, arg+1);
    func = 
          ((void *)0)
              ; mask = 0; count = 0;
  }
  else {
    const char *smask = (luaL_checklstring(L, (arg+2), 
                       ((void *)0)
                       ));
    luaL_checktype(L, arg+1, 6);
    count = (int)luaL_optinteger(L, arg + 3, 0);
    func = hookf; mask = makemask(smask, count);
  }
  if (!luaL_getsubtable(L, (-1000000 - 1000), HOOKKEY)) {

    lua_pushstring(L, "" "k");
    lua_setfield(L, -2, "__mode");
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);
  }
  checkstack(L, L1, 1);
  lua_pushthread(L1); lua_xmove(L1, L, 1);
  lua_pushvalue(L, arg + 1);
  lua_rawset(L, -3);
  lua_sethook(L1, func, mask, count);
  return 0;
}


static int db_gethook (lua_State *L) {
  int arg;
  lua_State *L1 = getthread(L, &arg);
  char buff[5];
  int mask = lua_gethookmask(L1);
  lua_Hook hook = lua_gethook(L1);
  if (hook == 
             ((void *)0)
                 ) {
    lua_pushnil(L);
    return 1;
  }
  else if (hook != hookf)
    lua_pushstring(L, "" "external hook");
  else {
    lua_getfield(L, (-1000000 - 1000), HOOKKEY);
    checkstack(L, L1, 1);
    lua_pushthread(L1); lua_xmove(L1, L, 1);
    lua_rawget(L, -2);
    (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
  }
  lua_pushstring(L, unmakemask(mask, buff));
  lua_pushinteger(L, lua_gethookcount(L1));
  return 3;
}


static int db_debug (lua_State *L) {
  for (;;) {
    char buffer[250];
    (fprintf(
   stderr
   , ("%s"), ("lua_debug> ")), fflush(
   stderr
   ));
    if (fgets(buffer, sizeof(buffer), 
                                     stdin
                                          ) == 
                                               ((void *)0) 
                                                    ||
        strcmp(buffer, "cont\n") == 0)
      return 0;
    if (luaL_loadbufferx(L,buffer,strlen(buffer),"=(debug command)",
       ((void *)0)
       ) ||
        lua_pcallk(L, (0), (0), (0), 0, 
       ((void *)0)
       ))
      (fprintf(
     stderr
     , ("%s\n"), (luaL_tolstring(L, -1, 
     ((void *)0)
     ))), fflush(
     stderr
     ));
    lua_settop(L, 0);
  }
}


static int db_traceback (lua_State *L) {
  int arg;
  lua_State *L1 = getthread(L, &arg);
  const char *msg = lua_tolstring(L, (arg + 1), 
                   ((void *)0)
                   );
  if (msg == 
            ((void *)0) 
                 && !(lua_type(L, (arg + 1)) <= 0))
    lua_pushvalue(L, arg + 1);
  else {
    int level = (int)luaL_optinteger(L, arg + 2, (L == L1) ? 1 : 0);
    luaL_traceback(L, L1, msg, level);
  }
  return 1;
}


static int db_setcstacklimit (lua_State *L) {
  int limit = (int)luaL_checkinteger(L, 1);
  int res = lua_setcstacklimit(L, limit);
  lua_pushinteger(L, res);
  return 1;
}


static const luaL_Reg dblib[] = {
  {"debug", db_debug},
  {"getuservalue", db_getuservalue},
  {"gethook", db_gethook},
  {"getinfo", db_getinfo},
  {"getlocal", db_getlocal},
  {"getregistry", db_getregistry},
  {"getmetatable", db_getmetatable},
  {"getupvalue", db_getupvalue},
  {"upvaluejoin", db_upvaluejoin},
  {"upvalueid", db_upvalueid},
  {"setuservalue", db_setuservalue},
  {"sethook", db_sethook},
  {"setlocal", db_setlocal},
  {"setmetatable", db_setmetatable},
  {"setupvalue", db_setupvalue},
  {"traceback", db_traceback},
  {"setcstacklimit", db_setcstacklimit},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


extern int luaopen_debug (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(dblib)/sizeof((dblib)[0]) - 1), luaL_setfuncs(L,dblib,0));
  return 1;
}
static int l_checkmode (const char *mode) {
  return (*mode != '\0' && strchr("rwa", *(mode++)) != 
                                                      ((void *)0) 
                                                           &&
         (*mode != '+' || ((void)(++mode), 1)) &&
         (strspn(mode, "b") == strlen(mode)));
}
typedef luaL_Stream LStream;







static int io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, "FILE*");
  if (p == 
          ((void *)0)
              )
    lua_pushnil(L);
  else if (((p)->closef == 
          ((void *)0)
          ))
    lua_pushstring(L, "" "closed file");
  else
    lua_pushstring(L, "" "file");
  return 1;
}


static int f_tostring (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  if (((p)->closef == 
     ((void *)0)
     ))
    lua_pushstring(L, "" "file (closed)");
  else
    lua_pushfstring(L, "file (%p)", p->f);
  return 1;
}


static FILE *tofile (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  if ((((p)->closef == 
     ((void *)0)
     )))
    luaL_error(L, "attempt to use a closed file");
  ((void)0);
  return p->f;
}







static LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdatauv(L, sizeof(LStream), 0);
  p->closef = 
             ((void *)0)
                 ;
  luaL_setmetatable(L, "FILE*");
  return p;
}







static int aux_close (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
   lua_CFunction cf = p->closef;
  p->closef = 
             ((void *)0)
                 ;
  return (*cf)(L);
}


static int f_close (lua_State *L) {
  tofile(L);
  return aux_close(L);
}


static int io_close (lua_State *L) {
  if ((lua_type(L, (1)) == (-1)))
    lua_getfield(L, (-1000000 - 1000), ("_IO_" "output"));
  return f_close(L);
}


static int f_gc (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  if (!((p)->closef == 
      ((void *)0)
      ) && p->f != 
                             ((void *)0)
                                 )
    aux_close(L);
  return 0;
}





static int io_fclose (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  int res = fclose(p->f);
  return luaL_fileresult(L, (res == 0), 
                                       ((void *)0)
                                           );
}


static LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->f = 
        ((void *)0)
            ;
  p->closef = &io_fclose;
  return p;
}


static void opencheck (lua_State *L, const char *fname, const char *mode) {
  LStream *p = newfile(L);
  p->f = 
        fopen64
             (fname, mode);
  if ((p->f == 
     ((void *)0)
     ))
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(
                                                               (*__errno_location ())
                                                                    ));
}


static int io_open (lua_State *L) {
  const char *filename = (luaL_checklstring(L, (1), 
                        ((void *)0)
                        ));
  const char *mode = (luaL_optlstring(L, (2), ("r"), 
                    ((void *)0)
                    ));
  LStream *p = newfile(L);
  const char *md = mode;
  ((void)((l_checkmode(md)) || luaL_argerror(L, (2), ("invalid mode"))));
  p->f = 
        fopen64
             (filename, mode);
  return (p->f == 
                 ((void *)0)
                     ) ? luaL_fileresult(L, 0, filename) : 1;
}





static int io_pclose (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  
 (*__errno_location ()) 
       = 0;
  return luaL_execresult(L, (pclose(p->f)));
}


static int io_popen (lua_State *L) {
  const char *filename = (luaL_checklstring(L, (1), 
                        ((void *)0)
                        ));
  const char *mode = (luaL_optlstring(L, (2), ("r"), 
                    ((void *)0)
                    ));
  LStream *p = newprefile(L);
  ((void)((((mode[0] == 'r' || mode[0] == 'w') && mode[1] == '\0')) || luaL_argerror(L, (2), ("invalid mode"))));
  p->f = (fflush(
        ((void *)0)
        ), popen(filename,mode));
  p->closef = &io_pclose;
  return (p->f == 
                 ((void *)0)
                     ) ? luaL_fileresult(L, 0, filename) : 1;
}


static int io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  p->f = 
        tmpfile64
               ();
  return (p->f == 
                 ((void *)0)
                     ) ? luaL_fileresult(L, 0, 
                                               ((void *)0)
                                                   ) : 1;
}


static FILE *getiofile (lua_State *L, const char *findex) {
  LStream *p;
  lua_getfield(L, (-1000000 - 1000), findex);
  p = (LStream *)lua_touserdata(L, -1);
  if ((((p)->closef == 
     ((void *)0)
     )))
    luaL_error(L, "default %s file is closed", findex + (sizeof("_IO_")/sizeof(char) - 1));
  return p->f;
}


static int g_iofile (lua_State *L, const char *f, const char *mode) {
  if (!(lua_type(L, (1)) <= 0)) {
    const char *filename = lua_tolstring(L, (1), 
                          ((void *)0)
                          );
    if (filename)
      opencheck(L, filename, mode);
    else {
      tofile(L);
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, (-1000000 - 1000), f);
  }

  lua_getfield(L, (-1000000 - 1000), f);
  return 1;
}


static int io_input (lua_State *L) {
  return g_iofile(L, ("_IO_" "input"), "r");
}


static int io_output (lua_State *L) {
  return g_iofile(L, ("_IO_" "output"), "w");
}


static int io_readline (lua_State *L);
static void aux_lines (lua_State *L, int toclose) {
  int n = lua_gettop(L) - 1;
  ((void)((n <= 250) || luaL_argerror(L, (250 + 2), ("too many arguments"))));
  lua_pushvalue(L, 1);
  lua_pushinteger(L, n);
  lua_pushboolean(L, toclose);
  lua_rotate(L, 2, 3);
  lua_pushcclosure(L, io_readline, 3 + n);
}


static int f_lines (lua_State *L) {
  tofile(L);
  aux_lines(L, 0);
  return 1;
}







static int io_lines (lua_State *L) {
  int toclose;
  if ((lua_type(L, (1)) == (-1))) lua_pushnil(L);
  if ((lua_type(L, (1)) == 0)) {
    lua_getfield(L, (-1000000 - 1000), ("_IO_" "input"));
    (lua_copy(L, -1, (1)), lua_settop(L, -(1)-1));
    tofile(L);
    toclose = 0;
  }
  else {
    const char *filename = (luaL_checklstring(L, (1), 
                          ((void *)0)
                          ));
    opencheck(L, filename, "r");
    (lua_copy(L, -1, (1)), lua_settop(L, -(1)-1));
    toclose = 1;
  }
  aux_lines(L, toclose);
  if (toclose) {
    lua_pushnil(L);
    lua_pushnil(L);
    lua_pushvalue(L, 1);
    return 4;
  }
  else
    return 1;
}
typedef struct {
  FILE *f;
  int c;
  int n;
  char buff[201];
} RN;





static int nextc (RN *rn) {
  if ((rn->n >= 200)) {
    rn->buff[0] = '\0';
    return 0;
  }
  else {
    rn->buff[rn->n++] = rn->c;
    rn->c = getc_unlocked(rn->f);
    return 1;
  }
}





static int test2 (RN *rn, const char *set) {
  if (rn->c == set[0] || rn->c == set[1])
    return nextc(rn);
  else return 0;
}





static int readdigits (RN *rn, int hex) {
  int count = 0;
  while ((hex ? 
               ((*__ctype_b_loc ())[(int) ((
               rn->c
               ))] & (unsigned short int) _ISxdigit) 
                               : 
                                 ((*__ctype_b_loc ())[(int) ((
                                 rn->c
                                 ))] & (unsigned short int) _ISdigit)
                                               ) && nextc(rn))
    count++;
  return count;
}







static int read_number (lua_State *L, FILE *f) {
  RN rn;
  int count = 0;
  int hex = 0;
  char decp[2];
  rn.f = f; rn.n = 0;
  decp[0] = (localeconv()->decimal_point[0]);
  decp[1] = '.';
  flockfile(rn.f);
  do { rn.c = getc_unlocked(rn.f); } while (
                                    ((*__ctype_b_loc ())[(int) ((
                                    rn.c
                                    ))] & (unsigned short int) _ISspace)
                                                 );
  test2(&rn, "-+");
  if (test2(&rn, "00")) {
    if (test2(&rn, "xX")) hex = 1;
    else count = 1;
  }
  count += readdigits(&rn, hex);
  if (test2(&rn, decp))
    count += readdigits(&rn, hex);
  if (count > 0 && test2(&rn, (hex ? "pP" : "eE"))) {
    test2(&rn, "-+");
    readdigits(&rn, 0);
  }
  ungetc(rn.c, rn.f);
  funlockfile(rn.f);
  rn.buff[rn.n] = '\0';
  if ((lua_stringtonumber(L, rn.buff)))
    return 1;
  else {
   lua_pushnil(L);
   return 0;
  }
}


static int test_eof (lua_State *L, FILE *f) {
  int c = getc(f);
  ungetc(c, f);
  lua_pushstring(L, "" "");
  return (c != 
              (-1)
                 );
}


static int read_line (lua_State *L, FILE *f, int chop) {
  luaL_Buffer b;
  int c;
  luaL_buffinit(L, &b);
  do {
    char *buff = luaL_prepbuffsize(&b, ((int)(16 * sizeof(void*) * sizeof(lua_Number))));
    int i = 0;
    flockfile(f);
    while (i < ((int)(16 * sizeof(void*) * sizeof(lua_Number))) && (c = getc_unlocked(f)) != 
                                                    (-1) 
                                                        && c != '\n')
      buff[i++] = c;
    funlockfile(f);
    ((&b)->n += (i));
  } while (c != 
               (-1) 
                   && c != '\n');
  if (!chop && c == '\n')
    ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (c)));
  luaL_pushresult(&b);

  return (c == '\n' || lua_rawlen(L, -1) > 0);
}


static void read_all (lua_State *L, FILE *f) {
  size_t nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {
    char *p = luaL_prepbuffsize(&b, ((int)(16 * sizeof(void*) * sizeof(lua_Number))));
    nr = fread(p, sizeof(char), ((int)(16 * sizeof(void*) * sizeof(lua_Number))), f);
    ((&b)->n += (nr));
  } while (nr == ((int)(16 * sizeof(void*) * sizeof(lua_Number))));
  luaL_pushresult(&b);
}


static int read_chars (lua_State *L, FILE *f, size_t n) {
  size_t nr;
  char *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);
  nr = fread(p, sizeof(char), n, f);
  ((&b)->n += (nr));
  luaL_pushresult(&b);
  return (nr > 0);
}


static int g_read (lua_State *L, FILE *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int n, success;
  clearerr(f);
  if (nargs == 0) {
    success = read_line(L, f, 1);
    n = first + 1;
  }
  else {

    luaL_checkstack(L, nargs+20, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == 3) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = (luaL_checklstring(L, (n), 
                       ((void *)0)
                       ));
        if (*p == '*') p++;
        switch (*p) {
          case 'n':
            success = read_number(L, f);
            break;
          case 'l':
            success = read_line(L, f, 1);
            break;
          case 'L':
            success = read_line(L, f, 0);
            break;
          case 'a':
            read_all(L, f);
            success = 1;
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (ferror(f))
    return luaL_fileresult(L, 0, 
                                ((void *)0)
                                    );
  if (!success) {
    lua_settop(L, -(1)-1);
    lua_pushnil(L);
  }
  return n - first;
}


static int io_read (lua_State *L) {
  return g_read(L, getiofile(L, ("_IO_" "input")), 1);
}


static int f_read (lua_State *L) {
  return g_read(L, tofile(L), 2);
}





static int io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, ((-1000000 - 1000) - (1)));
  int i;
  int n = (int)lua_tointegerx(L,(((-1000000 - 1000) - (2))),
              ((void *)0)
              );
  if (((p)->closef == 
     ((void *)0)
     ))
    return luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  for (i = 1; i <= n; i++)
    lua_pushvalue(L, ((-1000000 - 1000) - (3 + i)));
  n = g_read(L, p->f, 2);
  ((void)0);
  if (lua_toboolean(L, -n))
    return n;
  else {
    if (n > 1) {

      return luaL_error(L, "%s", lua_tolstring(L, (-n + 1), 
                                ((void *)0)
                                ));
    }
    if (lua_toboolean(L, ((-1000000 - 1000) - (3)))) {
      lua_settop(L, 0);
      lua_pushvalue(L, ((-1000000 - 1000) - (1)));
      aux_close(L);
    }
    return 0;
  }
}




static int g_write (lua_State *L, FILE *f, int arg) {
  int nargs = lua_gettop(L) - arg;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == 3) {

      int len = lua_isinteger(L, arg)
                ? fprintf(f, "%" "ll" "d",
                             (long long)lua_tointegerx(L,(arg),
                                         ((void *)0)
                                         ))
                : fprintf(f, "%.14g",
                             (double)lua_tonumberx(L,(arg),
                                            ((void *)0)
                                            ));
      status = status && (len > 0);
    }
    else {
      size_t l;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (fwrite(s, sizeof(char), l, f) == l);
    }
  }
  if ((status))
    return 1;
  else return luaL_fileresult(L, status, 
                                        ((void *)0)
                                            );
}


static int io_write (lua_State *L) {
  return g_write(L, getiofile(L, ("_IO_" "output")), 1);
}


static int f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);
  return g_write(L, f, 2);
}


static int f_seek (lua_State *L) {
  static const int mode[] = {
                            0
                                    , 
                                      1
                                              , 
                                                2
                                                        };
  static const char *const modenames[] = {"set", "cur", "end", 
                                                              ((void *)0)
                                                                  };
  FILE *f = tofile(L);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  off_t offset = (off_t)p3;
  ((void)(((lua_Integer)offset == p3) || luaL_argerror(L, (3), ("not an integer in proper range"))))
                                                   ;
  op = 
      fseeko64
      (f,offset,mode[op]);
  if ((op))
    return luaL_fileresult(L, 0, 
                                ((void *)0)
                                    );
  else {
    lua_pushinteger(L, (lua_Integer)
                                   ftello64
                                   (f));
    return 1;
  }
}


static int f_setvbuf (lua_State *L) {
  static const int mode[] = {
                            2
                                  , 
                                    0
                                          , 
                                            1
                                                  };
  static const char *const modenames[] = {"no", "full", "line", 
                                                               ((void *)0)
                                                                   };
  FILE *f = tofile(L);
  int op = luaL_checkoption(L, 2, 
                                 ((void *)0)
                                     , modenames);
  lua_Integer sz = luaL_optinteger(L, 3, ((int)(16 * sizeof(void*) * sizeof(lua_Number))));
  int res = setvbuf(f, 
                      ((void *)0)
                          , mode[op], (size_t)sz);
  return luaL_fileresult(L, res == 0, 
                                     ((void *)0)
                                         );
}



static int io_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(getiofile(L, ("_IO_" "output"))) == 0, 
                                                                 ((void *)0)
                                                                     );
}


static int f_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(tofile(L)) == 0, 
                                                   ((void *)0)
                                                       );
}





static const luaL_Reg iolib[] = {
  {"close", io_close},
  {"flush", io_flush},
  {"input", io_input},
  {"lines", io_lines},
  {"open", io_open},
  {"output", io_output},
  {"popen", io_popen},
  {"read", io_read},
  {"tmpfile", io_tmpfile},
  {"type", io_type},
  {"write", io_write},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};





static const luaL_Reg meth[] = {
  {"read", f_read},
  {"write", f_write},
  {"lines", f_lines},
  {"flush", f_flush},
  {"seek", f_seek},
  {"close", f_close},
  {"setvbuf", f_setvbuf},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};





static const luaL_Reg metameth[] = {
  {"__index", 
             ((void *)0)
                 },
  {"__gc", f_gc},
  {"__close", f_gc},
  {"__tostring", f_tostring},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


static void createmeta (lua_State *L) {
  luaL_newmetatable(L, "FILE*");
  luaL_setfuncs(L, metameth, 0);
  lua_createtable(L, 0, sizeof(meth)/sizeof((meth)[0]) - 1);
  luaL_setfuncs(L, meth, 0);
  lua_setfield(L, -2, "__index");
  lua_settop(L, -(1)-1);
}





static int io_noclose (lua_State *L) {
  LStream *p = ((LStream *)luaL_checkudata(L, 1, "FILE*"));
  p->closef = &io_noclose;
  lua_pushnil(L);
  lua_pushstring(L, "" "cannot close standard file");
  return 2;
}


static void createstdfile (lua_State *L, FILE *f, const char *k,
                           const char *fname) {
  LStream *p = newprefile(L);
  p->f = f;
  p->closef = &io_noclose;
  if (k != 
          ((void *)0)
              ) {
    lua_pushvalue(L, -1);
    lua_setfield(L, (-1000000 - 1000), k);
  }
  lua_setfield(L, -2, fname);
}


extern int luaopen_io (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(iolib)/sizeof((iolib)[0]) - 1), luaL_setfuncs(L,iolib,0));
  createmeta(L);

  createstdfile(L, 
                  stdin
                       , ("_IO_" "input"), "stdin");
  createstdfile(L, 
                  stdout
                        , ("_IO_" "output"), "stdout");
  createstdfile(L, 
                  stderr
                        , 
                          ((void *)0)
                              , "stderr");
  return 1;
}
static int math_abs (lua_State *L) {
  if (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointegerx(L,(1),
                   ((void *)0)
                   );
    if (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  else
    lua_pushnumber(L, fabs(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sin (lua_State *L) {
  lua_pushnumber(L, sin(luaL_checknumber(L, 1)));
  return 1;
}

static int math_cos (lua_State *L) {
  lua_pushnumber(L, cos(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tan (lua_State *L) {
  lua_pushnumber(L, tan(luaL_checknumber(L, 1)));
  return 1;
}

static int math_asin (lua_State *L) {
  lua_pushnumber(L, asin(luaL_checknumber(L, 1)));
  return 1;
}

static int math_acos (lua_State *L) {
  lua_pushnumber(L, acos(luaL_checknumber(L, 1)));
  return 1;
}

static int math_atan (lua_State *L) {
  lua_Number y = luaL_checknumber(L, 1);
  lua_Number x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, atan2(y, x));
  return 1;
}


static int math_toint (lua_State *L) {
  int valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  if ((valid))
    lua_pushinteger(L, n);
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}


static void pushnumint (lua_State *L, lua_Number d) {
  lua_Integer n;
  if (((d) >= (double)(
     (-0x7fffffffffffffffLL - 1LL)
     ) && (d) < -(double)(
     (-0x7fffffffffffffffLL - 1LL)
     ) && (*(&n) = (long long)(d), 1)))
    lua_pushinteger(L, n);
  else
    lua_pushnumber(L, d);
}


static int math_floor (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);
  else {
    lua_Number d = floor(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_ceil (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);
  else {
    lua_Number d = ceil(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_fmod (lua_State *L) {
  if (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointegerx(L,(2),
                   ((void *)0)
                   );
    if ((lua_Unsigned)d + 1u <= 1u) {
      ((void)((d != 0) || luaL_argerror(L, (2), ("zero"))));
      lua_pushinteger(L, 0);
    }
    else
      lua_pushinteger(L, lua_tointegerx(L,(1),
                        ((void *)0)
                        ) % d);
  }
  else
    lua_pushnumber(L, fmod(luaL_checknumber(L, 1),
                                     luaL_checknumber(L, 2)));
  return 1;
}







static int math_modf (lua_State *L) {
  if (lua_isinteger(L ,1)) {
    lua_settop(L, 1);
    lua_pushnumber(L, 0);
  }
  else {
    lua_Number n = luaL_checknumber(L, 1);

    lua_Number ip = (n < 0) ? ceil(n) : floor(n);
    pushnumint(L, ip);

    lua_pushnumber(L, (n == ip) ? 0.0 : (n - ip));
  }
  return 2;
}


static int math_sqrt (lua_State *L) {
  lua_pushnumber(L, sqrt(luaL_checknumber(L, 1)));
  return 1;
}


static int math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  return 1;
}

static int math_log (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number res;
  if ((lua_type(L, (2)) <= 0))
    res = log(x);
  else {
    lua_Number base = luaL_checknumber(L, 2);

    if (base == 2.0)
      res = log2(x);
    else

    if (base == 10.0)
      res = log10(x);
    else
      res = log(x)/log(base);
  }
  lua_pushnumber(L, res);
  return 1;
}

static int math_exp (lua_State *L) {
  lua_pushnumber(L, exp(luaL_checknumber(L, 1)));
  return 1;
}

static int math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (180.0 / (3.141592653589793238462643383279502884)));
  return 1;
}

static int math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * ((3.141592653589793238462643383279502884) / 180.0));
  return 1;
}


static int math_min (lua_State *L) {
  int n = lua_gettop(L);
  int imin = 1;
  int i;
  ((void)((n >= 1) || luaL_argerror(L, (1), ("value expected"))));
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, i, imin, 1))
      imin = i;
  }
  lua_pushvalue(L, imin);
  return 1;
}


static int math_max (lua_State *L) {
  int n = lua_gettop(L);
  int imax = 1;
  int i;
  ((void)((n >= 1) || luaL_argerror(L, (1), ("value expected"))));
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, imax, i, 1))
      imax = i;
  }
  lua_pushvalue(L, imax);
  return 1;
}


static int math_type (lua_State *L) {
  if (lua_type(L, 1) == 3)
    lua_pushstring(L, (lua_isinteger(L, 1)) ? "integer" : "float");
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}
static unsigned long rotl (unsigned long x, int n) {
  return (x << n) | (((x) & 0xffffffffffffffffu) >> (64 - n));
}

static unsigned long nextrand (unsigned long *state) {
  unsigned long state0 = state[0];
  unsigned long state1 = state[1];
  unsigned long state2 = state[2] ^ state0;
  unsigned long state3 = state[3] ^ state1;
  unsigned long res = rotl(state1 * 5, 7) * 9;
  state[0] = state0 ^ state3;
  state[1] = state1 ^ state2;
  state[2] = state2 ^ (state1 << 17);
  state[3] = rotl(state3, 45);
  return res;
}
static lua_Number I2d (unsigned long x) {
  return (lua_Number)(((x) & 0xffffffffffffffffu) >> (64 - (53
                                  ))) * (0.5 / ((unsigned long)1 << ((53
                                                 ) - 1)));
}
typedef struct {
  unsigned long s[4];
} RanState;
static lua_Unsigned project (lua_Unsigned ran, lua_Unsigned n,
                             RanState *state) {
  if ((n & (n + 1)) == 0)
    return ran & n;
  else {
    lua_Unsigned lim = n;

    lim |= (lim >> 1);
    lim |= (lim >> 2);
    lim |= (lim >> 4);
    lim |= (lim >> 8);
    lim |= (lim >> 16);

    lim |= (lim >> 32);

    ((void)0)

                        ;
    while ((ran &= lim) > n)
      ran = ((lua_Unsigned)((nextrand(state->s)) & 0xffffffffffffffffu));
    return ran;
  }
}


static int math_random (lua_State *L) {
  lua_Integer low, up;
  lua_Unsigned p;
  RanState *state = (RanState *)lua_touserdata(L, ((-1000000 - 1000) - (1)));
  unsigned long rv = nextrand(state->s);
  switch (lua_gettop(L)) {
    case 0: {
      lua_pushnumber(L, I2d(rv));
      return 1;
    }
    case 1: {
      low = 1;
      up = luaL_checkinteger(L, 1);
      if (up == 0) {
        lua_pushinteger(L, ((lua_Unsigned)((rv) & 0xffffffffffffffffu)));
        return 1;
      }
      break;
    }
    case 2: {
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }

  ((void)((low <= up) || luaL_argerror(L, (1), ("interval is empty"))));

  p = project(((lua_Unsigned)((rv) & 0xffffffffffffffffu)), (lua_Unsigned)up - (lua_Unsigned)low, state);
  lua_pushinteger(L, p + (lua_Unsigned)low);
  return 1;
}


static void setseed (lua_State *L, unsigned long *state,
                     lua_Unsigned n1, lua_Unsigned n2) {
  int i;
  state[0] = ((unsigned long)(n1));
  state[1] = ((unsigned long)(0xff));
  state[2] = ((unsigned long)(n2));
  state[3] = ((unsigned long)(0));
  for (i = 0; i < 16; i++)
    nextrand(state);
  lua_pushinteger(L, n1);
  lua_pushinteger(L, n2);
}







static void randseed (lua_State *L, RanState *state) {
  lua_Unsigned seed1 = (lua_Unsigned)time(
                                         ((void *)0)
                                             );
  lua_Unsigned seed2 = (lua_Unsigned)(size_t)L;
  setseed(L, state->s, seed1, seed2);
}


static int math_randomseed (lua_State *L) {
  RanState *state = (RanState *)lua_touserdata(L, ((-1000000 - 1000) - (1)));
  if ((lua_type(L, (1)) == (-1))) {
    randseed(L, state);
  }
  else {
    lua_Integer n1 = luaL_checkinteger(L, 1);
    lua_Integer n2 = luaL_optinteger(L, 2, 0);
    setseed(L, state->s, n1, n2);
  }
  return 2;
}


static const luaL_Reg randfuncs[] = {
  {"random", math_random},
  {"randomseed", math_randomseed},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};





static void setrandfunc (lua_State *L) {
  RanState *state = (RanState *)lua_newuserdatauv(L, sizeof(RanState), 0);
  randseed(L, state);
  lua_settop(L, -(2)-1);
  luaL_setfuncs(L, randfuncs, 1);
}
static const luaL_Reg mathlib[] = {
  {"abs", math_abs},
  {"acos", math_acos},
  {"asin", math_asin},
  {"atan", math_atan},
  {"ceil", math_ceil},
  {"cos", math_cos},
  {"deg", math_deg},
  {"exp", math_exp},
  {"tointeger", math_toint},
  {"floor", math_floor},
  {"fmod", math_fmod},
  {"ult", math_ult},
  {"log", math_log},
  {"max", math_max},
  {"min", math_min},
  {"modf", math_modf},
  {"rad", math_rad},
  {"sin", math_sin},
  {"sqrt", math_sqrt},
  {"tan", math_tan},
  {"type", math_type},
  {"random", 
            ((void *)0)
                },
  {"randomseed", 
                ((void *)0)
                    },
  {"pi", 
        ((void *)0)
            },
  {"huge", 
          ((void *)0)
              },
  {"maxinteger", 
                ((void *)0)
                    },
  {"mininteger", 
                ((void *)0)
                    },
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};





extern int luaopen_math (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(mathlib)/sizeof((mathlib)[0]) - 1), luaL_setfuncs(L,mathlib,0));
  lua_pushnumber(L, (3.141592653589793238462643383279502884));
  lua_setfield(L, -2, "pi");
  lua_pushnumber(L, (lua_Number)
                               1e10000
                                       );
  lua_setfield(L, -2, "huge");
  lua_pushinteger(L, 0x7fffffffffffffffLL
                                  );
  lua_setfield(L, -2, "maxinteger");
  lua_pushinteger(L, 
                    (-0x7fffffffffffffffLL - 1LL)
                                  );
  lua_setfield(L, -2, "mininteger");
  setrandfunc(L);
  return 1;
}
static const char *const CLIBS = "_CLIBS";
typedef void (*voidf)(void);
static void lsys_unloadlib (void *lib);







static void *lsys_load (lua_State *L, const char *path, int seeglb);






static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym);







extern void *dlopen (const char *__file, int __mode) ;



extern int dlclose (void *__handle) ;



extern void *dlsym (void * __handle,
      const char * __name) ;
extern char *dlerror (void) ;


static void lsys_unloadlib (void *lib) {
  dlclose(lib);
}


static void *lsys_load (lua_State *L, const char *path, int seeglb) {
  void *lib = dlopen(path, 
                          0x00002 
                                   | (seeglb ? 
                                               0x00100 
                                                           : 
                                                             0
                                                                       ));
  if ((lib == 
     ((void *)0)
     ))
    lua_pushstring(L, dlerror());
  return lib;
}


static lua_CFunction lsys_sym (lua_State *L, void *lib, const char *sym) {
  lua_CFunction f = ((lua_CFunction)(dlsym(lib, sym)));
  if ((f == 
     ((void *)0)
     ))
    lua_pushstring(L, dlerror());
  return f;
}
static int noenv (lua_State *L) {
  int b;
  lua_getfield(L, (-1000000 - 1000), "LUA_NOENV");
  b = lua_toboolean(L, -1);
  lua_settop(L, -(1)-1);
  return b;
}





static void setpath (lua_State *L, const char *fieldname,
                                   const char *envname,
                                   const char *dft) {
  const char *dftmark;
  const char *nver = lua_pushfstring(L, "%s%s", envname, "_" "5" "_" "4");
  const char *path = getenv(nver);
  if (path == 
             ((void *)0)
                 )
    path = getenv(envname);
  if (path == 
             ((void *)0) 
                  || noenv(L))
    lua_pushstring(L, dft);
  else if ((dftmark = strstr(path, ";" ";")) == 
                                                                 ((void *)0)
                                                                     )
    lua_pushstring(L, path);
  else {
    size_t len = strlen(path);
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    if (path < dftmark) {
      luaL_addlstring(&b, path, dftmark - path);
      ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*";")));
    }
    luaL_addstring(&b, dft);
    if (dftmark < path + len - 2) {
      ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*";")));
      luaL_addlstring(&b, dftmark + 2, (path + len - 2) - dftmark);
    }
    luaL_pushresult(&b);
  }
  ((void)0);
  lua_setfield(L, -3, fieldname);
  lua_settop(L, -(1)-1);
}







static void *checkclib (lua_State *L, const char *path) {
  void *plib;
  lua_getfield(L, (-1000000 - 1000), CLIBS);
  lua_getfield(L, -1, path);
  plib = lua_touserdata(L, -1);
  lua_settop(L, -(2)-1);
  return plib;
}






static void addtoclib (lua_State *L, const char *path, void *plib) {
  lua_getfield(L, (-1000000 - 1000), CLIBS);
  lua_pushlightuserdata(L, plib);
  lua_pushvalue(L, -1);
  lua_setfield(L, -3, path);
  lua_rawseti(L, -2, luaL_len(L, -2) + 1);
  lua_settop(L, -(1)-1);
}






static int gctm (lua_State *L) {
  lua_Integer n = luaL_len(L, 1);
  for (; n >= 1; n--) {
    lua_rawgeti(L, 1, n);
    lsys_unloadlib(lua_touserdata(L, -1));
    lua_settop(L, -(1)-1);
  }
  return 0;
}
static int lookforfunc (lua_State *L, const char *path, const char *sym) {
  void *reg = checkclib(L, path);
  if (reg == 
            ((void *)0)
                ) {
    reg = lsys_load(L, path, *sym == '*');
    if (reg == 
              ((void *)0)
                  ) return 1;
    addtoclib(L, path, reg);
  }
  if (*sym == '*') {
    lua_pushboolean(L, 1);
    return 0;
  }
  else {
    lua_CFunction f = lsys_sym(L, reg, sym);
    if (f == 
            ((void *)0)
                )
      return 2;
    lua_pushcclosure(L, (f), 0);
    return 0;
  }
}


static int ll_loadlib (lua_State *L) {
  const char *path = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  const char *init = (luaL_checklstring(L, (2), 
                    ((void *)0)
                    ));
  int stat = lookforfunc(L, path, init);
  if ((stat == 0))
    return 1;
  else {
    lua_pushnil(L);
    lua_rotate(L, (-2), 1);
    lua_pushstring(L, (stat == 1) ? "open" : "init");
    return 3;
  }
}
static int readable (const char *filename) {
  FILE *f = 
           fopen64
                (filename, "r");
  if (f == 
          ((void *)0)
              ) return 0;
  fclose(f);
  return 1;
}







static const char *getnextfilename (char **path, char *end) {
  char *sep;
  char *name = *path;
  if (name == end)
    return 
          ((void *)0)
              ;
  else if (*name == '\0') {
    *name = *";";
    name++;
  }
  sep = strchr(name, *";");
  if (sep == 
            ((void *)0)
                )
    sep = end;
  *sep = '\0';
  *path = sep;
  return name;
}
static void pusherrornotfound (lua_State *L, const char *path) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  luaL_addstring(&b, "no file '");
  luaL_addgsub(&b, path, ";", "'\n\tno file '");
  luaL_addstring(&b, "'");
  luaL_pushresult(&b);
}


static const char *searchpath (lua_State *L, const char *name,
                                             const char *path,
                                             const char *sep,
                                             const char *dirsep) {
  luaL_Buffer buff;
  char *pathname;
  char *endpathname;
  const char *filename;

  if (*sep != '\0' && strchr(name, *sep) != 
                                           ((void *)0)
                                               )
    name = luaL_gsub(L, name, sep, dirsep);
  luaL_buffinit(L, &buff);

  luaL_addgsub(&buff, path, "?", name);
  ((void)((&buff)->n < (&buff)->size || luaL_prepbuffsize((&buff), 1)), ((&buff)->b[(&buff)->n++] = ('\0')));
  pathname = ((&buff)->b);
  endpathname = pathname + ((&buff)->n) - 1;
  while ((filename = getnextfilename(&pathname, endpathname)) != 
                                                                ((void *)0)
                                                                    ) {
    if (readable(filename))
      return lua_pushstring(L, filename);
  }
  luaL_pushresult(&buff);
  pusherrornotfound(L, lua_tolstring(L, (-1), 
                      ((void *)0)
                      ));
  return 
        ((void *)0)
            ;
}


static int ll_searchpath (lua_State *L) {
  const char *f = searchpath(L, (luaL_checklstring(L, (1), 
                               ((void *)0)
                               )),
                                (luaL_checklstring(L, (2), 
                               ((void *)0)
                               )),
                                (luaL_optlstring(L, (3), ("."), 
                               ((void *)0)
                               )),
                                (luaL_optlstring(L, (4), ("/"), 
                               ((void *)0)
                               )));
  if (f != 
          ((void *)0)
              ) return 1;
  else {
    lua_pushnil(L);
    lua_rotate(L, (-2), 1);
    return 2;
  }
}


static const char *findfile (lua_State *L, const char *name,
                                           const char *pname,
                                           const char *dirsep) {
  const char *path;
  lua_getfield(L, ((-1000000 - 1000) - (1)), pname);
  path = lua_tolstring(L, (-1), 
        ((void *)0)
        );
  if ((path == 
     ((void *)0)
     ))
    luaL_error(L, "'package.%s' must be a string", pname);
  return searchpath(L, name, path, ".", dirsep);
}


static int checkload (lua_State *L, int stat, const char *filename) {
  if ((stat)) {
    lua_pushstring(L, filename);
    return 2;
  }
  else
    return luaL_error(L, "error loading module '%s' from file '%s':\n\t%s",
                          lua_tolstring(L, (1), 
                         ((void *)0)
                         ), filename, lua_tolstring(L, (-1), 
                                                       ((void *)0)
                                                       ));
}


static int searcher_Lua (lua_State *L) {
  const char *filename;
  const char *name = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  filename = findfile(L, name, "path", "/");
  if (filename == 
                 ((void *)0)
                     ) return 1;
  return checkload(L, (luaL_loadfilex(L,filename,
                      ((void *)0)
                      ) == 0), filename);
}
static int loadfunc (lua_State *L, const char *filename, const char *modname) {
  const char *openfunc;
  const char *mark;
  modname = luaL_gsub(L, modname, ".", "_");
  mark = strchr(modname, *"-");
  if (mark) {
    int stat;
    openfunc = lua_pushlstring(L, modname, mark - modname);
    openfunc = lua_pushfstring(L, "luaopen_""%s", openfunc);
    stat = lookforfunc(L, filename, openfunc);
    if (stat != 2) return stat;
    modname = mark + 1;
  }
  openfunc = lua_pushfstring(L, "luaopen_""%s", modname);
  return lookforfunc(L, filename, openfunc);
}


static int searcher_C (lua_State *L) {
  const char *name = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  const char *filename = findfile(L, name, "cpath", "/");
  if (filename == 
                 ((void *)0)
                     ) return 1;
  return checkload(L, (loadfunc(L, filename, name) == 0), filename);
}


static int searcher_Croot (lua_State *L) {
  const char *filename;
  const char *name = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  const char *p = strchr(name, '.');
  int stat;
  if (p == 
          ((void *)0)
              ) return 0;
  lua_pushlstring(L, name, p - name);
  filename = findfile(L, lua_tolstring(L, (-1), 
                        ((void *)0)
                        ), "cpath", "/");
  if (filename == 
                 ((void *)0)
                     ) return 1;
  if ((stat = loadfunc(L, filename, name)) != 0) {
    if (stat != 2)
      return checkload(L, 0, filename);
    else {
      lua_pushfstring(L, "no module '%s' in file '%s'", name, filename);
      return 1;
    }
  }
  lua_pushstring(L, filename);
  return 2;
}


static int searcher_preload (lua_State *L) {
  const char *name = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  lua_getfield(L, (-1000000 - 1000), "_PRELOAD");
  if (lua_getfield(L, -1, name) == 0) {
    lua_pushfstring(L, "no field package.preload['%s']", name);
    return 1;
  }
  else {
    lua_pushstring(L, "" ":preload:");
    return 2;
  }
}


static void findloader (lua_State *L, const char *name) {
  int i;
  luaL_Buffer msg;

  if ((lua_getfield(L, ((-1000000 - 1000) - (1)), "searchers") != 5)
                               )
    luaL_error(L, "'package.searchers' must be a table");
  luaL_buffinit(L, &msg);

  for (i = 1; ; i++) {
    luaL_addstring(&msg, "\n\t");
    if ((lua_rawgeti(L, 3, i) == 0)) {
      lua_settop(L, -(1)-1);
      ((&msg)->n -= (2));
      luaL_pushresult(&msg);
      luaL_error(L, "module '%s' not found:%s", name, lua_tolstring(L, (-1), 
                                                     ((void *)0)
                                                     ));
    }
    lua_pushstring(L, name);
    lua_callk(L, (1), (2), 0, 
   ((void *)0)
   );
    if ((lua_type(L, (-2)) == 6))
      return;
    else if (lua_isstring(L, -2)) {
      lua_settop(L, -(1)-1);
      luaL_addvalue(&msg);
    }
    else {
      lua_settop(L, -(2)-1);
      ((&msg)->n -= (2));
    }
  }
}


static int ll_require (lua_State *L) {
  const char *name = (luaL_checklstring(L, (1), 
                    ((void *)0)
                    ));
  lua_settop(L, 1);
  lua_getfield(L, (-1000000 - 1000), "_LOADED");
  lua_getfield(L, 2, name);
  if (lua_toboolean(L, -1))
    return 1;

  lua_settop(L, -(1)-1);
  findloader(L, name);
  lua_rotate(L, -2, 1);
  lua_pushvalue(L, 1);
  lua_pushvalue(L, -3);

  lua_callk(L, (2), (1), 0, 
 ((void *)0)
 );

  if (!(lua_type(L, (-1)) == 0))
    lua_setfield(L, 2, name);
  else
    lua_settop(L, -(1)-1);
  if (lua_getfield(L, 2, name) == 0) {
    lua_pushboolean(L, 1);
    lua_copy(L, -1, -2);
    lua_setfield(L, 2, name);
  }
  lua_rotate(L, -2, 1);
  return 2;
}






static const luaL_Reg pk_funcs[] = {
  {"loadlib", ll_loadlib},
  {"searchpath", ll_searchpath},

  {"preload", 
             ((void *)0)
                 },
  {"cpath", 
           ((void *)0)
               },
  {"path", 
          ((void *)0)
              },
  {"searchers", 
               ((void *)0)
                   },
  {"loaded", 
            ((void *)0)
                },
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


static const luaL_Reg ll_funcs[] = {
  {"require", ll_require},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


static void createsearcherstable (lua_State *L) {
  static const lua_CFunction searchers[] = {
    searcher_preload,
    searcher_Lua,
    searcher_C,
    searcher_Croot,
    
   ((void *)0)
  
 };
  int i;

  lua_createtable(L, sizeof(searchers)/sizeof(searchers[0]) - 1, 0);

  for (i=0; searchers[i] != 
                           ((void *)0)
                               ; i++) {
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, searchers[i], 1);
    lua_rawseti(L, -2, i+1);
  }
  lua_setfield(L, -2, "searchers");
}






static void createclibstable (lua_State *L) {
  luaL_getsubtable(L, (-1000000 - 1000), CLIBS);
  lua_createtable(L, 0, 1);
  lua_pushcclosure(L, (gctm), 0);
  lua_setfield(L, -2, "__gc");
  lua_setmetatable(L, -2);
}


extern int luaopen_package (lua_State *L) {
  createclibstable(L);
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(pk_funcs)/sizeof((pk_funcs)[0]) - 1), luaL_setfuncs(L,pk_funcs,0));
  createsearcherstable(L);

  setpath(L, "path", "LUA_PATH", "/usr/local/" "share/lua/" "5" "." "4" "/""?.lua;" "/usr/local/" "share/lua/" "5" "." "4" "/""?/init.lua;" "/usr/local/" "lib/lua/" "5" "." "4" "/""?.lua;" "/usr/local/" "lib/lua/" "5" "." "4" "/""?/init.lua;" "./?.lua;" "./?/init.lua");
  setpath(L, "cpath", "LUA_CPATH", "/usr/local/" "lib/lua/" "5" "." "4" "/""?.so;" "/usr/local/" "lib/lua/" "5" "." "4" "/""loadall.so;" "./?.so");

  lua_pushstring(L, "" "/" "\n" ";" "\n" "?" "\n" "!" "\n" "-" "\n")
                                                       ;
  lua_setfield(L, -2, "config");

  luaL_getsubtable(L, (-1000000 - 1000), "_LOADED");
  lua_setfield(L, -2, "loaded");

  luaL_getsubtable(L, (-1000000 - 1000), "_PRELOAD");
  lua_setfield(L, -2, "preload");
  ((void)lua_rawgeti(L, (-1000000 - 1000), 2));
  lua_pushvalue(L, -2);
  luaL_setfuncs(L, ll_funcs, 1);
  lua_settop(L, -(1)-1);
  return 1;
}





typedef __socklen_t socklen_t;
extern int access (const char *__name, int __type) ;
extern __off64_t lseek64 (int __fd, __off64_t __offset, int __whence)
     ;






extern int close (int __fd);
extern ssize_t read (int __fd, void *__buf, size_t __nbytes)
    ;





extern ssize_t write (int __fd, const void *__buf, size_t __n)
    ;
extern ssize_t pread64 (int __fd, void *__buf, size_t __nbytes,
   __off64_t __offset)
    ;


extern ssize_t pwrite64 (int __fd, const void *__buf, size_t __n,
    __off64_t __offset)
    ;







extern int pipe (int __pipedes[2]) ;
extern unsigned int alarm (unsigned int __seconds) ;
extern unsigned int sleep (unsigned int __seconds);







extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval)
     ;






extern int usleep (__useconds_t __useconds);
extern int pause (void);



extern int chown (const char *__file, __uid_t __owner, __gid_t __group)
     ;



extern int fchown (int __fd, __uid_t __owner, __gid_t __group) ;




extern int lchown (const char *__file, __uid_t __owner, __gid_t __group)
     ;
extern int chdir (const char *__path) ;



extern int fchdir (int __fd) ;
extern char *getcwd (char *__buf, size_t __size) ;
extern char *getwd (char *__buf)
    
    ;




extern int dup (int __fd) ;


extern int dup2 (int __fd, int __fd2) ;
extern char **__environ;







extern int execve (const char *__path, char *const __argv[],
     char *const __envp[]) ;
extern int execv (const char *__path, char *const __argv[])
     ;



extern int execle (const char *__path, const char *__arg, ...)
     ;



extern int execl (const char *__path, const char *__arg, ...)
     ;



extern int execvp (const char *__file, char *const __argv[])
     ;




extern int execlp (const char *__file, const char *__arg, ...)
     ;
extern int nice (int __inc) ;




extern void _exit (int __status) ;





enum
  {
    _PC_LINK_MAX,

    _PC_MAX_CANON,

    _PC_MAX_INPUT,

    _PC_NAME_MAX,

    _PC_PATH_MAX,

    _PC_PIPE_BUF,

    _PC_CHOWN_RESTRICTED,

    _PC_NO_TRUNC,

    _PC_VDISABLE,

    _PC_SYNC_IO,

    _PC_ASYNC_IO,

    _PC_PRIO_IO,

    _PC_SOCK_MAXBUF,

    _PC_FILESIZEBITS,

    _PC_REC_INCR_XFER_SIZE,

    _PC_REC_MAX_XFER_SIZE,

    _PC_REC_MIN_XFER_SIZE,

    _PC_REC_XFER_ALIGN,

    _PC_ALLOC_SIZE_MIN,

    _PC_SYMLINK_MAX,

    _PC_2_SYMLINKS

  };


enum
  {
    _SC_ARG_MAX,

    _SC_CHILD_MAX,

    _SC_CLK_TCK,

    _SC_NGROUPS_MAX,

    _SC_OPEN_MAX,

    _SC_STREAM_MAX,

    _SC_TZNAME_MAX,

    _SC_JOB_CONTROL,

    _SC_SAVED_IDS,

    _SC_REALTIME_SIGNALS,

    _SC_PRIORITY_SCHEDULING,

    _SC_TIMERS,

    _SC_ASYNCHRONOUS_IO,

    _SC_PRIORITIZED_IO,

    _SC_SYNCHRONIZED_IO,

    _SC_FSYNC,

    _SC_MAPPED_FILES,

    _SC_MEMLOCK,

    _SC_MEMLOCK_RANGE,

    _SC_MEMORY_PROTECTION,

    _SC_MESSAGE_PASSING,

    _SC_SEMAPHORES,

    _SC_SHARED_MEMORY_OBJECTS,

    _SC_AIO_LISTIO_MAX,

    _SC_AIO_MAX,

    _SC_AIO_PRIO_DELTA_MAX,

    _SC_DELAYTIMER_MAX,

    _SC_MQ_OPEN_MAX,

    _SC_MQ_PRIO_MAX,

    _SC_VERSION,

    _SC_PAGESIZE,


    _SC_RTSIG_MAX,

    _SC_SEM_NSEMS_MAX,

    _SC_SEM_VALUE_MAX,

    _SC_SIGQUEUE_MAX,

    _SC_TIMER_MAX,




    _SC_BC_BASE_MAX,

    _SC_BC_DIM_MAX,

    _SC_BC_SCALE_MAX,

    _SC_BC_STRING_MAX,

    _SC_COLL_WEIGHTS_MAX,

    _SC_EQUIV_CLASS_MAX,

    _SC_EXPR_NEST_MAX,

    _SC_LINE_MAX,

    _SC_RE_DUP_MAX,

    _SC_CHARCLASS_NAME_MAX,


    _SC_2_VERSION,

    _SC_2_C_BIND,

    _SC_2_C_DEV,

    _SC_2_FORT_DEV,

    _SC_2_FORT_RUN,

    _SC_2_SW_DEV,

    _SC_2_LOCALEDEF,


    _SC_PII,

    _SC_PII_XTI,

    _SC_PII_SOCKET,

    _SC_PII_INTERNET,

    _SC_PII_OSI,

    _SC_POLL,

    _SC_SELECT,

    _SC_UIO_MAXIOV,

    _SC_IOV_MAX = _SC_UIO_MAXIOV,

    _SC_PII_INTERNET_STREAM,

    _SC_PII_INTERNET_DGRAM,

    _SC_PII_OSI_COTS,

    _SC_PII_OSI_CLTS,

    _SC_PII_OSI_M,

    _SC_T_IOV_MAX,



    _SC_THREADS,

    _SC_THREAD_SAFE_FUNCTIONS,

    _SC_GETGR_R_SIZE_MAX,

    _SC_GETPW_R_SIZE_MAX,

    _SC_LOGIN_NAME_MAX,

    _SC_TTY_NAME_MAX,

    _SC_THREAD_DESTRUCTOR_ITERATIONS,

    _SC_THREAD_KEYS_MAX,

    _SC_THREAD_STACK_MIN,

    _SC_THREAD_THREADS_MAX,

    _SC_THREAD_ATTR_STACKADDR,

    _SC_THREAD_ATTR_STACKSIZE,

    _SC_THREAD_PRIORITY_SCHEDULING,

    _SC_THREAD_PRIO_INHERIT,

    _SC_THREAD_PRIO_PROTECT,

    _SC_THREAD_PROCESS_SHARED,


    _SC_NPROCESSORS_CONF,

    _SC_NPROCESSORS_ONLN,

    _SC_PHYS_PAGES,

    _SC_AVPHYS_PAGES,

    _SC_ATEXIT_MAX,

    _SC_PASS_MAX,


    _SC_XOPEN_VERSION,

    _SC_XOPEN_XCU_VERSION,

    _SC_XOPEN_UNIX,

    _SC_XOPEN_CRYPT,

    _SC_XOPEN_ENH_I18N,

    _SC_XOPEN_SHM,


    _SC_2_CHAR_TERM,

    _SC_2_C_VERSION,

    _SC_2_UPE,


    _SC_XOPEN_XPG2,

    _SC_XOPEN_XPG3,

    _SC_XOPEN_XPG4,


    _SC_CHAR_BIT,

    _SC_CHAR_MAX,

    _SC_CHAR_MIN,

    _SC_INT_MAX,

    _SC_INT_MIN,

    _SC_LONG_BIT,

    _SC_WORD_BIT,

    _SC_MB_LEN_MAX,

    _SC_NZERO,

    _SC_SSIZE_MAX,

    _SC_SCHAR_MAX,

    _SC_SCHAR_MIN,

    _SC_SHRT_MAX,

    _SC_SHRT_MIN,

    _SC_UCHAR_MAX,

    _SC_UINT_MAX,

    _SC_ULONG_MAX,

    _SC_USHRT_MAX,


    _SC_NL_ARGMAX,

    _SC_NL_LANGMAX,

    _SC_NL_MSGMAX,

    _SC_NL_NMAX,

    _SC_NL_SETMAX,

    _SC_NL_TEXTMAX,


    _SC_XBS5_ILP32_OFF32,

    _SC_XBS5_ILP32_OFFBIG,

    _SC_XBS5_LP64_OFF64,

    _SC_XBS5_LPBIG_OFFBIG,


    _SC_XOPEN_LEGACY,

    _SC_XOPEN_REALTIME,

    _SC_XOPEN_REALTIME_THREADS,


    _SC_ADVISORY_INFO,

    _SC_BARRIERS,

    _SC_BASE,

    _SC_C_LANG_SUPPORT,

    _SC_C_LANG_SUPPORT_R,

    _SC_CLOCK_SELECTION,

    _SC_CPUTIME,

    _SC_THREAD_CPUTIME,

    _SC_DEVICE_IO,

    _SC_DEVICE_SPECIFIC,

    _SC_DEVICE_SPECIFIC_R,

    _SC_FD_MGMT,

    _SC_FIFO,

    _SC_PIPE,

    _SC_FILE_ATTRIBUTES,

    _SC_FILE_LOCKING,

    _SC_FILE_SYSTEM,

    _SC_MONOTONIC_CLOCK,

    _SC_MULTI_PROCESS,

    _SC_SINGLE_PROCESS,

    _SC_NETWORKING,

    _SC_READER_WRITER_LOCKS,

    _SC_SPIN_LOCKS,

    _SC_REGEXP,

    _SC_REGEX_VERSION,

    _SC_SHELL,

    _SC_SIGNALS,

    _SC_SPAWN,

    _SC_SPORADIC_SERVER,

    _SC_THREAD_SPORADIC_SERVER,

    _SC_SYSTEM_DATABASE,

    _SC_SYSTEM_DATABASE_R,

    _SC_TIMEOUTS,

    _SC_TYPED_MEMORY_OBJECTS,

    _SC_USER_GROUPS,

    _SC_USER_GROUPS_R,

    _SC_2_PBS,

    _SC_2_PBS_ACCOUNTING,

    _SC_2_PBS_LOCATE,

    _SC_2_PBS_MESSAGE,

    _SC_2_PBS_TRACK,

    _SC_SYMLOOP_MAX,

    _SC_STREAMS,

    _SC_2_PBS_CHECKPOINT,


    _SC_V6_ILP32_OFF32,

    _SC_V6_ILP32_OFFBIG,

    _SC_V6_LP64_OFF64,

    _SC_V6_LPBIG_OFFBIG,


    _SC_HOST_NAME_MAX,

    _SC_TRACE,

    _SC_TRACE_EVENT_FILTER,

    _SC_TRACE_INHERIT,

    _SC_TRACE_LOG,


    _SC_LEVEL1_ICACHE_SIZE,

    _SC_LEVEL1_ICACHE_ASSOC,

    _SC_LEVEL1_ICACHE_LINESIZE,

    _SC_LEVEL1_DCACHE_SIZE,

    _SC_LEVEL1_DCACHE_ASSOC,

    _SC_LEVEL1_DCACHE_LINESIZE,

    _SC_LEVEL2_CACHE_SIZE,

    _SC_LEVEL2_CACHE_ASSOC,

    _SC_LEVEL2_CACHE_LINESIZE,

    _SC_LEVEL3_CACHE_SIZE,

    _SC_LEVEL3_CACHE_ASSOC,

    _SC_LEVEL3_CACHE_LINESIZE,

    _SC_LEVEL4_CACHE_SIZE,

    _SC_LEVEL4_CACHE_ASSOC,

    _SC_LEVEL4_CACHE_LINESIZE,



    _SC_IPV6 = _SC_LEVEL1_ICACHE_SIZE + 50,

    _SC_RAW_SOCKETS,


    _SC_V7_ILP32_OFF32,

    _SC_V7_ILP32_OFFBIG,

    _SC_V7_LP64_OFF64,

    _SC_V7_LPBIG_OFFBIG,


    _SC_SS_REPL_MAX,


    _SC_TRACE_EVENT_NAME_MAX,

    _SC_TRACE_NAME_MAX,

    _SC_TRACE_SYS_MAX,

    _SC_TRACE_USER_EVENT_MAX,


    _SC_XOPEN_STREAMS,


    _SC_THREAD_ROBUST_PRIO_INHERIT,

    _SC_THREAD_ROBUST_PRIO_PROTECT,


    _SC_MINSIGSTKSZ,


    _SC_SIGSTKSZ

  };


enum
  {
    _CS_PATH,


    _CS_V6_WIDTH_RESTRICTED_ENVS,



    _CS_GNU_LIBC_VERSION,

    _CS_GNU_LIBPTHREAD_VERSION,


    _CS_V5_WIDTH_RESTRICTED_ENVS,



    _CS_V7_WIDTH_RESTRICTED_ENVS,



    _CS_LFS_CFLAGS = 1000,

    _CS_LFS_LDFLAGS,

    _CS_LFS_LIBS,

    _CS_LFS_LINTFLAGS,

    _CS_LFS64_CFLAGS,

    _CS_LFS64_LDFLAGS,

    _CS_LFS64_LIBS,

    _CS_LFS64_LINTFLAGS,


    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,

    _CS_XBS5_ILP32_OFF32_LDFLAGS,

    _CS_XBS5_ILP32_OFF32_LIBS,

    _CS_XBS5_ILP32_OFF32_LINTFLAGS,

    _CS_XBS5_ILP32_OFFBIG_CFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,

    _CS_XBS5_ILP32_OFFBIG_LIBS,

    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,

    _CS_XBS5_LP64_OFF64_CFLAGS,

    _CS_XBS5_LP64_OFF64_LDFLAGS,

    _CS_XBS5_LP64_OFF64_LIBS,

    _CS_XBS5_LP64_OFF64_LINTFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,

    _CS_XBS5_LPBIG_OFFBIG_LIBS,

    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFF32_LIBS,

    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V6_LP64_OFF64_CFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V6_LP64_OFF64_LIBS,

    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS,


    _CS_POSIX_V7_ILP32_OFF32_CFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFF32_LIBS,

    _CS_POSIX_V7_ILP32_OFF32_LINTFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_CFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_ILP32_OFFBIG_LIBS,

    _CS_POSIX_V7_ILP32_OFFBIG_LINTFLAGS,

    _CS_POSIX_V7_LP64_OFF64_CFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LDFLAGS,

    _CS_POSIX_V7_LP64_OFF64_LIBS,

    _CS_POSIX_V7_LP64_OFF64_LINTFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_CFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LDFLAGS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LIBS,

    _CS_POSIX_V7_LPBIG_OFFBIG_LINTFLAGS,


    _CS_V6_ENV,

    _CS_V7_ENV

  };


extern long int pathconf (const char *__path, int __name)
     ;


extern long int fpathconf (int __fd, int __name) ;


extern long int sysconf (int __name) ;



extern size_t confstr (int __name, char *__buf, size_t __len)
    ;




extern __pid_t getpid (void) ;


extern __pid_t getppid (void) ;


extern __pid_t getpgrp (void) ;


extern __pid_t __getpgid (__pid_t __pid) ;

extern __pid_t getpgid (__pid_t __pid) ;






extern int setpgid (__pid_t __pid, __pid_t __pgid) ;
extern int setpgrp (void) ;






extern __pid_t setsid (void) ;



extern __pid_t getsid (__pid_t __pid) ;



extern __uid_t getuid (void) ;


extern __uid_t geteuid (void) ;


extern __gid_t getgid (void) ;


extern __gid_t getegid (void) ;




extern int getgroups (int __size, __gid_t __list[])
    ;
extern int setuid (__uid_t __uid) ;




extern int setreuid (__uid_t __ruid, __uid_t __euid) ;




extern int seteuid (__uid_t __uid) ;






extern int setgid (__gid_t __gid) ;




extern int setregid (__gid_t __rgid, __gid_t __egid) ;




extern int setegid (__gid_t __gid) ;
extern __pid_t fork (void) ;







extern __pid_t vfork (void) ;
extern char *ttyname (int __fd) ;



extern int ttyname_r (int __fd, char *__buf, size_t __buflen)
    
     ;



extern int isatty (int __fd) ;
extern int link (const char *__from, const char *__to)
     ;
extern int symlink (const char *__from, const char *__to)
     ;




extern ssize_t readlink (const char * __path,
    char * __buf, size_t __len)
    
     ;
extern int unlink (const char *__name) ;
extern int rmdir (const char *__path) ;



extern __pid_t tcgetpgrp (int __fd) ;


extern int tcsetpgrp (int __fd, __pid_t __pgrp_id) ;






extern char *getlogin (void);







extern int getlogin_r (char *__name, size_t __name_len)
    ;








extern char *optarg;
extern int optind;




extern int opterr;



extern int optopt;
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts)
       ;












extern int gethostname (char *__name, size_t __len)
    ;
extern int fsync (int __fd);
extern long int gethostid (void);


extern void sync (void) ;
extern int truncate64 (const char *__file, __off64_t __length)
     ;
extern int ftruncate64 (int __fd, __off64_t __length) ;
extern int lockf64 (int __fd, int __cmd, __off64_t __len) ;
extern int fdatasync (int __fildes);
extern void swab (const void * __from, void * __to,
    ssize_t __n)
   
    ;



static int os_execute (lua_State *L) {
  const char *cmd = (luaL_optlstring(L, (1), (
                   ((void *)0)
                   ), 
                   ((void *)0)
                   ));
  int stat;
  
 (*__errno_location ()) 
       = 0;
  stat = system(cmd);
  if (cmd != 
            ((void *)0)
                )
    return luaL_execresult(L, stat);
  else {
    lua_pushboolean(L, stat);
    return 1;
  }
}


static int os_remove (lua_State *L) {
  const char *filename = (luaL_checklstring(L, (1), 
                        ((void *)0)
                        ));
  return luaL_fileresult(L, remove(filename) == 0, filename);
}


static int os_rename (lua_State *L) {
  const char *fromname = (luaL_checklstring(L, (1), 
                        ((void *)0)
                        ));
  const char *toname = (luaL_checklstring(L, (2), 
                      ((void *)0)
                      ));
  return luaL_fileresult(L, rename(fromname, toname) == 0, 
                                                          ((void *)0)
                                                              );
}


static int os_tmpname (lua_State *L) {
  char buff[32];
  int err;
  { strcpy(buff, "/tmp/lua_XXXXXX"); err = 
 mkstemp64
 (buff); if (err != -1) close(err); err = (err == -1); };
  if ((err))
    return luaL_error(L, "unable to generate a unique filename");
  lua_pushstring(L, buff);
  return 1;
}


static int os_getenv (lua_State *L) {
  lua_pushstring(L, getenv((luaL_checklstring(L, (1), 
                          ((void *)0)
                          ))));
  return 1;
}


static int os_clock (lua_State *L) {
  lua_pushnumber(L, ((lua_Number)clock())/(lua_Number)
                                                     ((__clock_t) 1000000)
                                                                   );
  return 1;
}
static void setfield (lua_State *L, const char *key, int value, int delta) {




  lua_pushinteger(L, (lua_Integer)value + delta);
  lua_setfield(L, -2, key);
}


static void setboolfield (lua_State *L, const char *key, int value) {
  if (value < 0)
    return;
  lua_pushboolean(L, value);
  lua_setfield(L, -2, key);
}





static void setallfields (lua_State *L, struct tm *stm) {
  setfield(L, "year", stm->tm_year, 1900);
  setfield(L, "month", stm->tm_mon, 1);
  setfield(L, "day", stm->tm_mday, 0);
  setfield(L, "hour", stm->tm_hour, 0);
  setfield(L, "min", stm->tm_min, 0);
  setfield(L, "sec", stm->tm_sec, 0);
  setfield(L, "yday", stm->tm_yday, 1);
  setfield(L, "wday", stm->tm_wday, 1);
  setboolfield(L, "isdst", stm->tm_isdst);
}


static int getboolfield (lua_State *L, const char *key) {
  int res;
  res = (lua_getfield(L, -1, key) == 0) ? -1 : lua_toboolean(L, -1);
  lua_settop(L, -(1)-1);
  return res;
}


static int getfield (lua_State *L, const char *key, int d, int delta) {
  int isnum;
  int t = lua_getfield(L, -1, key);
  lua_Integer res = lua_tointegerx(L, -1, &isnum);
  if (!isnum) {
    if ((t != 0))
      return luaL_error(L, "field '%s' is not an integer", key);
    else if ((d < 0))
      return luaL_error(L, "field '%s' missing in date table", key);
    res = d;
  }
  else {
    if (!(res >= 0 ? res - delta <= 0x7fffffff 
                                           : 
                                             (-0x7fffffff - 1) 
                                                     + delta <= res))
      return luaL_error(L, "field '%s' is out-of-bound", key);
    res -= delta;
  }
  lua_settop(L, -(1)-1);
  return (int)res;
}


static const char *checkoption (lua_State *L, const char *conv,
                                ptrdiff_t convlen, char *buff) {
  const char *option = "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%" "||" "EcECExEXEyEY" "OdOeOHOIOmOMOSOuOUOVOwOWOy";
  int oplen = 1;
  for (; *option != '\0' && oplen <= convlen; option += oplen) {
    if (*option == '|')
      oplen++;
    else if (memcmp(conv, option, oplen) == 0) {
      memcpy(buff, conv, oplen);
      buff[oplen] = '\0';
      return conv + oplen;
    }
  }
  luaL_argerror(L, 1,
    lua_pushfstring(L, "invalid conversion specifier '%%%s'", conv));
  return conv;
}


static time_t l_checktime (lua_State *L, int arg) {
  lua_Integer t = luaL_checkinteger(L, arg);
  ((void)(((time_t)t == t) || luaL_argerror(L, (arg), ("time out-of-bounds"))));
  return (time_t)t;
}






static int os_date (lua_State *L) {
  size_t slen;
  const char *s = luaL_optlstring(L, 1, "%c", &slen);
  time_t t = ((lua_type(L, ((2))) <= 0) ? (time(
            ((void *)0)
            )) : l_checktime(L,(2)));
  const char *se = s + slen;
  struct tm tmr, *stm;
  if (*s == '!') {
    stm = gmtime_r(&t,&tmr);
    s++;
  }
  else
    stm = localtime_r(&t,&tmr);
  if (stm == 
            ((void *)0)
                )
    return luaL_error(L,
                 "date result cannot be represented in this installation");
  if (strcmp(s, "*t") == 0) {
    lua_createtable(L, 0, 9);
    setallfields(L, stm);
  }
  else {
    char cc[4];
    luaL_Buffer b;
    cc[0] = '%';
    luaL_buffinit(L, &b);
    while (s < se) {
      if (*s != '%')
        ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*s++)));
      else {
        size_t reslen;
        char *buff = luaL_prepbuffsize(&b, 250);
        s++;
        s = checkoption(L, s, se - s, cc + 1);
        reslen = strftime(buff, 250, cc, stm);
        ((&b)->n += (reslen));
      }
    }
    luaL_pushresult(&b);
  }
  return 1;
}


static int os_time (lua_State *L) {
  time_t t;
  if ((lua_type(L, (1)) <= 0))
    t = time(
            ((void *)0)
                );
  else {
    struct tm ts;
    luaL_checktype(L, 1, 5);
    lua_settop(L, 1);
    ts.tm_year = getfield(L, "year", -1, 1900);
    ts.tm_mon = getfield(L, "month", -1, 1);
    ts.tm_mday = getfield(L, "day", -1, 0);
    ts.tm_hour = getfield(L, "hour", 12, 0);
    ts.tm_min = getfield(L, "min", 0, 0);
    ts.tm_sec = getfield(L, "sec", 0, 0);
    ts.tm_isdst = getboolfield(L, "isdst");
    t = mktime(&ts);
    setallfields(L, &ts);
  }
  if (t != (time_t)(lua_Integer)t || t == (time_t)(-1))
    return luaL_error(L,
                  "time result cannot be represented in this installation");
  lua_pushinteger(L,(lua_Integer)(t));
  return 1;
}


static int os_difftime (lua_State *L) {
  time_t t1 = l_checktime(L, 1);
  time_t t2 = l_checktime(L, 2);
  lua_pushnumber(L, (lua_Number)difftime(t1, t2));
  return 1;
}




static int os_setlocale (lua_State *L) {
  static const int cat[] = {
                           6
                                 , 
                                   3
                                             , 
                                               0
                                                       , 
                                                         4
                                                                    ,
                      
                     1
                               , 
                                 2
                                        };
  static const char *const catnames[] = {"all", "collate", "ctype", "monetary",
     "numeric", "time", 
                       ((void *)0)
                           };
  const char *l = (luaL_optlstring(L, (1), (
                 ((void *)0)
                 ), 
                 ((void *)0)
                 ));
  int op = luaL_checkoption(L, 2, "all", catnames);
  lua_pushstring(L, setlocale(cat[op], l));
  return 1;
}


static int os_exit (lua_State *L) {
  int status;
  if ((lua_type(L, (1)) == 1))
    status = (lua_toboolean(L, 1) ? 
                                   0 
                                                : 
                                                  1
                                                              );
  else
    status = (int)luaL_optinteger(L, 1, 
                                       0
                                                   );
  if (lua_toboolean(L, 2))
    lua_close(L);
  if (L) exit(status);
  return 0;
}


static const luaL_Reg syslib[] = {
  {"clock", os_clock},
  {"date", os_date},
  {"difftime", os_difftime},
  {"execute", os_execute},
  {"exit", os_exit},
  {"getenv", os_getenv},
  {"remove", os_remove},
  {"rename", os_rename},
  {"setlocale", os_setlocale},
  {"time", os_time},
  {"tmpname", os_tmpname},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};





extern int luaopen_os (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(syslib)/sizeof((syslib)[0]) - 1), luaL_setfuncs(L,syslib,0));
  return 1;
}


static int str_len (lua_State *L) {
  size_t l;
  luaL_checklstring(L, 1, &l);
  lua_pushinteger(L, (lua_Integer)l);
  return 1;
}
static size_t posrelatI (lua_Integer pos, size_t len) {
  if (pos > 0)
    return (size_t)pos;
  else if (pos == 0)
    return 1;
  else if (pos < -(lua_Integer)len)
    return 1;
  else return len + (size_t)pos + 1;
}







static size_t getendpos (lua_State *L, int arg, lua_Integer def,
                         size_t len) {
  lua_Integer pos = luaL_optinteger(L, arg, def);
  if (pos > (lua_Integer)len)
    return len;
  else if (pos >= 0)
    return (size_t)pos;
  else if (pos < -(lua_Integer)len)
    return 0;
  else return len + (size_t)pos + 1;
}


static int str_sub (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  size_t start = posrelatI(luaL_checkinteger(L, 2), l);
  size_t end = getendpos(L, 3, -1, l);
  if (start <= end)
    lua_pushlstring(L, s + start - 1, (end - start) + 1);
  else lua_pushstring(L, "" "");
  return 1;
}


static int str_reverse (lua_State *L) {
  size_t l, i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i = 0; i < l; i++)
    p[i] = s[l - i - 1];
  luaL_pushresultsize(&b, l);
  return 1;
}


static int str_lower (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = tolower(((unsigned char)(s[i])));
  luaL_pushresultsize(&b, l);
  return 1;
}


static int str_upper (lua_State *L) {
  size_t l;
  size_t i;
  luaL_Buffer b;
  const char *s = luaL_checklstring(L, 1, &l);
  char *p = luaL_buffinitsize(L, &b, l);
  for (i=0; i<l; i++)
    p[i] = toupper(((unsigned char)(s[i])));
  luaL_pushresultsize(&b, l);
  return 1;
}


static int str_rep (lua_State *L) {
  size_t l, lsep;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_Integer n = luaL_checkinteger(L, 2);
  const char *sep = luaL_optlstring(L, 3, "", &lsep);
  if (n <= 0)
    lua_pushstring(L, "" "");
  else if ((l + lsep < l || l + lsep > (sizeof(size_t) < sizeof(int) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffff
          )) / n))
    return luaL_error(L, "resulting string too large");
  else {
    size_t totallen = (size_t)n * l + (size_t)(n - 1) * lsep;
    luaL_Buffer b;
    char *p = luaL_buffinitsize(L, &b, totallen);
    while (n-- > 1) {
      memcpy(p, s, l * sizeof(char)); p += l;
      if (lsep > 0) {
        memcpy(p, sep, lsep * sizeof(char));
        p += lsep;
      }
    }
    memcpy(p, s, l * sizeof(char));
    luaL_pushresultsize(&b, totallen);
  }
  return 1;
}


static int str_byte (lua_State *L) {
  size_t l;
  const char *s = luaL_checklstring(L, 1, &l);
  lua_Integer pi = luaL_optinteger(L, 2, 1);
  size_t posi = posrelatI(pi, l);
  size_t pose = getendpos(L, 3, pi, l);
  int n, i;
  if (posi > pose) return 0;
  if ((pose - posi >= (size_t)0x7fffffff
     ))
    return luaL_error(L, "string slice too long");
  n = (int)(pose - posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  for (i=0; i<n; i++)
    lua_pushinteger(L, ((unsigned char)(s[posi+i-1])));
  return n;
}


static int str_char (lua_State *L) {
  int n = lua_gettop(L);
  int i;
  luaL_Buffer b;
  char *p = luaL_buffinitsize(L, &b, n);
  for (i=1; i<=n; i++) {
    lua_Unsigned c = (lua_Unsigned)luaL_checkinteger(L, i);
    ((void)((c <= (lua_Unsigned)
   (0x7f * 2 + 1)
   ) || luaL_argerror(L, (i), ("value out of range"))));
    p[i - 1] = ((unsigned char)(c));
  }
  luaL_pushresultsize(&b, n);
  return 1;
}
struct str_Writer {
  int init;
  luaL_Buffer B;
};


static int writer (lua_State *L, const void *b, size_t size, void *ud) {
  struct str_Writer *state = (struct str_Writer *)ud;
  if (!state->init) {
    state->init = 1;
    luaL_buffinit(L, &state->B);
  }
  luaL_addlstring(&state->B, (const char *)b, size);
  return 0;
}


static int str_dump (lua_State *L) {
  struct str_Writer state;
  int strip = lua_toboolean(L, 2);
  luaL_checktype(L, 1, 6);
  lua_settop(L, 1);
  state.init = 0;
  if ((lua_dump(L, writer, &state, strip) != 0))
    return luaL_error(L, "unable to dump given function");
  luaL_pushresult(&state.B);
  return 1;
}
static int tonum (lua_State *L, int arg) {
  if (lua_type(L, arg) == 3) {
    lua_pushvalue(L, arg);
    return 1;
  }
  else {
    size_t len;
    const char *s = lua_tolstring(L, arg, &len);
    return (s != 
                ((void *)0) 
                     && lua_stringtonumber(L, s) == len + 1);
  }
}


static void trymt (lua_State *L, const char *mtname) {
  lua_settop(L, 2);
  if ((lua_type(L, 2) == 4 || !luaL_getmetafield(L, 2, mtname))
                                                  )
    luaL_error(L, "attempt to %s a '%s' with a '%s'", mtname + 2,
                  lua_typename(L, lua_type(L,(-2))), lua_typename(L, lua_type(L,(-1))));
  lua_rotate(L, (-3), 1);
  lua_callk(L, (2), (1), 0, 
 ((void *)0)
 );
}


static int arith (lua_State *L, int op, const char *mtname) {
  if (tonum(L, 1) && tonum(L, 2))
    lua_arith(L, op);
  else
    trymt(L, mtname);
  return 1;
}


static int arith_add (lua_State *L) {
  return arith(L, 0, "__add");
}

static int arith_sub (lua_State *L) {
  return arith(L, 1, "__sub");
}

static int arith_mul (lua_State *L) {
  return arith(L, 2, "__mul");
}

static int arith_mod (lua_State *L) {
  return arith(L, 3, "__mod");
}

static int arith_pow (lua_State *L) {
  return arith(L, 4, "__pow");
}

static int arith_div (lua_State *L) {
  return arith(L, 5, "__div");
}

static int arith_idiv (lua_State *L) {
  return arith(L, 6, "__idiv");
}

static int arith_unm (lua_State *L) {
  return arith(L, 12, "__unm");
}


static const luaL_Reg stringmetamethods[] = {
  {"__add", arith_add},
  {"__sub", arith_sub},
  {"__mul", arith_mul},
  {"__mod", arith_mod},
  {"__pow", arith_pow},
  {"__div", arith_div},
  {"__idiv", arith_idiv},
  {"__unm", arith_unm},
  {"__index", 
             ((void *)0)
                 },
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};
typedef struct MatchState {
  const char *src_init;
  const char *src_end;
  const char *p_end;
  lua_State *L;
  int matchdepth;
  unsigned char level;
  struct {
    const char *init;
    ptrdiff_t len;
  } capture[32];
} MatchState;



static const char *match (MatchState *ms, const char *s, const char *p);
static int check_capture (MatchState *ms, int l) {
  l -= '1';
  if ((l < 0 || l >= ms->level || ms->capture[l].len == (-1))
                                                      )
    return luaL_error(ms->L, "invalid capture index %%%d", l + 1);
  return l;
}


static int capture_to_close (MatchState *ms) {
  int level = ms->level;
  for (level--; level>=0; level--)
    if (ms->capture[level].len == (-1)) return level;
  return luaL_error(ms->L, "invalid pattern capture");
}


static const char *classend (MatchState *ms, const char *p) {
  switch (*p++) {
    case '%': {
      if ((p == ms->p_end))
        luaL_error(ms->L, "malformed pattern (ends with '%%')");
      return p+1;
    }
    case '[': {
      if (*p == '^') p++;
      do {
        if ((p == ms->p_end))
          luaL_error(ms->L, "malformed pattern (missing ']')");
        if (*(p++) == '%' && p < ms->p_end)
          p++;
      } while (*p != ']');
      return p+1;
    }
    default: {
      return p;
    }
  }
}


static int match_class (int c, int cl) {
  int res;
  switch (tolower(cl)) {
    case 'a' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISalpha)
                              ; break;
    case 'c' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _IScntrl)
                              ; break;
    case 'd' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISdigit)
                              ; break;
    case 'g' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISgraph)
                              ; break;
    case 'l' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISlower)
                              ; break;
    case 'p' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISpunct)
                              ; break;
    case 's' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISspace)
                              ; break;
    case 'u' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISupper)
                              ; break;
    case 'w' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISalnum)
                              ; break;
    case 'x' : res = 
                    ((*__ctype_b_loc ())[(int) ((
                    c
                    ))] & (unsigned short int) _ISxdigit)
                               ; break;
    case 'z' : res = (c == 0); break;
    default: return (cl == c);
  }
  return (
         ((*__ctype_b_loc ())[(int) ((
         cl
         ))] & (unsigned short int) _ISlower) 
                     ? res : !res);
}


static int matchbracketclass (int c, const char *p, const char *ec) {
  int sig = 1;
  if (*(p+1) == '^') {
    sig = 0;
    p++;
  }
  while (++p < ec) {
    if (*p == '%') {
      p++;
      if (match_class(c, ((unsigned char)(*p))))
        return sig;
    }
    else if ((*(p+1) == '-') && (p+2 < ec)) {
      p+=2;
      if (((unsigned char)(*(p-2))) <= c && c <= ((unsigned char)(*p)))
        return sig;
    }
    else if (((unsigned char)(*p)) == c) return sig;
  }
  return !sig;
}


static int singlematch (MatchState *ms, const char *s, const char *p,
                        const char *ep) {
  if (s >= ms->src_end)
    return 0;
  else {
    int c = ((unsigned char)(*s));
    switch (*p) {
      case '.': return 1;
      case '%': return match_class(c, ((unsigned char)(*(p+1))));
      case '[': return matchbracketclass(c, p, ep-1);
      default: return (((unsigned char)(*p)) == c);
    }
  }
}


static const char *matchbalance (MatchState *ms, const char *s,
                                   const char *p) {
  if ((p >= ms->p_end - 1))
    luaL_error(ms->L, "malformed pattern (missing arguments to '%%b')");
  if (*s != *p) return 
                      ((void *)0)
                          ;
  else {
    int b = *p;
    int e = *(p+1);
    int cont = 1;
    while (++s < ms->src_end) {
      if (*s == e) {
        if (--cont == 0) return s+1;
      }
      else if (*s == b) cont++;
    }
  }
  return 
        ((void *)0)
            ;
}


static const char *max_expand (MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  ptrdiff_t i = 0;
  while (singlematch(ms, s + i, p, ep))
    i++;

  while (i>=0) {
    const char *res = match(ms, (s+i), ep+1);
    if (res) return res;
    i--;
  }
  return 
        ((void *)0)
            ;
}


static const char *min_expand (MatchState *ms, const char *s,
                                 const char *p, const char *ep) {
  for (;;) {
    const char *res = match(ms, s, ep+1);
    if (res != 
              ((void *)0)
                  )
      return res;
    else if (singlematch(ms, s, p, ep))
      s++;
    else return 
               ((void *)0)
                   ;
  }
}


static const char *start_capture (MatchState *ms, const char *s,
                                    const char *p, int what) {
  const char *res;
  int level = ms->level;
  if (level >= 32) luaL_error(ms->L, "too many captures");
  ms->capture[level].init = s;
  ms->capture[level].len = what;
  ms->level = level+1;
  if ((res=match(ms, s, p)) == 
                              ((void *)0)
                                  )
    ms->level--;
  return res;
}


static const char *end_capture (MatchState *ms, const char *s,
                                  const char *p) {
  int l = capture_to_close(ms);
  const char *res;
  ms->capture[l].len = s - ms->capture[l].init;
  if ((res = match(ms, s, p)) == 
                                ((void *)0)
                                    )
    ms->capture[l].len = (-1);
  return res;
}


static const char *match_capture (MatchState *ms, const char *s, int l) {
  size_t len;
  l = check_capture(ms, l);
  len = ms->capture[l].len;
  if ((size_t)(ms->src_end-s) >= len &&
      memcmp(ms->capture[l].init, s, len) == 0)
    return s+len;
  else return 
             ((void *)0)
                 ;
}


static const char *match (MatchState *ms, const char *s, const char *p) {
  if ((ms->matchdepth-- == 0))
    luaL_error(ms->L, "pattern too complex");
  init:
  if (p != ms->p_end) {
    switch (*p) {
      case '(': {
        if (*(p + 1) == ')')
          s = start_capture(ms, s, p + 2, (-2));
        else
          s = start_capture(ms, s, p + 1, (-1));
        break;
      }
      case ')': {
        s = end_capture(ms, s, p + 1);
        break;
      }
      case '$': {
        if ((p + 1) != ms->p_end)
          goto dflt;
        s = (s == ms->src_end) ? s : 
                                    ((void *)0)
                                        ;
        break;
      }
      case '%': {
        switch (*(p + 1)) {
          case 'b': {
            s = matchbalance(ms, s, p + 2);
            if (s != 
                    ((void *)0)
                        ) {
              p += 4; goto init;
            }
            break;
          }
          case 'f': {
            const char *ep; char previous;
            p += 2;
            if ((*p != '['))
              luaL_error(ms->L, "missing '[' after '%%f' in pattern");
            ep = classend(ms, p);
            previous = (s == ms->src_init) ? '\0' : *(s - 1);
            if (!matchbracketclass(((unsigned char)(previous)), p, ep - 1) &&
               matchbracketclass(((unsigned char)(*s)), p, ep - 1)) {
              p = ep; goto init;
            }
            s = 
               ((void *)0)
                   ;
            break;
          }
          case '0': case '1': case '2': case '3':
          case '4': case '5': case '6': case '7':
          case '8': case '9': {
            s = match_capture(ms, s, ((unsigned char)(*(p + 1))));
            if (s != 
                    ((void *)0)
                        ) {
              p += 2; goto init;
            }
            break;
          }
          default: goto dflt;
        }
        break;
      }
      default: dflt: {
        const char *ep = classend(ms, p);

        if (!singlematch(ms, s, p, ep)) {
          if (*ep == '*' || *ep == '?' || *ep == '-') {
            p = ep + 1; goto init;
          }
          else
            s = 
               ((void *)0)
                   ;
        }
        else {
          switch (*ep) {
            case '?': {
              const char *res;
              if ((res = match(ms, s + 1, ep + 1)) != 
                                                     ((void *)0)
                                                         )
                s = res;
              else {
                p = ep + 1; goto init;
              }
              break;
            }
            case '+':
              s++;

            case '*':
              s = max_expand(ms, s, p, ep);
              break;
            case '-':
              s = min_expand(ms, s, p, ep);
              break;
            default:
              s++; p = ep; goto init;
          }
        }
        break;
      }
    }
  }
  ms->matchdepth++;
  return s;
}



static const char *lmemfind (const char *s1, size_t l1,
                               const char *s2, size_t l2) {
  if (l2 == 0) return s1;
  else if (l2 > l1) return 
                          ((void *)0)
                              ;
  else {
    const char *init;
    l2--;
    l1 = l1-l2;
    while (l1 > 0 && (init = (const char *)memchr(s1, *s2, l1)) != 
                                                                  ((void *)0)
                                                                      ) {
      init++;
      if (memcmp(init, s2+1, l2) == 0)
        return init-1;
      else {
        l1 -= init-s1;
        s1 = init;
      }
    }
    return 
          ((void *)0)
              ;
  }
}
static size_t get_onecapture (MatchState *ms, int i, const char *s,
                              const char *e, const char **cap) {
  if (i >= ms->level) {
    if ((i != 0))
      luaL_error(ms->L, "invalid capture index %%%d", i + 1);
    *cap = s;
    return e - s;
  }
  else {
    ptrdiff_t capl = ms->capture[i].len;
    *cap = ms->capture[i].init;
    if ((capl == (-1)))
      luaL_error(ms->L, "unfinished capture");
    else if (capl == (-2))
      lua_pushinteger(ms->L, (ms->capture[i].init - ms->src_init) + 1);
    return capl;
  }
}





static void push_onecapture (MatchState *ms, int i, const char *s,
                                                    const char *e) {
  const char *cap;
  ptrdiff_t l = get_onecapture(ms, i, s, e, &cap);
  if (l != (-2))
    lua_pushlstring(ms->L, cap, l);

}


static int push_captures (MatchState *ms, const char *s, const char *e) {
  int i;
  int nlevels = (ms->level == 0 && s) ? 1 : ms->level;
  luaL_checkstack(ms->L, nlevels, "too many captures");
  for (i = 0; i < nlevels; i++)
    push_onecapture(ms, i, s, e);
  return nlevels;
}



static int nospecials (const char *p, size_t l) {
  size_t upto = 0;
  do {
    if (strpbrk(p + upto, "^$*+?.([%-"))
      return 0;
    upto += strlen(p + upto) + 1;
  } while (upto <= l);
  return 1;
}


static void prepstate (MatchState *ms, lua_State *L,
                       const char *s, size_t ls, const char *p, size_t lp) {
  ms->L = L;
  ms->matchdepth = 200;
  ms->src_init = s;
  ms->src_end = s + ls;
  ms->p_end = p + lp;
}


static void reprepstate (MatchState *ms) {
  ms->level = 0;
  ((void)0);
}


static int str_find_aux (lua_State *L, int find) {
  size_t ls, lp;
  const char *s = luaL_checklstring(L, 1, &ls);
  const char *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  if (init > ls) {
    lua_pushnil(L);
    return 1;
  }

  if (find && (lua_toboolean(L, 4) || nospecials(p, lp))) {

    const char *s2 = lmemfind(s + init, ls - init, p, lp);
    if (s2) {
      lua_pushinteger(L, (s2 - s) + 1);
      lua_pushinteger(L, (s2 - s) + lp);
      return 2;
    }
  }
  else {
    MatchState ms;
    const char *s1 = s + init;
    int anchor = (*p == '^');
    if (anchor) {
      p++; lp--;
    }
    prepstate(&ms, L, s, ls, p, lp);
    do {
      const char *res;
      reprepstate(&ms);
      if ((res=match(&ms, s1, p)) != 
                                    ((void *)0)
                                        ) {
        if (find) {
          lua_pushinteger(L, (s1 - s) + 1);
          lua_pushinteger(L, res - s);
          return push_captures(&ms, 
                                   ((void *)0)
                                       , 0) + 2;
        }
        else
          return push_captures(&ms, s1, res);
      }
    } while (s1++ < ms.src_end && !anchor);
  }
  lua_pushnil(L);
  return 1;
}


static int str_find (lua_State *L) {
  return str_find_aux(L, 1);
}


static int str_match (lua_State *L) {
  return str_find_aux(L, 0);
}



typedef struct GMatchState {
  const char *src;
  const char *p;
  const char *lastmatch;
  MatchState ms;
} GMatchState;


static int gmatch_aux (lua_State *L) {
  GMatchState *gm = (GMatchState *)lua_touserdata(L, ((-1000000 - 1000) - (3)));
  const char *src;
  gm->ms.L = L;
  for (src = gm->src; src <= gm->ms.src_end; src++) {
    const char *e;
    reprepstate(&gm->ms);
    if ((e = match(&gm->ms, src, gm->p)) != 
                                           ((void *)0) 
                                                && e != gm->lastmatch) {
      gm->src = gm->lastmatch = e;
      return push_captures(&gm->ms, src, e);
    }
  }
  return 0;
}


static int gmatch (lua_State *L) {
  size_t ls, lp;
  const char *s = luaL_checklstring(L, 1, &ls);
  const char *p = luaL_checklstring(L, 2, &lp);
  size_t init = posrelatI(luaL_optinteger(L, 3, 1), ls) - 1;
  GMatchState *gm;
  lua_settop(L, 2);
  gm = (GMatchState *)lua_newuserdatauv(L, sizeof(GMatchState), 0);
  if (init > ls)
    init = ls + 1;
  prepstate(&gm->ms, L, s, ls, p, lp);
  gm->src = s + init; gm->p = p; gm->lastmatch = 
                                                ((void *)0)
                                                    ;
  lua_pushcclosure(L, gmatch_aux, 3);
  return 1;
}


static void add_s (MatchState *ms, luaL_Buffer *b, const char *s,
                                                   const char *e) {
  size_t l;
  lua_State *L = ms->L;
  const char *news = lua_tolstring(L, 3, &l);
  const char *p;
  while ((p = (char *)memchr(news, '%', l)) != 
                                                ((void *)0)
                                                    ) {
    luaL_addlstring(b, news, p - news);
    p++;
    if (*p == '%')
      ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = (*p)));
    else if (*p == '0')
        luaL_addlstring(b, s, e - s);
    else if (
            ((*__ctype_b_loc ())[(int) ((
            ((unsigned char)(*p))
            ))] & (unsigned short int) _ISdigit)
                              ) {
      const char *cap;
      ptrdiff_t resl = get_onecapture(ms, *p - '1', s, e, &cap);
      if (resl == (-2))
        luaL_addvalue(b);
      else
        luaL_addlstring(b, cap, resl);
    }
    else
      luaL_error(L, "invalid use of '%c' in replacement string", '%');
    l -= p + 1 - news;
    news = p + 1;
  }
  luaL_addlstring(b, news, l);
}







static int add_value (MatchState *ms, luaL_Buffer *b, const char *s,
                                      const char *e, int tr) {
  lua_State *L = ms->L;
  switch (tr) {
    case 6: {
      int n;
      lua_pushvalue(L, 3);
      n = push_captures(ms, s, e);
      lua_callk(L, (n), (1), 0, 
     ((void *)0)
     );
      break;
    }
    case 5: {
      push_onecapture(ms, 0, s, e);
      lua_gettable(L, 3);
      break;
    }
    default: {
      add_s(ms, b, s, e);
      return 1;
    }
  }
  if (!lua_toboolean(L, -1)) {
    lua_settop(L, -(1)-1);
    luaL_addlstring(b, s, e - s);
    return 0;
  }
  else if ((!lua_isstring(L, -1)))
    return luaL_error(L, "invalid replacement value (a %s)",
                         lua_typename(L, lua_type(L,(-1))));
  else {
    luaL_addvalue(b);
    return 1;
  }
}


static int str_gsub (lua_State *L) {
  size_t srcl, lp;
  const char *src = luaL_checklstring(L, 1, &srcl);
  const char *p = luaL_checklstring(L, 2, &lp);
  const char *lastmatch = 
                         ((void *)0)
                             ;
  int tr = lua_type(L, 3);
  lua_Integer max_s = luaL_optinteger(L, 4, srcl + 1);
  int anchor = (*p == '^');
  lua_Integer n = 0;
  int changed = 0;
  MatchState ms;
  luaL_Buffer b;
  ((void)((tr == 3 || tr == 4 || tr == 6 || tr == 5) || luaL_typeerror(L, (3), ("string/function/table"))))

                                              ;
  luaL_buffinit(L, &b);
  if (anchor) {
    p++; lp--;
  }
  prepstate(&ms, L, src, srcl, p, lp);
  while (n < max_s) {
    const char *e;
    reprepstate(&ms);
    if ((e = match(&ms, src, p)) != 
                                   ((void *)0) 
                                        && e != lastmatch) {
      n++;
      changed = add_value(&ms, &b, src, e, tr) | changed;
      src = lastmatch = e;
    }
    else if (src < ms.src_end)
      ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*src++)));
    else break;
    if (anchor) break;
  }
  if (!changed)
    lua_pushvalue(L, 1);
  else {
    luaL_addlstring(&b, src, ms.src_end-src);
    luaL_pushresult(&b);
  }
  lua_pushinteger(L, n);
  return 2;
}
static void addquoted (luaL_Buffer *b, const char *s, size_t len) {
  ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = ('"')));
  while (len--) {
    if (*s == '"' || *s == '\\' || *s == '\n') {
      ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = ('\\')));
      ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = (*s)));
    }
    else if (
            ((*__ctype_b_loc ())[(int) ((
            ((unsigned char)(*s))
            ))] & (unsigned short int) _IScntrl)
                              ) {
      char buff[10];
      if (!
          ((*__ctype_b_loc ())[(int) ((
          ((unsigned char)(*(s+1)))
          ))] & (unsigned short int) _ISdigit)
                                )
        snprintf(buff,sizeof(buff),"\\%d",(int)((unsigned char)(*s)));
      else
        snprintf(buff,sizeof(buff),"\\%03d",(int)((unsigned char)(*s)));
      luaL_addstring(b, buff);
    }
    else
      ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = (*s)));
    s++;
  }
  ((void)((b)->n < (b)->size || luaL_prepbuffsize((b), 1)), ((b)->b[(b)->n++] = ('"')));
}
static int quotefloat (lua_State *L, char *buff, lua_Number n) {
  const char *s;
  if (n == (lua_Number)
                      1e10000
                              )
    s = "1e9999";
  else if (n == -(lua_Number)
                            1e10000
                                    )
    s = "-1e9999";
  else if (n != n)
    s = "(0/0)";
  else {
    int nb = ((void)L, snprintf(buff,120,"%" "" "a",(double)(n)))
                                                              ;

    if (memchr(buff, '.', nb) == 
                                ((void *)0)
                                    ) {
      char point = (localeconv()->decimal_point[0]);
      char *ppoint = (char *)memchr(buff, point, nb);
      if (ppoint) *ppoint = '.';
    }
    return nb;
  }

  return snprintf(buff,120,"%s",s);
}


static void addliteral (lua_State *L, luaL_Buffer *b, int arg) {
  switch (lua_type(L, arg)) {
    case 4: {
      size_t len;
      const char *s = lua_tolstring(L, arg, &len);
      addquoted(b, s, len);
      break;
    }
    case 3: {
      char *buff = luaL_prepbuffsize(b, 120);
      int nb;
      if (!lua_isinteger(L, arg))
        nb = quotefloat(L, buff, lua_tonumberx(L,(arg),
                                ((void *)0)
                                ));
      else {
        lua_Integer n = lua_tointegerx(L,(arg),
                       ((void *)0)
                       );
        const char *format = (n == 
                                  (-0x7fffffffffffffffLL - 1LL)
                                                )
                           ? "0x%" "ll" "x"
                           : "%" "ll" "d";
        nb = snprintf(buff,120,format,(long long)n);
      }
      ((b)->n += (nb));
      break;
    }
    case 0: case 1: {
      luaL_tolstring(L, arg, 
                            ((void *)0)
                                );
      luaL_addvalue(b);
      break;
    }
    default: {
      luaL_argerror(L, arg, "value has no literal form");
    }
  }
}


static const char *get2digits (const char *s) {
  if (
     ((*__ctype_b_loc ())[(int) ((
     ((unsigned char)(*s))
     ))] & (unsigned short int) _ISdigit)
                       ) {
    s++;
    if (
       ((*__ctype_b_loc ())[(int) ((
       ((unsigned char)(*s))
       ))] & (unsigned short int) _ISdigit)
                         ) s++;
  }
  return s;
}
static void checkformat (lua_State *L, const char *form, const char *flags,
                                       int precision) {
  const char *spec = form + 1;
  spec += strspn(spec, flags);
  if (*spec != '0') {
    spec = get2digits(spec);
    if (*spec == '.' && precision) {
      spec++;
      spec = get2digits(spec);
    }
  }
  if (!
      ((*__ctype_b_loc ())[(int) ((
      ((unsigned char)(*spec))
      ))] & (unsigned short int) _ISalpha)
                           )
    luaL_error(L, "invalid conversion specification: '%s'", form);
}






static const char *getformat (lua_State *L, const char *strfrmt,
                                            char *form) {

  size_t len = strspn(strfrmt, "-+#0 " "123456789.");
  len++;

  if (len >= 32 - 10)
    luaL_error(L, "invalid format (too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, len * sizeof(char));
  *(form + len) = '\0';
  return strfrmt + len - 1;
}





static void addlenmod (char *form, const char *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  char spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


static int str_format (lua_State *L) {
  int top = lua_gettop(L);
  int arg = 1;
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  const char *flags;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != '%')
      ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*strfrmt++)));
    else if (*++strfrmt == '%')
      ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (*strfrmt++)));
    else {
      char form[32];
      int maxitem = 120;
      char *buff = luaL_prepbuffsize(&b, maxitem);
      int nb = 0;
      if (++arg > top)
        return luaL_argerror(L, arg, "no value");
      strfrmt = getformat(L, strfrmt, form);
      switch (*strfrmt++) {
        case 'c': {
          checkformat(L, form, "-", 0);
          nb = snprintf(buff,maxitem,form,(int)luaL_checkinteger(L, arg));
          break;
        }
        case 'd': case 'i':
          flags = "-+0 ";
          goto intcase;
        case 'u':
          flags = "-0";
          goto intcase;
        case 'o': case 'x': case 'X':
          flags = "-#0";
         intcase: {
          lua_Integer n = luaL_checkinteger(L, arg);
          checkformat(L, form, flags, 1);
          addlenmod(form, "ll");
          nb = snprintf(buff,maxitem,form,(long long)n);
          break;
        }
        case 'a': case 'A':
          checkformat(L, form, "-+#0 ", 1);
          addlenmod(form, "");
          nb = ((void)L, snprintf(buff,maxitem,form,(double)(luaL_checknumber(L, arg))))
                                                           ;
          break;
        case 'f':
          maxitem = (110 + (308
                   ));
          buff = luaL_prepbuffsize(&b, maxitem);

        case 'e': case 'E': case 'g': case 'G': {
          lua_Number n = luaL_checknumber(L, arg);
          checkformat(L, form, "-+#0 ", 1);
          addlenmod(form, "");
          nb = snprintf(buff,maxitem,form,(double)n);
          break;
        }
        case 'p': {
          const void *p = lua_topointer(L, arg);
          checkformat(L, form, "-", 0);
          if (p == 
                  ((void *)0)
                      ) {
            p = "(null)";
            form[strlen(form) - 1] = 's';
          }
          nb = snprintf(buff,maxitem,form,p);
          break;
        }
        case 'q': {
          if (form[2] != '\0')
            return luaL_error(L, "specifier '%%q' cannot have modifiers");
          addliteral(L, &b, arg);
          break;
        }
        case 's': {
          size_t l;
          const char *s = luaL_tolstring(L, arg, &l);
          if (form[2] == '\0')
            luaL_addvalue(&b);
          else {
            ((void)((l == strlen(s)) || luaL_argerror(L, (arg), ("string contains zeros"))));
            checkformat(L, form, "-", 1);
            if (strchr(form, '.') == 
                                    ((void *)0) 
                                         && l >= 100) {

              luaL_addvalue(&b);
            }
            else {
              nb = snprintf(buff,maxitem,form,s);
              lua_settop(L, -(1)-1);
            }
          }
          break;
        }
        default: {
          return luaL_error(L, "invalid conversion '%s' to 'format'", form);
        }
      }
      ((void)0);
      ((&b)->n += (nb));
    }
  }
  luaL_pushresult(&b);
  return 1;
}
static const union {
  int dummy;
  char little;
} nativeendian = {1};





typedef struct Header {
  lua_State *L;
  int islittle;
  int maxalign;
} Header;





typedef enum KOption {
  Kint,
  Kuint,
  Kfloat,
  Knumber,
  Kdouble,
  Kchar,
  Kstring,
  Kzstr,
  Kpadding,
  Kpaddalign,
  Knop
} KOption;






static int digit (int c) { return '0' <= c && c <= '9'; }

static int getnum (const char **fmt, int df) {
  if (!digit(**fmt))
    return df;
  else {
    int a = 0;
    do {
      a = a*10 + (*((*fmt)++) - '0');
    } while (digit(**fmt) && a <= ((int)(sizeof(size_t) < sizeof(int) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffff
                                       )) - 9)/10);
    return a;
  }
}






static int getnumlimit (Header *h, const char **fmt, int df) {
  int sz = getnum(fmt, df);
  if ((sz > 16 || sz <= 0))
    return luaL_error(h->L, "integral size (%d) out of limits [1,%d]",
                            sz, 16);
  return sz;
}





static void initheader (lua_State *L, Header *h) {
  h->L = L;
  h->islittle = nativeendian.little;
  h->maxalign = 1;
}





static KOption getoption (Header *h, const char **fmt, int *size) {

  struct cD { char c; union { lua_Number n; double u; void *s; lua_Integer i; long l; } u; };
  int opt = *((*fmt)++);
  *size = 0;
  switch (opt) {
    case 'b': *size = sizeof(char); return Kint;
    case 'B': *size = sizeof(char); return Kuint;
    case 'h': *size = sizeof(short); return Kint;
    case 'H': *size = sizeof(short); return Kuint;
    case 'l': *size = sizeof(long); return Kint;
    case 'L': *size = sizeof(long); return Kuint;
    case 'j': *size = sizeof(lua_Integer); return Kint;
    case 'J': *size = sizeof(lua_Integer); return Kuint;
    case 'T': *size = sizeof(size_t); return Kuint;
    case 'f': *size = sizeof(float); return Kfloat;
    case 'n': *size = sizeof(lua_Number); return Knumber;
    case 'd': *size = sizeof(double); return Kdouble;
    case 'i': *size = getnumlimit(h, fmt, sizeof(int)); return Kint;
    case 'I': *size = getnumlimit(h, fmt, sizeof(int)); return Kuint;
    case 's': *size = getnumlimit(h, fmt, sizeof(size_t)); return Kstring;
    case 'c':
      *size = getnum(fmt, -1);
      if ((*size == -1))
        luaL_error(h->L, "missing size for format option 'c'");
      return Kchar;
    case 'z': return Kzstr;
    case 'x': *size = 1; return Kpadding;
    case 'X': return Kpaddalign;
    case ' ': break;
    case '<': h->islittle = 1; break;
    case '>': h->islittle = 0; break;
    case '=': h->islittle = nativeendian.little; break;
    case '!': {
      const int maxalign = 
                          ((long)&((struct cD
                          *)0)->u
                          )
                                                ;
      h->maxalign = getnumlimit(h, fmt, maxalign);
      break;
    }
    default: luaL_error(h->L, "invalid format option '%c'", opt);
  }
  return Knop;
}
static KOption getdetails (Header *h, size_t totalsize,
                           const char **fmt, int *psize, int *ntoalign) {
  KOption opt = getoption(h, fmt, psize);
  int align = *psize;
  if (opt == Kpaddalign) {
    if (**fmt == '\0' || getoption(h, fmt, &align) == Kchar || align == 0)
      luaL_argerror(h->L, 1, "invalid next option for option 'X'");
  }
  if (align <= 1 || opt == Kchar)
    *ntoalign = 0;
  else {
    if (align > h->maxalign)
      align = h->maxalign;
    if (((align & (align - 1)) != 0))
      luaL_argerror(h->L, 1, "format asks for alignment not power of 2");
    *ntoalign = (align - (int)(totalsize & (align - 1))) & (align - 1);
  }
  return opt;
}
static void packint (luaL_Buffer *b, lua_Unsigned n,
                     int islittle, int size, int neg) {
  char *buff = luaL_prepbuffsize(b, size);
  int i;
  buff[islittle ? 0 : size - 1] = (char)(n & ((1 << 8
                                            ) - 1));
  for (i = 1; i < size; i++) {
    n >>= 8
           ;
    buff[islittle ? i : size - 1 - i] = (char)(n & ((1 << 8
                                                  ) - 1));
  }
  if (neg && size > ((int)sizeof(lua_Integer))) {
    for (i = ((int)sizeof(lua_Integer)); i < size; i++)
      buff[islittle ? i : size - 1 - i] = (char)((1 << 8
                                               ) - 1);
  }
  ((b)->n += (size));
}






static void copywithendian (char *dest, const char *src,
                            int size, int islittle) {
  if (islittle == nativeendian.little)
    memcpy(dest, src, size);
  else {
    dest += size - 1;
    while (size-- != 0)
      *(dest--) = *(src++);
  }
}


static int str_pack (lua_State *L) {
  luaL_Buffer b;
  Header h;
  const char *fmt = (luaL_checklstring(L, (1), 
                   ((void *)0)
                   ));
  int arg = 1;
  size_t totalsize = 0;
  initheader(L, &h);
  lua_pushnil(L);
  luaL_buffinit(L, &b);
  while (*fmt != '\0') {
    int size, ntoalign;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    totalsize += ntoalign + size;
    while (ntoalign-- > 0)
     ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (0x00)));
    arg++;
    switch (opt) {
      case Kint: {
        lua_Integer n = luaL_checkinteger(L, arg);
        if (size < ((int)sizeof(lua_Integer))) {
          lua_Integer lim = (lua_Integer)1 << ((size * 8
                                                        ) - 1);
          ((void)((-lim <= n && n < lim) || luaL_argerror(L, (arg), ("integer overflow"))));
        }
        packint(&b, (lua_Unsigned)n, h.islittle, size, (n < 0));
        break;
      }
      case Kuint: {
        lua_Integer n = luaL_checkinteger(L, arg);
        if (size < ((int)sizeof(lua_Integer)))
          ((void)(((lua_Unsigned)n < ((lua_Unsigned)1 << (size * 8
         ))) || luaL_argerror(L, (arg), ("unsigned overflow"))))
                                                    ;
        packint(&b, (lua_Unsigned)n, h.islittle, size, 0);
        break;
      }
      case Kfloat: {
        float f = (float)luaL_checknumber(L, arg);
        char *buff = luaL_prepbuffsize(&b, sizeof(f));

        copywithendian(buff, (char *)&f, sizeof(f), h.islittle);
        ((&b)->n += (size));
        break;
      }
      case Knumber: {
        lua_Number f = luaL_checknumber(L, arg);
        char *buff = luaL_prepbuffsize(&b, sizeof(f));

        copywithendian(buff, (char *)&f, sizeof(f), h.islittle);
        ((&b)->n += (size));
        break;
      }
      case Kdouble: {
        double f = (double)luaL_checknumber(L, arg);
        char *buff = luaL_prepbuffsize(&b, sizeof(f));

        copywithendian(buff, (char *)&f, sizeof(f), h.islittle);
        ((&b)->n += (size));
        break;
      }
      case Kchar: {
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        ((void)((len <= (size_t)size) || luaL_argerror(L, (arg), ("string longer than given size"))))
                                                         ;
        luaL_addlstring(&b, s, len);
        while (len++ < (size_t)size)
          ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (0x00)));
        break;
      }
      case Kstring: {
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        ((void)((size >= (int)sizeof(size_t) || len < ((size_t)1 << (size * 8
       ))) || luaL_argerror(L, (arg), ("string length does not fit in given size"))))

                                                                         ;
        packint(&b, (lua_Unsigned)len, h.islittle, size, 0);
        luaL_addlstring(&b, s, len);
        totalsize += len;
        break;
      }
      case Kzstr: {
        size_t len;
        const char *s = luaL_checklstring(L, arg, &len);
        ((void)((strlen(s) == len) || luaL_argerror(L, (arg), ("string contains zeros"))));
        luaL_addlstring(&b, s, len);
        ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = ('\0')));
        totalsize += len + 1;
        break;
      }
      case Kpadding: ((void)((&b)->n < (&b)->size || luaL_prepbuffsize((&b), 1)), ((&b)->b[(&b)->n++] = (0x00)));
      case Kpaddalign: case Knop:
        arg--;
        break;
    }
  }
  luaL_pushresult(&b);
  return 1;
}


static int str_packsize (lua_State *L) {
  Header h;
  const char *fmt = (luaL_checklstring(L, (1), 
                   ((void *)0)
                   ));
  size_t totalsize = 0;
  initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    KOption opt = getdetails(&h, totalsize, &fmt, &size, &ntoalign);
    ((void)((opt != Kstring && opt != Kzstr) || luaL_argerror(L, (1), ("variable-length format"))))
                                              ;
    size += ntoalign;
    ((void)((totalsize <= (sizeof(size_t) < sizeof(int) ? ((size_t)(~(size_t)0)) : (size_t)(0x7fffffff
   )) - size) || luaL_argerror(L, (1), ("format result too large"))))
                                               ;
    totalsize += size;
  }
  lua_pushinteger(L, (lua_Integer)totalsize);
  return 1;
}
static lua_Integer unpackint (lua_State *L, const char *str,
                              int islittle, int size, int issigned) {
  lua_Unsigned res = 0;
  int i;
  int limit = (size <= ((int)sizeof(lua_Integer))) ? size : ((int)sizeof(lua_Integer));
  for (i = limit - 1; i >= 0; i--) {
    res <<= 8
             ;
    res |= (lua_Unsigned)(unsigned char)str[islittle ? i : size - 1 - i];
  }
  if (size < ((int)sizeof(lua_Integer))) {
    if (issigned) {
      lua_Unsigned mask = (lua_Unsigned)1 << (size*8 
                                                     - 1);
      res = ((res ^ mask) - mask);
    }
  }
  else if (size > ((int)sizeof(lua_Integer))) {
    int mask = (!issigned || (lua_Integer)res >= 0) ? 0 : ((1 << 8
                                                         ) - 1);
    for (i = limit; i < size; i++) {
      if (((unsigned char)str[islittle ? i : size - 1 - i] != mask))
        luaL_error(L, "%d-byte integer does not fit into Lua Integer", size);
    }
  }
  return (lua_Integer)res;
}


static int str_unpack (lua_State *L) {
  Header h;
  const char *fmt = (luaL_checklstring(L, (1), 
                   ((void *)0)
                   ));
  size_t ld;
  const char *data = luaL_checklstring(L, 2, &ld);
  size_t pos = posrelatI(luaL_optinteger(L, 3, 1), ld) - 1;
  int n = 0;
  ((void)((pos <= ld) || luaL_argerror(L, (3), ("initial position out of string"))));
  initheader(L, &h);
  while (*fmt != '\0') {
    int size, ntoalign;
    KOption opt = getdetails(&h, pos, &fmt, &size, &ntoalign);
    ((void)(((size_t)ntoalign + size <= ld - pos) || luaL_argerror(L, (2), ("data string too short"))))
                                            ;
    pos += ntoalign;

    luaL_checkstack(L, 2, "too many results");
    n++;
    switch (opt) {
      case Kint:
      case Kuint: {
        lua_Integer res = unpackint(L, data + pos, h.islittle, size,
                                       (opt == Kint));
        lua_pushinteger(L, res);
        break;
      }
      case Kfloat: {
        float f;
        copywithendian((char *)&f, data + pos, sizeof(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        break;
      }
      case Knumber: {
        lua_Number f;
        copywithendian((char *)&f, data + pos, sizeof(f), h.islittle);
        lua_pushnumber(L, f);
        break;
      }
      case Kdouble: {
        double f;
        copywithendian((char *)&f, data + pos, sizeof(f), h.islittle);
        lua_pushnumber(L, (lua_Number)f);
        break;
      }
      case Kchar: {
        lua_pushlstring(L, data + pos, size);
        break;
      }
      case Kstring: {
        size_t len = (size_t)unpackint(L, data + pos, h.islittle, size, 0);
        ((void)((len <= ld - pos - size) || luaL_argerror(L, (2), ("data string too short"))));
        lua_pushlstring(L, data + pos + size, len);
        pos += len;
        break;
      }
      case Kzstr: {
        size_t len = strlen(data + pos);
        ((void)((pos + len < ld) || luaL_argerror(L, (2), ("unfinished string for format 'z'"))))
                                                            ;
        lua_pushlstring(L, data + pos, len);
        pos += len + 1;
        break;
      }
      case Kpaddalign: case Kpadding: case Knop:
        n--;
        break;
    }
    pos += size;
  }
  lua_pushinteger(L, pos + 1);
  return n + 1;
}




static const luaL_Reg strlib[] = {
  {"byte", str_byte},
  {"char", str_char},
  {"dump", str_dump},
  {"find", str_find},
  {"format", str_format},
  {"gmatch", gmatch},
  {"gsub", str_gsub},
  {"len", str_len},
  {"lower", str_lower},
  {"match", str_match},
  {"rep", str_rep},
  {"reverse", str_reverse},
  {"sub", str_sub},
  {"upper", str_upper},
  {"pack", str_pack},
  {"packsize", str_packsize},
  {"unpack", str_unpack},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


static void createmetatable (lua_State *L) {

  lua_createtable(L, 0, sizeof(stringmetamethods)/sizeof((stringmetamethods)[0]) - 1);
  luaL_setfuncs(L, stringmetamethods, 0);
  lua_pushstring(L, "" "");
  lua_pushvalue(L, -2);
  lua_setmetatable(L, -2);
  lua_settop(L, -(1)-1);
  lua_pushvalue(L, -2);
  lua_setfield(L, -2, "__index");
  lua_settop(L, -(1)-1);
}





extern int luaopen_string (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(strlib)/sizeof((strlib)[0]) - 1), luaL_setfuncs(L,strlib,0));
  createmetatable(L);
  return 1;
}
static int checkfield (lua_State *L, const char *key, int n) {
  lua_pushstring(L, key);
  return (lua_rawget(L, -n) != 0);
}






static void checktab (lua_State *L, int arg, int what) {
  if (lua_type(L, arg) != 5) {
    int n = 1;
    if (lua_getmetatable(L, arg) &&
        (!(what & 1) || checkfield(L, "__index", ++n)) &&
        (!(what & 2) || checkfield(L, "__newindex", ++n)) &&
        (!(what & 4) || checkfield(L, "__len", ++n))) {
      lua_settop(L, -(n)-1);
    }
    else
      luaL_checktype(L, arg, 5);
  }
}


static int tinsert (lua_State *L) {
  lua_Integer pos;
  lua_Integer e = (checktab(L, 1, ((1 | 2)) | 4), luaL_len(L, 1));
  e = ((lua_Integer)((lua_Unsigned)(e) + (lua_Unsigned)(1)));
  switch (lua_gettop(L)) {
    case 2: {
      pos = e;
      break;
    }
    case 3: {
      lua_Integer i;
      pos = luaL_checkinteger(L, 2);

      ((void)(((lua_Unsigned)pos - 1u < (lua_Unsigned)e) || luaL_argerror(L, (2), ("position out of bounds"))))
                                                ;
      for (i = e; i > pos; i--) {
        lua_geti(L, 1, i - 1);
        lua_seti(L, 1, i);
      }
      break;
    }
    default: {
      return luaL_error(L, "wrong number of arguments to 'insert'");
    }
  }
  lua_seti(L, 1, pos);
  return 0;
}


static int tremove (lua_State *L) {
  lua_Integer size = (checktab(L, 1, ((1 | 2)) | 4), luaL_len(L, 1));
  lua_Integer pos = luaL_optinteger(L, 2, size);
  if (pos != size)

    ((void)(((lua_Unsigned)pos - 1u <= (lua_Unsigned)size) || luaL_argerror(L, (2), ("position out of bounds"))))
                                              ;
  lua_geti(L, 1, pos);
  for ( ; pos < size; pos++) {
    lua_geti(L, 1, pos + 1);
    lua_seti(L, 1, pos);
  }
  lua_pushnil(L);
  lua_seti(L, 1, pos);
  return 1;
}
static int tmove (lua_State *L) {
  lua_Integer f = luaL_checkinteger(L, 2);
  lua_Integer e = luaL_checkinteger(L, 3);
  lua_Integer t = luaL_checkinteger(L, 4);
  int tt = !(lua_type(L, (5)) <= 0) ? 5 : 1;
  checktab(L, 1, 1);
  checktab(L, tt, 2);
  if (e >= f) {
    lua_Integer n, i;
    ((void)((f > 0 || e < 0x7fffffffffffffffLL 
   + f) || luaL_argerror(L, (3), ("too many elements to move"))))
                                              ;
    n = e - f + 1;
    ((void)((t <= 0x7fffffffffffffffLL 
   - n + 1) || luaL_argerror(L, (4), ("destination wrap around"))))
                                            ;
    if (t > e || t <= f || (tt != 1 && !lua_compare(L, 1, tt, 0))) {
      for (i = 0; i < n; i++) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
    else {
      for (i = n - 1; i >= 0; i--) {
        lua_geti(L, 1, f + i);
        lua_seti(L, tt, t + i);
      }
    }
  }
  lua_pushvalue(L, tt);
  return 1;
}


static void addfield (lua_State *L, luaL_Buffer *b, lua_Integer i) {
  lua_geti(L, 1, i);
  if ((!lua_isstring(L, -1)))
    luaL_error(L, "invalid value (%s) at index %I in table for 'concat'",
                  lua_typename(L, lua_type(L,(-1))), (long long)i);
  luaL_addvalue(b);
}


static int tconcat (lua_State *L) {
  luaL_Buffer b;
  lua_Integer last = (checktab(L, 1, (1) | 4), luaL_len(L, 1));
  size_t lsep;
  const char *sep = luaL_optlstring(L, 2, "", &lsep);
  lua_Integer i = luaL_optinteger(L, 3, 1);
  last = luaL_optinteger(L, 4, last);
  luaL_buffinit(L, &b);
  for (; i < last; i++) {
    addfield(L, &b, i);
    luaL_addlstring(&b, sep, lsep);
  }
  if (i == last)
    addfield(L, &b, i);
  luaL_pushresult(&b);
  return 1;
}
static int tpack (lua_State *L) {
  int i;
  int n = lua_gettop(L);
  lua_createtable(L, n, 1);
  lua_rotate(L, (1), 1);
  for (i = n; i >= 1; i--)
    lua_seti(L, 1, i);
  lua_pushinteger(L, n);
  lua_setfield(L, 1, "n");
  return 1;
}


static int tunpack (lua_State *L) {
  lua_Unsigned n;
  lua_Integer i = luaL_optinteger(L, 2, 1);
  lua_Integer e = ((lua_type(L, ((3))) <= 0) ? (luaL_len(L, 1)) : luaL_checkinteger(L,(3)));
  if (i > e) return 0;
  n = (lua_Unsigned)e - i;
  if ((n >= (unsigned int)0x7fffffff 
     || !lua_checkstack(L, (int)(++n)))
                                                )
    return luaL_error(L, "too many results to unpack");
  for (; i < e; i++) {
    lua_geti(L, 1, i);
  }
  lua_geti(L, 1, e);
  return (int)n;
}
typedef unsigned int IdxT;
static unsigned int l_randomizePivot (void) {
  clock_t c = clock();
  time_t t = time(
                 ((void *)0)
                     );
  unsigned int buff[(sizeof(c) / sizeof(unsigned int)) + (sizeof(t) / sizeof(unsigned int))];
  unsigned int i, rnd = 0;
  memcpy(buff, &c, (sizeof(c) / sizeof(unsigned int)) * sizeof(unsigned int));
  memcpy(buff + (sizeof(c) / sizeof(unsigned int)), &t, (sizeof(t) / sizeof(unsigned int)) * sizeof(unsigned int));
  for (i = 0; i < (sizeof(buff) / sizeof(unsigned int)); i++)
    rnd += buff[i];
  return rnd;
}
static void set2 (lua_State *L, IdxT i, IdxT j) {
  lua_seti(L, 1, i);
  lua_seti(L, 1, j);
}






static int sort_comp (lua_State *L, int a, int b) {
  if ((lua_type(L, (2)) == 0))
    return lua_compare(L, a, b, 1);
  else {
    int res;
    lua_pushvalue(L, 2);
    lua_pushvalue(L, a-1);
    lua_pushvalue(L, b-2);
    lua_callk(L, (2), (1), 0, 
   ((void *)0)
   );
    res = lua_toboolean(L, -1);
    lua_settop(L, -(1)-1);
    return res;
  }
}
static IdxT partition (lua_State *L, IdxT lo, IdxT up) {
  IdxT i = lo;
  IdxT j = up - 1;

  for (;;) {

    while ((void)lua_geti(L, 1, ++i), sort_comp(L, -1, -2)) {
      if ((i == up - 1))
        luaL_error(L, "invalid order function for sorting");
      lua_settop(L, -(1)-1);
    }


    while ((void)lua_geti(L, 1, --j), sort_comp(L, -3, -1)) {
      if ((j < i))
        luaL_error(L, "invalid order function for sorting");
      lua_settop(L, -(1)-1);
    }

    if (j < i) {

      lua_settop(L, -(1)-1);

      set2(L, up - 1, i);
      return i;
    }

    set2(L, i, j);
  }
}






static IdxT choosePivot (IdxT lo, IdxT up, unsigned int rnd) {
  IdxT r4 = (up - lo) / 4;
  IdxT p = rnd % (r4 * 2) + (lo + r4);
  ((void)0);
  return p;
}





static void auxsort (lua_State *L, IdxT lo, IdxT up,
                                   unsigned int rnd) {
  while (lo < up) {
    IdxT p;
    IdxT n;

    lua_geti(L, 1, lo);
    lua_geti(L, 1, up);
    if (sort_comp(L, -1, -2))
      set2(L, lo, up);
    else
      lua_settop(L, -(2)-1);
    if (up - lo == 1)
      return;
    if (up - lo < 100u || rnd == 0)
      p = (lo + up)/2;
    else
      p = choosePivot(lo, up, rnd);
    lua_geti(L, 1, p);
    lua_geti(L, 1, lo);
    if (sort_comp(L, -2, -1))
      set2(L, p, lo);
    else {
      lua_settop(L, -(1)-1);
      lua_geti(L, 1, up);
      if (sort_comp(L, -1, -2))
        set2(L, p, up);
      else
        lua_settop(L, -(2)-1);
    }
    if (up - lo == 2)
      return;
    lua_geti(L, 1, p);
    lua_pushvalue(L, -1);
    lua_geti(L, 1, up - 1);
    set2(L, p, up - 1);
    p = partition(L, lo, up);

    if (p - lo < up - p) {
      auxsort(L, lo, p - 1, rnd);
      n = p - lo;
      lo = p + 1;
    }
    else {
      auxsort(L, p + 1, up, rnd);
      n = up - p;
      up = p - 1;
    }
    if ((up - lo) / 128 > n)
      rnd = l_randomizePivot();
  }
}


static int sort (lua_State *L) {
  lua_Integer n = (checktab(L, 1, ((1 | 2)) | 4), luaL_len(L, 1));
  if (n > 1) {
    ((void)((n < 0x7fffffff
   ) || luaL_argerror(L, (1), ("array too big"))));
    if (!(lua_type(L, (2)) <= 0))
      luaL_checktype(L, 2, 6);
    lua_settop(L, 2);
    auxsort(L, 1, (IdxT)n, 0);
  }
  return 0;
}




static const luaL_Reg tab_funcs[] = {
  {"concat", tconcat},
  {"insert", tinsert},
  {"pack", tpack},
  {"unpack", tunpack},
  {"remove", tremove},
  {"move", tmove},
  {"sort", sort},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


extern int luaopen_table (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(tab_funcs)/sizeof((tab_funcs)[0]) - 1), luaL_setfuncs(L,tab_funcs,0));
  return 1;
}




extern void __assert_fail (const char *__assertion, const char *__file,
      unsigned int __line, const char *__function)
     ;


extern void __assert_perror_fail (int __errnum, const char *__file,
      unsigned int __line, const char *__function)
     ;




extern void __assert (const char *__assertion, const char *__file, int __line)
     ;




typedef unsigned int utfint;
static lua_Integer u_posrelat (lua_Integer pos, size_t len) {
  if (pos >= 0) return pos;
  else if (0u - (size_t)pos > len) return 0;
  else return (lua_Integer)len + pos + 1;
}
static const char *utf8_decode (const char *s, utfint *val, int strict) {
  static const utfint limits[] =
        {~(utfint)0, 0x80, 0x800, 0x10000u, 0x200000u, 0x4000000u};
  unsigned int c = (unsigned char)s[0];
  utfint res = 0;
  if (c < 0x80)
    res = c;
  else {
    int count = 0;
    for (; c & 0x40; c <<= 1) {
      unsigned int cc = (unsigned char)s[++count];
      if (!(((cc) & 0xC0) == 0x80))
        return 
              ((void *)0)
                  ;
      res = (res << 6) | (cc & 0x3F);
    }
    res |= ((utfint)(c & 0x7F) << (count * 5));
    if (count > 5 || res > 0x7FFFFFFFu || res < limits[count])
      return 
            ((void *)0)
                ;
    s += count;
  }
  if (strict) {

    if (res > 0x10FFFFu || (0xD800u <= res && res <= 0xDFFFu))
      return 
            ((void *)0)
                ;
  }
  if (val) *val = res;
  return s + 1;
}







static int utflen (lua_State *L) {
  lua_Integer n = 0;
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer posj = u_posrelat(luaL_optinteger(L, 3, -1), len);
  int lax = lua_toboolean(L, 4);
  ((void)((1 <= posi && --posi <= (lua_Integer)len) || luaL_argerror(L, (2), ("initial position out of bounds"))))
                                                    ;
  ((void)((--posj < (lua_Integer)len) || luaL_argerror(L, (3), ("final position out of bounds"))))
                                                  ;
  while (posi <= posj) {
    const char *s1 = utf8_decode(s + posi, 
                                          ((void *)0)
                                              , !lax);
    if (s1 == 
             ((void *)0)
                 ) {
      lua_pushnil(L);
      lua_pushinteger(L, posi + 1);
      return 2;
    }
    posi = s1 - s;
    n++;
  }
  lua_pushinteger(L, n);
  return 1;
}






static int codepoint (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer posi = u_posrelat(luaL_optinteger(L, 2, 1), len);
  lua_Integer pose = u_posrelat(luaL_optinteger(L, 3, posi), len);
  int lax = lua_toboolean(L, 4);
  int n;
  const char *se;
  ((void)((posi >= 1) || luaL_argerror(L, (2), ("out of bounds"))));
  ((void)((pose <= (lua_Integer)len) || luaL_argerror(L, (3), ("out of bounds"))));
  if (posi > pose) return 0;
  if (pose - posi >= 0x7fffffff
                           )
    return luaL_error(L, "string slice too long");
  n = (int)(pose - posi) + 1;
  luaL_checkstack(L, n, "string slice too long");
  n = 0;
  se = s + pose;
  for (s += posi - 1; s < se;) {
    utfint code;
    s = utf8_decode(s, &code, !lax);
    if (s == 
            ((void *)0)
                )
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, code);
    n++;
  }
  return n;
}


static void pushutfchar (lua_State *L, int arg) {
  lua_Unsigned code = (lua_Unsigned)luaL_checkinteger(L, arg);
  ((void)((code <= 0x7FFFFFFFu) || luaL_argerror(L, (arg), ("value out of range"))));
  lua_pushfstring(L, "%U", (long)code);
}





static int utfchar (lua_State *L) {
  int n = lua_gettop(L);
  if (n == 1)
    pushutfchar(L, 1);
  else {
    int i;
    luaL_Buffer b;
    luaL_buffinit(L, &b);
    for (i = 1; i <= n; i++) {
      pushutfchar(L, i);
      luaL_addvalue(&b);
    }
    luaL_pushresult(&b);
  }
  return 1;
}






static int byteoffset (lua_State *L) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Integer n = luaL_checkinteger(L, 2);
  lua_Integer posi = (n >= 0) ? 1 : len + 1;
  posi = u_posrelat(luaL_optinteger(L, 3, posi), len);
  ((void)((1 <= posi && --posi <= (lua_Integer)len) || luaL_argerror(L, (3), ("position out of bounds"))))
                                            ;
  if (n == 0) {

    while (posi > 0 && (((*(s + posi)) & 0xC0) == 0x80)) posi--;
  }
  else {
    if ((((*(s + posi)) & 0xC0) == 0x80))
      return luaL_error(L, "initial position is a continuation byte");
    if (n < 0) {
       while (n < 0 && posi > 0) {
         do {
           posi--;
         } while (posi > 0 && (((*(s + posi)) & 0xC0) == 0x80));
         n++;
       }
     }
     else {
       n--;
       while (n > 0 && posi < (lua_Integer)len) {
         do {
           posi++;
         } while ((((*(s + posi)) & 0xC0) == 0x80));
         n--;
       }
     }
  }
  if (n == 0)
    lua_pushinteger(L, posi + 1);
  else
    lua_pushnil(L);
  return 1;
}


static int iter_aux (lua_State *L, int strict) {
  size_t len;
  const char *s = luaL_checklstring(L, 1, &len);
  lua_Unsigned n = (lua_Unsigned)lua_tointegerx(L,(2),
                                ((void *)0)
                                );
  if (n < len) {
    while ((((*(s + n)) & 0xC0) == 0x80)) n++;
  }
  if (n >= len)
    return 0;
  else {
    utfint code;
    const char *next = utf8_decode(s + n, &code, strict);
    if (next == 
               ((void *)0) 
                    || (((*(next)) & 0xC0) == 0x80))
      return luaL_error(L, "invalid UTF-8 code");
    lua_pushinteger(L, n + 1);
    lua_pushinteger(L, code);
    return 2;
  }
}


static int iter_auxstrict (lua_State *L) {
  return iter_aux(L, 1);
}

static int iter_auxlax (lua_State *L) {
  return iter_aux(L, 0);
}


static int iter_codes (lua_State *L) {
  int lax = lua_toboolean(L, 2);
  const char *s = (luaL_checklstring(L, (1), 
                 ((void *)0)
                 ));
  ((void)((!(((*(s)) & 0xC0) == 0x80)) || luaL_argerror(L, (1), ("invalid UTF-8 code"))));
  lua_pushcclosure(L, (lax ? iter_auxlax : iter_auxstrict), 0);
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 0);
  return 3;
}






static const luaL_Reg funcs[] = {
  {"offset", byteoffset},
  {"codepoint", codepoint},
  {"char", utfchar},
  {"len", utflen},
  {"codes", iter_codes},

  {"charpattern", 
                 ((void *)0)
                     },
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


extern int luaopen_utf8 (lua_State *L) {
  (luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number))), lua_createtable(L, 0, sizeof(funcs)/sizeof((funcs)[0]) - 1), luaL_setfuncs(L,funcs,0));
  lua_pushlstring(L, "[\0-\x7F\xC2-\xFD][\x80-\xBF]*", sizeof("[\0-\x7F\xC2-\xFD][\x80-\xBF]*")/sizeof(char) - 1);
  lua_setfield(L, -2, "charpattern");
  return 1;
}
static const luaL_Reg loadedlibs[] = {
  {"_G", luaopen_base},
  {"package", luaopen_package},
  {"coroutine", luaopen_coroutine},
  {"table", luaopen_table},
  {"io", luaopen_io},
  {"os", luaopen_os},
  {"string", luaopen_string},
  {"math", luaopen_math},
  {"utf8", luaopen_utf8},
  {"debug", luaopen_debug},
  {
  ((void *)0)
      , 
        ((void *)0)
            }
};


extern void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib;

  for (lib = loadedlibs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_settop(L, -(1)-1);
  }
}


static lua_State *globalL = 
                           ((void *)0)
                               ;

static const char *progname = "lua";







static void setsignal (int sig, void (*handler)(int)) {
  struct sigaction sa;
  sa.
    __sigaction_handler.sa_handler 
               = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  sigaction(sig, &sa, 
                     ((void *)0)
                         );
}
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;
  lua_sethook(L, 
                ((void *)0)
                    , 0, 0);
  luaL_error(L, "interrupted!");
}
static void laction (int i) {
  int flag = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
  setsignal(i, 
              ((__sighandler_t) 0)
                     );
  lua_sethook(globalL, lstop, flag, 1);
}


static void print_usage (const char *badoption) {
  (fprintf(
 stderr
 , ("%s: "), (progname)), fflush(
 stderr
 ));
  if (badoption[1] == 'e' || badoption[1] == 'l')
    (fprintf(
   stderr
   , ("'%s' needs argument\n"), (badoption)), fflush(
   stderr
   ));
  else
    (fprintf(
   stderr
   , ("unrecognized option '%s'\n"), (badoption)), fflush(
   stderr
   ));
  (fprintf(
 stderr
 , ("usage: %s [options] [script [args]]\n" "Available options are:\n" "  -e stat   execute string 'stat'\n" "  -i        enter interactive mode after executing 'script'\n" "  -l mod    require library 'mod' into global 'mod'\n" "  -l g=mod  require library 'mod' into global 'g'\n" "  -v        show version information\n" "  -E        ignore environment variables\n" "  -W        turn warnings on\n" "  --        stop handling options\n" "  -         stop handling options and execute stdin\n"), (progname)), fflush(
 stderr
 ))
           ;
}






static void l_message (const char *pname, const char *msg) {
  if (pname) (fprintf(
            stderr
            , ("%s: "), (pname)), fflush(
            stderr
            ));
  (fprintf(
 stderr
 , ("%s\n"), (msg)), fflush(
 stderr
 ));
}







static int report (lua_State *L, int status) {
  if (status != 0) {
    const char *msg = lua_tolstring(L, (-1), 
                     ((void *)0)
                     );
    l_message(progname, msg);
    lua_settop(L, -(1)-1);
  }
  return status;
}





static int msghandler (lua_State *L) {
  const char *msg = lua_tolstring(L, (1), 
                   ((void *)0)
                   );
  if (msg == 
            ((void *)0)
                ) {
    if (luaL_callmeta(L, 1, "__tostring") &&
        lua_type(L, -1) == 4)
      return 1;
    else
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               lua_typename(L, lua_type(L,(1))));
  }
  luaL_traceback(L, L, msg, 1);
  return 1;
}






static int docall (lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;
  lua_pushcclosure(L, (msghandler), 0);
  lua_rotate(L, (base), 1);
  globalL = L;
  setsignal(
           2
                 , laction);
  status = lua_pcallk(L, (narg), (nres), (base), 0, 
          ((void *)0)
          );
  setsignal(
           2
                 , 
                   ((__sighandler_t) 0)
                          );
  (lua_rotate(L, (base), -1), lua_settop(L, -(1)-1));
  return status;
}


static void print_version (void) {
  fwrite(("Lua " "5" "." "4" "." "6" "  Copyright (C) 1994-2023 Lua.org, PUC-Rio"), sizeof(char), (strlen("Lua " "5" "." "4" "." "6" "  Copyright (C) 1994-2023 Lua.org, PUC-Rio")), 
 stdout
 );
  (fwrite(("\n"), sizeof(char), (1), 
 stdout
 ), fflush(
 stdout
 ));
}
static void createargtable (lua_State *L, char **argv, int argc, int script) {
  int i, narg;
  narg = argc - (script + 1);
  lua_createtable(L, narg, script + 1);
  for (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static int dochunk (lua_State *L, int status) {
  if (status == 0) status = docall(L, 0, 0);
  return report(L, status);
}


static int dofile (lua_State *L, const char *name) {
  return dochunk(L, luaL_loadfilex(L,name,
                   ((void *)0)
                   ));
}


static int dostring (lua_State *L, const char *s, const char *name) {
  return dochunk(L, luaL_loadbufferx(L,s,strlen(s),name,
                   ((void *)0)
                   ));
}





static int dolibrary (lua_State *L, char *globname) {
  int status;
  char *modname = strchr(globname, '=');
  if (modname == 
                ((void *)0)
                    )
    modname = globname;
  else {
    *modname = '\0';
    modname++;
  }
  lua_getglobal(L, "require");
  lua_pushstring(L, modname);
  status = docall(L, 1, 1);
  if (status == 0)
    lua_setglobal(L, globname);
  return report(L, status);
}





static int pushargs (lua_State *L) {
  int i, n;
  if (lua_getglobal(L, "arg") != 5)
    luaL_error(L, "'arg' is not a table");
  n = (int)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  for (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  (lua_rotate(L, (-i), -1), lua_settop(L, -(1)-1));
  return n;
}


static int handle_script (lua_State *L, char **argv) {
  int status;
  const char *fname = argv[0];
  if (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = 
           ((void *)0)
               ;
  status = luaL_loadfilex(L,fname,
          ((void *)0)
          );
  if (status == 0) {
    int n = pushargs(L);
    status = docall(L, n, (-1));
  }
  return report(L, status);
}
static int collectargs (char **argv, int *first) {
  int args = 0;
  int i;
  if (argv[0] != 
                ((void *)0)
                    ) {
    if (argv[0][0])
      progname = argv[0];
  }
  else {
    *first = -1;
    return 0;
  }
  for (i = 1; argv[i] != 
                        ((void *)0)
                            ; i++) {
    *first = i;
    if (argv[i][0] != '-')
        return args;
    switch (argv[i][1]) {
      case '-':
        if (argv[i][2] != '\0')
          return 1;
        *first = i + 1;
        return args;
      case '\0':
        return args;
      case 'E':
        if (argv[i][2] != '\0')
          return 1;
        args |= 16;
        break;
      case 'W':
        if (argv[i][2] != '\0')
          return 1;
        break;
      case 'i':
        args |= 2;
      case 'v':
        if (argv[i][2] != '\0')
          return 1;
        args |= 4;
        break;
      case 'e':
        args |= 8;
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == 
                        ((void *)0) 
                             || argv[i][0] == '-')
            return 1;
        }
        break;
      default:
        return 1;
    }
  }
  *first = 0;
  return args;
}







static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    int option = argv[i][1];
    ((void)0);
    switch (option) {
      case 'e': case 'l': {
        int status;
        char *extra = argv[i] + 2;
        if (*extra == '\0') extra = argv[++i];
        ((void)0);
        status = (option == 'e')
                 ? dostring(L, extra, "=(command line)")
                 : dolibrary(L, extra);
        if (status != 0) return 0;
        break;
      }
      case 'W':
        lua_warning(L, "@on", 0);
        break;
    }
  }
  return 1;
}


static int handle_luainit (lua_State *L) {
  const char *name = "=" "LUA_INIT" "_" "5" "_" "4";
  const char *init = getenv(name + 1);
  if (init == 
             ((void *)0)
                 ) {
    name = "=" "LUA_INIT";
    init = getenv(name + 1);
  }
  if (init == 
             ((void *)0)
                 ) return 0;
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, name);
}
static const char *get_prompt (lua_State *L, int firstline) {
  if (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == 0)
    return (firstline ? "> " : ">> ");
  else {
    const char *p = luaL_tolstring(L, -1, 
                                         ((void *)0)
                                             );
    (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
    return p;
  }
}
static int incomplete (lua_State *L, int status) {
  if (status == 3) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    if (lmsg >= (sizeof("<eof>")/sizeof(char) - 1) && strcmp(msg + lmsg - (sizeof("<eof>")/sizeof(char) - 1), "<eof>") == 0) {
      lua_settop(L, -(1)-1);
      return 1;
    }
  }
  return 0;
}





static int pushline (lua_State *L, int firstline) {
  char buffer[512];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  int readstatus = ((void)L, fputs(prmt, 
                  stdout
                  ), fflush(
                  stdout
                  ), fgets(b, 512, 
                  stdin
                  ) != 
                  ((void *)0)
                  );
  if (readstatus == 0)
    return 0;
  lua_settop(L, -(1)-1);
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')
    b[--l] = '\0';
  if (firstline && b[0] == '=')
    lua_pushfstring(L, "return %s", b + 1);
  else
    lua_pushlstring(L, b, l);
  { (void)L; (void)b; };
  return 1;
}






static int addreturn (lua_State *L) {
  const char *line = lua_tolstring(L, (-1), 
                    ((void *)0)
                    );
  const char *retline = lua_pushfstring(L, "return %s;", line);
  int status = luaL_loadbufferx(L,retline,strlen(retline),"=stdin",
              ((void *)0)
              );
  if (status == 0) {
    (lua_rotate(L, (-2), -1), lua_settop(L, -(1)-1));
    if (line[0] != '\0')
      { (void)L; (void)line; };
  }
  else
    lua_settop(L, -(2)-1);
  return status;
}





static int multiline (lua_State *L) {
  for (;;) {
    size_t len;
    const char *line = lua_tolstring(L, 1, &len);
    int status = luaL_loadbufferx(L,line,len,"=stdin",
                ((void *)0)
                );
    if (!incomplete(L, status) || !pushline(L, 0)) {
      { (void)L; (void)line; };
      return status;
    }
    lua_pushstring(L, "" "\n");
    lua_rotate(L, (-2), 1);
    lua_concat(L, 3);
  }
}
static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;
  if ((status = addreturn(L)) != 0)
    status = multiline(L);
  (lua_rotate(L, (1), -1), lua_settop(L, -(1)-1));
  ((void)0);
  return status;
}





static void l_print (lua_State *L) {
  int n = lua_gettop(L);
  if (n > 0) {
    luaL_checkstack(L, 20, "too many results to print");
    lua_getglobal(L, "print");
    lua_rotate(L, (1), 1);
    if (lua_pcallk(L, (n), (0), (0), 0, 
       ((void *)0)
       ) != 0)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tolstring(L, (-1), 
                                            ((void *)0)
                                            )));
  }
}






static void doREPL (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = 
            ((void *)0)
                ;
  ((void)L);
  while ((status = loadline(L)) != -1) {
    if (status == 0)
      status = docall(L, 0, (-1));
    if (status == 0) l_print(L);
    else report(L, status);
  }
  lua_settop(L, 0);
  (fwrite(("\n"), sizeof(char), (1), 
 stdout
 ), fflush(
 stdout
 ));
  progname = oldprogname;
}
static int pmain (lua_State *L) {
  int argc = (int)lua_tointegerx(L,(1),
                 ((void *)0)
                 );
  char **argv = (char **)lua_touserdata(L, 2);
  int script;
  int args = collectargs(argv, &script);
  int optlim = (script > 0) ? script : argc;
  luaL_checkversion_(L, 504, (sizeof(lua_Integer)*16 + sizeof(lua_Number)));
  if (args == 1) {
    print_usage(argv[script]);
    return 0;
  }
  if (args & 4)
    print_version();
  if (args & 16) {
    lua_pushboolean(L, 1);
    lua_setfield(L, (-1000000 - 1000), "LUA_NOENV");
  }
  luaL_openlibs(L);
  createargtable(L, argv, argc, script);
  lua_gc(L, 1);
  lua_gc(L, 10, 0, 0);
  if (!(args & 16)) {
    if (handle_luainit(L) != 0)
      return 0;
  }
  if (!runargs(L, argv, optlim))
    return 0;
  if (script > 0) {
    if (handle_script(L, argv + script) != 0)
      return 0;
  }
  if (args & 2)
    doREPL(L);
  else if (script < 1 && !(args & (8 | 4))) {
    if (isatty(0)) {
      print_version();
      doREPL(L);
    }
    else dofile(L, 
                  ((void *)0)
                      );
  }
  lua_pushboolean(L, 1);
  return 1;
}


int main (int argc, char **argv) {
  int status, result;
  lua_State *L = luaL_newstate();
  if (L == 
          ((void *)0)
              ) {
    l_message(argv[0], "cannot create state: not enough memory");
    return 
          1
                      ;
  }
  lua_gc(L, 0);
  lua_pushcclosure(L, (&pmain), 0);
  lua_pushinteger(L, argc);
  lua_pushlightuserdata(L, argv);
  status = lua_pcallk(L, (2), (1), (0), 0, 
          ((void *)0)
          );
  result = lua_toboolean(L, -1);
  report(L, status);
  lua_close(L);
  return (result && status == 0) ? 
                                       0 
                                                    : 
                                                      1
                                                                  ;
}
