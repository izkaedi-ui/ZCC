#!/usr/bin/env python3
"""Fix experiments 3, 4, 5 for ZCC runtime crashes.
Root causes:
  - Exp3: VLA RSP save/restore corruption (const-int treated as VLA)
  - Exp4: Struct-by-value passing (Sphere) + VLA RSP corruption
  - Exp5: Struct-by-value passing (Vec3) + misaligned stack + VLA
  
Fix strategy: Replace VLAs with fixed-size arrays (sizes are known constants),
and change struct-by-value params to pointer params where crashing.
"""
import re

def fix_exp3():
    with open('exp3_audio_visualizer.c', 'r') as f:
        src = f.read()
    
    # Replace VLA declarations with fixed-size arrays
    # const int fft_size = 512; → use literal 512
    src = src.replace('Complex fft_data[fft_size];', 'Complex fft_data[512];')
    src = src.replace('float audio_samples[fft_size];', 'float audio_samples[512];')
    src = src.replace('unsigned char framebuffer[height][width][3];', 
                      'unsigned char framebuffer[480][640][3];')
    src = src.replace('Particle particles[num_particles];',
                      'Particle particles[256];')
    # Also the band_energies 
    src = src.replace('float band_energies[num_freq_bands];',
                      'float band_energies[8];')
    
    # Remove the 'const' that makes ZCC think these are VLAs
    # (actually the issue is ZCC doesn't fold const int into compile-time constants for array sizes)
    
    with open('exp3_audio_visualizer.c', 'w') as f:
        f.write(src)
    print("Fixed exp3_audio_visualizer.c")

def fix_exp4():
    with open('exp4_vr_stereo.c', 'r') as f:
        src = f.read()
    
    # Fix VLA framebuffers
    src = src.replace('unsigned char left_framebuffer[eye_height][eye_width][3];',
                      'unsigned char left_framebuffer[480][640][3];')
    src = src.replace('unsigned char right_framebuffer[eye_height][eye_width][3];',
                      'unsigned char right_framebuffer[480][640][3];')
    
    # Fix ray_sphere_intersect: Sphere passed by value (22 bytes) -> pass by pointer
    # Change declaration
    src = src.replace(
        'static int ray_sphere_intersect(Vec3 origin, Vec3 dir, Sphere sphere, float *t_out) {',
        'static int ray_sphere_intersect(Vec3 *origin_p, Vec3 *dir_p, Sphere *sphere_p, float *t_out) {\n'
        '    Vec3 origin = *origin_p; Vec3 dir = *dir_p; Sphere sphere = *sphere_p;'
    )
    
    # Fix call site in trace_ray: spheres[i] -> &spheres[i], etc.
    src = src.replace(
        'if (ray_sphere_intersect(origin, dir, spheres[i], &t)) {',
        'if (ray_sphere_intersect(&origin, &dir, &spheres[i], &t)) {'
    )
    
    # Fix trace_ray: Vec3 params by pointer
    src = src.replace(
        'static void trace_ray(Vec3 origin, Vec3 dir, Sphere *spheres, int num_spheres,\n'
        '                     unsigned char *r, unsigned char *g, unsigned char *b) {',
        'static void trace_ray(Vec3 *origin_p, Vec3 *dir_p, Sphere *spheres, int num_spheres,\n'
        '                     unsigned char *r, unsigned char *g, unsigned char *b) {\n'
        '    Vec3 origin = *origin_p; Vec3 dir = *dir_p;'
    )
    
    # Fix trace_ray call site in render_eye
    src = src.replace(
        'trace_ray(eye.position, ray_dir, spheres, num_spheres, &r, &g, &b);',
        'trace_ray(&eye.position, &ray_dir, spheres, num_spheres, &r, &g, &b);'
    )
    
    # Fix render_eye: pass Eye by pointer, VRConfig by pointer
    src = src.replace(
        'static void render_eye(Eye eye, Sphere *spheres, int num_spheres,\n'
        '                      unsigned char framebuffer[][640][3], \n'
        '                      int height, int width, VRConfig config) {',
        'static void render_eye(Eye *eye_p, Sphere *spheres, int num_spheres,\n'
        '                      unsigned char framebuffer[][640][3], \n'
        '                      int height, int width, VRConfig *config_p) {\n'
        '    Eye eye = *eye_p; VRConfig config = *config_p;'
    )
    
    # Fix render_eye call sites in main
    src = src.replace(
        'render_eye(left_eye, spheres, num_spheres, left_framebuffer, eye_height, eye_width, config);',
        'render_eye(&left_eye, spheres, num_spheres, left_framebuffer, eye_height, eye_width, &config);'
    )
    src = src.replace(
        'render_eye(right_eye, spheres, num_spheres, right_framebuffer, eye_height, eye_width, config);',
        'render_eye(&right_eye, spheres, num_spheres, right_framebuffer, eye_height, eye_width, &config);'
    )
    
    # Fix vec3_create usage (returns Vec3 by value - this is fine for ZCC for small returns via rax+rdx)
    # The issue is primarily passing structs as params, not returns
    
    with open('exp4_vr_stereo.c', 'w') as f:
        f.write(src)
    print("Fixed exp4_vr_stereo.c")

def fix_exp5():
    with open('exp5_physics_engine.c', 'r') as f:
        src = f.read()
    
    # Fix VLAs
    src = src.replace('RigidBody bodies[num_bodies];',
                      'RigidBody bodies[8];')
    src = src.replace('unsigned char framebuffer[height][width][3];',
                      'unsigned char framebuffer[480][640][3];')
    src = src.replace('Contact contacts[64];',
                      'Contact contacts[64];')  # This is already fixed-size
    
    # Fix simd_sphere_distance: Vec3 by value -> by pointer
    src = src.replace(
        'static inline float simd_sphere_distance(Vec3 a, Vec3 b) {',
        'static inline float simd_sphere_distance(Vec3 *a_p, Vec3 *b_p) {\n'
        '    Vec3 a = *a_p; Vec3 b = *b_p;'
    )
    
    # Fix call site
    src = src.replace(
        "float distance = simd_sphere_distance(a->position, b->position);",
        "float distance = simd_sphere_distance(&a->position, &b->position);"
    )
    
    with open('exp5_physics_engine.c', 'w') as f:
        f.write(src)
    print("Fixed exp5_physics_engine.c")

if __name__ == '__main__':
    fix_exp3()
    fix_exp4()
    fix_exp5()
    print("All experiments patched.")
