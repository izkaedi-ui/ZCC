import glob, re

def main():
    # exp1
    with open("exp1_raytracer_simd.c", "r") as f:
        txt = f.read()
    txt = txt.replace("const int samples_per_pixel = 30;", "const int samples_per_pixel = 1;")
    txt = txt.replace("trace_ray(&scene, ray, 3);", "trace_ray(&scene, ray, 1);")
    with open("exp1_raytracer_simd.c", "w") as f:
        f.write(txt)
        
    # exp2
    with open("exp2_voxel_engine.c", "r") as f:
        txt = f.read()
    # It's 16x32x16 chunks, rendering is 640x480. 
    # Voxel raycast max step = 100. Let's reduce resolution if needed, but 640x480 raymarch might be slow.
    txt = txt.replace("const int width = 640;", "const int width = 160;")
    txt = txt.replace("const int height = 480;", "const int height = 120;")
    txt = txt.replace("raycast_voxel(chunk, camera, dir, 100);", "raycast_voxel(chunk, camera, dir, 20);")
    with open("exp2_voxel_engine.c", "w") as f:
        f.write(txt)
        
    # exp3
    with open("exp3_audio_visualizer.c", "r") as f:
        txt = f.read()
    txt = txt.replace("const int num_particles = 256;", "const int num_particles = 64;")
    txt = txt.replace("for (int frame = 0; frame < 30; frame++)", "for (int frame = 0; frame < 2; frame++)")
    with open("exp3_audio_visualizer.c", "w") as f:
        f.write(txt)
        
    # exp4
    with open("exp4_vr_stereo.c", "r") as f:
        txt = f.read()
    txt = txt.replace("const int eye_width = 640;", "const int eye_width = 160;")
    txt = txt.replace("const int eye_height = 480;", "const int eye_height = 120;")
    with open("exp4_vr_stereo.c", "w") as f:
        f.write(txt)
        
if __name__ == "__main__":
    main()
