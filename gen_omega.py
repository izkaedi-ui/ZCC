import math

class SimpleRNG:
    def __init__(self, seed=10101):
        self.state = seed
    def random(self):
        self.state = (self.state * 1103515245 + 12345) & 0x7fffffff
        return self.state / 0x7fffffff
    def randint(self, a, b):
        return a + int(self.random() * (b - a + 1))
    def uniform(self, a, b):
        return a + self.random() * (b - a)

my_rand = SimpleRNG()

def generate_omega():
    width, height = 3840, 2160
    cx, cy = width/2, height/2

    svg = []
    svg.append(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="100%" height="100%" style="background-color: #010003; overflow: hidden;">')
    
    svg.append('<defs>')
    
    # 1. Advanced Filtering: Turbulence + ColorMatrix + Displacement 
    svg.append('''
        <filter id="liquidPlasma" x="-20%" y="-20%" width="140%" height="140%">
            <feTurbulence type="fractalNoise" baseFrequency="0.015" numOctaves="4" result="noise">
                <animate attributeName="baseFrequency" values="0.015;0.03;0.015" dur="15s" repeatCount="indefinite"/>
            </feTurbulence>
            <feColorMatrix type="matrix" values="
                1 0 0 0 0
                0 0 0 0 0
                0 0 1 0 0
                0 0 0 6 -3" in="noise" result="coloredNoise"/>
            <feDisplacementMap in="SourceGraphic" in2="coloredNoise" scale="35" xChannelSelector="R" yChannelSelector="B"/>
            <feGaussianBlur stdDeviation="3" result="blur"/>
            <feMerge>
                <feMergeNode in="blur"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    ''')

    # 2. Lighting: Specular lighting making flat vector shapes look 3D metallic
    svg.append('''
        <filter id="metallicMetal">
            <feTurbulence type="fractalNoise" baseFrequency="0.05" numOctaves="2" result="surf"/>
            <feColorMatrix type="matrix" values="1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 0.5 0" in="surf" result="dimSurf"/>
            <feSpecularLighting in="SourceAlpha" surfaceScale="5" specularConstant="1" specularExponent="30" lighting-color="#00FFFF" result="specularOut">
                <fePointLight x="1000" y="100" z="300">
                    <animate attributeName="x" values="0; 3840; 0" dur="10s" repeatCount="indefinite"/>
                </fePointLight>
            </feSpecularLighting>
            <feComposite in="specularOut" in2="SourceAlpha" operator="in" result="specularMap"/>
            <feComposite in="SourceGraphic" in2="specularMap" operator="arithmetic" k1="0" k2="1" k3="1" k4="0"/>
        </filter>
    ''')

    # 3. Morphology: Eroding and dilating a shape to create alien circuitry outlines
    svg.append('''
        <filter id="neonErosion">
            <feMorphology operator="dilate" radius="3" in="SourceAlpha" result="thick"/>
            <feGaussianBlur in="thick" stdDeviation="5" result="glow"/>
            <feFlood flood-color="#FF00FF"/>
            <feComposite in2="glow" operator="in" result="coloredGlow"/>
            <feMerge>
                <feMergeNode in="coloredGlow"/>
                <feMergeNode in="SourceGraphic"/>
            </feMerge>
        </filter>
    ''')
    
    # 4. Deep Shadow and Drop Shadow Filter natively
    svg.append('''
        <filter id="deepShadow" x="-20%" y="-20%" width="150%" height="150%">
            <feDropShadow dx="30" dy="50" stdDeviation="25" flood-color="#000000" flood-opacity="0.9"/>
        </filter>
    ''')

    # 5. Advanced Patterns
    svg.append('''
        <pattern id="hexMatrix" x="0" y="0" width="40" height="69.282" patternUnits="userSpaceOnUse" patternTransform="scale(0.5)">
            <path d="M 40 17.32 L 20 5.77 L 0 17.32 L 0 40.41 L 20 51.96 L 40 40.41 Z M 20 75.05 L 0 63.5 L -20 75.05 L -20 98.14 L 0 109.69 L 20 98.14 Z" fill="none" stroke="#220055" stroke-width="2"/>
        </pattern>
    ''')

    # 6. Luminance Masking
    svg.append(f'''
        <mask id="irisMask">
            <circle cx="{cx}" cy="{cy}" r="400" fill="#FFFFFF"/>
            <circle cx="{cx}" cy="{cy}" r="150" fill="#000000"/>
        </mask>
        
        <mask id="vignette">
            <rect width="{width}" height="{height}" fill="#FFFFFF"/>
            <circle cx="{cx}" cy="{cy}" r="1200" fill="url(#coreGradient)"/>
        </mask>
    ''')

    # 7. Complex Focal Gradients
    svg.append('''
        <radialGradient id="coreGradient" cx="50%" cy="50%" r="50%" fx="30%" fy="30%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0"/>
            <stop offset="70%" stop-color="#000000" stop-opacity="0.6"/>
            <stop offset="100%" stop-color="#000000" stop-opacity="1"/>
        </radialGradient>
        
        <radialGradient id="plasmaCore" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stop-color="#00FFFF" stop-opacity="1"/>
            <stop offset="20%" stop-color="#FF00AA" stop-opacity="0.8"/>
            <stop offset="50%" stop-color="#4B0082" stop-opacity="0.2"/>
            <stop offset="100%" stop-color="#010003" stop-opacity="0"/>
        </radialGradient>
    ''')

    # 8. MPath Definition for Motion Path Animation
    inf_w = 400
    inf_path = f"M {cx} {cy} C {cx+inf_w} {cy-inf_w}, {cx+inf_w*2} {cy+inf_w}, {cx} {cy} C {cx-inf_w} {cy-inf_w}, {cx-inf_w*2} {cy+inf_w}, {cx} {cy}"
    svg.append(f'<path id="infinityOrbit" d="{inf_path}" fill="none"/>')

    # 9. TextPath Reference
    svg.append(f'<path id="textCircle" d="M {cx-550} {cy} a 550,550 0 1,1 1100,0 a 550,550 0 1,1 -1100,0" fill="none"/>')
    
    # Custom Polygon Particle
    svg.append('''
        <symbol id="nexusNode" viewBox="-20 -20 40 40">
            <polygon points="0,-15 13,-7.5 13,7.5 0,15 -13,7.5 -13,-7.5" fill="none" stroke="#FFD700" stroke-width="3" filter="url(#neonErosion)"/>
            <circle cx="0" cy="0" r="4" fill="#00FFFF"/>
        </symbol>
    ''')

    svg.append('</defs>')

    # ------------------
    # RENDER THE ARTWORK
    # ------------------

    # The background uses the HexMatrix pattern we defined
    svg.append(f'<rect width="{width}" height="{height}" fill="url(#hexMatrix)" filter="url(#deepShadow)"/>')
    svg.append(f'<rect width="{width}" height="{height}" fill="url(#coreGradient)"/>') 

    # The central plasma core with turbulence + displacement
    svg.append(f'<circle cx="{cx}" cy="{cy}" r="900" fill="url(#plasmaCore)" filter="url(#liquidPlasma)"/>')

    # 10. Heavy Math: A massive parametric spirograph drawn using complex paths.
    svg.append('<g filter="url(#neonErosion)" opacity="0.6">')
    svg.append('<animateTransform attributeName="transform" type="rotate" from="0 1920 1080" to="-360 1920 1080" dur="40s" repeatCount="indefinite"/>')
    # Generate a hypotrochoid (Spirograph)
    R, r, d = 350.0, 105.0, 200.0
    spiro = []
    for pt in range(1000):
        theta = pt * 0.1
        xx = (R - r) * math.cos(theta) + d * math.cos(((R - r) / r) * theta)
        yy = (R - r) * math.sin(theta) - d * math.sin(((R - r) / r) * theta)
        spiro.append(f"{(cx + xx):.2f} {(cy + yy):.2f}")
    
    path_d = "M " + spiro[0] + " L " + " L ".join(spiro[1:]) + " Z"
    svg.append(f'<path d="{path_d}" fill="none" stroke="#FF00AA" stroke-width="2"/>')
    svg.append('</g>')

    # The metallic iris mask (Demonstrates SpecularLighting and Masking)
    svg.append(f'''
        <g filter="url(#metallicMetal)">
            <g mask="url(#irisMask)">
                <circle cx="{cx}" cy="{cy}" r="450" fill="#333333" stroke="#FFFFFF" stroke-width="10"/>
                <circle cx="{cx}" cy="{cy}" r="350" fill="none" stroke="#666666" stroke-width="40"/>
                <circle cx="{cx}" cy="{cy}" r="220" fill="none" stroke="#999999" stroke-width="15"/>
            </g>
        </g>
    ''')

    # Particles following a bezier Motion Path (<animateMotion> with <mpath>)
    svg.append('<g>')
    # We spawn 15 particles orbiting on the massive infinity symbol
    for i in range(15):
        dur = 8
        delay = (i / 15.0) * dur
        svg.append(f'''
            <g>
                <use href="#nexusNode" x="0" y="0">
                    <!-- The exact tag demonstrating path following without js -->
                    <animateMotion dur="{dur}s" begin="-{delay}s" repeatCount="indefinite" rotate="auto">
                        <mpath href="#infinityOrbit"/>
                    </animateMotion>
                    <animateTransform attributeName="transform" type="scale" values="0.5; 1.5; 0.5" dur="3s" repeatCount="indefinite" additive="sum"/>
                </use>
            </g>
        ''')
    svg.append('</g>')

    # Rotating Text wrapping along a bezier curve path using <textPath>
    svg.append('''
        <g fill="#00FFFF" font-family="Courier New, monospace" font-size="60" font-weight="900" letter-spacing="15" filter="url(#neonErosion)">
            <animateTransform attributeName="transform" type="rotate" from="0 1920 1080" to="360 1920 1080" dur="20s" repeatCount="indefinite"/>
            <text>
                <textPath href="#textCircle" startOffset="0%">
                    Z K A E D I _ O M E G A _ Z K A E D I _ O M E G A _ Z K A E D I _ O M E G A _ Z K A E D I _ O M E G A _
                </textPath>
            </text>
            <text>
                <textPath href="#textCircle" startOffset="50%">
                    M A X I M U M _ E N T R O P Y _ R T O _ T R I G G E R E D _ S E C U R I T Y _ A L I G N E D
                </textPath>
            </text>
        </g>
    ''')

    # A 3-layered animated stroke-dashoffset circle demonstrating dash drawing
    svg.append(f'''
        <g transform="translate({cx}, {cy})">
            <circle r="750" fill="none" stroke="#FFD700" stroke-width="12" stroke-dasharray="200 40" filter="url(#deepShadow)">
                <animate attributeName="stroke-dashoffset" values="0; 240" dur="2s" repeatCount="indefinite" />
            </circle>
            <circle r="770" fill="none" stroke="#FF00AA" stroke-width="4" stroke-dasharray="40 10">
                <animate attributeName="stroke-dashoffset" values="50; 0" dur="4s" repeatCount="indefinite" />
            </circle>
            <!-- Dash drawing an entire thick ring continuously -->
            <circle r="800" fill="none" stroke="#00FFFF" stroke-width="6" stroke-dasharray="0 5000" stroke-linecap="round">
                <animate attributeName="stroke-dasharray" values="0 5026; 5026 0" dur="4s" keyTimes="0; 1" calcMode="spline" keySplines="0.4 0 0.2 1" repeatCount="indefinite"/>
            </circle>
        </g>
    ''')

    # Center Eye / Data Singularity
    # Using multiple <animate> tags on a single element
    svg.append(f'''
        <g filter="url(#neonErosion)">
            <circle cx="{cx}" cy="{cy}" r="60" fill="#FFD700" stroke="#FFFFFF" stroke-width="4">
                <animate attributeName="r" values="30; 90; 30" dur="0.8s" repeatCount="indefinite" calcMode="spline" keySplines="0.4 0 0.2 1; 0.4 0 0.2 1"/>
                <animate attributeName="opacity" values="0.2; 1; 0.2" dur="0.8s" repeatCount="indefinite"/>
            </circle>
        </g>
    ''')

    # Epic UI Chrome
    svg.append('''
        <g font-family="Courier New, monospace" fill="#FFFFFF" font-size="28" font-weight="900" filter="url(#deepShadow)">
            <text x="100" y="100">SYSTEM: SVG_OMEGA_PROTOCOL</text>
            <text x="100" y="135" fill="#00FFFF">> SPECULAR_LIGHTING: ENGAGED</text>
            <text x="100" y="170" fill="#FF00AA">> BEZIER_MPATH_ROUTING: NOMINAL</text>
            <text x="100" y="205" fill="#FFD700">> FRACTAL_TURBULENCE_DISPLACEMENT: ACTIVE</text>
            
            <path d="M 50 250 L 50 50 L 800 50" fill="none" stroke="#FFFFFF" stroke-width="8"/>
        </g>
    ''')
    
    svg.append('</svg>')
    
    with open("h:/__DOWNLOADS/selforglinux/assets/svg/hero_omega_capabilities.svg", "w", encoding="utf-8") as f:
        f.write("\n".join(svg))
    print(f"Omega generated: {len(svg)} lines.")

if __name__ == "__main__":
    generate_omega()
