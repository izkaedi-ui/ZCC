import math

class SimpleRNG:
    def __init__(self, seed=1337):
        self.state = seed
    def random(self):
        self.state = (self.state * 1103515245 + 12345) & 0x7fffffff
        return self.state / 0x7fffffff
    def randint(self, a, b):
        return a + int(self.random() * (b - a + 1))
    def uniform(self, a, b):
        return a + self.random() * (b - a)

my_rand = SimpleRNG()

def generate_still():
    width, height = 3840, 2160  # 4K resolution
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #030006;">')
    
    svg.append('<defs>')
    
    # Gradients
    svg.append('''
        <radialGradient id="sunGrad" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="1"/>
            <stop offset="10%" stop-color="#FFD700" stop-opacity="1"/>
            <stop offset="30%" stop-color="#FF0055" stop-opacity="0.8"/>
            <stop offset="70%" stop-color="#4B0082" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#030006" stop-opacity="0"/>
        </radialGradient>
        
        <linearGradient id="buildingFront" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#00FFFF" stop-opacity="0.3"/>
            <stop offset="100%" stop-color="#030006" stop-opacity="0.9"/>
        </linearGradient>

        <linearGradient id="buildingSide" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FF00FF" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#030006" stop-opacity="0.9"/>
        </linearGradient>
        
        <filter id="bloom">
            <feGaussianBlur stdDeviation="8" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
        
        <filter id="chromatic">
            <feOffset dx="-6" dy="0" in="SourceGraphic" result="red-shift"/>
            <feOffset dx="6" dy="0" in="SourceGraphic" result="blue-shift"/>
            <feColorMatrix type="matrix" values="1 0 0 0 0  0 0 0 0 0  0 0 0 0 0  0 0 0 1 0" in="red-shift" result="red"/>
            <feColorMatrix type="matrix" values="0 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 1 0" in="blue-shift" result="cyan"/>
            <feBlend mode="screen" in="red" in2="cyan" result="combo"/>
            <feMerge>
                <feMergeNode in="combo"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    ''')
    svg.append('</defs>')

    # Background
    svg.append(f'<rect width="{width}" height="{height}" fill="#030006"/>')
    
    # Synthwave Sun
    # A massive setting sun at the horizon
    horizon_y = height * 0.6
    svg.append(f'<circle cx="{cx}" cy="{horizon_y}" r="600" fill="url(#sunGrad)"/>')
    # Draw horizontal slits through the sun (synthwave style)
    for i in range(15):
        y_pos = horizon_y + (i * 40)
        thickness = 5 + (i * 2)
        svg.append(f'<rect x="{cx-600}" y="{y_pos}" width="1200" height="{thickness}" fill="#030006"/>')

    svg.append('<g opacity="0.3">')
    x = 0
    while x < width:
        bw = my_rand.randint(40, 150)
        bh = my_rand.randint(200, 600)
        svg.append(f'<rect x="{x}" y="{horizon_y - bh}" width="{bw}" height="{bh}" fill="#110022"/>')
        x += bw + my_rand.randint(0, 20)
    svg.append('</g>')

    # Cyber Grid Ground
    svg.append(f'<g transform="translate(0, {horizon_y})">')
    # Horizon line
    svg.append(f'<line x1="0" y1="0" x2="{width}" y2="0" stroke="#00FFFF" stroke-width="6" filter="url(#bloom)"/>')
    
    # Horizontal grid lines
    ground_h = height - horizon_y
    for i in range(1, 40):
        y = (i**2.2) * 0.5
        if y > ground_h: break
        opacity = 1.0 - (i/40.0)
        sw = 1 + (y/300.0)
        svg.append(f'<line x1="0" y1="{y}" x2="{width}" y2="{y}" stroke="#FF00FF" stroke-width="{sw}" opacity="{opacity}"/>')
        
    # Vertical grid lines
    for i in range(-50, 50):
        dx = (i * 100)
        x_bottom = cx + (dx * 15)
        x_top = cx + (dx * 0.5)
        svg.append(f'<line x1="{x_top}" y1="0" x2="{x_bottom}" y2="{ground_h}" stroke="#FF00FF" stroke-width="2" opacity="0.4"/>')
    svg.append('</g>')

    # Foreground buildings (isometric 3D style)
    svg.append('<g filter="url(#chromatic)">')
    buildings = [
        # x, y_bottom, width, depth, height
        (200, height, 300, 150, 800),
        (600, height+100, 200, 100, 1200),
        (900, height-50, 250, 150, 900),
        (width-500, height-100, 200, 150, 700),
        (width-800, height+200, 350, 100, 1400)
    ]
    for b in buildings:
        bx, by, bw, bd, bh = b
        # Front face
        svg.append(f'<polygon points="{bx},{by} {bx+bw},{by} {bx+bw},{by-bh} {bx},{by-bh}" fill="url(#buildingFront)" stroke="#00FFFF" stroke-width="2"/>')
        # Side face (isometric projection)
        svg.append(f'<polygon points="{bx+bw},{by} {bx+bw+bd},{by-bd*0.5} {bx+bw+bd},{by-bh-bd*0.5} {bx+bw},{by-bh}" fill="url(#buildingSide)" stroke="#FF00FF" stroke-width="2"/>')
        # Top face
        svg.append(f'<polygon points="{bx},{by-bh} {bx+bw},{by-bh} {bx+bw+bd},{by-bh-bd*0.5} {bx+bd},{by-bh-bd*0.5}" fill="#0A001F" stroke="#FFFFFF" stroke-width="2"/>')
        
        # Windows on front
        for win_y in range(int(by-bh+80), int(by-80), 60):
            if my_rand.random() > 0.2:
                svg.append(f'<rect x="{bx+30}" y="{win_y}" width="{bw-60}" height="20" fill="#00FFFF" opacity="0.8" filter="url(#bloom)"/>')

    svg.append('</g>')
    
    # Epic Overlay Typography
    svg.append('<g font-family="Courier New, monospace" text-anchor="middle">')
    svg.append(f'<text x="{cx}" y="250" font-size="160" font-weight="900" fill="#FFFFFF" letter-spacing="80" filter="url(#chromatic)">H A S H _ C I T Y</text>')
    svg.append(f'<text x="{cx}" y="320" font-size="30" fill="#FF00FF" letter-spacing="20" opacity="0.8">STATIC_CANVAS_RENDER // ZERO_ANIMATION</text>')
    
    # Left targeting data
    svg.append(f'<text x="150" y="200" text-anchor="start" font-size="32" fill="#00FFFF" opacity="0.6">LAT: 35.6895</text>')
    svg.append(f'<text x="150" y="250" text-anchor="start" font-size="32" fill="#00FFFF" opacity="0.6">LON: 139.6917</text>')
    svg.append(f'<text x="150" y="300" text-anchor="start" font-size="32" fill="#00FFFF" opacity="0.6">VRAM: 14.2TB</text>')
    
    svg.append('</g>')

    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_neural_city_still.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Still generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_still()
