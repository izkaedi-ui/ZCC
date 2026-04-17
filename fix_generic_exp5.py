with open("exp5_physics_engine.c", "r") as f:
    text = f.read()

bad1 = """#define vec3_add(a, b) _Generic((a), \\
    Vec3: vec3_add_impl \\
)(a, b)"""
good1 = "#define vec3_add(a, b) vec3_add_impl(a, b)"

bad2 = """#define vec3_sub(a, b) _Generic((a), \\
    Vec3: vec3_sub_impl \\
)(a, b)"""
good2 = "#define vec3_sub(a, b) vec3_sub_impl(a, b)"

bad3 = """#define vec3_scale(v, s) _Generic((v), \\
    Vec3: vec3_scale_impl \\
)(v, s)"""
good3 = "#define vec3_scale(v, s) vec3_scale_impl(v, s)"

bad4 = """#define vec3_dot(a, b) _Generic((a), \\
    Vec3: vec3_dot_impl \\
)(a, b)"""
good4 = "#define vec3_dot(a, b) vec3_dot_impl(a, b)"

text = text.replace(bad1, good1)
text = text.replace(bad2, good2)
text = text.replace(bad3, good3)
text = text.replace(bad4, good4)

with open("exp5_physics_engine.c", "w") as f:
    f.write(text)
