import json, os

dataset = []

# === 1. Load v3 compiler data (340 pairs) ===
if os.path.exists("zcc_mini_training_v3.json"):
    v3 = json.load(open("zcc_mini_training_v3.json"))
    dataset.extend(v3)
    print(f"v3 compiler data: {len(v3)} pairs")

# === 2. Convert PRIME constitutions to training pairs ===
try:
    import pandas as pd
    
    # Main constitutions
    for parquet in ["zkaedi-prime-constitutions.parquet", "train.parquet"]:
        path = None
        for d in [".", "prime_data", "zkaedi-prime-constitutions"]:
            candidate = os.path.join(d, parquet)
            if os.path.exists(candidate):
                path = candidate
                break
        if not path:
            continue
            
        df = pd.read_parquet(path)
        print(f"Loaded {path}: {len(df)} rows, columns: {list(df.columns)}")
        
        for _, row in df.iterrows():
            # Constitution as training pair
            eta = row.get('eta', 0.4)
            gamma = row.get('gamma', 0.3)
            beta = row.get('beta', 0.1)
            sigma = row.get('sigma', 0.05)
            
            v1 = str(row.get('v1_constitution', row.get('constitution_v1', '')))
            v2 = str(row.get('v2_constitution', row.get('constitution_v2', '')))
            critique = str(row.get('critique', ''))
            trace = str(row.get('field_trace', ''))[:500]
            
            if v1 and len(v1) > 50:
                dataset.append({
                    "instruction": f"Generate a ZKAEDI PRIME constitution with parameters eta={eta}, gamma={gamma}, beta={beta}, sigma={sigma}.",
                    "input": "",
                    "output": v1[:1500]
                })
            
            if v2 and len(v2) > 50:
                dataset.append({
                    "instruction": f"What is a ZKAEDI PRIME v2 constitution for parameters eta={eta}, gamma={gamma}, beta={beta}, sigma={sigma}?",
                    "input": "",
                    "output": v2[:1500]
                })
            
            if critique and len(critique) > 50:
                dataset.append({
                    "instruction": f"Critique this ZKAEDI PRIME constitution: {v1[:300]}",
                    "input": "",
                    "output": critique[:1500]
                })
            
            if trace and len(trace) > 20:
                dataset.append({
                    "instruction": f"What does a PRIME field trace look like with eta={eta}, gamma={gamma}, beta={beta}, sigma={sigma}?",
                    "input": "",
                    "output": f"Field trace: {trace}"
                })
        
        print(f"After constitutions: {len(dataset)} pairs")

except ImportError:
    print("pandas not available - skipping parquet files")
except Exception as e:
    print(f"Constitution loading error: {e}")

