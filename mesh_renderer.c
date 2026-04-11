#include <stdio.h>
#include <math.h>

double sqrt(double);
double pow(double, double);
double floor(double);

typedef struct { double x, y, z; } Vec3;
typedef struct { int v0, v1, v2; } Tri;

Vec3 add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 mul(Vec3 a, double d) { return (Vec3){a.x * d, a.y * d, a.z * d}; }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(Vec3 a, Vec3 b) {
    return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
double v_length(Vec3 v) {
    double d = dot(v, v);
    if (d < 0.0) d = 0.0;
    return sqrt(d);
}

Vec3 normalize(Vec3 v) {
    double l = v_length(v);
    if (l == 0.0) return (Vec3){0.0, 0.0, 0.0};
    return (Vec3){v.x / l, v.y / l, v.z / l};
}

int load_obj(const char *path, Vec3 *verts, Tri *tris, int *nv, int *nt) {
    FILE *f = fopen(path, "r");
    char line[256];
    *nv = 0; *nt = 0;
    if (!f) return 0;
    while (fgets(line, 256, f)) {
        if (line[0] == 'v' && line[1] == ' ') {
            double x, y, z;
            sscanf(line+2, "%lf %lf %lf", &x, &y, &z);
            verts[*nv] = (Vec3){x, y, z};
            (*nv)++;
        } else if (line[0] == 'f' && line[1] == ' ') {
            int a, b, c;
            /* handle simple "f v1 v2 v3" assuming no slashes, or wait, teapot.obj might have slashes */
            /* a simple sscanf matching %d usually ignores trailing text until space... wait! */
            /* Teapot obj has just "f 1 2 3 4" ? sscanf might not parse slashes right if we just ask for %d %d %d */
            /* Let's be safe for basic obj */
            int i, v[10], vcount = 0;
            char *p = line + 2;
            while (*p) {
                while (*p == ' ') p++;
                if (!*p || *p == '\n') break;
                sscanf(p, "%d", &v[vcount++]);
                while (*p && *p != ' ') p++;
            }
            if (vcount >= 3) {
                tris[*nt].v0 = v[0] - 1;
                tris[*nt].v1 = v[1] - 1;
                tris[*nt].v2 = v[2] - 1;
                (*nt)++;
                /* Handle quads by triangulating */
                if (vcount >= 4) {
                    tris[*nt].v0 = v[0] - 1;
                    tris[*nt].v1 = v[2] - 1;
                    tris[*nt].v2 = v[3] - 1;
                    (*nt)++;
                }
            }
        }
    }
    fclose(f);
    return *nt;
}

double hit_triangle(Vec3 orig, Vec3 dir, Vec3 v0, Vec3 v1, Vec3 v2) {
    Vec3 e1 = sub(v1, v0);
    Vec3 e2 = sub(v2, v0);
    Vec3 h = cross(dir, e2);
    double a = dot(e1, h);
    if (a > -0.00001 && a < 0.00001) return -1.0;
    double f = 1.0 / a;
    Vec3 s = sub(orig, v0);
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return -1.0;
    Vec3 q = cross(s, e1);
    double v = f * dot(dir, q);
    if (v < 0.0 || u + v > 1.0) return -1.0;
    double t = f * dot(e2, q);
    if (t > 0.0001) return t;
    return -1.0;
}

Vec3 verts[10000];
Tri tris[20000];
int nv, nt;

/* Global light */
Vec3 light_dir;

Vec3 trace(Vec3 ro, Vec3 rd, int depth) {
    /* Bounding sphere radius ~1.0 for the teapot since it's centered at 0,0,0 and scaled down */
    double min_t = 1e9;
    int hit_idx = -1;
    int i;
    
    Vec3 oc = ro; /* Center is 0,0,0 */
    double b = dot(oc, rd);
    double c_sph = dot(oc, oc) - 1.2 * 1.2;
    double h_sph = b*b - c_sph;
    if (h_sph < 0.0) return (Vec3){0.2, 0.2, 0.2};

    /* Hit mesh */
    for (i = 0; i < nt; i++) {
        double t = hit_triangle(ro, rd, verts[tris[i].v0], verts[tris[i].v1], verts[tris[i].v2]);
        if (t > 0.0 && t < min_t) {
            min_t = t;
            hit_idx = i;
        }
    }

    if (hit_idx != -1) {
        Vec3 v0 = verts[tris[hit_idx].v0];
        Vec3 v1 = verts[tris[hit_idx].v1];
        Vec3 v2 = verts[tris[hit_idx].v2];
        Vec3 p = add(ro, mul(rd, min_t));
        Vec3 n = normalize(cross(sub(v1, v0), sub(v2, v0)));
        if (dot(rd, n) > 0.0) n = mul(n, -1.0); /* Backface */

        /* Shadow ray bounds check */
        int shadow_hit = 0;
        Vec3 shadow_ro = add(p, mul(n, 0.001));
        Vec3 soc = shadow_ro;
        double sb = dot(soc, light_dir);
        double sc_sph = dot(soc, soc) - 1.2 * 1.2;
        double sh_sph = sb*sb - sc_sph;
        
        if (sh_sph >= 0.0) {
            for (i = 0; i < nt; i++) {
                if (i == hit_idx) continue;
                double t = hit_triangle(shadow_ro, light_dir, verts[tris[i].v0], verts[tris[i].v1], verts[tris[i].v2]);
                if (t > 0.0) {
                    shadow_hit = 1;
                    break;
                }
            }
        }

        double diff = dot(n, light_dir);
        if (diff < 0.0) diff = 0.0;
        
        if (shadow_hit) diff *= 0.2;

        /* Ambient + Diffuse */
        double c = 0.1 + diff * 0.8;
        return (Vec3){c, 0.7 * c, 0.3 * c}; /* Golden teapot */
    }

    /* Sky */
    return (Vec3){0.2, 0.2, 0.2};
}

int main(int argc, char **argv) {
    if (argc < 2) return 1;
    load_obj(argv[1], verts, tris, &nv, &nt);
    fprintf(stderr, "Loaded %d verts, %d tris\n", nv, nt);

    light_dir = normalize((Vec3){1.0, 1.0, -1.0});

    int W = 1920;
    int H = 1080;
    
    printf("P3\n%d %d\n255\n", W, H);
    
    /* Center and scale teapot */
    int i;
    Vec3 center = (Vec3){0.0, 0.0, 0.0};
    for (i = 0; i < nv; i++) {
        center = add(center, verts[i]);
    }
    center = mul(center, 1.0 / nv);
    
    for (i = 0; i < nv; i++) {
        verts[i] = sub(verts[i], center);
        /* scale down teapot */
        verts[i] = mul(verts[i], 0.2);
    }
    
    int y, x;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            double nx = (2.0 * (x + 0.5) / (double)W - 1.0) * (double)W / (double)H;
            double ny = -(2.0 * (y + 0.5) / (double)H - 1.0);
            
            Vec3 ro = (Vec3){0.0, 3.0, 5.0};
            Vec3 target = (Vec3){nx, ny + 1.0, 1.0}; /* look down slightly */
            Vec3 rd = normalize(sub(target, ro));
            
            Vec3 c = trace(ro, rd, 0);
            int ir = (int)(255.99 * c.x);
            int ig = (int)(255.99 * c.y);
            int ib = (int)(255.99 * c.z);
            if (ir > 255) ir = 255;
            if (ig > 255) ig = 255;
            if (ib > 255) ib = 255;
            printf("%d %d %d\n", ir, ig, ib);
        }
        if (y % 100 == 0) fprintf(stderr, "Row %d\n", y);
    }

    return 0;
}
