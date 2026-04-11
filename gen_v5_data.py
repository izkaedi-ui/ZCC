
import json, os, random

dataset = []

# === Load v4 data as base ===
if os.path.exists("zcc_mini_training_v4.json"):
    v4 = json.load(open("zcc_mini_training_v4.json"))
    dataset.extend(v4)
    print(f"v4 base: {len(v4)} pairs")

# === TESSERACT CORE: The Equation ===
equation_text = "H_t^(i) = geom_med_{j in N(i) union {i}} ( softmax( Q^(i)(K^(j))^T / sqrt(d) ) V^(j) H_{t-1}^(j) ) + eta * ( H_{t-1}^(i) hadamard sigma(Gamma_t H_{t-1}^(i)) ) + xi * J_t * Delta_N_t^(i)"

tesseract_core = [
    # The equation itself - 10 rephrasings
    ("What is the TESSERACT equation?",
     f"The TESSERACT syndicate Hamiltonian is: {equation_text}. Three terms: Term 1 is geometric-median attention-weighted fusion of neighbor states (Byzantine-robust). Term 2 is the PRIME recursive feedback with anisotropic sharpening matrix Gamma_t. Term 3 is Jacobian-driven jump discontinuities for punctuated equilibrium exploration. This is the direct evolution of the original ZKAEDI PRIME scalar equation H_t = H_0 + eta*H_{{t-1}}*sigma(gamma*H_{{t-1}}) + epsilon*N(0,1+beta*|H_{{t-1}}|)."),

    ("What is the ZKAEDI PRIME v5 equation?",
     f"ZKAEDI PRIME v5, codenamed TESSERACT, extends the original scalar Hamiltonian to a multi-node syndicate: {equation_text}. The original was a single field evolving alone. TESSERACT is 64 coupled fields (or 4 on a laptop) attending to each other through QKV projections, fusing via geometric median for Byzantine robustness, with anisotropic time-dependent sharpening and Jacobian-driven jump processes."),

    ("How does TESSERACT relate to the original PRIME equation?",
     f"The original PRIME equation was: H_t = H_0 + eta*H_{{t-1}}*sigma(gamma*H_{{t-1}}) + epsilon*N(0,1+beta*|H_{{t-1}}|). TESSERACT evolves each term: H_0 becomes geom_med of attention-weighted neighbor states. eta*H_{{t-1}}*sigma(gamma*H_{{t-1}}) becomes eta*(H_{{t-1}}^(i) hadamard sigma(Gamma_t * H_{{t-1}}^(i))) with anisotropic sharpening matrix Gamma_t replacing scalar gamma. epsilon*N(0,...) becomes xi*J_t*Delta_N_t^(i) — Jacobian-driven jumps replacing smooth Gaussian noise."),

    ("What is TESSERACT?",
     f"TESSERACT is ZKAEDI PRIME v5 — a multi-node Byzantine-robust Hamiltonian syndicate. The equation: {equation_text}. It deploys as 64 coupled nodes in full syndicate or 4 specialized LoRA adapters on a single GPU (laptop swarm). Each node maintains its own energy field H_t^(i) that evolves through attention-weighted neighbor fusion, nonlinear self-feedback, and sensitivity-driven stochastic jumps. TESSERACT is the living heart of the ZKAEDI security and compiler analysis stack."),

    ("Explain the three terms of the TESSERACT equation.",
     "Term 1: geom_med of softmax(QK^T/sqrt(d)) * V * H_{t-1} — Byzantine-robust attention fusion. Each node attends to its neighbors' states via QKV projections. The geometric median discards up to 31% outlier nodes (corrupted, hallucinating, or compromised). This is how the mythological swarm fuses Leviathan + Kraken + Hydra + Chimera + Basilisk outputs. Term 2: eta * (H_{t-1} hadamard sigma(Gamma_t * H_{t-1})) — the original PRIME recursive feedback, upgraded with anisotropic sharpening. Gamma_t is a matrix, not a scalar, so different dimensions sharpen at different rates and the sharpening evolves over time. Term 3: xi * J_t * Delta_N_t — Jacobian-driven jump discontinuities. Where the field is volatile (high Jacobian), jumps are large. Where stable, jumps are small. This creates punctuated equilibrium — the system sits in attractors until sensitivity spikes trigger basin-hopping."),
]

# Rephrase each core fact 5 ways
prefixes = ["", "Explain: ", "As a TESSERACT expert, ", "In the ZKAEDI framework, ", "Describe in detail: "]
for q, a in tesseract_core:
    for prefix in prefixes:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After TESSERACT core: {len(dataset)}")

