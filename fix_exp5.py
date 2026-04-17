with open("exp5_physics_engine.c", "r") as f:
    text = f.read()

text = text.replace("return (Vec3){0.0f, 1.0f, 0.0f};", "Vec3 r; r.x=0.0f; r.y=1.0f; r.z=0.0f; return r;")
text = text.replace("return (Vec3){-q.x, -q.y, -q.z};", "Vec3 r; r.x=-q.x; r.y=-q.y; r.z=-q.z; return r;")

text = text.replace("body->velocity = (Vec3){0.0f, 0.0f, 0.0f};", "body->velocity.x=0.0f; body->velocity.y=0.0f; body->velocity.z=0.0f;")
text = text.replace("body->force = (Vec3){0.0f, 0.0f, 0.0f};", "body->force.x=0.0f; body->force.y=0.0f; body->force.z=0.0f;")
text = text.replace("body->angular_velocity = (Vec3){0.0f, 0.0f, 0.0f};", "body->angular_velocity.x=0.0f; body->angular_velocity.y=0.0f; body->angular_velocity.z=0.0f;")
text = text.replace("body->torque = (Vec3){0.0f, 0.0f, 0.0f};", "body->torque.x=0.0f; body->torque.y=0.0f; body->torque.z=0.0f;")

text = text.replace("rigidbody_init(&bodies[0], (Vec3){0.0f, -5.0f, 0.0f}, 0.0f, 10.0f, 128, 128, 128);", 
"Vec3 p0; p0.x=0.0f; p0.y=-5.0f; p0.z=0.0f; rigidbody_init(&bodies[0], p0, 0.0f, 10.0f, 128, 128, 128);")
text = text.replace("rigidbody_init(&bodies[1], (Vec3){0.0f, 5.0f, 0.0f}, 1.0f, 0.5f, 255, 0, 0);",
"Vec3 p1; p1.x=0.0f; p1.y=5.0f; p1.z=0.0f; rigidbody_init(&bodies[1], p1, 1.0f, 0.5f, 255, 0, 0);")
text = text.replace("rigidbody_init(&bodies[2], (Vec3){-2.0f, 7.0f, 0.0f}, 1.0f, 0.5f, 0, 255, 0);",
"Vec3 p2; p2.x=-2.0f; p2.y=7.0f; p2.z=0.0f; rigidbody_init(&bodies[2], p2, 1.0f, 0.5f, 0, 255, 0);")
text = text.replace("rigidbody_init(&bodies[3], (Vec3){2.0f, 6.0f, 0.0f}, 1.0f, 0.5f, 0, 0, 255);",
"Vec3 p3; p3.x=2.0f; p3.y=6.0f; p3.z=0.0f; rigidbody_init(&bodies[3], p3, 1.0f, 0.5f, 0, 0, 255);")
text = text.replace("rigidbody_init(&bodies[4], (Vec3){-1.0f, 9.0f, 0.0f}, 0.8f, 0.4f, 255, 255, 0);",
"Vec3 p4; p4.x=-1.0f; p4.y=9.0f; p4.z=0.0f; rigidbody_init(&bodies[4], p4, 0.8f, 0.4f, 255, 255, 0);")
text = text.replace("rigidbody_init(&bodies[5], (Vec3){1.0f, 8.0f, 0.0f}, 0.8f, 0.4f, 255, 0, 255);",
"Vec3 p5; p5.x=1.0f; p5.y=8.0f; p5.z=0.0f; rigidbody_init(&bodies[5], p5, 0.8f, 0.4f, 255, 0, 255);")
text = text.replace("rigidbody_init(&bodies[6], (Vec3){0.0f, 11.0f, 0.0f}, 0.6f, 0.3f, 0, 255, 255);",
"Vec3 p6; p6.x=0.0f; p6.y=11.0f; p6.z=0.0f; rigidbody_init(&bodies[6], p6, 0.6f, 0.3f, 0, 255, 255);")
text = text.replace("rigidbody_init(&bodies[7], (Vec3){-0.5f, 13.0f, 0.0f}, 0.5f, 0.3f, 255, 128, 0);",
"Vec3 p7; p7.x=-0.5f; p7.y=13.0f; p7.z=0.0f; rigidbody_init(&bodies[7], p7, 0.5f, 0.3f, 255, 128, 0);")

text = text.replace("bodies[2].velocity = (Vec3){1.0f, 0.0f, 0.0f};", "bodies[2].velocity.x=1.0f; bodies[2].velocity.y=0.0f; bodies[2].velocity.z=0.0f;")
text = text.replace("bodies[3].velocity = (Vec3){-1.0f, 0.0f, 0.0f};", "bodies[3].velocity.x=-1.0f; bodies[3].velocity.y=0.0f; bodies[3].velocity.z=0.0f;")
text = text.replace("bodies[4].angular_velocity = (Vec3){0.0f, 0.0f, 2.0f};", "bodies[4].angular_velocity.x=0.0f; bodies[4].angular_velocity.y=0.0f; bodies[4].angular_velocity.z=2.0f;")

text = text.replace("bodies[i].force = vec3_add(bodies[i].force, (Vec3){0.0f, -9.81f * bodies[i].mass, 0.0f});",
"Vec3 grav; grav.x=0.0f; grav.y=-9.81f * bodies[i].mass; grav.z=0.0f; bodies[i].force = vec3_add(bodies[i].force, grav);")

with open("exp5_physics_engine.c", "w") as f:
    f.write(text)
