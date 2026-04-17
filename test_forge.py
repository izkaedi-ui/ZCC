import re
text = open('curl_ir_raw.txt', 'r').read()
funcs = re.findall(r'(; func .*?; end .*? nodes=\d+)', text, re.DOTALL)
print(len(funcs))
