import glob, re

def main():
    for f in glob.glob("exp*.c"):
        with open(f, "r") as file:
            txt = file.read()
            
        txt = re.sub(r'return \(Vec3\)\{([^,]+),\s*([^,]+),\s*([^}]+)\};', r'Vec3 r; r.x=\1; r.y=\2; r.z=\3; return r;', txt)
        txt = re.sub(r'return \(Quat\)\{([^,]+),\s*([^,]+),\s*([^,]+),\s*([^}]+)\};', r'Quat r; r.w=\1; r.x=\2; r.y=\3; r.z=\4; return r;', txt)

        # local variable assignment (Vec3){...}
        txt = re.sub(r'\(Vec3\)\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\}', r'vec3_create(\1, \2, \3)', txt)
        
        if "vec3_create" not in txt and "typedef struct {\n    float x, y, z;\n} Vec3;" in txt:
            txt = txt.replace("typedef struct {\n    float x, y, z;\n} Vec3;", 
            "typedef struct {\n    float x, y, z;\n} Vec3;\nstatic inline Vec3 vec3_create(float x, float y, float z) { Vec3 r; r.x=x; r.y=y; r.z=z; return r; }")
            
        elif "vec3_create" not in txt and "typedef struct { float x, y, z; } Vec3;" in txt:
            txt = txt.replace("typedef struct { float x, y, z; } Vec3;", 
            "typedef struct { float x, y, z; } Vec3;\nstatic inline Vec3 vec3_create(float x, float y, float z) { Vec3 r; r.x=x; r.y=y; r.z=z; return r; }")

        # Fix exp3 particle init
        txt = re.sub(r'particles\[i\]\s*=\s*\(Particle\)\{\s*\.pos\s*=\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\},\s*\.vel\s*=\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\},\s*\.life\s*=\s*([^,}]+),\s*\.color\s*=\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\}\s*\};',
                     r'particles[i].pos.x=\1; particles[i].pos.y=\2; particles[i].pos.z=\3; particles[i].vel.x=\4; particles[i].vel.y=\5; particles[i].vel.z=\6; particles[i].life=\7; particles[i].color.r=\8; particles[i].color.g=\9; particles[i].color.b=\10;',
                     txt)

        # Fix exp4 spheres
        for i in range(6):
            txt = re.sub(r'spheres\[' + str(i) + r'\]\s*=\s*\(Sphere\)\{\s*\.center\s*=\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\},\s*\.radius\s*=\s*([^,]+),\s*\.mat\s*=\s*\{([^,]+),\s*([^,]+),\s*([^,]+),\s*([^,]+),\s*([^,]+),\s*([^}]+)\}\s*\};',
                         r'spheres[' + str(i) + r'].center.x=\1; spheres[' + str(i) + r'].center.y=\2; spheres[' + str(i) + r'].center.z=\3; spheres[' + str(i) + r'].radius=\4; spheres[' + str(i) + r'].mat.r=\5; spheres[' + str(i) + r'].mat.g=\6; spheres[' + str(i) + r'].mat.b=\7; spheres[' + str(i) + r'].mat.reflective=\8; spheres[' + str(i) + r'].mat.metallic=\9; spheres[' + str(i) + r'].mat.emissive=\10;',
                         txt)

        with open(f, "w") as file:
            file.write(txt)

if __name__ == "__main__":
    main()
