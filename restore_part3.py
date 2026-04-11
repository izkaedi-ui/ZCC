with open('zcc_pp.c', 'r', encoding='utf-8') as f:
    content = f.read()
parts = content.split('/* ZKAEDI FORCE RENDER CACHE INVALIDATION */')
# part 0 is before part1? No, part1 is parts[0], part2 is parts[1], part3 is parts[2]
part3_out = parts[2].strip() + '\n/* ZKAEDI FORCE RENDER CACHE INVALIDATION */\n'
with open('part3.c', 'w', encoding='utf-8') as f:
    f.write(part3_out)
print('Restored part3.c! bytes:', len(part3_out))
