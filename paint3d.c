#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 1024
#define HEIGHT 768

typedef struct { double x, y, z; } Vec3;
typedef struct { int v0, v1, v2; } Tri;

inline Vec3 v_add(Vec3 a, Vec3 b) { return (Vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 v_sub(Vec3 a, Vec3 b) { return (Vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3 v_mul(Vec3 a, double d) { return (Vec3){a.x*d, a.y*d, a.z*d}; }
inline double v_dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 v_cross(Vec3 a, Vec3 b) { return (Vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
inline double v_len(Vec3 a) { return sqrt(v_dot(a, a)); }
inline Vec3 v_norm(Vec3 a) { double l = v_len(a); return l ? v_mul(a, 1.0/l) : (Vec3){0,0,0}; }

/* UI State */
Vec3 cam_pos = {0.0, 0.0, 3.0};
double yaw = 0.0;
double pitch = 0.0;
double zoom = 0.5;

Vec3 verts[20000];
Tri tris[40000];
int nv = 0, nt = 0;

int *framebuffer;
double *zbuffer;

/* Helper: Load OBJ */
int load_obj(const char *path) {
    FILE *f = fopen(path, "r");
    if(!f) return 0;
    char line[512];
    nv = nt = 0;
    while(fgets(line, 512, f)) {
        if(line[0]=='v' && line[1]==' ') {
            double x,y,z; sscanf(line+2, "%lf %lf %lf", &x, &y, &z);
            verts[nv++] = (Vec3){x,y,z};
        } else if(line[0]=='f' && line[1]==' ') {
            int v[10], vcount = 0;
            char *p = line+2;
            while(*p) {
                while(*p==' ') p++;
                if(!*p || *p=='\n') break;
                sscanf(p, "%d", &v[vcount++]);
                while(*p && *p!=' ') p++;
            }
            if(vcount>=3) {
                tris[nt++] = (Tri){v[0]-1, v[1]-1, v[2]-1};
                if(vcount>=4) tris[nt++] = (Tri){v[0]-1, v[2]-1, v[3]-1};
            }
        }
    }
    fclose(f);
    
    // Center it
    Vec3 c = {0,0,0};
    for(int i=0; i<nv; i++) c = v_add(c, verts[i]);
    c = v_mul(c, 1.0/nv);
    for(int i=0; i<nv; i++) verts[i] = v_sub(verts[i], c);
    return nt;
}

/* Projection and Rendering */
Vec3 project(Vec3 p) {
    // Basic rotation matrix using yaw and pitch
    double cy=cos(yaw), sy=sin(yaw);
    double cp=cos(pitch), sp=sin(pitch);
    
    // Rotate Y (yaw) -> object spins around its origin
    double x1 = p.x*cy + p.z*sy;
    double z1 = -p.x*sy + p.z*cy;
    double y1 = p.y;
    
    // Rotate X (pitch)
    double y2 = y1*cp - z1*sp;
    double z2 = y1*sp + z1*cp;
    double x2 = x1;
    
    // Scale and translate
    x2 *= zoom; y2 *= zoom; z2 *= zoom;
    z2 += cam_pos.z;
    
    // Perspective projection
    double fov = 500.0;
    if(z2 <= 0.0) z2 = 0.0001; // clip
    double px = (x2 / z2) * fov + WIDTH/2.0;
    double py = -(y2 / z2) * fov + HEIGHT/2.0;
    
    return (Vec3){px, py, z2};
}

void put_pixel(int x, int y, double z, int color) {
    if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT) return;
    int idx = y*WIDTH + x;
    if(z < zbuffer[idx]) {
        zbuffer[idx] = z;
        framebuffer[idx] = color;
    }
}

void draw_triangle(Vec3 p0, Vec3 p1, Vec3 p2, int color) {
    // Sort by y
    if(p0.y > p1.y) { Vec3 t=p0; p0=p1; p1=t; }
    if(p0.y > p2.y) { Vec3 t=p0; p0=p2; p2=t; }
    if(p1.y > p2.y) { Vec3 t=p1; p1=p2; p2=t; }
    
    int total_height = p2.y - p0.y;
    if(total_height == 0) return;
    
    for(int i=0; i<total_height; i++) {
        int second_half = i > p1.y - p0.y || p1.y == p0.y;
        int segment_height = second_half ? p2.y - p1.y : p1.y - p0.y;
        double alpha = (double)i / total_height;
        double beta  = (double)(i - (second_half ? p1.y - p0.y : 0)) / segment_height;
        
        Vec3 A = v_add(p0, v_mul(v_sub(p2, p0), alpha));
        Vec3 B = second_half ? v_add(p1, v_mul(v_sub(p2, p1), beta)) : v_add(p0, v_mul(v_sub(p1, p0), beta));
        if(A.x > B.x) { Vec3 t=A; A=B; B=t; }
        
        for(int j=A.x; j<=B.x; j++) {
            double phi = B.x==A.x ? 1.0 : (j-A.x)/(B.x-A.x);
            Vec3 P = v_add(A, v_mul(v_sub(B, A), phi));
            put_pixel(j, p0.y + i, P.z, color);
        }
    }
}

void render_frame() {
    for(int i=0; i<WIDTH*HEIGHT; i++) {
        framebuffer[i] = 0x1E1E2E; // Dark background
        zbuffer[i] = 1e9;
    }
    
    Vec3 light_dir = v_norm((Vec3){1,1,1});
    Vec3 rotated_verts[20000];
    for(int i=0; i<nv; i++) rotated_verts[i] = project(verts[i]);
    
    for(int i=0; i<nt; i++) {
        Vec3 v0 = verts[tris[i].v0];
        Vec3 v1 = verts[tris[i].v1];
        Vec3 v2 = verts[tris[i].v2];
        Vec3 n = v_norm(v_cross(v_sub(v1, v0), v_sub(v2, v0)));
        
        // Backface culling mapping to object space approx
        Vec3 cam_obj = {-cam_pos.x, -cam_pos.y, -cam_pos.z}; // crude approx
        // A better approach for backface culling in perspective:
        Vec3 p0 = rotated_verts[tris[i].v0];
        Vec3 p1 = rotated_verts[tris[i].v1];
        Vec3 p2 = rotated_verts[tris[i].v2];
        
        // Z-buffer will handle deep sorting, but cheap 2D cross product for culling:
        double area = (p1.x - p0.x)*(p2.y - p0.y) - (p1.y - p0.y)*(p2.x - p0.x);
        if (area <= 0) continue; // culled
        
        double intensity = v_dot(n, light_dir);
        if(intensity < 0.1) intensity = 0.1;

        int r = 200 * intensity;
        int g = 200 * intensity;
        int b = 250 * intensity;
        int color = (r<<16) | (g<<8) | b;
        
        draw_triangle(p0, p1, p2, color);
    }
}

int main(int argc, char** argv) {
    if(!load_obj(argc > 1 ? argv[1] : "teapot.obj")) {
        printf("Failed to load generic mesh! Exiting...\n"); return 1;
    }
    
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;
    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, WIDTH, HEIGHT, 1, 
                                   BlackPixel(d, s), WhitePixel(d, s));
    XSelectInput(d, w, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
    XMapWindow(d, w);
    
    framebuffer = malloc(WIDTH*HEIGHT*4);
    zbuffer = malloc(WIDTH*HEIGHT*8);
    
    XImage *img = XCreateImage(d, DefaultVisual(d, s), 24, ZPixmap, 0, (char*)framebuffer, WIDTH, HEIGHT, 32, 0);
    
    GC gc = XCreateGC(d, w, 0, NULL);
    
    XEvent e;
    int drags = 0;
    int last_x=0, last_y=0;
    
    while (1) {
        if (XPending(d)) {
            XNextEvent(d, &e);
            if (e.type == KeyPress) {
                KeySym key = XLookupKeysym(&e.xkey, 0);
                if (key == XK_Escape) break;
                if (key == XK_w) zoom *= 1.1;
                if (key == XK_s) zoom /= 1.1;
            }
            if (e.type == ButtonPress) { drags = 1; last_x = e.xbutton.x; last_y = e.xbutton.y; }
            if (e.type == ButtonRelease) { drags = 0; }
            if (e.type == MotionNotify && drags) {
                yaw += (e.xmotion.x - last_x) * 0.01;
                pitch += (e.xmotion.y - last_y) * 0.01;
                last_x = e.xmotion.x;
                last_y = e.xmotion.y;
            }
        }
        
        render_frame();
        XPutImage(d, w, gc, img, 0, 0, 0, 0, WIDTH, HEIGHT);
    }
    
    XDestroyImage(img); // frees framebuffer
    free(zbuffer);
    XCloseDisplay(d);
    return 0;
}