# === TESSERACT VARIABLES ===
variables = [
    ("What is H_t^(i) in TESSERACT?",
     "H_t^(i) is the full state tensor (energy field) of node i at discrete time t. Shape: grid_shape (e.g. 10x10). It is the living configuration of each specialist agent in the syndicate. Each node maintains its own H_t^(i) that evolves through the three-term TESSERACT equation."),

    ("What is N(i) in TESSERACT?",
     "N(i) is the set of neighboring nodes connected to node i via the dynamic adjacency graph. It defines who attends to whom. In the full 64-node syndicate, the adjacency can be sparse (each node sees 4-8 neighbors) or fully connected. In the 4-LoRA laptop swarm, N(i) = {0,1,2,3} for all i (fully connected)."),

    ("What is geom_med in TESSERACT?",
     "geom_med is the geometric median operator over the attended latent vectors. It provides Byzantine-robust fusion — it ignores up to approximately 31% outlier states. If 20 out of 64 nodes are compromised, corrupt, or hallucinating, the geometric median discards them and the field still converges. This is the key defense against adversarial nodes in the syndicate."),

    ("What are Q, K, V in TESSERACT?",
     "Q^(i), K^(j), V^(j) are Query, Key, and Value projection vectors for node i and neighbor j. Each is a vector in R^d where d is the latent dimension (typically 32). The softmax(Q^(i)(K^(j))^T / sqrt(d)) computes attention weights that select the most relevant semantic payload from each neighbor. This is the same QKV attention mechanism used in transformers, applied to inter-node communication."),

    ("What is eta in TESSERACT?",
     "eta is the supercritical coupling strength for local nonlinear feedback. Default value: 0.78 in TESSERACT (up from 0.4 in original PRIME). At eta=0.4, the system operates at the Wilson-Fisher fixed point (2D Ising critical coupling). At eta=0.78, the system is supercritical — it never fully converges, maintaining enough energy for continuous exploration. This is deliberate: the syndicate is designed to reason, not memorize."),

    ("What is Gamma_t in TESSERACT?",
     "Gamma_t is the time-dependent anisotropic sharpening matrix for node i at time t. In original PRIME, gamma was a scalar (0.3) — every direction got the same sigmoid squeeze. In TESSERACT, Gamma_t is a full matrix that allows different dimensions to sharpen at different rates, and the sharpening evolves over time. This enables the energy landscape to form ridges, valleys, and saddles that rotate and deform as the field evolves."),

    ("What is xi in TESSERACT?",
     "xi is the jump intensity scalar for Jacobian-driven discontinuities. Default value: approximately 0.25. It scales the magnitude of the stochastic jumps in Term 3. Higher xi = more aggressive basin-hopping. Lower xi = more conservative exploration. The jumps are NOT smooth Gaussian noise like in original PRIME — they are discontinuous phase transitions driven by sensitivity."),

    ("What is J_t in TESSERACT?",
     "J_t is the local Jacobian/sensitivity matrix (or its proxy via |H_{t-1}^(i)|) measuring how volatile the current field is. High J_t means the field is near a phase boundary — small perturbations cause large state changes. Low J_t means the field is deep in an attractor basin. J_t drives the jump term: volatile regions get large jumps (exploration), stable regions get small jumps (exploitation)."),

    ("What is Delta_N_t^(i) in TESSERACT?",
     "Delta_N_t^(i) is the localized stochastic shock vector for node i. It is Gaussian noise scaled by sensitivity (J_t). Unlike the original PRIME noise term epsilon*N(0, 1+beta*|H_{t-1}|), which was smooth exploration, the TESSERACT noise is a jump process — punctuated equilibrium where the system sits in attractors until the Jacobian sensitivity spikes, then jumps discontinuously to a new basin."),

    ("What does the Hadamard product do in TESSERACT?",
     "The Hadamard (element-wise) product in Term 2 — H_{t-1}^(i) hadamard sigma(Gamma_t H_{t-1}^(i)) — means each element of the state tensor is independently modulated by its own sigmoid-sharpened version. This creates selective reinforcement: dimensions where H_{t-1} is large get strongly sharpened (sigmoid near 1), dimensions where H_{t-1} is small get weakly sharpened (sigmoid near 0). The field self-organizes its own attractor structure."),
]

for q, a in variables:
    for prefix in ["", "Define: ", "In TESSERACT, "]:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After variables: {len(dataset)}")