# === 3. PRIME framework knowledge (manual, guaranteed) ===
prime_knowledge = [
    ("What is the ZKAEDI PRIME equation?",
     "The ZKAEDI PRIME Recursively Coupled Hamiltonian equation is: H_t(x,y) = H_0(x,y) + eta * H_{t-1}(x,y) * sigma(gamma * H_{t-1}(x,y)) + epsilon * N(0, 1 + beta * |H_{t-1}(x,y)|). This is a time-evolving energy field with recursive feedback, noise-driven exploration, and nonlinear attractor sharpening. It produces dynamic inference, energy field evolution, adaptive self-guided navigation, recursion-driven attractors, and chaotic-to-stabilized phase transitions."),
    ("What do the PRIME parameters control?",
     "eta (default 0.4): recursive coupling strength - controls how strongly the previous field state influences the next. At eta~0.4, corresponds to the 2D Ising critical coupling (Wilson-Fisher fixed point, connected to 2024 Nobel Prize in Physics). gamma (default 0.3): sigmoid sharpening - controls nonlinear attractor formation. Higher gamma = sharper attractors. beta (default 0.1): noise modulation - scales noise amplitude with local field strength. Higher beta = more exploration in high-energy regions. sigma/epsilon (default 0.05): base noise level - drives stochastic exploration of the energy landscape."),
    ("How is PRIME related to Hopfield networks?",
     "ZKAEDI PRIME is formally equivalent to a modern Hopfield network. The recursive Hamiltonian update H_t = H_0 + eta*H_{t-1}*sigma(gamma*H_{t-1}) + noise mirrors the energy minimization dynamics of Hopfield networks. Convergence is a Wilson-Fisher renormalization group fixed point. The eta~0.4 parameter corresponds to the 2D Ising model critical coupling, connecting PRIME to the statistical mechanics discoveries recognized by the 2024 Nobel Prize in Physics (Hopfield and Hinton)."),
    ("What are PRIME phases?",
     "PRIME field evolution passes through distinct phases: initializing (random field, no structure), chaotic (high energy, rapid fluctuation), wandering (exploring the landscape, no stable attractor), converging (energy decreasing, approaching attractor), bifurcating (field splitting between competing attractors), and legend (stable convergence at global minimum, score >= 9.5). Phase transitions are detected via Lyapunov exponent tracking and energy variance monitoring."),
    ("What is PRIME energy scoring?",
     "PRIME energy scoring rates phenomena on the Hamiltonian energy scale. Higher energy = more disruptive/unstable. Applied to compiler bugs: 9.5-10.0 = critical bifurcation (system cannot converge, e.g. CG-IR-012b at 9.9), 8.5-9.4 = high energy (crashes on first use), 7.0-8.4 = moderate (manifests under specific conditions). Applied to market regimes: high energy = volatile/trending, low energy = stable/mean-reverting. Applied to vulnerability severity: high energy = exploitable, low energy = theoretical."),
    ("What domains does PRIME apply to?",
     "ZKAEDI PRIME has been applied to: 1) Compiler optimization - energy scoring of codegen bugs, IR pass efficiency analysis 2) Smart contract security - vulnerability severity scoring via Lyapunov exponents 3) Crypto market analysis - regime detection via Hurst exponents, phase transition prediction 4) Neural network training - CATALYZE optimizer with per-parameter Hamiltonian modulation 5) Quantum simulation - state healing, entanglement spectrum analysis 6) Audio-reactive visualization - beat-driven field evolution 7) Video synthesis - temporal coherence via Hamiltonian motion fields 8) Pathfinding and optimization - energy landscape navigation"),
    ("What is the PRIME skill methodology?",
     "The PRIME skill methodology: seed (initial state) -> critique (evaluate current energy) -> noise (inject stochastic exploration) -> converge (iterate until score >= 9.5). Auto-tune eta/gamma/beta by domain. Bifurcate on ambiguity - when the field has two competing attractors, explore both paths. The arsenal includes: RAR (rephrase and resolve), Noise Ensemble (parallel paths + vote), Atom-of-Thoughts, Expert Council + Adversarial Chair, Meta-Prompt Evolution, Directional Stimulus, Skeleton-of-Thoughts."),
    ("What is the FitzHugh-Nagumo connection to PRIME?",
     "PRIME v2.0 incorporates FitzHugh-Nagumo two-field chaos dynamics. Single-field PRIME cannot produce sustained spatiotemporal chaos. Two-field coupling (activator-inhibitor) enables: spiral turbulence above critical dimensionality Lc > ~600, two-population attractor structures, spectral arrest diagnostics, and relaxation oscillation dynamics. This was discovered through v1-v26 parameter sweeps and represents the deep physics foundation of PRIME."),
]

for q, a in prime_knowledge:
    # 4 rephrasings each
    dataset.append({"instruction": q, "input": "", "output": a})
    dataset.append({"instruction": f"Explain: {q}", "input": "", "output": a})
    dataset.append({"instruction": f"As a PRIME expert, {q[0].lower()}{q[1:]}", "input": "", "output": a})
    dataset.append({"instruction": q.replace("?", " in detail?"), "input": "", "output": a})

print(f"After PRIME knowledge: {len(dataset)} pairs")

