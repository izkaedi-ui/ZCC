import zipfile
with zipfile.ZipFile('zcc_ready.zip') as z:
    content = z.read('zcc_cloud_export/zcc.c').decode('utf-8')
parts = content.split('/* ZKAEDI FORCE RENDER CACHE INVALIDATION */')
# part1 is parts[0], part2 is parts[1], part3 is parts[2]
part3 = parts[2].strip() + '\n/* ZKAEDI FORCE RENDER CACHE INVALIDATION */\n'
with open('part3.c', 'w') as f:
    f.write(part3)
print('Restored part3.c! length:', len(part3))
