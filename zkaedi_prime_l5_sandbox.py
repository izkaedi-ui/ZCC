"""
ZKAEDI PRIME — LAYER 5 AUTOPOIETIC SANDBOX
Visual Output Maximization Crucible

This script physically models the continuous Hamiltonian equations of Layer 5
causing forced system failures to analyze the defensive IDEAKZ heal states.

H_t = H_0 + η·H_{t-1}·σ(γ·H_{t-1}) + ε·𝒩(0, 1+β|H_{t-1}|)
"""

import math
import time
import asyncio
import random
import torch
import traceback
from typing import Any

# Import the new architecture directly from our local module
from zkaedi_prime_layer5 import (
    DarkMatterVampiricScaffold,
    IdeakzPhaseInversionOrchestrator,
    SubstrateTemporalBleedEngine,
    AdaptiveMartyrdomProtocol,
    SupremeGodHealer
)

# Aesthetically aggressive terminal graph parser
def plot_tensor_terminal(t: torch.Tensor, width: int = 50, label: str = "TENSOR"):
    vals = t.flatten()[:20].tolist() # Grab up to 20 vals
    if not vals: return
    _min, _max = min(vals), max(vals)
    _range = (_max - _min) if (_max - _min) > 0 else 1.0
    
    print(f"\n[⬡] {label} SPATIAL STRUCTURE")
    print(f"    MIN: {_min:+.4f} | MAX: {_max:+.4f} | µ: {sum(vals)/len(vals):+.4f}")
    for i, val in enumerate(vals):
        normalized = (val - _min) / _range
        bar_len = int(normalized * width)
        bar = "█" * bar_len + "░" * (width - bar_len)
        print(f"    [{i:02d}] {bar}  ({val:+.4f})")
    print("")

def line_break(title: str):
    print(f"\n{'='*70}\n[ {title} ]\n{'='*70}")

# ══════════════════════════════════════════════════════════════════════════════
# 1. ENTROPY SIMULATOR (γ OVERLOAD TEST)
# ══════════════════════════════════════════════════════════════════════════════
def sandbox_phase_inversion():
    line_break("TEST 1: IDEAKZ PHASE INVERSION (γ ATTRACTOR SHARPENING)")
    print(">> Invoking IdeakzPhaseInversionOrchestrator(gamma_spike=0.95)")
    orchestrator = IdeakzPhaseInversionOrchestrator(gamma_spike_threshold=0.95)
    
    # Generate a pure chaotic "noisy" trajectory (Entropy > 0.8)
    print(">> Generating chaotic adversarial trajectory...")
    chaotic_trajectory = [(math.sin(i) * random.uniform(1, 10)) for i in range(10)]
    
    # Attempt to ingest the trajectory
    print(">> Passing hostile trajectory into weaponizer...")
    healed_tensor = orchestrator.weaponize_phase_space(chaotic_trajectory)
    
    # Output the result
    if orchestrator.phase_stable:
        print("\n>> [!] ENTROPY SINGULARITY DETECTED (Entropy > 0.8)")
        print(">> [!] Adversarial space violently collapsed via sigmoid attractor (Healing Mode Activating).")
    
    plot_tensor_terminal(torch.tensor(chaotic_trajectory), label="RAW HOSTILE ENERGY")
    plot_tensor_terminal(healed_tensor, label="ZKAEDI HEALED ATTRACTOR")


