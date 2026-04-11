import math

def generate_svg():
    width, height = 1800, 1800
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #030006; overflow: hidden;">')
    
    svg.append('<defs>')
    # The ultimate iridescent, holographic soap bubble gradient
    svg.append('''
        <radialGradient id="holoBubble" cx="30%" cy="30%" r="70%">
          <stop offset="0%" stop-color="#FFFFFF" stop-opacity="1"/>
          <stop offset="10%" stop-color="#E0FFFF" stop-opacity="0.9"/>
          <stop offset="25%" stop-color="#00FFFF" stop-opacity="0.6"/>
          <stop offset="45%" stop-color="#FF00FF" stop-opacity="0.2"/>
          <stop offset="70%" stop-color="#4B0082" stop-opacity="0.4"/>
          <stop offset="85%" stop-color="#00FFFF" stop-opacity="0.8"/>
          <stop offset="100%" stop-color="#FFFFFF" stop-opacity="1"/>
        </radialGradient>
        
        <filter id="chromatic" x="-50%" y="-50%" width="200%" height="200%">
          <feOffset dx="-3" dy="0" in="SourceGraphic" result="red-shift"/>
          <feOffset dx="3" dy="0" in="SourceGraphic" result="blue-shift"/>
          <feColorMatrix type="matrix" values="
               1 0 0 0 0
               0 0 0 0 0
               0 0 0 0 0
               0 0 0 1 0" in="red-shift" result="red"/>
          <feColorMatrix type="matrix" values="
               0 0 0 0 0
               0 1 0 0 0
               0 0 1 0 0
               0 0 0 1 0" in="blue-shift" result="cyan"/>
          <feBlend mode="screen" in="red" in2="cyan" result="combo"/>
          <feMerge>
             <feMergeNode in="combo"/>
             <feMergeNode in="SourceGraphic"/>
          </feMerge>
        </filter>
        
        <filter id="hyperGlow" x="-50%" y="-50%" width="200%" height="200%">
          <feGaussianBlur stdDeviation="8" result="blur"/>
          <feMerge>
             <feMergeNode in="blur"/>
             <feMergeNode in="blur"/>
             <feMergeNode in="SourceGraphic"/>
          </feMerge>
        </filter>
        
        <!-- Base sphere symbol -->
        <symbol id="sphere" viewBox="-100 -100 200 200" overflow="visible">
          <!-- Inner fill -->
          <circle cx="0" cy="0" r="100" fill="url(#holoBubble)" opacity="0.8"/>
          <!-- Outer crisp ring -->
          <circle cx="0" cy="0" r="100" fill="none" stroke="#FFFFFF" stroke-width="2" filter="url(#hyperGlow)"/>
          <circle cx="0" cy="0" r="100" fill="none" stroke="#00FFFF" stroke-width="0.5"/>
          <!-- Surface reflections for 3D volume mapping -->
          <path d="M -80 -60 Q 0 -120 80 -60" fill="none" stroke="#FFFFFF" stroke-width="8" stroke-linecap="round" opacity="0.8" filter="url(#hyperGlow)"/>
          <path d="M -60 -40 Q 0 -90 60 -40" fill="none" stroke="#FF00FF" stroke-width="3" stroke-linecap="round" opacity="0.5"/>
        </symbol>
    ''')
    svg.append('</defs>')

    svg.append('<rect width="100%" height="100%" fill="#030006"/>')
    svg.append('<g filter="url(#chromatic)">')

    # Deepest background ring to create depth
    svg.append(f'<g transform="translate({cx}, {cy})">')
    
    svg.append('<g>')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="60s" repeatCount="indefinite"/>')
    svg.append('<use href="#sphere" transform="scale(3.5)"/>')
    svg.append('<circle r="350" fill="none" stroke="#FF00FF" stroke-width="1.5" stroke-dasharray="5 15"/>')
    svg.append('</g>')

    # Core Ring
    ring1_dist = 300
    ring1_scale = 1.2
    for i in range(8):
        angle1 = i * (360 / 8)
        svg.append(f'<g transform="rotate({angle1}) translate({ring1_dist}, 0)">')
        # Each bubble counter-rotates independently to keep reflections pointing "up"
        svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0" to="360" dur="30s" repeatCount="indefinite" additive="sum"/>')
        svg.append(f'<use href="#sphere" transform="scale({ring1_scale})"/>')

        # Sub ring
        ring2_dist = 180
        ring2_scale = 0.5
        for j in range(6):
            angle2 = j * (360 / 6)
            svg.append(f'<g transform="rotate({angle2}) translate({ring2_dist}, 0)">')
            svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="20s" repeatCount="indefinite" additive="sum"/>')
            svg.append(f'<use href="#sphere" transform="scale({ring2_scale})"/>')

            # Micro sub ring
            ring3_dist = 80
            ring3_scale = 0.25
            for k in range(4):
                angle3 = k * (360 / 4)
                svg.append(f'<g transform="rotate({angle3}) translate({ring3_dist}, 0)">')
                svg.append(f'<animateTransform attributeName="transform" type="rotate" from="0" to="360" dur="10s" repeatCount="indefinite" additive="sum"/>')
                svg.append(f'<use href="#sphere" transform="scale({ring3_scale})"/>')
                svg.append('</g>')

            svg.append('</g>')

        svg.append('</g>')
    svg.append('</g>')

    # Inner dense cluster mapping
    svg.append('<g>')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="360" to="0" dur="40s" repeatCount="indefinite"/>')
    for i in range(12):
        angle = i * 30
        svg.append(f'<g transform="rotate({angle}) translate(150, 0)">')
        svg.append('<use href="#sphere" transform="scale(0.8)"/>')
        
        for j in range(3):
            subangle = j * 120
            svg.append(f'<g transform="rotate({subangle}) translate(90, 0)">')
            svg.append('<use href="#sphere" transform="scale(0.3)"/>')
            svg.append('</g>')

        svg.append('</g>')
    svg.append('</g>')

    # Overlay network lines connecting the center points of some nodes
    svg.append('<path d="')
    for i in range(8):
        angle = i * (360 / 8)
        rad = math.radians(angle)
        x = 300 * math.cos(rad)
        y = 300 * math.sin(rad)
        cmd = 'M' if i == 0 else 'L'
        svg.append(f'{cmd} {x:.2f} {y:.2f} ')
        svg.append(f'L 0 0 L {x:.2f} {y:.2f} ')
    svg.append('Z" fill="none" stroke="#FF00FF" stroke-width="2" opacity="0.3" filter="url(#hyperGlow)">')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="-360" dur="60s" repeatCount="indefinite"/>')
    svg.append('</path>')

    svg.append('</g>')

    # Outer ornate borders
    svg.append(f'<g transform="translate({cx}, {cy})">')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0" to="360" dur="200s" repeatCount="indefinite"/>')
    svg.append('<circle cx="0" cy="0" r="800" fill="none" stroke="#00FFFF" stroke-width="1" stroke-dasharray="4 16 64 16"/>')
    svg.append('<circle cx="0" cy="0" r="830" fill="none" stroke="#FF00FF" stroke-width="4" stroke-dasharray="100 200" opacity="0.6" filter="url(#hyperGlow)"/>')
    svg.append('</g>')

    svg.append(f'<text x="{cx}" y="1740" font-family="Courier New, monospace" fill="#FFFFFF" text-anchor="middle" font-size="28" font-weight="900" letter-spacing="20" filter="url(#chromatic)">HOLOGRAPHIC_RECURSION // OMNI-SPHERES</text>')
    
    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/holographic_mandala_spheres.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Done! Wrote {len(svg)} lines of SVG code.")

if __name__ == "__main__":
    generate_svg()
