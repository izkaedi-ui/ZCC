import os

target_files = ["part2.c", "part3.c", "part4.c", "zcc.c"]

for f in target_files:
    if not os.path.exists(f):
        continue
    with open(f, "r") as file:
        lines = file.readlines()
    
    with open(f, "w") as file:
        for line in lines:
            if "ZCC:AST" not in line and "ZCC:DEBUG" not in line and "ZCC:PARSE" not in line and "ZCC:EMIT" not in line:
                file.write(line)
    
    print(f"Cleaned {f}")