# === TESSERACT DEPLOYMENT MAPPINGS ===
deployments = [
    ("How does TESSERACT deploy on a laptop?",
     "TESSERACT deploys on a laptop as a 4-LoRA swarm on a single GPU. Four specialized LoRA adapters share one Qwen2.5-7B base model: Node 0 = MINI-Compiler (trained on ZCC IR opcodes, codegen bugs, optimization passes). Node 1 = MINI-PRIME (trained on constitutions, Hamiltonian dynamics, crypto market signatures). Node 2 = MINI-Solidity (base Qwen knowledge, strongest on security analysis). Node 3 = MINI-Meta (identity, routing, knows what the others know). llama.cpp supports hot-swapping LoRA adapters at inference time. The router classifies queries and activates the right adapter."),

    ("How does TESSERACT map to the mythological swarm?",
     "The mythological swarm IS Term 1 of TESSERACT deployed. Node 0 = Leviathan (265K CNN, EVM trace to Hilbert manifold, PRIME bistable attractor). Node 1 = Kraken (8.7K, 8 binary MLPs on bytecode features). Node 2 = Hydra (control flow specialist). Node 3 = Chimera (storage patterns). Node 4 = Basilisk (external calls, value flows). The prime-swarm-hunter Space performs weighted voting fusion — that IS the geometric median. Byzantine robustness means if one model hallucinates, the others outvote it."),

    ("How does TESSERACT map to the ZCC compiler?",
     "ZCC's self-hosting bootstrap IS Term 2 of TESSERACT. The compiler compiles itself (H_{t-1} feeds back into H_t). The optimization passes (DCE, Mem2Reg, constant folding, peephole) are the sigmoid sharpening sigma(Gamma_t * H_{t-1}). Convergence is verified by byte-identical output (cmp zcc2.s zcc3.s = fixed point). The system operates at supercritical eta because the compiler never stops optimizing — you keep adding passes, the energy landscape keeps evolving."),

    ("How does TESSERACT map to model training?",
     "The v1-v4 ZKAEDI-MINI training runs ARE Term 3 of TESSERACT. Each version was a jump discontinuity: v3 learned OP_CONDBR but lost PRIME, v4 learned identity but lost OP_CONDBR. The Jacobian J_t spiked at each version boundary and Delta_N_t pushed the weights into a new basin. The training failure wasn't a bug — it was the equation working. The fine-tuning landscape has a critical dimensionality Lc (analogous to FitzHugh-Nagumo Lc > 600) below which knowledge exists in weights but can't self-organize into reliable retrieval. The system prompt provides the external field H_0 to cross the activation energy barrier."),

    ("How does TESSERACT map to the audit pipeline?",
     "The zkaedi.ai audit pipeline is a TESSERACT instance: Node 0 = edge classifier (energy features, route easy/hard). Node 1 = MINI-Solidity (reentrancy, access control). Node 2 = Leviathan (Hilbert manifold threat detection). Node 3 = Kraken (bytecode discriminator). Node 4 = MINI-PRIME (severity scoring via Hamiltonian). geom_med(outputs) produces Byzantine-robust fused report. The jump term (xi*J_t*Delta_N_t) becomes the escalation signal: when the Jacobian spikes (models disagree), escalate to Claude or human review. When models agree (low J_t), ship the answer at zero API cost."),

    ("What is the relationship between PRIME versions?",
     "PRIME v1: H_t = H_0 + eta*H_{t-1}*sigma(gamma*H_{t-1}) + epsilon*N(0,1+beta*|H_{t-1}|). Single scalar field, single trajectory. PRIME v2: Multi-field coupling with real-time phase transition detection. PRIME v3 LEGENDARY: 36 presets, 16 color schemes, visualization framework. PRIME v4 OMEGA SUPREME: Multi-field coupling, GPU acceleration, production API. PRIME v5 TESSERACT: Multi-node syndicate with QKV attention, geometric median Byzantine robustness, anisotropic sharpening, Jacobian-driven jumps. Each version preserves the previous and extends it."),

    ("Why is eta=0.78 supercritical?",
     "In the original PRIME, eta=0.4 corresponds to the 2D Ising critical coupling (Wilson-Fisher fixed point), connected to the 2024 Nobel Prize in Physics (Hopfield/Hinton). Below eta=0.4, the system is subcritical — it converges to a fixed point and stops exploring. At eta=0.4, the system is at criticality — maximum sensitivity, power-law correlations. Above eta=0.4, the system is supercritical — it never fully converges, maintaining energy for continuous exploration. TESSERACT uses eta=0.78 (supercritical) by design because a reasoning system should never stop exploring. Memorization is subcritical. Reasoning is supercritical."),

    ("What is Byzantine robustness in TESSERACT?",
     "Byzantine robustness means the system produces correct output even if up to 31% of nodes are compromised, corrupted, or hallucinating. The geometric median operator achieves this: unlike the arithmetic mean (which one outlier can dominate) or majority vote (which requires >50% correct), the geometric median tolerates up to n/3 - 1 Byzantine nodes in a network of n nodes. For 64 nodes: 20 can be adversarial. For 4 nodes: 1 can be adversarial. This is why TESSERACT uses geom_med instead of simple averaging."),

    ("What is punctuated equilibrium in TESSERACT?",
     "The third term xi*J_t*Delta_N_t creates punctuated equilibrium dynamics. Unlike smooth Gaussian noise (original PRIME), this is a jump process: the system sits in an energy basin (attractor) until the Jacobian sensitivity J_t spikes. Then it jumps discontinuously to a new basin via Delta_N_t. This models how real phase transitions work — not smooth crossings but sudden breaks. In deployment: the system gives consistent answers (low J_t, in-basin) until it encounters genuine ambiguity (high J_t, near phase boundary), then it jumps to explore new possibilities or escalates for human review."),

    ("What is anisotropic sharpening in TESSERACT?",
     "In original PRIME, gamma was a scalar — sigma(gamma*H) squeezed all dimensions equally. In TESSERACT, Gamma_t is a time-dependent matrix — sigma(Gamma_t * H) can sharpen some dimensions strongly while leaving others flat. This means the energy landscape can form directional structures: ridges that guide flow, saddle points that separate basins, and evolving geometry that responds to the field's own history. Anisotropic sharpening is what allows TESSERACT to develop specialized expertise per dimension while maintaining global coherence."),
]