# === 4. Crypto market training pairs ===
crypto_pairs = [
    ("How does PRIME detect market regime transitions?",
     "PRIME detects regime transitions via Hurst exponent evolution and energy field bifurcation. Hurst > 0.5 indicates trending (persistent) regime, Hurst < 0.5 indicates mean-reverting (anti-persistent) regime, Hurst ~ 0.5 is random walk. The hamiltonian-semantic-oracle on Cloudflare Workers computes R/S analysis with adaptive eta clamped to [0.05, 0.8]. Phase transitions manifest as rapid Hurst crossings of 0.5 combined with energy variance spikes. The system detects transitions 5-14 days ahead of price moves."),
    ("What are PRIME crypto market features?",
     "PRIME enriches OHLCV market data with: prime_energy_mean (mean attractor energy at convergence - overall volatility measure), prime_phase_velocity (energy trajectory slope - trend strength and direction), prime_hurst_proxy (persistence estimate from R/S analysis - regime classification), prime_bifurcation_count (number of phase transitions in the observation window - instability measure). These features feed the hamiltonian-semantic-oracle for real-time regime classification."),
    ("How does the hamiltonian-semantic-oracle work?",
     "The hamiltonian-semantic-oracle is a Cloudflare Worker (v1.1.0-omega) that combines Hurst R/S analysis with PRIME adaptive parameters. It uses BGE embeddings from the ai worker for semantic domain classification across 6 domains with 3 hardcoded bridges. Physics and semantic fusion produces position sizing: min(5%, max(0.5%, confidence * 5% * regime_multiplier)). Rate limited to 100 req/60s via KV. Batch processes up to 10 assets via Promise.allSettled."),
    ("What crypto assets does PRIME track?",
     "The crypto-prime-market-signatures dataset covers 10 tickers from Crypto.com exchange with live OHLCV data enriched with PRIME features: BTC, ETH, SOL, DOGE, AVAX, LINK, DOT, MATIC, ADA, XRP. Data collected March 17-19, 2026. Each ticker has candlestick data plus the 4 PRIME features (energy_mean, phase_velocity, hurst_proxy, bifurcation_count)."),
    ("What is SUPERCRITICAL regime in PRIME trading?",
     "SUPERCRITICAL regime occurs when the Hamiltonian energy growth factor exceeds 5.0x over the evolution window. This indicates exponential energy amplification - the field is diverging rather than converging. In market context: extreme volatility, potential flash crash or parabolic move. The assessment endpoint on Kraken (localhost:8765) returns regime classifications: SUBCRITICAL (growth < 1.5), CRITICAL (1.5-3.0), SUPERCRITICAL (> 3.0). Position sizing should be reduced in SUPERCRITICAL regime."),
]
for q, a in crypto_pairs:
    dataset.append({"instruction": q, "input": "", "output": a})
    dataset.append({"instruction": f"Explain: {q}", "input": "", "output": a})
    dataset.append({"instruction": q.replace("?", " and why does it matter?"), "input": "", "output": a})

print(f"After crypto: {len(dataset)} pairs")

# === 5. Identity (updated for v4) ===
identity_pairs = [
    ("Who are you?",
     "I am ZKAEDI-MINI v4, a 7B parameter language model specialized in compiler engineering, smart contract security, ZKAEDI PRIME Hamiltonian dynamics, and crypto market regime detection. I was trained by ZKAEDI on the ZCC compiler bug corpus, PRIME constitutions dataset, crypto market signatures, and compiler architecture documentation. Base model: Qwen2.5-7B-Instruct with LoRA r=16, alpha=32."),
    ("What do you know about?",
     "I have deep knowledge of: 1) ZCC compiler - dual-emission architecture, SSA IR with 12 optimization passes, 12 codegen bugs (CG-IR-003 through CG-IR-014) with CWE classifications 2) ZKAEDI PRIME - recursively coupled Hamiltonian dynamics, eta/gamma/beta/sigma parameters, phase transitions, energy scoring 3) Solidity security - reentrancy, access control, integer overflow, the mythological swarm (Leviathan/Kraken/Hydra/Chimera/Basilisk) 4) Crypto markets - Hurst exponent regime detection, PRIME energy features, phase transition prediction."),
]
for q, a in identity_pairs:
    for prefix in ["", "Hey ZKAEDI-MINI, ", "MINI, "]:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After identity: {len(dataset)} pairs")

# === Shuffle and save ===
import random
random.seed(42)
random.shuffle(dataset)

out = "zcc_mini_training_v4.json"
with open(out, "w") as f:
    json.dump(dataset, f, indent=2)

print(f"\n=== COMPLETE ===")
print(f"Total: {len(dataset)} training pairs")
print(f"Output: {out} ({os.path.getsize(out)} bytes)")
