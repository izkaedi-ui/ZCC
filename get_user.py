import sys
with open('zcc_doom.asm') as f:
    text = f.read()

start = text.find('<Z_Malloc>:')
end = text.find('<Z_FreeTags>:', start)
print(text[start:end])
