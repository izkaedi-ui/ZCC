import re

def fix(filename):
    with open(filename, "r") as f:
        text = f.read()
    
    if "vec3_create" in text and "static inline Vec3 vec3_create" not in text:
        text = re.sub(
            r'typedef struct \{\s*float x, y, z;\s*\} Vec3;',
            r'typedef struct { float x, y, z; } Vec3;\nstatic inline Vec3 vec3_create(float x, float y, float z) { Vec3 r; r.x=x; r.y=y; r.z=z; return r; }',
            text
        )
        with open(filename, "w") as f:
            f.write(text)

fix("exp4_vr_stereo.c")
fix("exp5_physics_engine.c")
