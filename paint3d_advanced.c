#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define WIDTH 1024
#define HEIGHT 768
#define MAX_OBJECTS 256

typedef struct { double x, y, z; } Vec3;
typedef struct { int v0, v1, v2; } Tri;

inline Vec3 v_add(Vec3 a, Vec3 b) { return (Vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
inline Vec3 v_sub(Vec3 a, Vec3 b) { return (Vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
inline Vec3 v_mul(Vec3 a, double d) { return (Vec3){a.x*d, a.y*d, a.z*d}; }
inline double v_dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 v_cross(Vec3 a, Vec3 b) { return (Vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
inline double v_len(Vec3 a) { return sqrt(v_dot(a, a)); }
inline Vec3 v_norm(Vec3 a) { double l = v_len(a); return l ? v_mul(a, 1.0/l) : (Vec3){0,0,0}; }

typedef struct {
    int v_start, v_count;
    int t_start, t_count;
} Mesh;

// Global Vertex/Tris Buffer
Vec3 g_verts[100000];
Tri g_tris[100000];
int g_nv = 0, g_nt = 0;

typedef struct {
    Mesh mesh;
    Vec3 pos;
    Vec3 rot; // euler
    double scale;
    int color;
    int type; // 0=teapot, 1=cube
} Object;

Object objects[MAX_OBJECTS];
int obj_count = 0;
int active_obj = -1;

Vec3 cam_pos = {0.0, 1.0, 4.0};
double cam_yaw = 0.0, cam_pitch = -0.2;

int *framebuffer;
double *zbuffer;

Mesh mesh_teapot;
Mesh mesh_cube;

void put_pixel(int x, int y, double z, int color) {
    if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT) return;
    int idx = y*WIDTH + x;
    if(z < zbuffer[idx]) {
        zbuffer[idx] = z;
        framebuffer[idx] = color;
    }
}

void draw_triangle(Vec3 p0, Vec3 p1, Vec3 p2, int color) {
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

void build_cube() {
    mesh_cube.v_start = g_nv;
    mesh_cube.t_start = g_nt;
    Vec3 verts[] = {
        {-1,-1,-1}, {1,-1,-1}, {1,1,-1}, {-1,1,-1},
        {-1,-1,1},  {1,-1,1},  {1,1,1},  {-1,1,1}
    };
    int tris[] = {
        0,1,2, 0,2,3,  // front
        1,5,6, 1,6,2,  // right
        5,4,7, 5,7,6,  // back
        4,0,3, 4,3,7,  // left
        3,2,6, 3,6,7,  // top
        4,5,1, 4,1,0   // bottom
    };
    for(int i=0;i<8;i++) g_verts[g_nv++] = verts[i];
    for(int i=0;i<12;i++) g_tris[g_nt++] = (Tri){mesh_cube.v_start+tris[i*3], mesh_cube.v_start+tris[i*3+1], mesh_cube.v_start+tris[i*3+2]};
    mesh_cube.v_count = 8;
    mesh_cube.t_count = 12;
}

void build_teapot(const char *path) {
    mesh_teapot.v_start = g_nv;
    mesh_teapot.t_start = g_nt;
    FILE *f = fopen(path, "r");
    if(!f) return;
    char line[512];
    int start_v = g_nv;
    while(fgets(line, 512, f)) {
        if(line[0]=='v' && line[1]==' ') {
            double x,y,z; sscanf(line+2, "%lf %lf %lf", &x, &y, &z);
            g_verts[g_nv++] = (Vec3){x,y,z};
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
                g_tris[g_nt++] = (Tri){start_v+v[0]-1, start_v+v[1]-1, start_v+v[2]-1};
                if(vcount>=4) g_tris[g_nt++] = (Tri){start_v+v[0]-1, start_v+v[2]-1, start_v+v[3]-1};
            }
        }
    }
    fclose(f);
    mesh_teapot.v_count = g_nv - mesh_teapot.v_start;
    mesh_teapot.t_count = g_nt - mesh_teapot.t_start;

    Vec3 c = {0,0,0};
    for(int i=0; i<mesh_teapot.v_count; i++) c = v_add(c, g_verts[start_v+i]);
    c = v_mul(c, 1.0/mesh_teapot.v_count);
    for(int i=0; i<mesh_teapot.v_count; i++) g_verts[start_v+i] = v_sub(g_verts[start_v+i], c);
}

Vec3 transform(Vec3 p, Object *obj) {
    p = v_mul(p, obj->scale);
    // rot Y
    double cy=cos(obj->rot.y), sy=sin(obj->rot.y);
    double x1 = p.x*cy + p.z*sy;
    double z1 = -p.x*sy + p.z*cy;
    // rot X
    double cx=cos(obj->rot.x), sx=sin(obj->rot.x);
    double y2 = p.y*cx - z1*sx;
    double z2 = p.y*sx + z1*cx;
    
    return v_add((Vec3){x1,y2,z2}, obj->pos);
}

Vec3 project(Vec3 p) {
    p = v_sub(p, cam_pos);
    double cy=cos(cam_yaw), sy=sin(cam_yaw);
    double cp=cos(cam_pitch), sp=sin(cam_pitch);
    
    double x1 = p.x*cy - p.z*sy;
    double z1 = p.x*sy + p.z*cy;
    
    double y2 = p.y*cp - z1*sp;
    double z2 = p.y*sp + z1*cp;
    
    double fov = 800.0;
    if(z2 <= 0.0) z2 = 0.0001; 
    double px = (x1 / z2) * fov + WIDTH/2.0;
    double py = -(y2 / z2) * fov + HEIGHT/2.0;
    return (Vec3){px, py, z2};
}

void draw_grid() {
    for(int i=-10; i<=10; i++) {
        for(double t=-10; t<=10; t+=0.1) {
            Vec3 p1 = project((Vec3){i, -1, t});
            Vec3 p2 = project((Vec3){t, -1, i});
            if(p1.z > 0 && p1.z < 20.0) put_pixel(p1.x, p1.y, p1.z, 0x444444);
            if(p2.z > 0 && p2.z < 20.0) put_pixel(p2.x, p2.y, p2.z, 0x444444);
        }
    }
}

void render_frame() {
    for(int i=0; i<WIDTH*HEIGHT; i++) {
        framebuffer[i] = 0x1A1A24; 
        zbuffer[i] = 1e9;
    }
    
    draw_grid();
    
    Vec3 light_dir = v_norm((Vec3){1.0, 2.0, -1.0});
    
    for(int o=0; o<obj_count; o++) {
        Object *obj = &objects[o];
        Mesh *m = &obj->mesh;
        
        Vec3 proj_verts[20000];
        Vec3 world_verts[20000];
        for(int i=0; i<m->v_count; i++) {
            world_verts[i] = transform(g_verts[m->v_start + i], obj);
            proj_verts[i]  = project(world_verts[i]);
        }
        
        for(int i=0; i<m->t_count; i++) {
            Tri t = g_tris[m->t_start + i];
            t.v0 -= m->v_start; t.v1 -= m->v_start; t.v2 -= m->v_start;
            
            Vec3 w0 = world_verts[t.v0], w1 = world_verts[t.v1], w2 = world_verts[t.v2];
            Vec3 n = v_norm(v_cross(v_sub(w1, w0), v_sub(w2, w0)));
            
            /* Camera vector for perspective backface culling */
            Vec3 view_dir = v_sub(w0, cam_pos);
            if(v_dot(view_dir, n) > 0.0) continue;
            
            double intensity = v_dot(n, light_dir);
            if(intensity < 0.1) intensity = 0.1;

            int rc = (obj->color >> 16) & 0xFF;
            int gc = (obj->color >> 8) & 0xFF;
            int bc = obj->color & 0xFF;

            if (o == active_obj) {
                rc = (rc + 255) / 2;
                gc = (gc + 255) / 2;
                bc = (bc + 255) / 2;
            }

            int r = rc * intensity;
            int g = gc * intensity;
            int b = bc * intensity;
            if(r>255)r=255; if(g>255)g=255; if(b>255)b=255;
            
            int color = (r<<16) | (g<<8) | b;
            
            draw_triangle(proj_verts[t.v0], proj_verts[t.v1], proj_verts[t.v2], color);
        }
    }
}

void spawn_obj(int type) {
    if(obj_count >= MAX_OBJECTS) return;
    Object obj = {0};
    obj.type = type;
    if(type == 0) obj.mesh = mesh_teapot;
    else obj.mesh = mesh_cube;
    
    obj.pos = (Vec3){0, 0, 0};
    obj.scale = type == 0 ? 0.2 : 0.5;
    obj.color = type == 0 ? 0xFF5555 : 0x55AAFF;
    objects[obj_count++] = obj;
    active_obj = obj_count - 1;
}

int main() {
    build_cube();
    build_teapot("teapot.obj");
    spawn_obj(0); // initial teapot
    
    Display *d = XOpenDisplay(NULL);
    if (!d) return 1;
    int s = DefaultScreen(d);
    Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, WIDTH, HEIGHT, 1, BlackPixel(d, s), WhitePixel(d, s));
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
                if (key == XK_t) spawn_obj(0);
                if (key == XK_c) spawn_obj(1);
                if (key == XK_Tab) active_obj = (active_obj + 1) % obj_count;
                if (key == XK_Delete && obj_count > 0) {
                    for(int i=active_obj; i<obj_count-1; i++) objects[i] = objects[i+1];
                    obj_count--;
                    if(active_obj >= obj_count) active_obj = obj_count - 1;
                }
                
                // Move active object
                if (active_obj >= 0) {
                    Object *a = &objects[active_obj];
                    if(key == XK_i) a->pos.z -= 0.5;
                    if(key == XK_k) a->pos.z += 0.5;
                    if(key == XK_j) a->pos.x -= 0.5;
                    if(key == XK_l) a->pos.x += 0.5;
                    if(key == XK_u) a->pos.y += 0.5;
                    if(key == XK_o) a->pos.y -= 0.5;
                }
                
                // Camera
                if (key == XK_w) cam_pos = v_add(cam_pos, v_mul((Vec3){sin(cam_yaw), 0, -cos(cam_yaw)}, 0.5));
                if (key == XK_s) cam_pos = v_sub(cam_pos, v_mul((Vec3){sin(cam_yaw), 0, -cos(cam_yaw)}, 0.5));
                if (key == XK_a) cam_pos = v_add(cam_pos, v_mul((Vec3){-cos(cam_yaw), 0, -sin(cam_yaw)}, 0.5));
                if (key == XK_d) cam_pos = v_sub(cam_pos, v_mul((Vec3){-cos(cam_yaw), 0, -sin(cam_yaw)}, 0.5));
            }
            if (e.type == ButtonPress) { drags = e.xbutton.button; last_x = e.xbutton.x; last_y = e.xbutton.y; }
            if (e.type == ButtonRelease) { drags = 0; }
            if (e.type == MotionNotify && drags) {
                double dx = e.xmotion.x - last_x;
                double dy = e.xmotion.y - last_y;
                if(drags == 1) { // Left click = Rotate Camera
                    cam_yaw -= dx * 0.01;
                    cam_pitch -= dy * 0.01;
                } else if(drags == 3 && active_obj >= 0) { // Right click = Rotate Object
                    objects[active_obj].rot.y += dx * 0.02;
                    objects[active_obj].rot.x += dy * 0.02;
                }
                last_x = e.xmotion.x; last_y = e.xmotion.y;
            }
        }
        
        render_frame();
        XPutImage(d, w, gc, img, 0, 0, 0, 0, WIDTH, HEIGHT);
    }
    
    XDestroyImage(img); 
    free(zbuffer);
    XCloseDisplay(d);
    return 0;
}
