import random
from fuzz_zcc import gen_func, build_src

rng = random.Random(42)

for i in range(50):
    funcs = [gen_func(rng, j) for j in range(rng.randint(1, 4))]
    src = build_src(funcs)
    if i in [9, 16, 41]:
        with open(f"fail_ast_{i}.c", "w") as f:
            f.write(src)
