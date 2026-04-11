#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { double x, y, z; } Vec3;
typedef struct { double u, v; } Vec2;
typedef struct { double m[16]; } Mat4;
typedef struct { double w, x, y, z; } Quat;

/* --- Math Functions (Pointer based internals to bypass ZCC ABI 128-byte copy bug) --- */
void p_mat4_mul(Mat4 *r, const Mat4 *a, const Mat4 *b) {
    int i, j, k;
    for(i=0; i<4; i++) {
        for(j=0; j<4; j++) {
            double sum = 0.0;
            for(k=0; k<4; k++) {
                sum += a->m[i*4 + k] * b->m[k*4 + j];
            }
            r->m[i*4 + j] = sum;
        }
    }
}

void p_mat4_inv(Mat4 *inv, const Mat4 *m) {
    double det, invDet;
    int i;
    inv->m[0] = m->m[5]*m->m[10]*m->m[15] - m->m[5]*m->m[11]*m->m[14] - m->m[9]*m->m[6]*m->m[15] + m->m[9]*m->m[7]*m->m[14] + m->m[13]*m->m[6]*m->m[11] - m->m[13]*m->m[7]*m->m[10];
    inv->m[4] = -m->m[4]*m->m[10]*m->m[15] + m->m[4]*m->m[11]*m->m[14] + m->m[8]*m->m[6]*m->m[15] - m->m[8]*m->m[7]*m->m[14] - m->m[12]*m->m[6]*m->m[11] + m->m[12]*m->m[7]*m->m[10];
    inv->m[8] = m->m[4]*m->m[9]*m->m[15] - m->m[4]*m->m[11]*m->m[13] - m->m[8]*m->m[5]*m->m[15] + m->m[8]*m->m[7]*m->m[13] + m->m[12]*m->m[5]*m->m[11] - m->m[12]*m->m[7]*m->m[9];
    inv->m[12] = -m->m[4]*m->m[9]*m->m[14] + m->m[4]*m->m[10]*m->m[13] + m->m[8]*m->m[5]*m->m[14] - m->m[8]*m->m[6]*m->m[13] - m->m[12]*m->m[5]*m->m[10] + m->m[12]*m->m[6]*m->m[9];
    inv->m[1] = -m->m[1]*m->m[10]*m->m[15] + m->m[1]*m->m[11]*m->m[14] + m->m[9]*m->m[2]*m->m[15] - m->m[9]*m->m[3]*m->m[14] - m->m[13]*m->m[2]*m->m[11] + m->m[13]*m->m[3]*m->m[10];
    inv->m[5] = m->m[0]*m->m[10]*m->m[15] - m->m[0]*m->m[11]*m->m[14] - m->m[8]*m->m[2]*m->m[15] + m->m[8]*m->m[3]*m->m[14] + m->m[12]*m->m[2]*m->m[11] - m->m[12]*m->m[3]*m->m[10];
    inv->m[9] = -m->m[0]*m->m[9]*m->m[15] + m->m[0]*m->m[11]*m->m[13] + m->m[8]*m->m[1]*m->m[15] - m->m[8]*m->m[3]*m->m[13] - m->m[12]*m->m[1]*m->m[11] + m->m[12]*m->m[3]*m->m[9];
    inv->m[13] = m->m[0]*m->m[9]*m->m[14] - m->m[0]*m->m[10]*m->m[13] - m->m[8]*m->m[1]*m->m[14] + m->m[8]*m->m[2]*m->m[13] + m->m[12]*m->m[1]*m->m[10] - m->m[12]*m->m[2]*m->m[9];
    inv->m[2] = m->m[1]*m->m[6]*m->m[15] - m->m[1]*m->m[7]*m->m[14] - m->m[5]*m->m[2]*m->m[15] + m->m[5]*m->m[3]*m->m[14] + m->m[13]*m->m[2]*m->m[7] - m->m[13]*m->m[3]*m->m[6];
    inv->m[6] = -m->m[0]*m->m[6]*m->m[15] + m->m[0]*m->m[7]*m->m[14] + m->m[4]*m->m[2]*m->m[15] - m->m[4]*m->m[3]*m->m[14] - m->m[12]*m->m[2]*m->m[7] + m->m[12]*m->m[3]*m->m[6];
    inv->m[10] = m->m[0]*m->m[5]*m->m[15] - m->m[0]*m->m[7]*m->m[13] - m->m[4]*m->m[1]*m->m[15] + m->m[4]*m->m[3]*m->m[13] + m->m[12]*m->m[1]*m->m[7] - m->m[12]*m->m[3]*m->m[5];
    inv->m[14] = -m->m[0]*m->m[5]*m->m[14] + m->m[0]*m->m[6]*m->m[13] + m->m[4]*m->m[1]*m->m[14] - m->m[4]*m->m[2]*m->m[13] - m->m[12]*m->m[1]*m->m[6] + m->m[12]*m->m[2]*m->m[5];
    inv->m[3] = -m->m[1]*m->m[6]*m->m[11] + m->m[1]*m->m[7]*m->m[10] + m->m[5]*m->m[2]*m->m[11] - m->m[5]*m->m[3]*m->m[10] - m->m[9]*m->m[2]*m->m[7] + m->m[9]*m->m[3]*m->m[6];
    inv->m[7] = m->m[0]*m->m[6]*m->m[11] - m->m[0]*m->m[7]*m->m[10] - m->m[4]*m->m[2]*m->m[11] + m->m[4]*m->m[3]*m->m[10] + m->m[8]*m->m[2]*m->m[7] - m->m[8]*m->m[3]*m->m[6];
    inv->m[11] = -m->m[0]*m->m[5]*m->m[11] + m->m[0]*m->m[7]*m->m[9] + m->m[4]*m->m[1]*m->m[11] - m->m[4]*m->m[3]*m->m[9] - m->m[8]*m->m[1]*m->m[7] + m->m[8]*m->m[3]*m->m[5];
    inv->m[15] = m->m[0]*m->m[5]*m->m[10] - m->m[0]*m->m[6]*m->m[9] - m->m[4]*m->m[1]*m->m[10] + m->m[4]*m->m[2]*m->m[9] + m->m[8]*m->m[1]*m->m[6] - m->m[8]*m->m[2]*m->m[5];
    det = m->m[0]*inv->m[0] + m->m[1]*inv->m[4] + m->m[2]*inv->m[8] + m->m[3]*inv->m[12];
    if (det != 0.0) {
        invDet = 1.0 / det;
        for (i = 0; i < 16; i++) {
            inv->m[i] = inv->m[i] * invDet;
        }
    }
}

