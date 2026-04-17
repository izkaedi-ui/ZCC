#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
int main() {
    Display *display = XOpenDisplay(NULL);
    if (!display) { fprintf(stderr, "Cannot open display\n"); return 1; }
    int screen = DefaultScreen(display);
    Window window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 800, 600, 1,
                                        BlackPixel(display, screen), WhitePixel(display, screen));
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    XEvent event;
    XNextEvent(display, &event);
    XCloseDisplay(display);
    return 0;
}