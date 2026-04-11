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

def generate_hero():
    width, height = 1920, 1080
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #010002; overflow: hidden;">')
    
    svg.append('<defs>')
    
    svg.append('''
        <radialGradient id="vortexGlow" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#000000" stop-opacity="1"/>
            <stop offset="15%" stop-color="#FFFFFF" stop-opacity="1"/>
            <stop offset="25%" stop-color="#FFD700" stop-opacity="0.8"/>
            <stop offset="50%" stop-color="#FF0055" stop-opacity="0.5"/>
            <stop offset="80%" stop-color="#4B0082" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#010002" stop-opacity="0"/>
        </radialGradient>

        <linearGradient id="jetTop" x1="0%" y1="100%" x2="0%" y2="0%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.9"/>
            <stop offset="20%" stop-color="#00FFFF" stop-opacity="0.7"/>
            <stop offset="100%" stop-color="#010002" stop-opacity="0"/>
        </linearGradient>

        <linearGradient id="jetBottom" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FF00FF" stop-opacity="0.8"/>
            <stop offset="40%" stop-color="#4B0082" stop-opacity="0.4"/>
            <stop offset="100%" stop-color="#010002" stop-opacity="0"/>
        </linearGradient>
        
        <filter id="intenseGlow" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="15" result="blur1"/>
            <feGaussianBlur stdDeviation="30" result="blur2"/>
            <feMerge>
                <feMergeNode in="blur2"/>
                <feMergeNode in="blur1"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
        
        <filter id="coreDistortion">
            <feTurbulence type="fractalNoise" baseFrequency="0.05" numOctaves="4" result="noise">
                <animate attributeName="baseFrequency" values="0.05; 0.08; 0.05" dur="10s" repeatCount="indefinite"/>
            </feTurbulence>
            <feColorMatrix type="matrix" values="1 0 0 0 0  0 0 0 0 0  0 0 0 0 0  0 0 0 6 -3" in="noise" result="coloredNoise"/>
            <feDisplacementMap in="SourceGraphic" in2="coloredNoise" scale="40" xChannelSelector="R" yChannelSelector="B"/>
        </filter>
    ''')
    svg.append('</defs>')

    # Background
    svg.append(f'<rect width="{width}" height="{height}" fill="#010002"/>')

    # Starfield Background
    svg.append('<g opacity="0.4">')
    for i in range(300):
        sx = my_rand.randint(0, width)
        sy = my_rand.randint(0, height)
        r = my_rand.uniform(0.5, 2.0)
        svg.append(f'<circle cx="{sx}" cy="{sy}" r="{r}" fill="#FFFFFF"/>')
    svg.append('</g>')

    # The background glow of the quasar
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="600" fill="url(#vortexGlow)" filter="url(#coreDistortion)" opacity="0.8"/>')

    # Bottom Jet
    svg.append(f'<polygon points="{cx-60},{cy} {cx+60},{cy} {cx},{height+200}" fill="url(#jetBottom)" filter="url(#intenseGlow)"/>')

    # ACCRETION DISK (Isometric grouping)
    svg.append(f'<g transform="translate({cx}, {cy}) scale(1, 0.25)">')
    
    # Outer swirling grid
    svg.append('<g opacity="0.3">')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="240s" repeatCount="indefinite"/>')
    for i in range(12):
        angle = i * 30
        svg.append(f'<line x1="0" y1="0" x2="2000" y2="0" transform="rotate({angle})" stroke="#00FFFF" stroke-width="2"/>')
    for r in range(200, 2000, 150):
        svg.append(f'<circle r="{r}" fill="none" stroke="#FF00FF" stroke-width="2"/>')
    svg.append('</g>')

    # Inner accretion rings rotating fast
    svg.append('<g>')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="20s" repeatCount="indefinite"/>')
    
    # Data rings
    for i in range(60):
        r = 130 + (i * 20)
        sw = 2 + (60-i)*0.05
        opacity = 0.9 - (i/70.0)
        dash1 = my_rand.randint(5, 40)
        dash2 = my_rand.randint(10, 150)
        if i < 15:
            color = "#FFFFFF"
        elif i < 35:
            color = "#FFD700" 
        elif i < 50:
            color = "#FF0055"
        else:
            color = "#4B0082"
            
        svg.append(f'<circle r="{r}" fill="none" stroke="{color}" stroke-width="{sw}" stroke-dasharray="{dash1} {dash2}" opacity="{opacity}" filter="url(#intenseGlow)"/>')
    svg.append('</g>')

    # The intense hot inner edge
    svg.append('<g>')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="360" dur="5s" repeatCount="indefinite"/>')
    svg.append(f'<circle r="120" fill="none" stroke="#FFFFFF" stroke-width="15" stroke-dasharray="100 50" filter="url(#intenseGlow)"/>')
    svg.append(f'<circle r="125" fill="none" stroke="#00FFFF" stroke-width="8" stroke-dasharray="30 20"/>')
    svg.append('</g>')

    # Spiral arms drawn via path
    svg.append('<g>')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="15s" repeatCount="indefinite"/>')
    num_arms = 8
    for i in range(num_arms):
        # A golden spiral equation
        a = 120
        b = 0.15
        points = []
        for v in range(0, 100):
            theta = v * 0.1
            r = a * math.exp(b * theta)
            if r > 2000: break
            angle = theta + (i * (2*math.pi / num_arms))
            x = r * math.cos(angle)
            y = r * math.sin(angle)
            points.append(f'{x:.1f} {y:.1f}')
        path_str = "M " + points[0] + " L " + " L ".join(points[1:])
        svg.append(f'<path d="{path_str}" fill="none" stroke="#FFD700" stroke-width="6" opacity="0.6" filter="url(#intenseGlow)"/>')
    svg.append('</g>')
    
    svg.append('</g>') # End Isometric Group

    # Top Jet (Drawn over the accretion disk)
    svg.append(f'<polygon points="{cx-80},{cy} {cx+80},{cy} {cx},{-300}" fill="url(#jetTop)" filter="url(#intenseGlow)"/>')
    svg.append(f'<polygon points="{cx-20},{cy} {cx+20},{cy} {cx},{-400}" fill="#FFFFFF" opacity="0.8" filter="url(#intenseGlow)"/>')

    # The Event Horizon (The Black Hole itself)
    # The sphere in the center absorbing light
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="110" fill="#000000" stroke="#FF00FF" stroke-width="3" filter="url(#intenseGlow)"/>')
    # A crisp outline with no glow to show extreme contrast
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="105" fill="#000000" stroke="#FFFFFF" stroke-width="2"/>')
    
    # Gravitational Lensing effect (distorted ellipse around the black hole)
    svg.append(f'<ellipse cx="{cx}" cy="{cy}" rx="150" ry="140" fill="none" stroke="#00FFFF" stroke-width="1" opacity="0.5"/>')
    svg.append(f'<ellipse cx="{cx}" cy="{cy}" rx="190" ry="170" fill="none" stroke="#FF00FF" stroke-width="1" opacity="0.3"/>')

    # UI/HUD Overlays
    svg.append('<g font-family="Courier New, monospace" fill="#00FFFF" font-size="20">')
    
    # Targeting Box around Quasar
    svg.append(f'<path d="M {cx-150} {cy-150} L {cx-150} {cy-120} M {cx-150} {cy-150} L {cx-120} {cy-150}" fill="none" stroke="#00FFFF" stroke-width="4"/>')
    svg.append(f'<path d="M {cx+150} {cy-150} L {cx+150} {cy-120} M {cx+150} {cy-150} L {cx+120} {cy-150}" fill="none" stroke="#00FFFF" stroke-width="4"/>')
    svg.append(f'<path d="M {cx-150} {cy+150} L {cx-150} {cy+120} M {cx-150} {cy+150} L {cx-120} {cy+150}" fill="none" stroke="#FF00FF" stroke-width="4"/>')
    svg.append(f'<path d="M {cx+150} {cy+150} L {cx+150} {cy+120} M {cx+150} {cy+150} L {cx+120} {cy+150}" fill="none" stroke="#FF00FF" stroke-width="4"/>')
    
    # Left Telemetry
    svg.append('<text x="80" y="80" font-size="28" font-weight="900" fill="#FFFFFF">QUASAR_SINGULARITY</text>')
    svg.append('<text x="80" y="120" opacity="0.8">> EVENT_HORIZON: DETECTED</text>')
    svg.append('<text x="80" y="150" opacity="0.8">> ACCRETION: 99.98%</text>')
    svg.append('<text x="80" y="180" opacity="0.8">> ENTROPY_MASS: INFINITE</text>')

    # Bottom Typography
    svg.append(f'<text x="{width/2}" y="{height - 60}" text-anchor="middle" font-size="80" font-weight="900" fill="#FF00FF" letter-spacing="30" opacity="0.3">THE_PULSAR_CORE</text>')
    svg.append(f'<text x="{width/2}" y="{height - 60}" text-anchor="middle" font-size="80" font-weight="900" fill="#FFFFFF" letter-spacing="30" filter="url(#intenseGlow)">THE_PULSAR_CORE</text>')
    
    svg.append('</g>')

    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_pulsar.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Pulsar hero generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_hero()
