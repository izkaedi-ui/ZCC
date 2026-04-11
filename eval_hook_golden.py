"""
ZKAEDI Eval Hook — Golden Canonical Holdout
============================================
Immune system for the Ouroboros Daemon.

Two-track evaluation:
  Track A — shifting validation set (current distribution)
  Track B — frozen canonical holdout (immutable, seeded once at forge time)

The Daemon should only accept a new FUSED.pt if BOTH tracks pass.
Track B degradation is the early warning for basin drift / recency collapse.
"""

import torch
import torch.nn as nn
import torchvision
import torchvision.transforms as T
import numpy as np
import hashlib
import json
import os
from pathlib import Path
from datetime import datetime, timezone
from dataclasses import dataclass, asdict
from typing import Optional, Tuple
import sys

# ── Constants ────────────────────────────────────────────────────────────────

CANONICAL_SEED       = 20240101          # never change this
CANONICAL_N          = 2000             # 200 samples per class, fixed forever
CANONICAL_HASH_FILE  = "canonical_holdout.sha256"
CANONICAL_DATA_FILE  = "canonical_holdout.pt"

CIFAR10_MEAN = (0.4914, 0.4822, 0.4465)
CIFAR10_STD  = (0.2023, 0.1994, 0.2010)

BASELINE_ACCURACY: float = 85.5            # floor: reject anything below this
DEGRADATION_FLOOR: float = 0.8             # reject if canonical acc < 80% of baseline


# ── Data structures ──────────────────────────────────────────────────────────

@dataclass
class EvalResult:
    timestamp: str
    model_path: str
    track_a_acc: float          # shifting validation
    track_b_acc: float          # golden canonical (the immune system)
    track_b_delta: float        # vs. baseline — negative = regression
    canonical_hash: str
    passed: bool
    rejection_reason: Optional[str]

    def to_json(self) -> str:
        return json.dumps(asdict(self), indent=2)  # type: ignore


# ── Canonical holdout forge (run once, then immutable) ───────────────────────

def forge_canonical_holdout(save_dir: str = ".") -> str:
    """
    Build the immutable golden evaluation set from the full CIFAR-10 test split.
    Seeded with CANONICAL_SEED so the exact same 2000 samples are always chosen.
    
    Run this ONCE at project start. The resulting .pt file must never be modified.
    Returns the SHA-256 hash of the forged tensor (store this in source control).
    """
    save_path = Path(save_dir) / CANONICAL_DATA_FILE
    hash_path = Path(save_dir) / CANONICAL_HASH_FILE

    if save_path.exists():
        raise FileExistsError(
            f"Canonical holdout already exists at {save_path}. "
            "Delete it manually only if you are intentionally resetting the immune system "
            "(this will break comparability with all previous eval records)."
        )

    rng = np.random.default_rng(CANONICAL_SEED)
    transform = T.Compose([T.ToTensor(), T.Normalize(CIFAR10_MEAN, CIFAR10_STD)])

    full_test = torchvision.datasets.CIFAR10(
        root="/tmp/cifar10_eval", train=False, download=True, transform=transform
    )

    per_class = CANONICAL_N // 10
    indices = []
    labels = np.array(full_test.targets)
    for cls in range(10):
        cls_idx = np.where(labels == cls)[0]
        chosen  = rng.choice(cls_idx, size=per_class, replace=False)
        indices.extend(chosen.tolist())

    rng.shuffle(indices)

    images = torch.stack([full_test[i][0] for i in indices])
    targets = torch.tensor([full_test[i][1] for i in indices], dtype=torch.long)

    payload = {"images": images, "targets": targets, "seed": CANONICAL_SEED, "n": CANONICAL_N}
    torch.save(payload, save_path)

    raw = save_path.read_bytes()
    sha = hashlib.sha256(raw).hexdigest()
    hash_path.write_text(sha)

    print(f"Canonical holdout forged: {save_path}")
    print(f"SHA-256: {sha}")
    print("Store this hash in source control. Any mismatch = holdout corruption.")
    return sha


# ── Holdout loader with integrity check ─────────────────────────────────────

