#include <stdio.h>
#include <math.h>

double sqrt(double x);
double pow(double x, double y);
double floor(double x);

typedef struct { double x, y, z; } Vec3;

/* Properties: 0=matte, 1=mirror */
typedef struct { Vec3 c; double r; Vec3 color; double reflectivity; double shininess; } Sphere;

void make_vec3(double x, double y, double z, Vec3 *out) {
    out->x = x; out->y = y; out->z = z;
}

void v_add(Vec3 *a, Vec3 *b, Vec3 *out) { make_vec3(a->x+b->x, a->y+b->y, a->z+b->z, out); }
void v_sub(Vec3 *a, Vec3 *b, Vec3 *out) { make_vec3(a->x-b->x, a->y-b->y, a->z-b->z, out); }
void v_mul(Vec3 *a, double d, Vec3 *out) { make_vec3(a->x*d, a->y*d, a->z*d, out); }
void v_mul_v(Vec3 *a, Vec3 *b, Vec3 *out) { make_vec3(a->x*b->x, a->y*b->y, a->z*b->z, out); }
double v_dot(Vec3 *a, Vec3 *b) { return a->x*b->x + a->y*b->y + a->z*b->z; }
double v_length(Vec3 *v) { 
    double d = v_dot(v, v);
    if (d <= 0.0) return 0.0;
    return sqrt(d); 
}
void v_normalize(Vec3 *v, Vec3 *out) { 
    double l = v_length(v); 
    if (l > 0.0) make_vec3(v->x/l, v->y/l, v->z/l, out); 
    else make_vec3(0,0,0, out);
}
void v_reflect(Vec3 *I, Vec3 *N, Vec3 *out) {
    /* R = I - 2.0 * dot(N, I) * N */
    double d = v_dot(N, I);
    Vec3 t;
    v_mul(N, 2.0 * d, &t);
    v_sub(I, &t, out);
}

void make_sphere(Vec3 *c, double r, Vec3 *color, double refl, double shininess, Sphere *out) {
    out->c = *c; out->r = r; out->color = *color; 
    out->reflectivity = refl; out->shininess = shininess;
}

double intersect_sphere(Sphere *s, Vec3 *ray_origin, Vec3 *ray_dir) {
    Vec3 oc;
    v_sub(ray_origin, &s->c, &oc);
    double b = 2.0 * v_dot(&oc, ray_dir);
    double c = v_dot(&oc, &oc) - s->r * s->r;
    double d = b*b - 4.0*c;
    if (d < 0.0) return -1.0;
    d = sqrt(d);
    double t1 = (-b - d) / 2.0;
    double t2 = (-b + d) / 2.0;
    if (t1 > 0.001) return t1;
    if (t2 > 0.001) return t2;
    return -1.0;
}

Sphere spheres[5];

