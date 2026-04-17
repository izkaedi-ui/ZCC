import re

with open("exp2_voxel_engine.c", "r") as f:
    text = f.read()

# Fix Voxel returns
text = text.replace('return (Voxel){.type = VOXEL_AIR, .metadata = 0};', 
                    'Voxel r; r.type=VOXEL_AIR; r.metadata=0; return r;')

# Fix Vec3 returns
text = text.replace('return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};',
                    'Vec3 r; r.x=a.x+b.x; r.y=a.y+b.y; r.z=a.z+b.z; return r;')
text = text.replace('return (Vec3){v.x * s, v.y * s, v.z * s};',
                    'Vec3 r; r.x=v.x*s; r.y=v.y*s; r.z=v.z*s; return r;')

# Fix Color assignments
text = text.replace('color = (Color){255, 0, 255};',
                    'color.r=255; color.g=0; color.b=255;')
text = text.replace('color = (Color){\n                    (unsigned char)(135 * brightness),\n                    (unsigned char)(206 * brightness),\n                    (unsigned char)(235 * brightness)\n                };',
                    'color.r=(unsigned char)(135 * brightness); color.g=(unsigned char)(206 * brightness); color.b=(unsigned char)(235 * brightness);')

with open("exp2_voxel_engine.c", "w") as f:
    f.write(text)