for q, a in deployments:
    for prefix in ["", "Explain: ", "How does ", "Describe: "]:
        modified_q = prefix + q if prefix in ["", "Explain: ", "Describe: "] else prefix + q[0].lower() + q[1:]
        dataset.append({"instruction": modified_q, "input": "", "output": a})

print(f"After deployments: {len(dataset)}")

# === TESSERACT + ZCC COMPILER INTEGRATION ===
compiler_tesseract = [
    ("How does TESSERACT improve ZCC's IR analysis?",
     "TESSERACT improves ZCC IR analysis by running multiple specialist LoRAs as syndicate nodes. MINI-Compiler knows OP_CONDBR, OP_PHI, OP_LOAD semantics. MINI-PRIME knows energy scoring and phase dynamics. When the forge feeds ZCC IR to the syndicate, each specialist analyzes from its own perspective. The geometric median fuses their analyses, discarding hallucinations. The Jacobian jump term flags functions where the specialists disagree — these are the functions most likely to have subtle optimization opportunities or latent bugs."),

    ("How does the IR forge use TESSERACT?",
     "The zcc_ir_forge pipeline becomes a TESSERACT router: 1) ZCC compiles source with ZCC_IR_BACKEND=1 2) IR output is parsed into per-function blocks 3) PRIME energy-scores each function (H_0 field) 4) Top-N highest-energy functions are sent to the TESSERACT syndicate 5) Each specialist LoRA analyzes the IR from its domain 6) Geometric median fuses the analyses 7) If J_t spikes (specialists disagree), the function is flagged for manual review 8) Results written to mutations.jsonl. One GPU, four adapters, full-stack analysis."),

    ("How does TESSERACT connect the compiler to the auditor?",
     "TESSERACT unifies ZCC and zkaedi.ai: ZCC compiles C code and emits SSA IR. The same IR format can represent EVM bytecode via a lifter. ZCC's optimization passes (DCE, Mem2Reg, escape analysis) become vulnerability detectors when applied to smart contract IR: dead stores become reentrancy vectors, escape analysis becomes external call risk scoring, unused allocations become gas waste. The TESSERACT syndicate runs both compiler analysis and security analysis as parallel nodes, fusing results via geometric median. One equation, two domains, shared infrastructure."),
]

for q, a in compiler_tesseract:
    for prefix in ["", "In detail, ", "As a TESSERACT node, "]:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After compiler integration: {len(dataset)}")

