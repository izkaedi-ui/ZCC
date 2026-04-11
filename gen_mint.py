import math

class SimpleRNG:
    def __init__(self, seed=12345):
        self.state = seed
    def random(self):
        self.state = (self.state * 1103515245 + 12345) & 0x7fffffff
        return self.state / 0x7fffffff
    def randint(self, a, b):
        return a + int(self.random() * (b - a + 1))
    def uniform(self, a, b):
        return a + self.random() * (b - a)

my_rand = SimpleRNG()

def generate_mint():
    width, height = 1920, 1080
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #0D001A; overflow: hidden;">')
    
    svg.append('<defs>')
    
    # Gradients
    svg.append('''
        <linearGradient id="skyGrad" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#050011" stop-opacity="1"/>
            <stop offset="60%" stop-color="#1A0033" stop-opacity="1"/>
            <stop offset="100%" stop-color="#FF00AA" stop-opacity="0.3"/>
        </linearGradient>

        <linearGradient id="sunMintGrad" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#00FFAA" stop-opacity="1"/>
            <stop offset="40%" stop-color="#00FFBB" stop-opacity="0.9"/>
            <stop offset="100%" stop-color="#E6007A" stop-opacity="0.8"/>
        </linearGradient>
        
        <linearGradient id="mountainGrad" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#00FFBB" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#0D001A" stop-opacity="1"/>
        </linearGradient>
        
        <filter id="mintBloom">
            <feGaussianBlur stdDeviation="6" result="blur_small"/>
            <feGaussianBlur stdDeviation="15" result="blur_large"/>
            <feMerge>
                <feMergeNode in="blur_large"/>
                <feMergeNode in="blur_small"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
        
        <filter id="magentaBloom">
            <feGaussianBlur stdDeviation="8" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    ''')
    
    # Ground mask for the animated grid
    svg.append(f'''
        <clipPath id="groundClip">
            <rect x="0" y="{height * 0.55}" width="{width}" height="{height * 0.45}"/>
        </clipPath>
    ''')
    svg.append('</defs>')

    horizon_y = height * 0.55

    # Sky Background
    svg.append(f'<rect width="{width}" height="{horizon_y}" fill="url(#skyGrad)"/>')

    # Starfield Background
    svg.append('<g opacity="0.6">')
    for i in range(150):
        sx = my_rand.randint(0, width)
        sy = my_rand.randint(0, int(horizon_y))
        r = my_rand.uniform(0.5, 1.5)
        color = "#00FFBB" if my_rand.random() > 0.5 else "#FF00AA"
        svg.append(f'<circle cx="{sx}" cy="{sy}" r="{r}" fill="{color}"/>')
    svg.append('</g>')

    # Mint-Sunset Horizon Sun
    aurora_radius = 450
    svg.append(f'<circle cx="{cx}" cy="{horizon_y}" r="{aurora_radius}" fill="url(#sunMintGrad)" filter="url(#mintBloom)" opacity="0.9"/>')
    # Synthwave Sun lines (The horizontal cuts)
    svg.append('<g>')
    for i in range(18):
        y_cut = horizon_y - (i * 25)
        cut_height = 2 + (i * 1.5)
        svg.append(f'<rect x="{cx - aurora_radius}" y="{y_cut}" width="{aurora_radius*2}" height="{cut_height}" fill="#16002A"/>')
    svg.append('</g>')

    # Neon Mint Polygon Mountains
    svg.append('<g opacity="0.8">')
    
    def make_mountain(x, w, h, z_index):
        pts = f"{x - w/2},{horizon_y} {x},{horizon_y - h} {x + w/2},{horizon_y}"
        opacity = z_index * 0.2
        stroke_width = 1.5
        svg.append(f'<polygon points="{pts}" fill="url(#mountainGrad)" stroke="#00FFBB" stroke-width="{stroke_width}" opacity="{opacity}"/>')
        svg.append(f'<line x1="{x}" y1="{horizon_y}" x2="{x}" y2="{horizon_y - h}" stroke="#00FFBB" stroke-width="{stroke_width}" opacity="{opacity}"/>')

    # Back layer (less opacity/smaller)
    for i in range(20):
        xx = my_rand.randint(-400, width+400)
        make_mountain(xx, my_rand.randint(200, 400), my_rand.randint(150, 300), 2)
        
    # Front layer
    for i in range(12):
        xx = my_rand.randint(-400, width+400)
        if cx - 300 < xx < cx + 300: continue
        make_mountain(xx, my_rand.randint(300, 650), my_rand.randint(250, 480), 3)

    svg.append('</g>')

    # Ground base color
    svg.append(f'<rect x="0" y="{horizon_y}" width="{width}" height="{height-horizon_y}" fill="#05000A"/>')
    
    # Ground Animated Infinite Grid
    svg.append('<g clip-path="url(#groundClip)">')
    
    svg.append('<g filter="url(#magentaBloom)">')
    # Radiant vertical perspective lines (Static)
    for i in range(-30, 31):
        dx = i * 200
        x_bottom = cx + dx
        x_top = cx + (i * 20)
        # Taper opacity near the edge
        opacity = 0.8 - (abs(i) / 35.0)
        svg.append(f'<line x1="{x_top}" y1="{horizon_y}" x2="{x_bottom}" y2="{height + 400}" stroke="#FF00AA" stroke-width="3" opacity="{opacity}"/>')
    svg.append('</g>')
    
    # Sweeping horizontal lines (Motion effect)
    svg.append('<g>')
    num_horiz = 25
    for i in range(num_horiz):
        start_y = horizon_y
        end_y = height + 800
        dur_sec = 4.0
        delay = (i / num_horiz) * dur_sec
        # perspective ease-in
        svg.append(f'''
            <line x1="-2000" y1="{start_y}" x2="{width+2000}" y2="{start_y}" stroke="#00FFBB" stroke-width="1" opacity="0">
                <animate attributeName="y1" values="{start_y}; {end_y}" keyTimes="0; 1" calcMode="spline" keySplines="0.4 0 1 1" begin="-{delay}s" dur="{dur_sec}s" repeatCount="indefinite" />
                <animate attributeName="y2" values="{start_y}; {end_y}" keyTimes="0; 1" calcMode="spline" keySplines="0.4 0 1 1" begin="-{delay}s" dur="{dur_sec}s" repeatCount="indefinite" />
                <animate attributeName="stroke-width" values="0.5; 15" keyTimes="0; 1" calcMode="spline" keySplines="0.4 0 1 1" begin="-{delay}s" dur="{dur_sec}s" repeatCount="indefinite" />
                <animate attributeName="opacity" values="0.0; 0.8; 0.0" keyTimes="0; 0.8; 1" begin="-{delay}s" dur="{dur_sec}s" repeatCount="indefinite" />
            </line>
        ''')
    svg.append('</g>')
    
    # Glowing horizon edge
    svg.append(f'<line x1="0" y1="{horizon_y}" x2="{width}" y2="{horizon_y}" stroke="#00FFBB" stroke-width="8" filter="url(#mintBloom)"/>')
    svg.append(f'<line x1="0" y1="{horizon_y}" x2="{width}" y2="{horizon_y}" stroke="#FFFFFF" stroke-width="3"/>')

    svg.append('</g>') # End Ground

    # High-Tech Retrowave HUD
    svg.append('<g font-family="Courier New, monospace">')
    
    # Floating Title
    svg.append(f'''
        <text x="{cx}" y="180" font-size="110" font-weight="900" fill="#00FFBB" letter-spacing="40" text-anchor="middle" filter="url(#mintBloom)">
            N E O N // M I N T
        </text>
        <text x="{cx}" y="180" font-size="110" font-weight="900" fill="#FFFFFF" letter-spacing="40" text-anchor="middle">
            N E O N // M I N T
        </text>
    ''')

    # Telemetry Left
    svg.append('''
        <g opacity="0.9" font-size="22" fill="#00FFBB">
            <text x="80" y="80" fill="#FFFFFF" font-weight="900">SYSTEM: ZKAEDI_SYNTH_CORE</text>
            <text x="80" y="115">OVERDRIVE: ACTIVE</text>
            <text x="80" y="145">HORIZON_SYNC: 99.98%</text>
            <text x="80" y="175">OP_MODE: VAPORWAVE</text>
        </g>
    ''')

    # Framing HUD brackets
    svg.append('''
        <path d="M 50 250 L 50 50 L 250 50" fill="none" stroke="#FF00AA" stroke-width="4" filter="url(#magentaBloom)"/>
        <path d="M 1870 250 L 1870 50 L 1670 50" fill="none" stroke="#FF00AA" stroke-width="4" filter="url(#magentaBloom)"/>
        <path d="M 50 830 L 50 1030 L 250 1030" fill="none" stroke="#FF00AA" stroke-width="4" filter="url(#magentaBloom)"/>
        <path d="M 1870 830 L 1870 1030 L 1670 1030" fill="none" stroke="#FF00AA" stroke-width="4" filter="url(#magentaBloom)"/>
    ''')
    
    # Little retro grid in the center bottom
    svg.append(f'''
        <g transform="translate({cx-150}, {height-150})">
            <rect x="0" y="0" width="300" height="100" fill="none" stroke="#00FFBB" stroke-width="2"/>
            <text x="150" y="55" font-size="30" fill="#FF00AA" text-anchor="middle" font-weight="bold">MAXIMAL_DRIFT</text>
        </g>
    ''')

    svg.append('</g>')

    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_neon_mint.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Mint generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_mint()
