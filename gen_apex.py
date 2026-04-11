import math

class SimpleRNG:
    def __init__(self, seed=777):
        self.state = seed
    def random(self):
        self.state = (self.state * 1103515245 + 12345) & 0x7fffffff
        return self.state / 0x7fffffff
    def randint(self, a, b):
        return a + int(self.random() * (b - a + 1))
    def uniform(self, a, b):
        return a + self.random() * (b - a)

my_rand = SimpleRNG()

def generate_apex():
    width, height = 1920, 1080
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #010005; overflow: hidden;">')
    
    svg.append('<defs>')
    
    # Golden and Plasma Gradients
    svg.append('''
        <radialGradient id="spaceCore" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.8"/>
            <stop offset="10%" stop-color="#00FFFF" stop-opacity="0.5"/>
            <stop offset="40%" stop-color="#0011AA" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#010005" stop-opacity="0"/>
        </radialGradient>
        
        <filter id="godGlow">
            <feGaussianBlur stdDeviation="5" result="blur_small"/>
            <feGaussianBlur stdDeviation="20" result="blur_large"/>
            <feGaussianBlur stdDeviation="40" result="blur_mega"/>
            <feMerge>
                <feMergeNode in="blur_mega"/>
                <feMergeNode in="blur_large"/>
                <feMergeNode in="blur_small"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>

        <linearGradient id="ringGrad" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#FFD700" stop-opacity="1"/>
            <stop offset="50%" stop-color="#FF0055" stop-opacity="0.8"/>
            <stop offset="100%" stop-color="#00FFFF" stop-opacity="1"/>
        </linearGradient>
    ''')
    
    svg.append('</defs>')

    # Background Sky
    svg.append(f'<rect width="{width}" height="{height}" fill="#010005"/>')
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="700" fill="url(#spaceCore)" filter="url(#godGlow)"/>')

    # Background Nebulas/Stars
    svg.append('<g opacity="0.4">')
    for i in range(250):
        sx = my_rand.randint(0, width)
        sy = my_rand.randint(0, height)
        r = my_rand.uniform(0.5, 2.0)
        color = "#00FFFF" if my_rand.random() > 0.6 else "#FF0055"
        svg.append(f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="{color}"/>')
    svg.append('</g>')

    # 4D HYPERCUBE MATHEMATICS (The Metatron Core)
    vertices_4d = []
    for i in range(16):
        x = -1 if (i & 1) == 0 else 1
        y = -1 if (i & 2) == 0 else 1
        z = -1 if (i & 4) == 0 else 1
        w = -1 if (i & 8) == 0 else 1
        vertices_4d.append([x, y, z, w])

    edges = []
    for i in range(16):
        for j in range(i+1, 16):
            diffs = sum(1 for a, b in zip(vertices_4d[i], vertices_4d[j]) if a != b)
            if diffs == 1:
                edges.append((i, j))

    def rotate_xw(v, angle):
        x, y, z, w = v
        return [x*math.cos(angle) - w*math.sin(angle), y, z, x*math.sin(angle) + w*math.cos(angle)]

    def rotate_yz(v, angle):
        x, y, z, w = v
        return [x, y*math.cos(angle) - z*math.sin(angle), y*math.sin(angle) + z*math.cos(angle), w]

    def rotate_xyz(v3, rx, ry):
        x, y, z = v3
        # rot Y
        nx = x*math.cos(ry) - z*math.sin(ry)
        nz = x*math.sin(ry) + z*math.cos(ry)
        # rot X
        ny = y*math.cos(rx) - nz*math.sin(rx)
        nnz = y*math.sin(rx) + nz*math.cos(rx)
        return [nx, ny, nnz]

    num_frames = 60
    hyper_frames = []

    for frame in range(num_frames):
        angle_xw = (frame / num_frames) * 2 * math.pi
        angle_yz = (frame / num_frames) * 2 * math.pi  # Single phase rotation to loop perfectly
        
        path_data = []
        points_2d = []
        for v in vertices_4d:
            vr = rotate_xw(v, angle_xw)
            vr = rotate_yz(vr, angle_yz)
            # Project 4D to 3D
            dist4 = 2.8
            w_factor = 1.0 / (dist4 - vr[3])
            v3 = [vr[0]*w_factor, vr[1]*w_factor, vr[2]*w_factor]
            
            # Static 3D rotation so it looks dramatic
            v3r = rotate_xyz(v3, math.pi/6, math.pi/4)
            
            # Project 3D to 2D
            dist3 = 3.5
            z_factor = 450 / (dist3 - v3r[2])
            sx = cx + v3r[0]*z_factor
            sy = cy + v3r[1]*z_factor
            points_2d.append((sx, sy))
            
        for e in edges:
            p1 = points_2d[e[0]]
            p2 = points_2d[e[1]]
            path_data.append(f"M {p1[0]:.2f} {p1[1]:.2f} L {p2[0]:.2f} {p2[1]:.2f}")
            
        hyper_frames.append(" ".join(path_data))
        
    hyper_frames_str = "; ".join(hyper_frames) + "; " + hyper_frames[0]

    # Draw Majestic Outer Astrolabe Rings
    svg.append('<g filter="url(#godGlow)">')
    
    # Create 4 orbiting rings with intense geometry
    for ring_i in range(1, 6):
        rr = 180 + (ring_i * 75)
        sw = 4 if ring_i % 2 == 0 else 1.5
        dash = my_rand.randint(10, 50)
        space = my_rand.randint(20, 100)
        duration = 10 + ring_i * 8
        direction = 360 if ring_i % 2 == 0 else -360
        color = ["#FFD700", "#FF0055", "#00FFFF", "#FFFFFF", "#FF00AA"][ring_i-1]
        
        svg.append(f'<g>')
        # Animate the group spinning
        svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0 {cx} {cy}" to="{direction} {cx} {cy}" dur="{duration}s" repeatCount="indefinite"/>')
        
        svg.append(f'<circle cx="{cx}" cy="{cy}" r="{rr}" fill="none" stroke="{color}" stroke-width="{sw}" stroke-dasharray="{dash} {space}"/>')
        
        # Add little tracking nodes on the rings
        for knot in range(5):
            angle = knot * ((2*math.pi)/5)
            kx = cx + rr * math.cos(angle)
            ky = cy + rr * math.sin(angle)
            svg.append(f'<circle cx="{kx}" cy="{ky}" r="6" fill="#FFFFFF" stroke="{color}" stroke-width="2"/>')
            
        svg.append('</g>')

    # Inner intense energy sphere enclosing the tesseract
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="160" fill="none" stroke="#FFFFFF" stroke-width="2" opacity="0.3"/>')
    svg.append('</g>')

    # THE 4D HYPERCUBE
    svg.append(f'''
        <g filter="url(#godGlow)">
            <path d="{hyper_frames[0]}" fill="none" stroke="url(#ringGrad)" stroke-width="6" opacity="0.9" stroke-linecap="round" stroke-linejoin="round">
                <animate attributeName="d" values="{hyper_frames_str}" dur="8s" repeatCount="indefinite" />
            </path>
            <!-- Clone it in white for a hot core -->
            <path d="{hyper_frames[0]}" fill="none" stroke="#FFFFFF" stroke-width="2" opacity="1.0" stroke-linecap="round" stroke-linejoin="round">
                <animate attributeName="d" values="{hyper_frames_str}" dur="8s" repeatCount="indefinite" />
            </path>
        </g>
    ''')
    
    # High-Tech Retrowave HUD
    svg.append('<g font-family="Courier New, monospace" fill="#FFFFFF">')
    
    # Overlay lines framing the tesseract
    svg.append(f'<line x1="{cx}" y1="{cy-500}" x2="{cx}" y2="{cy-250}" stroke="#00FFFF" stroke-width="2" stroke-dasharray="10 5" filter="url(#godGlow)"/>')
    svg.append(f'<line x1="{cx}" y1="{cy+250}" x2="{cx}" y2="{cy+500}" stroke="#FF0055" stroke-width="2" stroke-dasharray="10 5" filter="url(#godGlow)"/>')
    
    # Screen Typography
    svg.append(f'''
        <text x="{cx}" y="120" font-size="60" font-weight="900" letter-spacing="40" text-anchor="middle" filter="url(#godGlow)">
            SERAPHIM_CORE_TESSERACT
        </text>
        <text x="{cx}" y="120" font-size="60" font-weight="900" opacity="0.9" letter-spacing="40" text-anchor="middle">
            SERAPHIM_CORE_TESSERACT
        </text>
    ''')

    # Telemetry
    svg.append(f'''
        <g opacity="0.8" font-size="24" fill="#00FFFF">
            <text x="120" y="100" fill="#FFFFFF" font-weight="900">DIMENSIONALITY: 4D</text>
            <text x="120" y="140">AXES: XW, YZ ACTIVE</text>
            <text x="120" y="180">MATRIX: HYPER-INTERPOLATION</text>
            <text x="120" y="220">VERTICES: 16_NODE (TESSERACT)</text>
            <text x="120" y="260">CLOCK: 8.0s LOOP_INF</text>
        </g>
    ''')
    
    # Bottom HUD
    svg.append(f'''
        <path d="M {cx-150} {height-100} L {cx+150} {height-100}" fill="none" stroke="#FF0055" stroke-width="6" filter="url(#godGlow)"/>
        <text x="{cx}" y="{height-50}" font-size="25" text-anchor="middle" fill="#FFFFFF" letter-spacing="20" opacity="0.7">
            ALGORITHM_NATIVE_VECTOR_MORPHING
        </text>
    ''')

    svg.append('</g>')

    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_apex_seraphim.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Apex Seraphim generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_apex()
