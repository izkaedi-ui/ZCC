typedef unsigned long Window;
typedef unsigned long GC;
typedef unsigned long Colormap;
typedef unsigned long Cursor;
typedef unsigned long Pixmap;
typedef struct { int function; } XGCValues;
typedef struct _Display Display;
typedef struct _XImage { int width, height, bytes_per_line, depth; char* data; } XImage;
typedef struct _Visual { int x; } Visual;
typedef struct { unsigned long pixel; unsigned short red, green, blue; char flags; char pad; } XColor;
typedef struct { Visual *visual; int visualid; int screen; int depth; int class_field; unsigned long red_mask, green_mask, blue_mask; int colormap_size; int bits_per_rgb; } XVisualInfo;
#define class class_field
typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; int x, y; int width, height; int count; } XExposeEvent;
typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; unsigned int state; unsigned int keycode; } XKeyEvent;
typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; unsigned int state; unsigned int button; int x, y; } XButtonEvent;
typedef struct { int type; unsigned long serial; int send_event; Display *display; Window window; unsigned int state; int x, y; } XMotionEvent;
typedef union _XEvent { int type; XExposeEvent xexpose; XKeyEvent xkey; XButtonEvent xbutton; XMotionEvent xmotion; long pad[24]; } XEvent;
