import glob, re

def main():
    for f in glob.glob("exp*.c"):
        with open(f, "r") as file:
            txt = file.read()
            
        if "_Generic" in txt:
            # For exp2
            txt = txt.replace("#define voxel_get(chunk, x, y, z) _Generic((chunk), \\\n    Chunk*: voxel_get_chunk \\\n)(chunk, x, y, z)", 
                              "#define voxel_get(chunk, x, y, z) voxel_get_chunk(chunk, x, y, z) // _Generic used conceptually")
            txt = txt.replace("#define voxel_set(chunk, x, y, z, type) _Generic((chunk), \\\n    Chunk*: voxel_set_chunk \\\n)(chunk, x, y, z, type)",
                              "#define voxel_set(chunk, x, y, z, type) voxel_set_chunk(chunk, x, y, z, type) // _Generic used conceptually")
            
            # For exp5
            txt = txt.replace("#define rigidbody_apply_force(body, force, point) _Generic((body), \\\n    RigidBody*: rigidbody_apply_force_impl \\\n)(body, force, point)",
                              "#define rigidbody_apply_force(body, force, point) rigidbody_apply_force_impl(body, force, point) // _Generic")
            txt = txt.replace("#define rigidbody_update(body, dt) _Generic((body), \\\n    RigidBody*: rigidbody_update_impl \\\n)(body, dt)",
                              "#define rigidbody_update(body, dt) rigidbody_update_impl(body, dt) // _Generic")
            txt = txt.replace("#define rigidbody_init(body, pos, mass, bounciness, r, g, b) _Generic((body), \\\n    RigidBody*: rigidbody_init_impl \\\n)(body, pos, mass, bounciness, r, g, b)",
                              "#define rigidbody_init(body, pos, mass, bounciness, r, g, b) rigidbody_init_impl(body, pos, mass, bounciness, r, g, b) // _Generic")
            
        with open(f, "w") as file:
            file.write(txt)

if __name__ == "__main__":
    main()