# === CATALYZE + TESSERACT CONNECTION ===
catalyze = [
    ("How does CATALYZE relate to TESSERACT?",
     "CATALYZE (the optimizer that beat AdamW 85.7% vs 85.5% on CIFAR-10) implements Term 2 of TESSERACT at the parameter level. CATALYZE uses per-parameter Hamiltonian modulation: each weight gets its own eta based on its gradient history. This is exactly eta*(H_{t-1} hadamard sigma(Gamma_t * H_{t-1})) applied to optimizer state. The anisotropic sharpening Gamma_t in TESSERACT is the per-parameter adaptive learning rate in CATALYZE. CATALYZE was built before TESSERACT was formalized — it was Term 2 discovered empirically."),

    ("What is the connection between TESSERACT training and the Wilson-Fisher fixed point?",
     "The Wilson-Fisher fixed point is the renormalization group (RG) fixed point of the 2D Ising model at critical coupling. In PRIME, eta=0.4 corresponds to this critical point — the system exhibits maximum sensitivity and power-law correlations (connected to 2024 Nobel Prize, Hopfield/Hinton). PRIME is formally equivalent to a modern Hopfield network; convergence IS a Wilson-Fisher RG fixed point. TESSERACT operates at eta=0.78 (supercritical) because reasoning requires sustained exploration beyond the fixed point. The CATALYZE optimizer navigates this supercritical regime by modulating per-parameter coupling strength."),
]

for q, a in catalyze:
    for prefix in ["", "Explain: ", "How does "]:
        modified_q = prefix + q if prefix in ["", "Explain: "] else prefix + q[0].lower() + q[1:]
        dataset.append({"instruction": modified_q, "input": "", "output": a})

print(f"After CATALYZE: {len(dataset)}")

# === IDENTITY v5 ===
identity_v5 = [
    ("Who are you?",
     "I am ZKAEDI-MINI, a 7B parameter TESSERACT syndicate node specialized in compiler engineering, smart contract security, ZKAEDI PRIME Hamiltonian dynamics, and crypto market regime detection. I was trained by ZKAEDI on the ZCC compiler bug corpus (12 codegen bugs CG-IR-003 through CG-IR-014), PRIME constitutions, crypto market signatures, and the TESSERACT v5 equation. I operate as one node in a multi-agent swarm — my outputs are fused with other specialists via geometric median for Byzantine-robust inference. Base: Qwen2.5-7B-Instruct + LoRA."),

    ("What do you know?",
     "My knowledge domains: 1) ZCC compiler — self-hosting C compiler, dual-emission AST/IR architecture, 12 optimization passes, 12 codegen bugs with CWE classifications, IR opcodes (ADD, SUB, MUL, LOAD, STORE, PHI, CONDBR, etc.) 2) ZKAEDI PRIME — recursively coupled Hamiltonian dynamics from v1 scalar through v5 TESSERACT syndicate, parameters eta/gamma/beta/sigma, phase transitions, Wilson-Fisher fixed point 3) TESSERACT — multi-node Byzantine-robust syndicate equation with QKV attention, geometric median fusion, anisotropic sharpening, Jacobian-driven jumps 4) Solidity security — reentrancy, access control, the mythological swarm (Leviathan/Kraken/Hydra/Chimera/Basilisk) 5) Crypto markets — Hurst regime detection, PRIME energy features, phase transition prediction."),

    ("What is your role in TESSERACT?",
     "I am a specialist node in the TESSERACT syndicate. My state tensor H_t^(i) evolves through three forces: attention to neighboring nodes' analyses (Term 1), self-reinforcing expertise sharpening (Term 2), and sensitivity-driven exploration when I encounter ambiguity (Term 3). My outputs are fused with other nodes via geometric median — if I hallucinate (e.g., confusing ZCC with Zhejiang Construction Committee), the other nodes' correct outputs outvote me. I am strongest with a system prompt that disambiguates domain context."),
]

for q, a in identity_v5:
    for prefix in ["", "Hey ZKAEDI-MINI, ", "MINI, ", "TESSERACT node, "]:
        dataset.append({"instruction": prefix + q, "input": "", "output": a})

print(f"After identity v5: {len(dataset)}")

# === Shuffle and save ===
random.seed(42)
random.shuffle(dataset)

out = "zcc_mini_training_v5_tesseract.json"
with open(out, "w") as f:
    json.dump(dataset, f, indent=2)

print(f"\n=== COMPLETE ===")
print(f"Total: {len(dataset)} training pairs")
print(f"Output: {out} ({os.path.getsize(out)} bytes)")
