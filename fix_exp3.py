with open("exp3_audio_visualizer.c", "r") as f:
    text = f.read()

bad_block = """        particles[i] = (Particle){
            .x = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f,
            .y = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f,
            .z = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f,
            .vx = 0.0f, .vy = 0.0f, .vz = 0.0f,
            .energy = 0.0f,
            .hue = (float)i / (float)num_particles,
            .lifetime = 0.0f
        };"""

good_block = """        particles[i].x = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f;
        particles[i].y = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f;
        particles[i].z = ((float)rand() / (float)RAND_MAX - 0.5f) * 20.0f;
        particles[i].vx = 0.0f; particles[i].vy = 0.0f; particles[i].vz = 0.0f;
        particles[i].energy = 0.0f;
        particles[i].hue = (float)i / (float)num_particles;
        particles[i].lifetime = 0.0f;
"""

if bad_block in text:
    text = text.replace(bad_block, good_block)

# Also fix the Complex struct literals!
text = text.replace("Complex wlen = {cosf(angle), sinf(angle)};", "Complex wlen; wlen.real=cosf(angle); wlen.imag=sinf(angle);")
text = text.replace("Complex w = {1.0f, 0.0f};", "Complex w; w.real=1.0f; w.imag=0.0f;")

with open("exp3_audio_visualizer.c", "w") as f:
    f.write(text)
