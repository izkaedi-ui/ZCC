with open("exp4_vr_stereo.c", "r") as f:
    text = f.read()

bad1 = """    VRConfig config = {
        .ipd_mm = 64,                    /* 64mm IPD (average) */
        .fov_degrees = 110,              /* 110° FOV */
        .target_fps = 90,                /* 90 FPS target */
        .distortion_enabled = 1,
        .chromatic_aberration = 0,
        .async_reprojection = 1,
        .low_persistence = 1
    };"""
good1 = """    VRConfig config;
    config.ipd_mm = 64;
    config.fov_degrees = 110;
    config.target_fps = 90;
    config.distortion_enabled = 1;
    config.chromatic_aberration = 0;
    config.async_reprojection = 1;
    config.low_persistence = 1;"""

bad2 = """    Eye left_eye = {
        .position = {-eye_separation, 0.0f, 0.0f},
        .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
        .fov = (float)config.fov_degrees
    };"""
good2 = """    Eye left_eye;
    left_eye.position.x = -eye_separation; left_eye.position.y = 0.0f; left_eye.position.z = 0.0f;
    left_eye.rotation.w = 1.0f; left_eye.rotation.x = 0.0f; left_eye.rotation.y = 0.0f; left_eye.rotation.z = 0.0f;
    left_eye.fov = (float)config.fov_degrees;"""

bad3 = """    Eye right_eye = {
        .position = {eye_separation, 0.0f, 0.0f},
        .rotation = {1.0f, 0.0f, 0.0f, 0.0f},
        .fov = (float)config.fov_degrees
    };"""
good3 = """    Eye right_eye;
    right_eye.position.x = eye_separation; right_eye.position.y = 0.0f; right_eye.position.z = 0.0f;
    right_eye.rotation.w = 1.0f; right_eye.rotation.x = 0.0f; right_eye.rotation.y = 0.0f; right_eye.rotation.z = 0.0f;
    right_eye.fov = (float)config.fov_degrees;"""

text = text.replace(bad1, good1)
text = text.replace(bad2, good2)
text = text.replace(bad3, good3)

with open("exp4_vr_stereo.c", "w") as f:
    f.write(text)