void trace(Vec3 *ray_origin, Vec3 *ray_dir, int depth, Vec3 *out) {
    int i;
    double t = 1000000.0;
    int obj_idx = -1;

    for (i = 0; i < 5; i++) {
        double it = intersect_sphere(&spheres[i], ray_origin, ray_dir);
        if (it > 0.001 && it < t) {
            t = it;
            obj_idx = i;
        }
    }
    
    /* floor intersection at y = -1.0 */
    double floor_t = -1.0;
    if (ray_dir->y < 0.0) {
        floor_t = (-1.0 - ray_origin->y) / ray_dir->y;
    }
    if (floor_t > 0.001 && floor_t < t) {
        t = floor_t;
        obj_idx = 100;
    }

    if (obj_idx == -1) {
        /* sky gradient */
        double a = 0.5 * (ray_dir->y + 1.0);
        Vec3 w1, w2, t1, t2;
        make_vec3(1.0, 1.0, 1.0, &w1);
        make_vec3(0.5, 0.7, 1.0, &w2);
        v_mul(&w1, 1.0 - a, &t1);
        v_mul(&w2, a, &t2);
        v_add(&t1, &t2, out);
        return;
    }

    Vec3 ray_dir_t, hit_p;
    v_mul(ray_dir, t, &ray_dir_t);
    v_add(ray_origin, &ray_dir_t, &hit_p);
    Vec3 normal;
    Vec3 color;
    double reflectivity = 0.0;
    double shininess = 0.0;

    if (obj_idx == 100) {
        make_vec3(0.0, 1.0, 0.0, &normal);
        /* checkerboard */
        int cx = (int)(floor(hit_p.x * 2.0));
        int cz = (int)(floor(hit_p.z * 2.0));
        int m = (cx + cz) % 2;
        if (m < 0) m = -m;
        if (m == 0) make_vec3(0.9, 0.9, 0.9, &color);
        else make_vec3(0.2, 0.2, 0.2, &color);
        reflectivity = 0.3; /* floor is slightly reflective */
        shininess = 0.0;
    } else {
        Vec3 sub_c;
        v_sub(&hit_p, &spheres[obj_idx].c, &sub_c);
        v_normalize(&sub_c, &normal);
        color = spheres[obj_idx].color;
        reflectivity = spheres[obj_idx].reflectivity;
        shininess = spheres[obj_idx].shininess;
    }

    /* Prevent shadow/reflection acne by bumping the hit point outward */
    Vec3 nudge;
    v_mul(&normal, 0.001, &nudge);
    v_add(&hit_p, &nudge, &hit_p);

    Vec3 raw_light, light_dir;
    make_vec3(1.0, 2.0, -1.5, &raw_light);
    v_normalize(&raw_light, &light_dir);
    
    /* shadow casting */
    int in_shadow = 0;
    for (i = 0; i < 5; i++) {
        if (intersect_sphere(&spheres[i], &hit_p, &light_dir) > 0.001) {
            in_shadow = 1;
            break;
        }
    }

    /* Ambient */
    Vec3 ambient;
    v_mul(&color, 0.15, &ambient);

    /* Diffuse */
    double diff = v_dot(&normal, &light_dir);
    if (diff < 0.0) diff = 0.0;
    if (in_shadow) diff = 0.0;
    Vec3 diffuse;
    v_mul(&color, diff * 0.8, &diffuse);

    /* Specular (Blinn-Phong) */
    Vec3 specular;
    make_vec3(0.0, 0.0, 0.0, &specular);
    if (!in_shadow && shininess > 0.0 && diff > 0.0) {
        Vec3 view_dir, half_vec, neg_ray;
        v_mul(ray_dir, -1.0, &neg_ray);
        v_normalize(&neg_ray, &view_dir);
        
        Vec3 light_plus_view;
        v_add(&light_dir, &view_dir, &light_plus_view);
        v_normalize(&light_plus_view, &half_vec);
        
        double spec_angle = v_dot(&normal, &half_vec);
        if (spec_angle > 0.0) {
            double spec_intensity = pow(spec_angle, shininess);
            make_vec3(1.0, 1.0, 1.0, &specular);
            v_mul(&specular, spec_intensity * 0.5, &specular);
        }
    }

    /* Combine lighting */
    Vec3 direct_light;
    v_add(&ambient, &diffuse, &direct_light);
    v_add(&direct_light, &specular, &direct_light);

    /* Reflection */
    if (reflectivity > 0.0 && depth < 3) {
        Vec3 refl_dir, refl_color;
        v_reflect(ray_dir, &normal, &refl_dir);
        v_normalize(&refl_dir, &refl_dir);
        trace(&hit_p, &refl_dir, depth + 1, &refl_color);
        
        Vec3 term1, term2;
        v_mul(&direct_light, 1.0 - reflectivity, &term1);
        v_mul(&refl_color, reflectivity, &term2);
        v_add(&term1, &term2, out);
    } else {
        *out = direct_light;
    }
}

int main() {
    int width = 1920;
    int height = 1080;
    int i, j;

    Vec3 c1, c2;
    /* Red Matte */
    make_vec3(-2.0, 0.0, -3.0, &c1);
    make_vec3(0.9, 0.2, 0.2, &c2);
    make_sphere(&c1, 1.0, &c2, 0.1, 50.0, &spheres[0]);

    /* Green Glass */
    make_vec3(2.0, 0.0, -4.0, &c1);
    make_vec3(0.2, 0.9, 0.2, &c2);
    make_sphere(&c1, 1.0, &c2, 0.4, 100.0, &spheres[1]);

    /* Chrome Mirror */
    make_vec3(0.0, 0.0, -5.0, &c1);
    make_vec3(0.9, 0.9, 0.9, &c2);
    make_sphere(&c1, 1.0, &c2, 0.8, 200.0, &spheres[2]);

    /* Blue Matte */
    make_vec3(-1.5, -0.6, -1.5, &c1);
    make_vec3(0.2, 0.2, 0.9, &c2);
    make_sphere(&c1, 0.4, &c2, 0.1, 30.0, &spheres[3]);

    /* Gold */
    make_vec3(1.5, -0.5, -2.0, &c1);
    make_vec3(0.9, 0.7, 0.1, &c2);
    make_sphere(&c1, 0.5, &c2, 0.5, 150.0, &spheres[4]);

    printf("P3\n%d %d\n255\n", width, height);

    for (j = height - 1; j >= 0; j--) {
        for (i = 0; i < width; i++) {
            double u = (double)i / (double)(width - 1);
            double v = (double)j / (double)(height - 1);
            
            Vec3 raw_dir, dir;
            make_vec3(u * 2.0 - 1.0, v * 2.0 - 1.0, -1.0, &raw_dir);
            raw_dir.x *= (double)width / (double)height;
            v_normalize(&raw_dir, &dir);
            
            Vec3 ro, col;
            make_vec3(0.0, 0.5, 1.0, &ro); /* Eye slightly elevated */
            trace(&ro, &dir, 0, &col);
            
            int ir = (int)(255.99 * col.x);
            int ig = (int)(255.99 * col.y);
            int ib = (int)(255.99 * col.z);
            if (ir > 255) ir = 255;
            if (ig > 255) ig = 255;
            if (ib > 255) ib = 255;
            if (ir < 0) ir = 0;
            if (ig < 0) ig = 0;
            if (ib < 0) ib = 0;
            
            printf("%d %d %d\n", ir, ig, ib);
        }
    }
    return 0;
}
