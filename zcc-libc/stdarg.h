typedef void* va_list;
#define va_start(v,l) (v=(void*)((char*)&l + sizeof(l)))
#define va_end(v) (v=0)
#define va_arg(v, l) (*(l*)((v=(void*)((char*)v + sizeof(l))) - sizeof(l)))
