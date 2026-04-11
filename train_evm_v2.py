#!/usr/bin/env python3
"""
🔱 EVM Energy Classifier — Live Training Pipeline v2.0.0
═════════════════════════════════════════════════════════

Trains the EVMEnergyClassifier on real Etherscan contract data
mapped through the PRIME energy bridge.

Input:  prime_training_data.json  (from etherscan_to_prime.py)
Output: evm_classifier_v2.pt     (trained model weights)
        training_report.json      (metrics, loss curves, confusion)

Architecture:
  23-D input → 128 → ReLU → Dropout(0.3) → 64 → ReLU → Dropout(0.2) → 19 classes
  Optimizer: AdamW (lr=1e-3, weight_decay=1e-2)
  Scheduler: CosineAnnealingLR (T_max=50)
  Loss: CrossEntropyLoss with class weights (handles imbalance)

PRIME Integration:
  - Features 0-18:  vulnerability energy vector (from Solidity analysis)
  - Features 19-22: PRIME Hamiltonian dimensions (base_cost, branch, call, energy)
  - The PRIME dimensions act as structural priors that regularize the vuln classifier

Usage:
    python3 train_evm_v2.py [training_data.json] [--epochs 100] [--lr 0.001]
"""

import json
import sys
import math
import time
import argparse
from pathlib import Path
from collections import defaultdict

try:
    import torch
    import torch.nn as nn
    import torch.optim as optim
    from torch.utils.data import DataLoader, TensorDataset, random_split
    HAS_TORCH = True
except ImportError:
    HAS_TORCH = False


# ═══════════════════════════════════════════════════════════════════
# Model Architecture
# ═══════════════════════════════════════════════════════════════════

if HAS_TORCH:

    class EVMEnergyClassifier(nn.Module):
        """
        23-D PRIME energy features → 19 vulnerability classes.

        Architecture designed for the specific feature structure:
        - First layer (128) captures cross-feature interactions
        - Second layer (64) compresses to vulnerability-relevant representations
        - Dropout prevents overfitting on small corpus sizes
        """

        def __init__(self, input_dim=23, num_classes=19, hidden_dims=(128, 64),
                     dropout_rates=(0.3, 0.2)):
            super().__init__()

            layers = []
            prev_dim = input_dim

            for i, (h_dim, drop) in enumerate(zip(hidden_dims, dropout_rates)):
                layers.extend([
                    nn.Linear(prev_dim, h_dim),
                    nn.BatchNorm1d(h_dim),
                    nn.ReLU(inplace=True),
                    nn.Dropout(drop),
                ])
                prev_dim = h_dim

            self.features = nn.Sequential(*layers)
            self.classifier = nn.Linear(prev_dim, num_classes)

            # Xavier initialization
            self._init_weights()

        def _init_weights(self):
            for m in self.modules():
                if isinstance(m, nn.Linear):
                    nn.init.xavier_uniform_(m.weight)
                    if m.bias is not None:
                        nn.init.zeros_(m.bias)

        def forward(self, x):
            h = self.features(x)
            return self.classifier(h)

        def predict_energy(self, x):
            """Return softmax probabilities (vulnerability energy distribution)."""
            with torch.no_grad():
                logits = self.forward(x)
                return torch.softmax(logits, dim=-1)


    # ═══════════════════════════════════════════════════════════════
    # PRIME-Adaptive Learning Rate Modulator
    # ═══════════════════════════════════════════════════════════════

    class PRIMELRModulator:
        """
        Modulates per-parameter learning rate using PRIME Hamiltonian dynamics.

        H_t(p) = H_0(p) + η·H_{t-1}(p)·σ(γ·H_{t-1}(p)) + ε·N(0, 1+β|H_{t-1}(p)|)

        This is ADDITIVE to AdamW (lesson from V10→V11 CATALYZE):
        the Hamiltonian field modulates per-parameter scaling only,
        never touches the base learning rate schedule.
        """

        def __init__(self, eta=0.4, gamma=0.3, beta=0.1, sigma=0.02):
            self.eta = eta
            self.gamma = gamma
            self.beta = beta
            self.sigma = sigma
            self.H_prev = {}
            self.step_count = 0
            self.phase_cooldown = 0

        def modulate(self, named_parameters):
            """
            Compute per-parameter energy modulation.
            Returns dict of parameter_name → scale_factor.
            """
            scales = {}
            self.step_count += 1

            for name, param in named_parameters:
                if param.grad is None:
                    continue

                # H_0: gradient magnitude squared (base energy)
                h0 = param.grad.data.norm().item() ** 2

                # Recursive feedback
                h_prev = self.H_prev.get(name, h0)
                sigmoid = 1.0 / (1.0 + math.exp(-self.gamma * min(h_prev, 20)))
                noise = torch.randn(1).item() * (1.0 + self.beta * abs(h_prev))

                # PRIME evolution
                h_t = h0 + self.eta * h_prev * sigmoid + self.sigma * noise

                # Clamp to prevent explosion
                h_t = max(min(h_t, 100.0), 0.001)
                self.H_prev[name] = h_t

                # Scale factor: high energy → larger step (explore), low → smaller (exploit)
                # Centered around 1.0 so it doesn't dominate AdamW
                scale = 1.0 + 0.1 * math.tanh(h_t - 1.0)
                scales[name] = scale

            return scales