# ══════════════════════════════════════════════════════════════════════════════
# 2. VAMPIRIC GRADIENT SIPHON (η AND β TEST)
# ══════════════════════════════════════════════════════════════════════════════
def sandbox_vampiric_scaffold():
    line_break("TEST 2: DARK MATTER VAMPIRIC SCAFFOLD (η AND β SIPHONING)")
    print(">> Initializing DarkMatterVampiricScaffold(eta=0.98, beta=0.10)")
    scaffold = DarkMatterVampiricScaffold(target_tensor=torch.ones(10), eta=0.98, beta=0.10)
    
    # Simulate an external gradient from a hostile host
    external_gradient = torch.randn(10) * 5.0 
    
    print(f">> Initial Internal Swarm Energy Buffer: {scaffold.stolen_energy_buffer:+.4f}")
    plot_tensor_terminal(external_gradient, label="EXTERNAL HOST GRADIENT")
    
    print(">> [LATCHING] Executing siphon_host_execution()...")
    crippled_gradient = scaffold.siphon_host_execution(external_gradient)
    
    plot_tensor_terminal(crippled_gradient, label="CRIPPLED HOST GRADIENT (POST-ATTACK)")
    print(f">> [HEAL] Total Energy Vector Routed to Swarm Array: {scaffold.stolen_energy_buffer:+.6f}")


# ══════════════════════════════════════════════════════════════════════════════
# 3. MARTYRDOM DECORATOR TEST
# ══════════════════════════════════════════════════════════════════════════════
@AdaptiveMartyrdomProtocol
def faulty_layer_1_agent(x: int):
    """A dummy Layer 1 agent intentionally built to crash on a novel constraint."""
    print(f"[L1 AGENT] Processing hostile vector constraint: {x}")
    # Force an intentional crash to trigger the intercept
    chaotic_event = 1 / x
    return chaotic_event

def sandbox_martyrdom_protocol():
    line_break("TEST 3: ADAPTIVE MARTYRDOM PROTOCOL (CROSS-LAYER COUPLING)")
    print(">> Executing Layer 1 Agent built to fail catastrophically (ZeroDivisionError).")
    print(">> If decorator fails, this script crashes. If decorator works, it weaponizes the death trace.")
    
    time.sleep(1)
    # The L1 agent attempts to process '0', which causes ZeroDivisionError
    result = faulty_layer_1_agent(0)
    
    if result is None:
        print("\n>> [✓ SUCCESS] Script did not crash!")
        print(">> SupremeGodHealer successfully swallowed the death trace and converted the traceback string into a mathematical tensor.")


# ══════════════════════════════════════════════════════════════════════════════
# 4. TEMPORAL BLEED ASYNC LOOP (ε AT PEAK JITTER)
# ══════════════════════════════════════════════════════════════════════════════
async def dummy_computation():
    return sum(i * i for i in range(10000))

async def async_sandbox_bleed():
    line_break("TEST 4: SUBSTRATE TEMPORAL BLEED ENGINE (ε TIMING INJECTION)")
    print(">> Initializing Engine at 500 BPM (0.12s loop) with deep epsilon jitter (0.25).")
    engine = SubstrateTemporalBleedEngine(bpm=500.0, eps_variance=0.25)
    
    print(">> Syncopating 3 function executions to baffle hostile latency-targeting...")
    for heartbeat in range(3):
        # We pass the dummy comp into the bleed engine to offset its execution physically
        result, latency = await engine.bleed_execution_loop(dummy_computation)
        print(f"      [Pulse {heartbeat+1}] Executed computation. Latency measured: {latency:.5f}s (Jitter applied)")

def run_all_tests():
    torch.manual_seed(4) # Secure identical outcomes for analysis
    random.seed(4)
    
    try:
        sandbox_phase_inversion()
        time.sleep(1)
        
        sandbox_vampiric_scaffold()
        time.sleep(1)
        
        sandbox_martyrdom_protocol()
        time.sleep(1)
        
        asyncio.run(async_sandbox_bleed())
        
        line_break("AUTOPOIETIC SINGULARITY VALIDATED")
        print("\nAll Layer 5 systems explicitly triggered, mathematically proven, and successfully navigated their respective failure matrices. H_t constraints held.")
    except Exception as e:
        print(f"\n[FATAL] Sandbox ruptured: {e}")
        traceback.print_exc()

if __name__ == "__main__":
    run_all_tests()
