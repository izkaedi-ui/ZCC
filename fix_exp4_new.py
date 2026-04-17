with open("exp4_vr_stereo.c", "r") as f:
    text = f.read()

# Replace struct literals:
# spheres[0] = (Sphere){\n        .position = {0.0f, -100.5f, -3.0f},\n        .radius = 100.0f,\n        .r = 128, .g = 128, .b = 128\n    };

good_spheres0 = "spheres[0].position.x=0.0f; spheres[0].position.y=-100.5f; spheres[0].position.z=-3.0f; spheres[0].radius=100.0f; spheres[0].r=128; spheres[0].g=128; spheres[0].b=128;"
text = text.replace("    spheres[0] = (Sphere){\n        .position = {0.0f, -100.5f, -3.0f},\n        .radius = 100.0f,\n        .r = 128, .g = 128, .b = 128\n    };", good_spheres0)

good_spheres1 = "spheres[1].position.x=0.0f; spheres[1].position.y=0.0f; spheres[1].position.z=-3.0f; spheres[1].radius=0.5f; spheres[1].r=255; spheres[1].g=0; spheres[1].b=0;"
text = text.replace("    spheres[1] = (Sphere){\n        .position = {0.0f, 0.0f, -3.0f},\n        .radius = 0.5f,\n        .r = 255, .g = 0, .b = 0\n    };", good_spheres1)

good_spheres2 = "spheres[2].position.x=-1.5f; spheres[2].position.y=0.0f; spheres[2].position.z=-3.0f; spheres[2].radius=0.5f; spheres[2].r=0; spheres[2].g=255; spheres[2].b=255;"
text = text.replace("    spheres[2] = (Sphere){\n        .position = {-1.5f, 0.0f, -3.0f},\n        .radius = 0.5f,\n        .r = 0, .g = 255, .b = 255\n    };", good_spheres2)

good_spheres3 = "spheres[3].position.x=1.5f; spheres[3].position.y=0.0f; spheres[3].position.z=-3.0f; spheres[3].radius=0.5f; spheres[3].r=255; spheres[3].g=0; spheres[3].b=255;"
text = text.replace("    spheres[3] = (Sphere){\n        .position = {1.5f, 0.0f, -3.0f},\n        .radius = 0.5f,\n        .r = 255, .g = 0, .b = 255\n    };", good_spheres3)

good_spheres4 = "spheres[4].position.x=0.0f; spheres[4].position.y=1.0f; spheres[4].position.z=-2.0f; spheres[4].radius=0.3f; spheres[4].r=255; spheres[4].g=255; spheres[4].b=0;"
text = text.replace("    spheres[4] = (Sphere){\n        .position = {0.0f, 1.0f, -2.0f},\n        .radius = 0.3f,\n        .r = 255, .g = 255, .b = 0\n    };", good_spheres4)

good_spheres5 = "spheres[5].position.x=0.0f; spheres[5].position.y=0.5f; spheres[5].position.z=-5.0f; spheres[5].radius=0.7f; spheres[5].r=0; spheres[5].g=255; spheres[5].b=0;"
text = text.replace("    spheres[5] = (Sphere){\n        .position = {0.0f, 0.5f, -5.0f},\n        .radius = 0.7f,\n        .r = 0, .g = 255, .b = 0\n    };", good_spheres5)

bad_config = """    VRConfig config = {
        .ipd_mm = 64,                    /* 64mm IPD (average) */
        .fov_degrees = 110,              /* 110° FOV */
        .target_fps = 90,                /* 90 FPS target */
        .distortion_enabled = 1,
        .chromatic_aberration = 0,
        .async_reprojection = 1,
        .low_persistence = 1
    };"""
good_config = """    VRConfig config;
    config.ipd_mm = 64;
    config.fov_degrees = 110;
    config.target_fps = 90;
    config.distortion_enabled = 1;
    config.chromatic_aberration = 0;
    config.async_reprojection = 1;
    config.low_persistence = 1;"""
text = text.replace(bad_config, good_config)

bad_left_eye = """    Eye left_eye = {
        .position = {-eye_separation, 0.0f, 0.0f},
        .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
        .fov = (float)config.fov_degrees
    };"""
good_left_eye = """    Eye left_eye;
    left_eye.position.x = -eye_separation; left_eye.position.y = 0.0f; left_eye.position.z = 0.0f;
    left_eye.rotation.w = 1.0f; left_eye.rotation.x = 0.0f; left_eye.rotation.y = 0.0f; left_eye.rotation.z = 0.0f;
    left_eye.fov = (float)config.fov_degrees;"""
text = text.replace(bad_left_eye, good_left_eye)

bad_right_eye = """    Eye right_eye = {
        .position = {eye_separation, 0.0f, 0.0f},
        .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
        .fov = (float)config.fov_degrees
    };"""
good_right_eye = """    Eye right_eye;
    right_eye.position.x = eye_separation; right_eye.position.y = 0.0f; right_eye.position.z = 0.0f;
    right_eye.rotation.w = 1.0f; right_eye.rotation.x = 0.0f; right_eye.rotation.y = 0.0f; right_eye.rotation.z = 0.0f;
    right_eye.fov = (float)config.fov_degrees;"""
text = text.replace(bad_right_eye, good_right_eye)

# (Vec3){...} -> fixed by fix_rest.py!
# But wait! I just restored exp4. I need to run `strip_headers.py`, `fix_rest.py`.

with open("exp4_vr_stereo.c", "w") as f:
    f.write(text)
