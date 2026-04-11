import fuzz_zcc, random
rng = random.Random(42)
for i in range(50):
    k = rng.randint(1, 4)
    funcs = [fuzz_zcc.gen_func(rng, j) for j in range(k)]
    src = fuzz_zcc.build_src(funcs)
    if i in [9, 16, 41]:
        with open(f"seed_{i}.c", "w") as f:
            f.write(src)
        print(f"Dumped seed_{i}.c")
