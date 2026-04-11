import random
from fuzz_zcc import gen_func, build_src
import os

os.makedirs("seeds", exist_ok=True)
rng = random.Random(42)

for i in range(50):
    funcs = [gen_func(rng, j) for j in range(rng.randint(1, 4))]
    src = build_src(funcs)
    with open(f"seeds/seed_{i}.c", "w") as f:
        f.write(src)
print("Dumped 50 seeds")