def load_canonical_holdout(save_dir: str = ".") -> tuple[torch.Tensor, torch.Tensor]:
    """Load and verify the canonical holdout. Raises on corruption."""
    save_path = Path(save_dir) / CANONICAL_DATA_FILE
    hash_path = Path(save_dir) / CANONICAL_HASH_FILE

    if not save_path.exists():
        raise FileNotFoundError(
            f"Canonical holdout not found at {save_path}. "
            "Run forge_canonical_holdout() once to create it."
        )

    expected_hash = hash_path.read_text().strip()
    actual_hash   = hashlib.sha256(save_path.read_bytes()).hexdigest()

    if actual_hash != expected_hash:
        raise RuntimeError(
            f"Canonical holdout CORRUPTED.\n"
            f"  Expected: {expected_hash}\n"
            f"  Got:      {actual_hash}\n"
            "The immune system has been tampered with. Do not proceed."
        )

    payload = torch.load(save_path, map_location="cpu", weights_only=True)
    return payload["images"], payload["targets"]


# ── Accuracy measurement ──────────────────────────────────────────────────────

@torch.no_grad()
def measure_accuracy(
    model: nn.Module,
    images: torch.Tensor,
    targets: torch.Tensor,
    device: str = "cpu",
    batch_size: int = 256,
) -> float:
    model.eval()
    model.to(device)
    images  = images.to(device)
    targets = targets.to(device)

    correct = 0
    total   = len(targets)

    for i in range(0, total, batch_size):
        xb = images[i : i + batch_size]
        yb = targets[i : i + batch_size]
        logits = model(xb)
        preds  = logits.argmax(dim=1)
        correct += torch.eq(preds, yb).sum().item()

    val = 100.0 * correct / total
    return float(int(val * 100.0 + 0.5)) / 100.0  # type: ignore


# ── Shifting validation track (Track A) ──────────────────────────────────────

def build_shifting_val_loader(val_data_dir: Optional[str] = None) -> tuple[torch.Tensor, torch.Tensor]:
    """
    Track A: current distribution validation.
    If val_data_dir is None, falls back to the standard CIFAR-10 test set.
    In production, point this at your most recent labeled batch.
    """
    transform = T.Compose([T.ToTensor(), T.Normalize(CIFAR10_MEAN, CIFAR10_STD)])

    if val_data_dir:
        dataset = torchvision.datasets.ImageFolder(val_data_dir, transform=transform)
    else:
        dataset = torchvision.datasets.CIFAR10(
            root="/tmp/cifar10_eval", train=False, download=True, transform=transform
        )

    images  = torch.stack([dataset[i][0] for i in range(len(dataset))])
    targets = torch.tensor([dataset[i][1] for i in range(len(dataset))], dtype=torch.long)
    return images, targets


# ── Primary eval hook (called by the Daemon) ─────────────────────────────────

