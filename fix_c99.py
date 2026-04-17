import os, glob, re

def main():
    for f in glob.glob("exp*.c"):
        with open(f, "r") as file:
            content = file.read()

        # Insert vec3_create and ray_create after vec3f definition
        if "vec3_create" not in content:
            content = re.sub(
                r'(typedef struct \{ float x, y, z; \} vec3f;)',
                r'\1\nstatic inline vec3f vec3_create(float x, float y, float z) { vec3f r; r.x=x; r.y=y; r.z=z; return r; }',
                content
            )
        
        if "ray_create" not in content and "typedef struct {\n    vec3f origin;\n    vec3f direction;\n} Ray;" in content:
            content = re.sub(
                r'(\} Ray;)',
                r'\1\nstatic inline Ray ray_create(vec3f o, vec3f d) { Ray r; r.origin=o; r.direction=d; return r; }',
                content
            )

        if "vec4_create" not in content:
            content = re.sub(
                r'(typedef struct \{ float x, y, z, w; \} vec4f;)',
                r'\1\nstatic inline vec4f vec4_create(float x, float y, float z, float w) { vec4f r; r.x=x; r.y=y; r.z=z; r.w=w; return r; }',
                content
            )

        # Fix return types
        content = re.sub(r'return\s*\(vec3f\)\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\};', r'return vec3_create(\1, \2, \3);', content)
        content = re.sub(r'return\s*\(vec4f\)\s*\{([^,]+),\s*([^,]+),\s*([^,]+),\s*([^}]+)\};', r'return vec4_create(\1, \2, \3, \4);', content)

        # Fix inline types
        content = re.sub(r'\(vec3f\)\s*\{([^,]+),\s*([^,]+),\s*([^}]+)\}', r'vec3_create(\1, \2, \3)', content)
        content = re.sub(r'\(vec4f\)\s*\{([^,]+),\s*([^,]+),\s*([^,]+),\s*([^}]+)\}', r'vec4_create(\1, \2, \3, \4)', content)
        content = re.sub(r'\(Ray\)\s*\{([^,]+),\s*([^}]+)\}', r'ray_create(\1, \2)', content)

        # Convert Scene initializers to C89 explicitly for exp1
        if "exp1" in f:
            content = content.replace("scene->spheres[0] = (Sphere){\n        .center = {0.0f, -100.5f, -1.0f},\n        .radius = 100.0f,\n        .mat = {.r = 128, .g = 128, .b = 128, .reflective = 0, .metallic = 0}\n    };", 
            "Sphere s0; s0.center = vec3_create(0.0f, -100.5f, -1.0f); s0.radius=100.0f; s0.mat.r=128; s0.mat.g=128; s0.mat.b=128; s0.mat.reflective=0; s0.mat.metallic=0; scene->spheres[0]=s0;")
            
            content = content.replace("scene->spheres[1] = (Sphere){\n        .center = {0.0f, 0.0f, -1.0f},\n        .radius = 0.5f,\n        .mat = {.r = 255, .g = 215, .b = 0, .reflective = 1, .metallic = 1}  // Gold\n    };",
            "Sphere s1; s1.center = vec3_create(0.0f, 0.0f, -1.0f); s1.radius=0.5f; s1.mat.r=255; s1.mat.g=215; s1.mat.b=0; s1.mat.reflective=1; s1.mat.metallic=1; scene->spheres[1]=s1;")
            
            content = content.replace("scene->spheres[2] = (Sphere){\n        .center = {-1.0f, 0.0f, -1.0f},\n        .radius = 0.5f,\n        .mat = {.r = 0, .g = 255, .b = 255, .reflective = 0, .metallic = 0}\n    };",
            "Sphere s2; s2.center = vec3_create(-1.0f, 0.0f, -1.0f); s2.radius=0.5f; s2.mat.r=0; s2.mat.g=255; s2.mat.b=255; s2.mat.reflective=0; s2.mat.metallic=0; scene->spheres[2]=s2;")
            
            content = content.replace("scene->spheres[3] = (Sphere){\n        .center = {1.0f, 0.0f, -1.0f},\n        .radius = 0.5f,\n        .mat = {.r = 255, .g = 0, .b = 255, .reflective = 1, .metallic = 0}\n    };",
            "Sphere s3; s3.center = vec3_create(1.0f, 0.0f, -1.0f); s3.radius=0.5f; s3.mat.r=255; s3.mat.g=0; s3.mat.b=255; s3.mat.reflective=1; s3.mat.metallic=0; scene->spheres[3]=s3;")
            
            content = content.replace("scene->spheres[4] = (Sphere){\n        .center = {0.0f, 2.0f, 0.0f},\n        .radius = 0.3f,\n        .mat = {.r = 255, .g = 255, .b = 255, .emissive = 1}\n    };",
            "Sphere s4; s4.center = vec3_create(0.0f, 2.0f, 0.0f); s4.radius=0.3f; s4.mat.r=255; s4.mat.g=255; s4.mat.b=255; s4.mat.emissive=1; scene->spheres[4]=s4;")

        with open(f, "w") as file:
            file.write(content)

if __name__ == "__main__":
    main()
