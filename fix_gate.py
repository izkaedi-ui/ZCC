c = open("part4.c").read()
old = "return 1;\n}\n\nvoid codegen_func"
new = "return 0;\n}\n\nvoid codegen_func"
if old in c:
    c = c.replace(old, new)
    open("part4.c","w").write(c)
    print("FIXED")
else:
    print("NOT FOUND")