Vec3 mat4_transform_point_p(const Mat4 *m, Vec3 p) {
    Vec3 r;
    r.x = m->m[0]*p.x + m->m[1]*p.y + m->m[2]*p.z + m->m[3];
    r.y = m->m[4]*p.x + m->m[5]*p.y + m->m[6]*p.z + m->m[7];
    r.z = m->m[8]*p.x + m->m[9]*p.y + m->m[10]*p.z + m->m[11];
    return r;
}

Vec3 mat4_transform_dir_p(const Mat4 *m, Vec3 d) {
    Vec3 r;
    r.x = m->m[0]*d.x + m->m[1]*d.y + m->m[2]*d.z;
    r.y = m->m[4]*d.x + m->m[5]*d.y + m->m[6]*d.z;
    r.z = m->m[8]*d.x + m->m[9]*d.y + m->m[10]*d.z;
    return r;
}

/* --- Required Math Functions --- */
Vec3 make_vec3(double x, double y, double z) { Vec3 v; v.x = x; v.y = y; v.z = z; return v; }
Vec3 add(Vec3 a, Vec3 b) { return make_vec3(a.x + b.x, a.y + b.y, a.z + b.z); }
Vec3 sub(Vec3 a, Vec3 b) { return make_vec3(a.x - b.x, a.y - b.y, a.z - b.z); }
Vec3 mul_scalar(Vec3 v, double s) { return make_vec3(v.x * s, v.y * s, v.z * s); }
double dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
Vec3 cross(Vec3 a, Vec3 b) { return make_vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
Vec3 normalize(Vec3 v) {
    double l = sqrt(dot(v, v));
    if (l > 0.0000001) return mul_scalar(v, 1.0 / l);
    return v;
}

Mat4 mat4_identity(void) {
    Mat4 r; int i;
    for(i=0; i<16; i++) r.m[i] = 0.0;
    r.m[0] = 1.0; r.m[5] = 1.0; r.m[10] = 1.0; r.m[15] = 1.0;
    return r;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b) {
    Mat4 r;
    p_mat4_mul(&r, &a, &b);
    return r;
}

Mat4 mat4_translate(double x, double y, double z) {
    Mat4 r = mat4_identity();
    r.m[3] = x; r.m[7] = y; r.m[11] = z;
    return r;
}

Mat4 mat4_rotate_x(double angle) {
    Mat4 r = mat4_identity();
    double c = cos(angle), s = sin(angle);
    r.m[5] = c; r.m[6] = -s; r.m[9] = s; r.m[10] = c;
    return r;
}

Mat4 mat4_rotate_y(double angle) {
    Mat4 r = mat4_identity();
    double c = cos(angle), s = sin(angle);
    r.m[0] = c; r.m[2] = s; r.m[8] = -s; r.m[10] = c;
    return r;
}

Mat4 mat4_rotate_z(double angle) {
    Mat4 r = mat4_identity();
    double c = cos(angle), s = sin(angle);
    r.m[0] = c; r.m[1] = -s; r.m[4] = s; r.m[5] = c;
    return r;
}

Mat4 mat4_perspective(double fov, double aspect, double near_p, double far_p) {
    Mat4 r; int i;
    double f = 1.0 / tan(fov * 0.5);
    for(i=0; i<16; i++) r.m[i] = 0.0;
    r.m[0] = f / aspect; r.m[5] = f;
    r.m[10] = (far_p + near_p) / (near_p - far_p);
    r.m[11] = (2.0 * far_p * near_p) / (near_p - far_p);
    r.m[14] = -1.0; r.m[15] = 0.0;
    return r;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Mat4 r; int i;
    Vec3 f = normalize(sub(center, eye));
    Vec3 s = normalize(cross(f, up));
    Vec3 u = cross(s, f);
    for(i=0; i<16; i++) r.m[i] = 0.0;
    r.m[0] = s.x;  r.m[1] = s.y;  r.m[2] = s.z;  r.m[3] = -dot(s, eye);
    r.m[4] = u.x;  r.m[5] = u.y;  r.m[6] = u.z;  r.m[7] = -dot(u, eye);
    r.m[8] = -f.x; r.m[9] = -f.y; r.m[10]= -f.z; r.m[11]= dot(f, eye);
    r.m[15] = 1.0;
    return r;
}

Vec3 mat4_transform_point(Mat4 m, Vec3 p) { return mat4_transform_point_p(&m, p); }
Vec3 mat4_transform_dir(Mat4 m, Vec3 d) { return mat4_transform_dir_p(&m, d); }

Mat4 mat4_inverse(Mat4 m) {
    Mat4 inv = mat4_identity();
    p_mat4_inv(&inv, &m);
    return inv;
}

Quat quat_from_axis_angle(Vec3 axis, double angle) {
    Quat q; double half_angle = angle * 0.5; double s = sin(half_angle);
    q.w = cos(half_angle); q.x = axis.x * s; q.y = axis.y * s; q.z = axis.z * s; return q;
}

Quat quat_multiply(Quat a, Quat b) {
    Quat q;
    q.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
    q.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y;
    q.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x;
    q.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w;
    return q;
}

Quat quat_slerp(Quat a, Quat b, double t) {
    Quat q; double scale0, scale1;
    double cosom = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    double abs_cosom = fabs(cosom);
    if ((1.0 - abs_cosom) > 0.0001) {
        double omega = acos(abs_cosom);
        double sinom = sin(omega);
        scale0 = sin((1.0 - t)*omega) / sinom;
        scale1 = sin(t*omega) / sinom;
    } else {
        scale0 = 1.0 - t; scale1 = t;
    }
    if (cosom < 0.0) scale1 = -scale1;
    q.x = scale0*a.x + scale1*b.x; q.y = scale0*a.y + scale1*b.y;
    q.z = scale0*a.z + scale1*b.z; q.w = scale0*a.w + scale1*b.w;
    return q;
}

Mat4 quat_to_mat4(Quat q) {
    Mat4 r = mat4_identity();
    double xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z;
    double xy = q.x*q.y, xz = q.x*q.z, yz = q.y*q.z;
    double wx = q.w*q.x, wy = q.w*q.y, wz = q.w*q.z;
    r.m[0] = 1.0 - 2.0*(yy + zz); r.m[1] = 2.0*(xy - wz); r.m[2] = 2.0*(xz + wy);
    r.m[4] = 2.0*(xy + wz); r.m[5] = 1.0 - 2.0*(xx + zz); r.m[6] = 2.0*(yz - wx);
    r.m[8] = 2.0*(xz - wy); r.m[9] = 2.0*(yz + wx); r.m[10]= 1.0 - 2.0*(xx + yy);
    return r;
}

Vec3 quat_rotate(Quat q, Vec3 v) {
    Mat4 m = quat_to_mat4(q);
    return mat4_transform_dir_p(&m, v);
}

/* --- Skeleton & Procedural Mesh --- */

typedef struct {
    char name[32];
    int parent;
    Mat4 local_transform;
    Mat4 world_transform;
    Mat4 inv_world_transform;
    double length;
    int is_sphere;
} Bone;

#define MAX_BONES 32
Bone bones[MAX_BONES];
int num_bones = 0;

int add_bone(const char* name, int parent, Mat4 local_transform, double length, int is_sphere) {
    int id = num_bones++;
    int i;
    for(i=0; i<31 && name[i]; i++) { bones[id].name[i] = name[i]; }
    bones[id].name[i] = '\0';
    bones[id].parent = parent;
    bones[id].local_transform = local_transform;
    bones[id].length = length;
    bones[id].is_sphere = is_sphere;
    return id;
}

int find_bone(const char* name) {
    int i;
    for (i = 0; i < num_bones; i++) {
        int match = 1, j = 0;
        while (name[j]) { 
            if (name[j] != bones[i].name[j]) { match = 0; break; } 
            j++; 
        }
        if (match && !bones[i].name[j]) return i;
    }
    return -1;
}

void update_skeleton(void) {
    int i;
    for (i = 0; i < num_bones; i++) {
        if (bones[i].parent == -1) {
            bones[i].world_transform = bones[i].local_transform;
        } else {
            p_mat4_mul(&bones[i].world_transform, &bones[bones[i].parent].world_transform, &bones[i].local_transform);
        }
        p_mat4_inv(&bones[i].inv_world_transform, &bones[i].world_transform);
    }
}

void init_skeleton(void) {
    int hips, spine, neck, head, l_shoulder, l_upper_arm, l_lower_arm;
    int r_shoulder, r_upper_arm, r_lower_arm, l_hip, l_upper_leg, l_lower_leg;
    int r_hip, r_upper_leg, r_lower_leg;
    double pi = acos(-1.0);

    hips = add_bone("hips", -1, mat4_translate(0, 1.0, 0), 0.0, 0);
    spine = add_bone("spine", hips, mat4_identity(), 0.5, 0);
    neck = add_bone("neck", spine, mat4_translate(0, 0.5, 0), 0.2, 0);
    head = add_bone("head", neck, mat4_translate(0, 0.2, 0), 0.0, 1);
    
    l_shoulder = add_bone("l_shoulder", spine, mat4_translate(-0.25, 0.5, 0), 0.0, 0);
    l_upper_arm = add_bone("l_upper_arm", l_shoulder, mat4_rotate_z(pi / 2.0), 0.3, 0);
    l_lower_arm = add_bone("l_lower_arm", l_upper_arm, mat4_translate(0, 0.3, 0), 0.3, 0);
    
    r_shoulder = add_bone("r_shoulder", spine, mat4_translate(0.25, 0.5, 0), 0.0, 0);
    r_upper_arm = add_bone("r_upper_arm", r_shoulder, mat4_rotate_z(-pi / 2.0), 0.3, 0);
    r_lower_arm = add_bone("r_lower_arm", r_upper_arm, mat4_translate(0, 0.3, 0), 0.3, 0);
    
    l_hip = add_bone("l_hip", hips, mat4_translate(-0.15, 0, 0), 0.0, 0);
    l_upper_leg = add_bone("l_upper_leg", l_hip, mat4_rotate_x(pi), 0.4, 0);
    l_lower_leg = add_bone("l_lower_leg", l_upper_leg, mat4_translate(0, 0.4, 0), 0.4, 0);
    
    r_hip = add_bone("r_hip", hips, mat4_translate(0.15, 0, 0), 0.0, 0);
    r_upper_leg = add_bone("r_upper_leg", r_hip, mat4_rotate_x(pi), 0.4, 0);
    r_lower_leg = add_bone("r_lower_leg", r_upper_leg, mat4_translate(0, 0.4, 0), 0.4, 0);
}

/* --- Ray Tracing core --- */

typedef struct { Vec3 origin, dir; } Ray;

double intersect_sphere(const Ray* ray, double radius) {
    double a = dot(ray->dir, ray->dir);
    double b = 2.0 * dot(ray->origin, ray->dir);
    double c = dot(ray->origin, ray->origin) - radius*radius;
    double desc = b*b - 4.0*a*c;
    if (desc < 0.0) return -1.0;
    else {
        double d = sqrt(desc);
        double t1 = (-b - d) / (2.0*a);
        double t2 = (-b + d) / (2.0*a);
        if (t1 > 0.001) return t1;
        if (t2 > 0.001) return t2;
        return -1.0;
    }
}

double intersect_cylinder(const Ray* ray, double radius, double length) {
    double a = ray->dir.x*ray->dir.x + ray->dir.z*ray->dir.z;
    double b = 2.0 * (ray->origin.x*ray->dir.x + ray->origin.z*ray->dir.z);
    double c = ray->origin.x*ray->origin.x + ray->origin.z*ray->origin.z - radius*radius;
    double desc = b*b - 4.0*a*c;
    double t_best = 1000000000.0;
    
    if (desc >= -0.0000001 && a > 0.0000001) {
        double d;
        if (desc < 0.0) desc = 0.0;
        d = sqrt(desc);
        
        {
            double t1 = (-b - d) / (2.0*a);
            double t2 = (-b + d) / (2.0*a);
            
            if (t1 > 0.001) {
                double y1 = ray->origin.y + t1 * ray->dir.y;
                if (y1 > 0.0 && y1 < length) t_best = t1;
            }
            if (t_best == 1000000000.0 && t2 > 0.001) {
                double y2 = ray->origin.y + t2 * ray->dir.y;
                if (y2 > 0.0 && y2 < length) t_best = t2;
            }
        }
    }
    
    if (fabs(ray->dir.y) > 0.000001) {
        double tc1 = (0.0 - ray->origin.y) / ray->dir.y;
        if (tc1 > 0.001 && tc1 < t_best) {
            double px = ray->origin.x + tc1 * ray->dir.x;
            double pz = ray->origin.z + tc1 * ray->dir.z;
            if (px*px + pz*pz <= radius*radius) t_best = tc1;
        }
        double tc2 = (length - ray->origin.y) / ray->dir.y;
        if (tc2 > 0.001 && tc2 < t_best) {
            double px = ray->origin.x + tc2 * ray->dir.x;
            double pz = ray->origin.z + tc2 * ray->dir.z;
            if (px*px + pz*pz <= radius*radius) t_best = tc2;
        }
    }
    
    return t_best < 500000000.0 ? t_best : -1.0;
}

Vec3 sphere_normal(Vec3 p) {
    return normalize(p);
}

Vec3 cylinder_normal(Vec3 p, double length) {
    if (p.y < 0.001) return make_vec3(0, -1, 0);
    if (p.y > length - 0.001) return make_vec3(0, 1, 0);
    return normalize(make_vec3(p.x, 0, p.z));
}

int main(void) {
    int w = 1920, h = 1080;
    int i, j, b;
    int l_arm, r_arm;
    Vec3 cam_pos, cam_dir, cam_right, cam_up, light_dir;
    double fov_scale;
    double pi = acos(-1.0);

    init_skeleton();
    
    l_arm = find_bone("l_upper_arm");
    if (l_arm != -1) {
        Mat4 rot = mat4_rotate_z(pi / 4.0);
        Mat4 prev = bones[l_arm].local_transform;
        p_mat4_mul(&bones[l_arm].local_transform, &prev, &rot);
    }
    
    r_arm = find_bone("r_upper_arm");
    if (r_arm != -1) {
        Mat4 rot = mat4_rotate_z(-pi / 4.0);
        Mat4 prev = bones[r_arm].local_transform;
        p_mat4_mul(&bones[r_arm].local_transform, &prev, &rot);
    }
    
    update_skeleton();
    
    cam_pos = make_vec3(0.0, 1.0, 3.5);
    cam_dir = normalize(make_vec3(0.0, 0.0, -1.0));
    cam_right = normalize(cross(cam_dir, make_vec3(0.0, 1.0, 0.0)));
    cam_up = cross(cam_right, cam_dir);
    fov_scale = tan(pi / 8.0);
    
    light_dir = normalize(make_vec3(1.0, 1.0, 1.0));
    
    printf("P3\n%d %d\n255\n", w, h);
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++) {
            double u = (2.0 * (i + 0.5) / (double)w - 1.0) * ((double)w / h) * fov_scale;
            double v = (1.0 - 2.0 * (j + 0.5) / (double)h) * fov_scale;
            double best_t = 1000000000.0;
            int hit_bone = -1;
            Vec3 hit_normal;
            Ray ray;
            
            Vec3 dir = normalize(add(cam_dir, add(mul_scalar(cam_right, u), mul_scalar(cam_up, v))));
            
            ray.origin = cam_pos;
            ray.dir = dir;
            hit_normal = make_vec3(0,0,0);
            
            for (b = 0; b < num_bones; b++) {
                Ray local_ray;
                double t = -1.0;
                Vec3 norm;
                
                if (bones[b].length == 0.0 && bones[b].is_sphere == 0) continue;
                
                local_ray.origin = mat4_transform_point_p(&bones[b].inv_world_transform, ray.origin);
                local_ray.dir = mat4_transform_dir_p(&bones[b].inv_world_transform, ray.dir);
                
                if (bones[b].is_sphere) {
                    t = intersect_sphere(&local_ray, 0.15);
                    if (t > 0.0) {
                        norm = sphere_normal(add(local_ray.origin, mul_scalar(local_ray.dir, t)));
                    }
                } else {
                    t = intersect_cylinder(&local_ray, 0.12, bones[b].length);
                    if (t > 0.0) {
                        norm = cylinder_normal(add(local_ray.origin, mul_scalar(local_ray.dir, t)), bones[b].length);
                    }
                }
                
                if (t > 0.0 && t < best_t) {
                    best_t = t;
                    hit_bone = b;
                    hit_normal = mat4_transform_dir_p(&bones[b].world_transform, norm);
                    hit_normal = normalize(hit_normal);
                }
            }
            
            if (hit_bone != -1) {
                double diff = dot(hit_normal, light_dir);
                int col;
                if (diff < 0.2) diff = 0.2;
                col = (int)(diff * 255.0 + 0.5);
                if (col > 255) col = 255;
                printf("%d %d %d\n", col, (int)(col * 0.8 + 0.5), (int)(col * (0.5 + (hit_bone % 5) * 0.1) + 0.5));
            } else {
                int bg;
                double b_val = 40.0 + (double)j / (double)h * 40.0;
                bg = (int)(b_val + 0.5);
                printf("%d %d %d\n", bg, bg, bg + 20);
            }
        }
    }
    return 0;
}