def eval_hook(
    model_path: str,
    model_class: type,
    holdout_dir: str = ".",
    val_data_dir: Optional[str] = None,
    device: str = "cpu",
    log_path: str = "eval_log.jsonl",
) -> Tuple[EvalResult, float]:
    """
    Two-track evaluation hook. Drop this into your Daemon's assimilation loop.

    Usage:
        result = eval_hook(
            model_path="ZKAEDI_OMEGA_FUSED.pt",
            model_class=YourCNN,
            holdout_dir="./canonical",
        )
        if result.passed:
            deploy(model_path)
        else:
            alert(result.rejection_reason)
    """
    hash_path = Path(holdout_dir) / CANONICAL_HASH_FILE
    canonical_hash = hash_path.read_text().strip() if hash_path.exists() else "unknown"

    model = model_class()
    state = torch.load(model_path, map_location="cpu", weights_only=True)

    if isinstance(state, dict) and "model" in state:
        state = state["model"]
    model.load_state_dict(state)

    # ── Track A: shifting distribution ───────────────────────────────────────
    val_images, val_targets = build_shifting_val_loader(val_data_dir)
    track_a = measure_accuracy(model, val_images, val_targets, device=device)

    # ── Track B: golden canonical (immune system) ─────────────────────────────
    canon_images, canon_targets = load_canonical_holdout(holdout_dir)
    track_b = measure_accuracy(model, canon_images, canon_targets, device=device)

    delta    = float(int((track_b - BASELINE_ACCURACY) * 100.0 + 0.5)) / 100.0  # type: ignore
    floor_b  = float(int((BASELINE_ACCURACY * DEGRADATION_FLOOR) * 100.0 + 0.5)) / 100.0  # type: ignore

    rejection = None
    if track_a < BASELINE_ACCURACY:
        rejection = f"Track A below baseline: {track_a:.1f}% < {BASELINE_ACCURACY}%"
    elif track_b < floor_b:
        rejection = (
            f"Track B canonical regression: {track_b:.1f}% < floor {floor_b:.1f}% "
            f"(Δ = {delta:+.1f}%). Possible recency collapse or disjoint basin merge."
        )

    result = EvalResult(
        timestamp        = datetime.now(timezone.utc).isoformat(),
        model_path       = model_path,
        track_a_acc      = track_a,
        track_b_acc      = track_b,
        track_b_delta    = delta,
        canonical_hash   = canonical_hash,
        passed           = rejection is None,
        rejection_reason = rejection,
    )

    with open(log_path, "a") as f:
        f.write(result.to_json() + "\n")

    status = "PASS" if result.passed else "REJECT"
    # Print rich logs to stderr so they don't break stdout floating point reading
    print(f"[eval_hook] {status} | Track A: {track_a:.1f}% | Track B: {track_b:.1f}% (Δ{delta:+.1f}%)", file=sys.stderr)
    if rejection:
        print(f"[eval_hook] Reason: {rejection}", file=sys.stderr)

    # Return the inverted Track B Accuracy as the official Loss metric 
    # (The ZKAEDI SDE minimizes this value; 100.0% accuracy = 0.0 Loss)
    return result, float(100.0 - track_b)


# ── CLI: forge or evaluate ────────────────────────────────────────────────────

if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="ZKAEDI Golden Holdout Eval Hook")
    sub = parser.add_subparsers(dest="cmd")

    forge_cmd = sub.add_parser("forge", help="Forge the canonical holdout (run once)")
    forge_cmd.add_argument("--dir", default=".", help="Directory to save holdout")

    verify_cmd = sub.add_parser("verify", help="Verify holdout integrity")
    verify_cmd.add_argument("--dir", default=".", help="Directory containing holdout")

    eval_cmd = sub.add_parser("eval", help="Evaluate a model checkpoint via the Golden Hook")
    eval_cmd.add_argument("pt_path", type=str, help="Path to the .pt checkpoint")
    eval_cmd.add_argument("--dir", default=".", help="Directory containing holdout")

    # If no arguments provided at all, fallback to pure eval logic to satisfy zkaedi_prime_fuser.py subprocess structure
    if len(sys.argv) == 2 and not sys.argv[1] in ["forge", "verify", "eval", "-h", "--help"]:
        # The SDE orchestrator invokes subprocess as `python eval_hook.py <pt_path>`
        # We auto-intercept this as `eval`
        pt_path = sys.argv[1]
        try:
            from model import CompactCNN # type: ignore # User must ensure this exists
        except ImportError:
            # Fallback mock for the fuser if architecture is missing
            raise RuntimeError("CRITICAL: Must define or import 'CompactCNN' in eval_hook_golden.py to perform live inference.")
            
        res, loss = eval_hook(pt_path, model_class=CompactCNN, holdout_dir=".")
        # Only print the float Loss to stdout, so the genetic algorithm can read it natively
        print(f"{loss:.8f}")
        sys.exit(0 if res.passed else 1)

    args = parser.parse_args()

    if args.cmd == "forge":
        forge_canonical_holdout(args.dir)

    elif args.cmd == "verify":
        try:
            load_canonical_holdout(args.dir)
            print("Holdout integrity verified.")
        except RuntimeError as e:
            print(f"INTEGRITY FAILURE: {e}")
            sys.exit(1)

    elif args.cmd == "eval":
        try:
            from model import CompactCNN # type: ignore # User must ensure this exists
        except ImportError:
            raise RuntimeError("CRITICAL: Must define or import 'CompactCNN' to run standalone evaluation.")
        
        res, loss = eval_hook(args.pt_path, model_class=CompactCNN, holdout_dir=args.dir)
        print(f"{loss:.8f}")
        sys.exit(0 if res.passed else 1)
    else:
        parser.print_help()
