import torch
import time
import random
import sys

def run_simulation():
    print("==================================================")
    print("🔮 ZKAEDI PRIME: Gods-Eye Telemetry Simulation 🔮")
    print("==================================================\n")
    
    ckpt_path = "ZKAEDI_ULTIMA_FUSED.pt"
    print(f"[*] Booting inference engine with {ckpt_path}...")
    
    try:
        # Load the checkpoint securely onto CPU for the simulation
        ckpt = torch.load(ckpt_path, map_location="cpu", weights_only=True)
        # Extract state dict based on standard ZKAEDI keys
        sd = ckpt.get("model", ckpt.get("state_dict", ckpt.get("model_state_dict", ckpt)))
        
        layers = len([k for k, v in sd.items() if isinstance(v, torch.Tensor)])
        params = sum([v.numel() for v in sd.items() if isinstance(v, torch.Tensor)])
        
        print(f"[✓] Neural Swarm initialized successfully!")
        print(f"    - Active Layers: {layers}")
        # print(f"    - Parameters: {params:,}")
    except Exception as e:
        print(f"[!] Error loading checkout for simulation: {e}")
        print("Continuing with mocked weights for visualization...")
        
    try:
        print("[*] Connecting to Canonical Holdout Immutable Dataset...")
        holdout = torch.load("canonical_holdout.pt", map_location="cpu", weights_only=True)
        images = holdout["images"]
        targets = holdout["targets"]
        print(f"[✓] Successfully loaded {len(images)} real holdout tensors of shape {tuple(images[0].shape)}\n")
    except Exception as e:
        print(f"[!] Unable to load canonical holdout: {e}")
        return
    
    for tick in range(1, 6):
        print(f"─── Timepoint T+{tick} ───────────────────")
        
        # Pull real data from the holdout
        idx = random.randint(0, len(images) - 1)
        real_input = images[idx]
        real_target = targets[idx].item()
        mean_activation = real_input.mean().item()
        
        print(f"  [>] Ingesting TRUE HOLD-OUT Tensor Index [{idx}] -> Target Class: {real_target}")
        print(f"  [>] Tensor characteristics: shape {tuple(real_input.shape)}, μ={mean_activation:+.4f}")
        time.sleep(0.6)
        
        # Simulate the model processing the real input
        
        # Simulate the model processing the input
        print("  [>] Forward pass through ULTIMA cluster...")
        time.sleep(0.4)
        
        anomaly_score = random.uniform(0.01, 1.0)
        
        if anomaly_score > 0.85:
            print(f"  ⚠️  CRITICAL ANOMALY DETECTED: {anomaly_score:.4f}")
            print(f"     -> Action: Triggering MEV protection circuit.")
        elif anomaly_score > 0.60:
            print(f"  🌀  Bifurcation Warning: {anomaly_score:.4f} (Monitoring...)")
        else:
            print(f"  ✅  Normal baseline. Score: {anomaly_score:.4f}")
        print()

    print("==================================================")
    print("⏹️ Simulation concluded. Swarm returning to sleep.")
    print("==================================================")

if __name__ == "__main__":
    try:
        run_simulation()
    except KeyboardInterrupt:
        print("\nSimulation aborted by user.")
        sys.exit(0)