# ═══════════════════════════════════════════════════════════════════
# Data Loading
# ═══════════════════════════════════════════════════════════════════

def load_training_data(path: str) -> tuple:
    """Load and validate PRIME training data."""
    with open(path) as f:
        data = json.load(f)

    samples = data.get('samples', [])
    if not samples:
        raise ValueError("No samples in training data")

    features = [s['features'] for s in samples]
    labels = [s['label'] for s in samples]

    meta = data.get('metadata', {})
    class_names = data.get('class_names', [f'class_{i}' for i in range(19)])

    return features, labels, class_names, meta


def compute_class_weights(labels: list, num_classes: int = 19) -> list:
    """Inverse frequency class weights for imbalanced data."""
    counts = defaultdict(int)
    for l in labels:
        counts[l] += 1

    total = len(labels)
    weights = []
    for i in range(num_classes):
        c = counts.get(i, 0)
        if c > 0:
            weights.append(total / (num_classes * c))
        else:
            weights.append(1.0)

    # Normalize so mean weight = 1.0
    mean_w = sum(weights) / len(weights)
    return [w / mean_w for w in weights]


# ═══════════════════════════════════════════════════════════════════
# Training Loop
# ═══════════════════════════════════════════════════════════════════

def train(args):
    """Full training pipeline."""

    print("🔱 EVM Energy Classifier — Training Pipeline v2.0.0")
    print("=" * 64)

    # Load data
    data_path = args.data
    if not Path(data_path).exists():
        print(f"[ERROR] Training data not found: {data_path}")
        print("  Run: python3 etherscan_to_prime.py first")
        sys.exit(1)

    features, labels, class_names, meta = load_training_data(data_path)
    print(f"  Data:     {data_path}")
    print(f"  Samples:  {len(features)}")
    print(f"  Features: {len(features[0])}-D")
    print(f"  Classes:  {len(class_names)}")

    # Tensors
    X = torch.tensor(features, dtype=torch.float32)
    y = torch.tensor(labels, dtype=torch.long)

    # Class weights
    weights = compute_class_weights(labels)
    weight_tensor = torch.tensor(weights, dtype=torch.float32)

    # Train/val split (80/20)
    total = len(X)
    train_size = int(0.8 * total)
    val_size = total - train_size

    dataset = TensorDataset(X, y)
    train_set, val_set = random_split(
        dataset, [train_size, val_size],
        generator=torch.Generator().manual_seed(42)
    )

    train_loader = DataLoader(train_set, batch_size=args.batch_size, shuffle=True)
    val_loader = DataLoader(val_set, batch_size=args.batch_size, shuffle=False)

    print(f"  Train:    {train_size} samples")
    print(f"  Val:      {val_size} samples")
    print(f"  Batch:    {args.batch_size}")
    print(f"  Epochs:   {args.epochs}")
    print(f"  LR:       {args.lr}")
    print(f"  Device:   {'cuda' if torch.cuda.is_available() else 'cpu'}")

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

    # Model
    model = EVMEnergyClassifier(
        input_dim=len(features[0]),
        num_classes=len(class_names),
    ).to(device)

    param_count = sum(p.numel() for p in model.parameters())
    print(f"  Params:   {param_count:,}")

    # Optimizer: AdamW (the proven baseline)
    optimizer = optim.AdamW(
        model.parameters(),
        lr=args.lr,
        weight_decay=args.weight_decay,
        betas=(0.9, 0.999),
    )

    # Scheduler: cosine annealing
    scheduler = optim.lr_scheduler.CosineAnnealingLR(
        optimizer, T_max=args.epochs, eta_min=args.lr * 0.01
    )

    # Loss with class weights
    criterion = nn.CrossEntropyLoss(weight=weight_tensor.to(device))

    # PRIME modulator (additive, not multiplicative — V11 lesson)
    prime_mod = PRIMELRModulator(
        eta=args.prime_eta,
        gamma=args.prime_gamma,
        beta=args.prime_beta,
        sigma=args.prime_sigma,
    )

    # Training history
    history = {
        'train_loss': [],
        'val_loss': [],
        'train_acc': [],
        'val_acc': [],
        'lr': [],
        'prime_energy': [],
    }

    best_val_acc = 0.0
    best_epoch = 0
    start_time = time.time()

    print(f"\n{'='*64}")
    print(f"  {'Epoch':>5} {'Train Loss':>11} {'Val Loss':>9} "
          f"{'Train Acc':>10} {'Val Acc':>8} {'LR':>10} {'H_mean':>8}")
    print(f"  {'-'*62}")

    for epoch in range(1, args.epochs + 1):

        # ── Train ──
        model.train()
        train_loss = 0.0
        train_correct = 0
        train_total = 0

        for batch_X, batch_y in train_loader:
            batch_X, batch_y = batch_X.to(device), batch_y.to(device)

            optimizer.zero_grad()
            logits = model(batch_X)
            loss = criterion(logits, batch_y)
            loss.backward()

            # PRIME modulation: scale gradients by Hamiltonian energy
            if args.use_prime:
                scales = prime_mod.modulate(model.named_parameters())
                for name, param in model.named_parameters():
                    if param.grad is not None and name in scales:
                        param.grad.data *= scales[name]

            # Gradient clipping (stability)
            torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=1.0)

            optimizer.step()

            train_loss += loss.item() * batch_X.size(0)
            preds = logits.argmax(dim=1)
            train_correct += (preds == batch_y).sum().item()
            train_total += batch_X.size(0)

        train_loss /= max(train_total, 1)
        train_acc = train_correct / max(train_total, 1)

        # ── Validate ──
        model.eval()
        val_loss = 0.0
        val_correct = 0
        val_total = 0

        with torch.no_grad():
            for batch_X, batch_y in val_loader:
                batch_X, batch_y = batch_X.to(device), batch_y.to(device)
                logits = model(batch_X)
                loss = criterion(logits, batch_y)

                val_loss += loss.item() * batch_X.size(0)
                preds = logits.argmax(dim=1)
                val_correct += (preds == batch_y).sum().item()
                val_total += batch_X.size(0)

        val_loss /= max(val_total, 1)
        val_acc = val_correct / max(val_total, 1)

        # Scheduler step
        scheduler.step()
        current_lr = scheduler.get_last_lr()[0]

        # PRIME energy average
        h_mean = sum(prime_mod.H_prev.values()) / max(len(prime_mod.H_prev), 1) \
                 if prime_mod.H_prev else 0.0

        # Record history
        history['train_loss'].append(round(train_loss, 6))
        history['val_loss'].append(round(val_loss, 6))
        history['train_acc'].append(round(train_acc, 4))
        history['val_acc'].append(round(val_acc, 4))
        history['lr'].append(round(current_lr, 8))
        history['prime_energy'].append(round(h_mean, 4))

        # Best model tracking
        if val_acc > best_val_acc:
            best_val_acc = val_acc
            best_epoch = epoch
            if args.save_model:
                torch.save({
                    'epoch': epoch,
                    'model_state_dict': model.state_dict(),
                    'optimizer_state_dict': optimizer.state_dict(),
                    'val_acc': val_acc,
                    'class_names': class_names,
                    'feature_dim': len(features[0]),
                }, args.output_model)

        # Print every N epochs
        if epoch % max(args.epochs // 20, 1) == 0 or epoch == 1 or epoch == args.epochs:
            marker = ' ◀' if epoch == best_epoch else ''
            print(f"  {epoch:>5} {train_loss:>11.6f} {val_loss:>9.6f} "
                  f"{train_acc:>9.2%} {val_acc:>7.2%} {current_lr:>10.6f} "
                  f"{h_mean:>8.3f}{marker}")

    elapsed = time.time() - start_time

    # ── Results ──
    print(f"\n{'='*64}")
    print(f"  TRAINING COMPLETE")
    print(f"{'='*64}")
    print(f"  Time:         {elapsed:.1f}s")
    print(f"  Best val acc: {best_val_acc:.2%} (epoch {best_epoch})")
    print(f"  Final LR:     {history['lr'][-1]:.8f}")

    if args.save_model:
        print(f"  Model saved:  {args.output_model}")

    # ── Per-class accuracy (final epoch) ──
    model.eval()
    class_correct = defaultdict(int)
    class_total = defaultdict(int)

    with torch.no_grad():
        for batch_X, batch_y in val_loader:
            batch_X, batch_y = batch_X.to(device), batch_y.to(device)
            preds = model(batch_X).argmax(dim=1)
            for pred, label in zip(preds, batch_y):
                class_total[label.item()] += 1
                if pred.item() == label.item():
                    class_correct[label.item()] += 1

    print(f"\n  Per-Class Validation Accuracy:")
    for i, name in enumerate(class_names):
        total = class_total.get(i, 0)
        correct = class_correct.get(i, 0)
        acc = correct / max(total, 1)
        if total > 0:
            bar = '█' * int(acc * 20)
            print(f"    {name:<25} {correct:>3}/{total:<3} {acc:>6.1%} {bar}")

    # ── Save training report ──
    report = {
        'config': {
            'epochs': args.epochs,
            'lr': args.lr,
            'batch_size': args.batch_size,
            'weight_decay': args.weight_decay,
            'prime_enabled': args.use_prime,
            'prime_params': {
                'eta': args.prime_eta,
                'gamma': args.prime_gamma,
                'beta': args.prime_beta,
                'sigma': args.prime_sigma,
            },
        },
        'results': {
            'best_val_acc': round(best_val_acc, 4),
            'best_epoch': best_epoch,
            'final_train_loss': history['train_loss'][-1],
            'final_val_loss': history['val_loss'][-1],
            'training_time_s': round(elapsed, 1),
            'param_count': param_count,
        },
        'history': history,
        'class_names': class_names,
    }

    report_path = args.output_model.replace('.pt', '_report.json')
    with open(report_path, 'w') as f:
        json.dump(report, f, indent=2)
    print(f"  Report:       {report_path}")

    # Ouroboros-scrapable summary
    print(f"\n🔱 EVM-TRAIN: {len(features)} samples, {args.epochs} epochs, "
          f"best_acc={best_val_acc:.2%} @ epoch {best_epoch}, "
          f"params={param_count:,}, "
          f"PRIME={'ON' if args.use_prime else 'OFF'}")


# ═══════════════════════════════════════════════════════════════════
# Fallback: NumPy-only training (no PyTorch)
# ═══════════════════════════════════════════════════════════════════

def train_numpy_fallback(args):
    """Minimal softmax classifier using only NumPy — for environments without PyTorch."""
    import numpy as np

    print("🔱 EVM Energy Classifier — NumPy Fallback Mode")
    print("=" * 64)
    print("  [WARN] PyTorch not available, using NumPy softmax classifier")

    features, labels, class_names, meta = load_training_data(args.data)

    X = np.array(features, dtype=np.float64)
    y = np.array(labels, dtype=np.int64)
    num_classes = 19

    # Train/val split
    n = len(X)
    idx = np.random.RandomState(42).permutation(n)
    split = int(0.8 * n)
    train_idx, val_idx = idx[:split], idx[split:]

    X_train, y_train = X[train_idx], y[train_idx]
    X_val, y_val = X[val_idx], y[val_idx]

    # Softmax classifier weights
    d = X.shape[1]
    W = np.random.randn(d, num_classes) * 0.01
    b = np.zeros(num_classes)

    lr = args.lr
    best_acc = 0.0

    print(f"  Samples: {n} (train={len(X_train)}, val={len(X_val)})")
    print(f"  Features: {d}-D, Classes: {num_classes}")
    print()

    for epoch in range(1, args.epochs + 1):
        # Forward
        logits = X_train @ W + b
        logits -= logits.max(axis=1, keepdims=True)  # stability
        exp_logits = np.exp(logits)
        probs = exp_logits / exp_logits.sum(axis=1, keepdims=True)

        # Cross-entropy loss
        log_probs = -np.log(probs[np.arange(len(y_train)), y_train] + 1e-12)
        loss = log_probs.mean()

        # Gradients
        grad_logits = probs.copy()
        grad_logits[np.arange(len(y_train)), y_train] -= 1
        grad_logits /= len(y_train)

        dW = X_train.T @ grad_logits
        db = grad_logits.sum(axis=0)

        # AdamW-style weight decay
        dW += args.weight_decay * W

        # Update
        W -= lr * dW
        b -= lr * db

        # Validation
        val_logits = X_val @ W + b
        val_preds = val_logits.argmax(axis=1)
        val_acc = (val_preds == y_val).mean()

        if val_acc > best_acc:
            best_acc = val_acc

        if epoch % max(args.epochs // 10, 1) == 0 or epoch == 1:
            train_preds = (X_train @ W + b).argmax(axis=1)
            train_acc = (train_preds == y_train).mean()
            print(f"  Epoch {epoch:>4}: loss={loss:.4f}  "
                  f"train_acc={train_acc:.2%}  val_acc={val_acc:.2%}")

        # LR decay
        lr *= 0.999

    print(f"\n  Best val accuracy: {best_acc:.2%}")

    # Save weights as JSON
    output = {
        'weights': W.tolist(),
        'bias': b.tolist(),
        'best_acc': round(float(best_acc), 4),
        'class_names': class_names,
    }
    out_path = args.output_model.replace('.pt', '_numpy.json')
    with open(out_path, 'w') as f:
        json.dump(output, f)
    print(f"  Saved: {out_path}")

    print(f"\n🔱 EVM-TRAIN-NUMPY: {n} samples, {args.epochs} epochs, "
          f"best_acc={best_acc:.2%}")


# ═══════════════════════════════════════════════════════════════════
# CLI
# ═══════════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(description='🔱 EVM Energy Classifier Training')
    parser.add_argument('data', nargs='?', default='prime_training_data.json',
                        help='Training data JSON (from etherscan_to_prime.py)')
    parser.add_argument('--epochs', type=int, default=100)
    parser.add_argument('--lr', type=float, default=1e-3)
    parser.add_argument('--batch-size', type=int, default=32)
    parser.add_argument('--weight-decay', type=float, default=1e-2)
    parser.add_argument('--output-model', default='evm_classifier_v2.pt')
    parser.add_argument('--save-model', action='store_true', default=True)
    parser.add_argument('--no-save', dest='save_model', action='store_false')

    # PRIME modulator params
    parser.add_argument('--use-prime', action='store_true', default=True,
                        help='Enable PRIME Hamiltonian LR modulation')
    parser.add_argument('--no-prime', dest='use_prime', action='store_false')
    parser.add_argument('--prime-eta', type=float, default=0.4)
    parser.add_argument('--prime-gamma', type=float, default=0.3)
    parser.add_argument('--prime-beta', type=float, default=0.1)
    parser.add_argument('--prime-sigma', type=float, default=0.02)

    args = parser.parse_args()

    if HAS_TORCH:
        train(args)
    else:
        train_numpy_fallback(args)


if __name__ == '__main__':
    main()
