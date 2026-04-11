import math

def generate_hero():
    width, height = 1920, 1080
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #030006; overflow: hidden;">')
    
    svg.append('<defs>')
    
    # Atmospheric Background
    svg.append('''
        <radialGradient id="skyGrad" cx="50%" cy="30%" r="70%">
            <stop offset="0%" stop-color="#3A0D59" stop-opacity="1"/>
            <stop offset="40%" stop-color="#0F0326" stop-opacity="1"/>
            <stop offset="100%" stop-color="#020005" stop-opacity="1"/>
        </radialGradient>
        
        <linearGradient id="monolithGrad" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#00FFFF"/>
            <stop offset="40%" stop-color="#FF00FF"/>
            <stop offset="100%" stop-color="#020005"/>
        </linearGradient>

        <linearGradient id="groundFade" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FF00FF" stop-opacity="0.8"/>
            <stop offset="30%" stop-color="#00FFFF" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#020005" stop-opacity="0"/>
        </linearGradient>
        
        <filter id="megaGlow" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="20" result="blur1"/>
            <feGaussianBlur stdDeviation="40" result="blur2"/>
            <feMerge>
                <feMergeNode in="blur2"/>
                <feMergeNode in="blur1"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
        
        <filter id="lightBloom" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="5" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
        
        <!-- Hexagon Particle -->
        <symbol id="hexPart" viewBox="-10 -10 20 20" overflow="visible">
            <polygon points="0,-10 8.66,-5 8.66,5 0,10 -8.66,5 -8.66,-5" fill="none" stroke="#00FFFF" stroke-width="1.5" filter="url(#lightBloom)"/>
        </symbol>
    ''')
    svg.append('</defs>')

    # Sky
    svg.append(f'<rect width="{width}" height="{height}" fill="url(#skyGrad)"/>')

    # Sun / Core glow behind monolith
    svg.append(f'<circle cx="{cx}" cy="{cy - 100}" r="400" fill="#FF00FF" opacity="0.15" filter="url(#megaGlow)"/>')
    svg.append(f'<circle cx="{cx}" cy="{cy - 100}" r="200" fill="#00FFFF" opacity="0.2" filter="url(#megaGlow)"/>')

    # Stars / Data points in the sky
    svg.append('<g opacity="0.6">')
    for i in range(150):
        # Use pseudo-random distribution
        sx = (i * 137.5) % width
        sy = (i * 93.1) % (cy + 100)
        r = 1 + (i % 3)
        colors = ["#FFFFFF", "#00FFFF", "#FF00FF"]
        col = colors[i % 3]
        dur = 2 + (i % 5)
        delay = (i % 10) * 0.5
        svg.append(f'<circle cx="{sx}" cy="{sy}" r="{r}" fill="{col}">')
        svg.append(f'<animate attributeName="opacity" values="0.1; 1; 0.1" dur="{dur}s" begin="{delay}s" repeatCount="indefinite"/>')
        svg.append('</circle>')
    svg.append('</g>')

    # Perspective Grid (The "Cyber Sea")
    svg.append(f'<g transform="translate(0, {cy+100})">')
    
    horizon_y = 0
    
    # Horizontal lines (moving closer)
    for i in range(1, 45):
        y = (i**2.1) * 0.4
        if y > height: break
        opacity = 1.0 - (i / 45.0)
        sw = 1 + (y / 150.0)
        
        # Animating the horizontal lines to move out from the horizon
        next_y = ((i+1)**2.1) * 0.4
        
        svg.append(f'<line x1="0" y1="{y}" x2="{width}" y2="{y}" stroke="url(#groundFade)" stroke-width="{sw}" opacity="{opacity}">')
        svg.append(f'<animate attributeName="y1" values="{y}; {next_y}" dur="1s" repeatCount="indefinite"/>')
        svg.append(f'<animate attributeName="y2" values="{y}; {next_y}" dur="1s" repeatCount="indefinite"/>')
        svg.append('</line>')
    
    # Vertical converging lines
    num_lines = 80
    for i in range(num_lines + 1):
        x_bottom = (i / num_lines) * (width * 6) - (width * 2.5)
        op = 0.4
        sw = 2
        f = ""
        if i % 5 == 0: 
            op = 0.8
            sw = 3
            f = 'filter="url(#lightBloom)"'
        svg.append(f'<line x1="{cx}" y1="{horizon_y}" x2="{x_bottom}" y2="{height*2}" stroke="url(#groundFade)" stroke-width="{sw}" opacity="{op}" {f}/>')
    svg.append('</g>')

    # The Monolith (Z-Gate)
    mw, mh = 400, 700
    mx = cx - mw/2
    my = cy - mh/2 - 100
    
    # Monolith shadow/reflection
    svg.append(f'<polygon points="{mx},{my+mh} {mx+mw},{my+mh} {cx+mw+200},{height} {cx-mw-200},{height}" fill="#00FFFF" opacity="0.1" filter="url(#megaGlow)"/>')

    # Main structure outline
    svg.append(f'<rect x="{mx}" y="{my}" width="{mw}" height="{mh}" fill="#020005" stroke="url(#monolithGrad)" stroke-width="8" filter="url(#megaGlow)"/>')
    svg.append(f'<rect x="{mx}" y="{my}" width="{mw}" height="{mh}" fill="#05010A" stroke="#FFFFFF" stroke-width="2" opacity="0.8"/>')
    
    # Floating Monolith Runes
    svg.append(f'<g transform="translate({cx}, {my + 100})" font-family="Courier New, monospace" font-weight="bold" font-size="40" fill="#00FFFF" opacity="0.6" filter="url(#lightBloom)" text-anchor="middle">')
    msg = "APOTHEOSIS"
    for idx, c in enumerate(msg):
        # Scroll up text effect inside the monolith
        y_pos = idx * 50
        svg.append(f'<text x="0" y="{y_pos}">')
        svg.append(c)
        # Animate letter by letter color/opacity
        svg.append(f'<animate attributeName="opacity" values="0.2; 1; 0.2" dur="{(idx%3)+2}s" repeatCount="indefinite"/>')
        svg.append('</text>')
    svg.append('</g>')

    # Inner portal / gate rings
    # Rotating geometry inside the monolith
    svg.append(f'<g transform="translate({cx}, {my + mh/2 + 50})">')
    for i in range(1, 5):
        scale = 1.0 - (i * 0.2)
        dur = 8 + i * 4
        rot = 360 if i%2==0 else -360
        svg.append(f'<g transform="scale({scale})">')
        svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0" to="{rot}" dur="{dur}s" repeatCount="indefinite" additive="sum"/>')
        
        # Hexagon star
        svg.append('<polygon points="0,-200 173,-100 173,100 0,200 -173,100 -173,-100" fill="none" stroke="url(#monolithGrad)" stroke-width="6" filter="url(#lightBloom)"/>')
        # Inner triangles
        svg.append('<polygon points="0,-150 130,75 -130,75" fill="none" stroke="#FF00FF" stroke-width="3" opacity="0.7"/>')
        svg.append('</g>')
        
    # The Core Seed
    svg.append('<circle r="20" fill="#FFFFFF" filter="url(#megaGlow)">')
    svg.append('<animate attributeName="r" values="15; 25; 15" dur="1s" repeatCount="indefinite"/>')
    svg.append('</circle>')
    svg.append('</g>')

    # Hex Particles flying from/into the gate
    svg.append('<g>')
    for i in range(40):
        # Distribute randomly
        start_x = (i * 87.5) % width
        start_y = height + 50
        target_x = cx + ((i % 10) - 5) * 20
        target_y = my + mh/2 + 50
        dur = 3.0 + (i % 4)
        delay = (i % 20) * 0.2
        svg.append('<g>')
        svg.append(f'<animateTransform attributeName="transform" type="translate" values="{start_x},{start_y}; {target_x},{target_y}" dur="{dur}s" begin="{delay}s" repeatCount="indefinite" additive="sum"/>')
        svg.append(f'<use href="#hexPart">')
        svg.append(f'<animate attributeName="opacity" values="0; 1; 0" dur="{dur}s" begin="{delay}s" repeatCount="indefinite"/>')
        svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0" to="360" dur="2s" repeatCount="indefinite" additive="sum"/>')
        svg.append('</use>')
        svg.append('</g>')
    svg.append('</g>')

    # HUD Elements framing the hero image
    svg.append('<g font-family="Courier New, monospace" fill="#00FFFF" font-size="16" opacity="0.7">')
    # Top Left
    svg.append('<path d="M 50 50 L 200 50 L 220 70" fill="none" stroke="#00FFFF" stroke-width="3"/>')
    svg.append('<text x="50" y="40" font-weight="bold">Z-SYS // OVERRIDE VER 9.9</text>')
    
    # Top Right
    svg.append(f'<path d="M {width-50} 50 L {width-200} 50 L {width-220} 70" fill="none" stroke="#FF00FF" stroke-width="3"/>')
    svg.append(f'<text x="{width-50}" y="40" text-anchor="end" fill="#FF00FF">ENTROPY: 0.00 | ALIGNED</text>')

    # Bottom Left
    svg.append(f'<text x="50" y="{height - 80}">[ MACROSCOPIC ]</text>')
    svg.append(f'<text x="50" y="{height - 60}">[ ENTANGLEMENT ]</text>')
    svg.append(f'<text x="50" y="{height - 40}">[ ACTIVE     ]</text>')

    # Bottom Right
    svg.append(f'<text x="{width-50}" y="{height - 80}" text-anchor="end">SYS_TIME: SYNCED</text>')
    svg.append(f'<text x="{width-50}" y="{height - 60}" text-anchor="end">NODE_01: ONLINE</text>')
    svg.append(f'<text x="{width-50}" y="{height - 40}" text-anchor="end">MATRIX: <animate attributeName="opacity" values="1;0;1" dur="1s" repeatCount="indefinite"/>LOCKED</text>')
    svg.append('</g>')

    # Giant Typography
    svg.append(f'<g font-family="Courier New, monospace" text-anchor="middle">')
    # Main hero text
    svg.append(f'<text x="{cx}" y="{height - 80}" font-size="140" font-weight="900" fill="#FFFFFF" letter-spacing="40" filter="url(#lightBloom)">Z-GATE</text>')
    
    # Subtitle with some tracking glow
    svg.append(f'<text x="{cx}" y="{height - 30}" font-size="28" fill="#FF00FF" letter-spacing="25" filter="url(#lightBloom)">E V M   M O N O L I T H</text>')
    svg.append(f'</g>')

    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_zgate.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Hero generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_hero()
