import sys
with open('zcc_doom.asm') as f:
    text = f.read()

start = text.find('<Z_Free>:')
end = text.find('<Z_Malloc>:', start)
print(text[start:end])
