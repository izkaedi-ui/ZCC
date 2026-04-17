with open("exp4_vr_stereo.c", "r") as f:
    text = f.read()

# Fix Spheres
bad_block0 = """    spheres[0] = (Sphere){
        .center = {0.0f, 0.0f, -10.0f},
        .radius = 3.0f,
        .mat = {.r = 255, .g = 100, .b = 100, .reflective = 0, .metallic = 0, .emissive = 0}
    };"""
good_block0 = "spheres[0].center.x=0.0f; spheres[0].center.y=0.0f; spheres[0].center.z=-10.0f; spheres[0].radius=3.0f; spheres[0].mat.r=255; spheres[0].mat.g=100; spheres[0].mat.b=100; spheres[0].mat.reflective=0; spheres[0].mat.metallic=0; spheres[0].mat.emissive=0;"
text = text.replace(bad_block0, good_block0)

bad_block1 = """    spheres[1] = (Sphere){
        .center = {-5.0f, 2.0f, -8.0f},
        .radius = 1.5f,
        .mat = {.r = 100, .g = 255, .b = 100, .reflective = 1, .metallic = 1, .emissive = 0}
    };"""
good_block1 = "spheres[1].center.x=-5.0f; spheres[1].center.y=2.0f; spheres[1].center.z=-8.0f; spheres[1].radius=1.5f; spheres[1].mat.r=100; spheres[1].mat.g=255; spheres[1].mat.b=100; spheres[1].mat.reflective=1; spheres[1].mat.metallic=1; spheres[1].mat.emissive=0;"
text = text.replace(bad_block1, good_block1)

bad_block2 = """    spheres[2] = (Sphere){
        .center = {4.0f, -1.0f, -6.0f},
        .radius = 2.0f,
        .mat = {.r = 100, .g = 100, .b = 255, .reflective = 0, .metallic = 0, .emissive = 0}
    };"""
good_block2 = "spheres[2].center.x=4.0f; spheres[2].center.y=-1.0f; spheres[2].center.z=-6.0f; spheres[2].radius=2.0f; spheres[2].mat.r=100; spheres[2].mat.g=100; spheres[2].mat.b=255; spheres[2].mat.reflective=0; spheres[2].mat.metallic=0; spheres[2].mat.emissive=0;"
text = text.replace(bad_block2, good_block2)

bad_block3 = """    spheres[3] = (Sphere){
        .center = {0.0f, -1004.0f, -10.0f},
        .radius = 1000.0f,
        .mat = {.r = 200, .g = 200, .b = 200, .reflective = 1, .metallic = 0, .emissive = 0}
    };"""
good_block3 = "spheres[3].center.x=0.0f; spheres[3].center.y=-1004.0f; spheres[3].center.z=-10.0f; spheres[3].radius=1000.0f; spheres[3].mat.r=200; spheres[3].mat.g=200; spheres[3].mat.b=200; spheres[3].mat.reflective=1; spheres[3].mat.metallic=0; spheres[3].mat.emissive=0;"
text = text.replace(bad_block3, good_block3)

bad_block4 = """    spheres[4] = (Sphere){
        .center = {-2.0f, 0.0f, -4.0f},
        .radius = 0.5f,
        .mat = {.r = 255, .g = 255, .b = 0, .reflective = 0, .metallic = 0, .emissive = 1}
    };"""
good_block4 = "spheres[4].center.x=-2.0f; spheres[4].center.y=0.0f; spheres[4].center.z=-4.0f; spheres[4].radius=0.5f; spheres[4].mat.r=255; spheres[4].mat.g=255; spheres[4].mat.b=0; spheres[4].mat.reflective=0; spheres[4].mat.metallic=0; spheres[4].mat.emissive=1;"
text = text.replace(bad_block4, good_block4)

bad_block5 = """    spheres[5] = (Sphere){
        .center = {2.0f, 1.0f, -3.0f},
        .radius = 0.8f,
        .mat = {.r = 0, .g = 255, .b = 255, .reflective = 1, .metallic = 1, .emissive = 0}
    };"""
good_block5 = "spheres[5].center.x=2.0f; spheres[5].center.y=1.0f; spheres[5].center.z=-3.0f; spheres[5].radius=0.8f; spheres[5].mat.r=0; spheres[5].mat.g=255; spheres[5].mat.b=255; spheres[5].mat.reflective=1; spheres[5].mat.metallic=1; spheres[5].mat.emissive=0;"
text = text.replace(bad_block5, good_block5)

text = text.replace("color = (Vec3){0.0f, 0.0f, 0.0f};", "color.x=0.0f; color.y=0.0f; color.z=0.0f;")
text = text.replace("light_dir = vec3_normalize((Vec3){0.5f, 1.0f, 0.3f});", "Vec3 ld; ld.x=0.5f; ld.y=1.0f; ld.z=0.3f; light_dir=vec3_normalize(ld);")
text = text.replace("color = vec3_add(vec3_scale(color, 1.0f - metallic), vec3_scale((Vec3){1.0f, 1.0f, 1.0f}, metallic));", 
"Vec3 w; w.x=1.0f; w.y=1.0f; w.z=1.0f; color = vec3_add(vec3_scale(color, 1.0f - metallic), vec3_scale(w, metallic));")

text = text.replace("Vec3 h = {1, 0, 0}; camera_pos = vec3_add(camera_pos, vec3_scale(h, ipd/2.0f));",
"Vec3 h; h.x=1; h.y=0; h.z=0; camera_pos = vec3_add(camera_pos, vec3_scale(h, ipd/2.0f));")

text = text.replace("Vec3 h = {1, 0, 0}; camera_pos = vec3_sub(camera_pos, vec3_scale(h, ipd/2.0f));",
"Vec3 h; h.x=1; h.y=0; h.z=0; camera_pos = vec3_sub(camera_pos, vec3_scale(h, ipd/2.0f));")

with open("exp4_vr_stereo.c", "w") as f:
    f.write(text)
