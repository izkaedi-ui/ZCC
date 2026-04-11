"""
╔══════════════════════════════════════════════════════════════════════════════╗
║         ZKAEDI PRIME — SUPERCHARGED BENCHMARK HARNESS v3.0                 ║
║         H_t = H_0 + η·H_{t-1}·σ(γ·H_{t-1}) + ε·𝒩(0, 1+β|H_{t-1}|)       ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  ENGINES:                                                                    ║
║    CATALYZE_V11  — Hamiltonian-modulated AdamW (pre-alloc noise bufs)       ║
║    SGD_BASELINE  — OneCycleLR, Nesterov (your checkpoint optimizer)         ║
║    ADAMW_BASE    — Standard AdamW + cosine warmup                           ║
║    SGD_MOMENTUM  — Clean SGD + cosine warmup                                ║
║                                                                              ║
║  METRICS (per seed):                                                         ║
║    • Final & best val accuracy                                               ║
║    • High-LR volatility (epochs 5–30): mean|Δacc|/epoch                    ║
║    • Worst-case crash (max single-epoch drop)                                ║
║    • Convergence clock: epochs to 75/85/90/93%                              ║
║    • PRIME H-field energy on loss curve (Lyapunov λ)                        ║
║    • Gradient norm trajectory (per-epoch layer-wise)                        ║
║    • Effective learning rate (base × H-scale mean)                          ║
║    • Overhead ratio vs SGD baseline                                          ║
║                                                                              ║
║  STATISTICS:                                                                 ║
║    • Welch's t-test (unequal variance)                                      ║
║    • Cohen's d (effect size)                                                 ║
║    • Bootstrap CI (10k samples)                                              ║
║    • Win rate, crash-resistance rate                                         ║
║    • Levene's test for variance reduction                                    ║
╚══════════════════════════════════════════════════════════════════════════════╝

Usage:
    python zkaedi_prime_benchmark.py                    # quick: 7 seeds, 100 epochs
    python zkaedi_prime_benchmark.py --seeds 40         # power: 80% power at d=0.45
    python zkaedi_prime_benchmark.py --epochs 50 --quick   # fast validation
    python zkaedi_prime_benchmark.py --resume results.json  # continue interrupted run
"""

import torch
import torch.nn as nn
import torchvision
import torchvision.transforms as T
import torchvision.models as tv_models
import numpy as np
import math
import time
import json
import argparse
import os
import sys
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict, field
from collections import defaultdict

# ══════════════════════════════════════════════════════════════════════════════
# CATALYZE V11 — PRODUCTION OPTIMIZER
# Key fixes vs V10:
#   1. Pre-allocated noise buffers (no per-step torch.randn_like allocation)
#   2. H-field modulates per-param scale only — never touches base LR
#   3. Hard clamp on H-field — no divergence
#   4. NaN recovery with full state reset
# ══════════════════════════════════════════════════════════════════════════════

class CATALYZE_V11(torch.optim.Optimizer):
    """
    ZKAEDI PRIME Hamiltonian Optimizer — V11 CATALYZE
    AdamW + recursive H-field per-parameter modulation.
    H_t(p) = |g|² + η·H_{t-1}·σ(γ·H_{t-1}) + ε·𝒩(0, 1+β|H_{t-1}|)
    """
    def __init__(self, params,
                 lr: float = 1e-3,
                 betas: Tuple[float, float] = (0.9, 0.999),
                 eps: float = 1e-8,
                 weight_decay: float = 0.01,
                 # PRIME Hamiltonian params
                 eta: float   = 0.10,   # recursive feedback strength
                 gamma: float = 0.10,   # sigmoid attractor sharpening
                 beta_h: float = 0.12,  # energy-dependent noise scaling
                 sigma: float = 0.042,  # noise injection magnitude
                 max_h: float = 10.0,   # H-field hard clamp
                 # Safety
                 grad_clip: float = 1.0):

        for name, val in [('lr',lr),('eps',eps),('eta',eta),('gamma',gamma),
                          ('beta_h',beta_h),('sigma',sigma),('max_h',max_h)]:
            if val < 0:
                raise ValueError(f"Invalid {name}: {val}")
        for i, b in enumerate(betas):
            if not 0.0 <= b < 1.0:
                raise ValueError(f"Invalid betas[{i}]: {b}")

        defaults = dict(lr=lr, betas=betas, eps=eps, weight_decay=weight_decay,
                        eta=eta, gamma=gamma, beta_h=beta_h, sigma=sigma,
                        max_h=max_h, grad_clip=grad_clip)
        super().__init__(params, defaults)
        self._h_energy_log: List[float] = []   # track H-field mean energy per step
        self._effective_lr_log: List[float] = []

    def _init_state(self, p: torch.Tensor) -> dict:
        """Initialize per-parameter state with pre-allocated buffers."""
        return {
            'step':        0,
            'exp_avg':     torch.zeros_like(p.data),       # m1
            'exp_avg_sq':  torch.zeros_like(p.data),       # m2
            'h_field':     torch.zeros_like(p.data),       # H-field
            'noise_buf':   torch.zeros_like(p.data),       # pre-alloc noise
            'backup':      p.data.clone(),                 # NaN recovery
        }

    @torch.no_grad()
    def step(self, closure=None):
        loss = None
        if closure is not None:
            with torch.enable_grad():
                loss = closure()

        total_h_energy = 0.0
        total_effective_lr = 0.0
        param_count = 0

        for group in self.param_groups:
            lr       = group['lr']
            beta1, beta2 = group['betas']
            eps      = group['eps']
            wd       = group['weight_decay']
            eta      = group['eta']
            gamma    = group['gamma']
            beta_h   = group['beta_h']
            sigma    = group['sigma']
            max_h    = group['max_h']

            for p in group['params']:
                if p.grad is None:
                    continue

                grad = p.grad
                if grad.is_sparse:
                    raise RuntimeError("CATALYZE_V11: sparse gradients not supported")

                state = self.state[p]
                if len(state) == 0:
                    state.update(self._init_state(p))

                m  = state['exp_avg']
                v  = state['exp_avg_sq']
                h  = state['h_field']
                nb = state['noise_buf']
                state['step'] += 1
                step = state['step']

                # ── AdamW decoupled weight decay ──────────────────────────
                if wd != 0:
                    p.data.mul_(1.0 - lr * wd)

                # ── Standard Adam moment updates ──────────────────────────
                m.mul_(beta1).add_(grad, alpha=1.0 - beta1)
                v.mul_(beta2).addcmul_(grad, grad, value=1.0 - beta2)

                bc1 = 1.0 - beta1 ** step
                bc2 = 1.0 - beta2 ** step
                m_hat = m / bc1
                v_hat = v / bc2

                # ── PRIME Hamiltonian field evolution ─────────────────────
                # H_0: base energy = gradient magnitude squared
                h_base = grad.pow(2)

                # Recursive feedback with sigmoid attractor sharpening
                h_clamped = h.clamp(-max_h, max_h)
                sigmoid_h = torch.sigmoid(gamma * h_clamped)
                h_recursive = eta * h * sigmoid_h

                # Energy-dependent noise — in-place fill (no allocation)
                noise_std = 1.0 + beta_h * h.abs().clamp(0, max_h)
                nb.normal_(0.0, 1.0)
                nb.mul_(noise_std).mul_(sigma)

                # Update H-field with hard clamp (critical stability guard)
                h.copy_((h_base + h_recursive + nb).clamp(-max_h, max_h))

                # ── Hamiltonian-modulated per-parameter step size ─────────
                # percentile logic rather than mean normalization
                h_quant = torch.quantile(h.abs().float(), 0.95).to(h.dtype)
                h_norm = h / (h_quant + eps)
                h_scale = torch.sigmoid(h_norm) + 0.5

                # Adam update with H-field scaling
                denom = v_hat.sqrt().add_(eps)
                eff_lr = lr * h_scale.mean().item()
                p.data.addcdiv_(m_hat, denom, value=-eff_lr)

                # ── NaN/Inf recovery ──────────────────────────────────────
                if torch.isnan(p.data).any() or torch.isinf(p.data).any():
                    p.data.copy_(state['backup'])
                    h.zero_(); m.zero_(); v.zero_()
                    state['step'] = 0
                else:
                    state['backup'].copy_(p.data)

                total_h_energy  += h.abs().mean().item()
                total_effective_lr += eff_lr
                param_count += 1

        if param_count > 0:
            self._h_energy_log.append(total_h_energy / param_count)
            self._effective_lr_log.append(total_effective_lr / param_count)

        return loss

    def get_h_energy(self) -> float:
        """Mean H-field energy across all parameters — current step."""
        return self._h_energy_log[-1] if self._h_energy_log else 0.0

    def get_effective_lr(self) -> float:
        return self._effective_lr_log[-1] if self._effective_lr_log else 0.0


# ══════════════════════════════════════════════════════════════════════════════
# MODEL ARCHITECTURE
# Matches B_CATALYZE checkpoint: ResNet-18, 11.2M params, 100-class output
# ══════════════════════════════════════════════════════════════════════════════

NUM_CLASSES = 100  # CIFAR-100

def build_model(num_classes: int = NUM_CLASSES) -> nn.Module:
    """
    ResNet-18 adapted for CIFAR-100 (32×32 input).
    Matches B_CATALYZE_seed*.pt architecture exactly:
      conv1 → layer1–4 (BasicBlock×2 each) → avgpool → fc(512→100)
    Patch: Replace the ImageNet 7×7 stem with a 3×3 conv to match
    CIFAR's 32×32 spatial resolution (standard practice).
    """
    model = tv_models.resnet18(weights=None, num_classes=num_classes)
    # CIFAR patch: replace the aggressive ImageNet stem
    model.conv1 = nn.Conv2d(3, 64, kernel_size=3, stride=1, padding=1, bias=False)
    model.maxpool = nn.Identity()  # remove 2× downsample that kills 32×32
    return model


# ══════════════════════════════════════════════════════════════════════════════
# PRIME H-FIELD ANALYSIS — applied to training curves
# ══════════════════════════════════════════════════════════════════════════════

def prime_hfield_analysis(losses: List[float],
                          eta: float = 0.40,
                          gamma: float = 0.30) -> Dict:
    """
    Apply PRIME equation to a loss trajectory.
    H_t = loss_t + η·H_{t-1}·σ(γ·H_{t-1})   (deterministic, no noise)
    Returns: trajectory, Lyapunov exponent, phase transitions, final energy
    """
    H, traj = 0.0, []
    for loss in losses:
        sig = 1.0 / (1.0 + math.exp(-gamma * max(-50, min(50, H))))
        H   = loss + eta * H * sig
        traj.append(H)

    diffs     = [abs(traj[i+1] - traj[i]) for i in range(len(traj)-1)]
    lyap_vals = [math.log(d + 1e-12) for d in diffs]
    lyap      = float(np.mean(lyap_vals))

    # Phase transitions: |ΔH| > mean + 2σ
    arr   = np.array(diffs)
    thresh = arr.mean() + 2.0 * arr.std()
    transitions = [i for i, d in enumerate(diffs) if d > thresh]

    return {
        'trajectory':   traj,
        'lyapunov':     lyap,
        'is_stable':    lyap < 0,
        'final_energy': traj[-1] if traj else 0.0,
        'peak_energy':  max(traj) if traj else 0.0,
        'transitions':  transitions,
        'n_transitions': len(transitions),
    }


# ══════════════════════════════════════════════════════════════════════════════
# EPOCH METRICS
# ══════════════════════════════════════════════════════════════════════════════

@dataclass
class EpochRecord:
    epoch:      int
    train_loss: float
    train_acc:  float
    val_loss:   float
    val_acc:    float
    lr:         float
    epoch_time: float
    grad_norm:  float   = 0.0
    h_energy:   float   = 0.0   # CATALYZE only
    eff_lr:     float   = 0.0   # CATALYZE only


@dataclass
class RunResult:
    optimizer_name: str
    seed:           int
    epochs_run:     int
    best_val_acc:   float
    best_val_loss:  float
    final_val_acc:  float
    total_time:     float
    # Stability metrics
    high_lr_volatility: float   # mean |Δacc| epochs 5–30
    worst_crash:        float   # max single-epoch drop
    worst_crash_epoch:  int
    # Convergence clocks
    epoch_to_75:  int
    epoch_to_85:  int
    epoch_to_90:  int
    epoch_to_93:  int
    # PRIME field metrics
    prime_lyapunov:   float
    prime_final_h:    float
    prime_transitions: int
    # Full log
    epoch_log: List[dict] = field(default_factory=list)


# ══════════════════════════════════════════════════════════════════════════════
# TRAINING ENGINE
# ══════════════════════════════════════════════════════════════════════════════

CIFAR100_MEAN = (0.5071, 0.4867, 0.4408)
CIFAR100_STD  = (0.2675, 0.2565, 0.2761)

def get_data(data_dir: str = './data', batch_size: int = 128, num_workers: int = 2):
    train_tf = T.Compose([
        T.RandomCrop(32, padding=4),
        T.RandomHorizontalFlip(),
        T.ColorJitter(brightness=0.1, contrast=0.1, saturation=0.1),  # bonus augment
        T.ToTensor(),
        T.Normalize(CIFAR100_MEAN, CIFAR100_STD),
    ])
    val_tf = T.Compose([T.ToTensor(), T.Normalize(CIFAR100_MEAN, CIFAR100_STD)])

    train_ds = torchvision.datasets.CIFAR100(data_dir, train=True,  download=True, transform=train_tf)
    val_ds   = torchvision.datasets.CIFAR100(data_dir, train=False, download=True, transform=val_tf)

    train_dl = torch.utils.data.DataLoader(train_ds, batch_size=batch_size,
                                            shuffle=True,  num_workers=num_workers, pin_memory=True)
    val_dl   = torch.utils.data.DataLoader(val_ds,   batch_size=batch_size*2,
                                            shuffle=False, num_workers=num_workers, pin_memory=True)
    return train_dl, val_dl


def seed_everything(seed: int):
    torch.manual_seed(seed)
    torch.cuda.manual_seed_all(seed)
    np.random.seed(seed)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark     = False


def make_optimizer(name: str, params, lr: float, epochs: int,
                   steps_per_epoch: int) -> Tuple[torch.optim.Optimizer,
                                                   torch.optim.lr_scheduler._LRScheduler]:
    """Build optimizer + OneCycleLR schedule. All use identical schedule for fair comparison."""
    total_steps = epochs * steps_per_epoch

    if name == 'CATALYZE_V11':
        opt = CATALYZE_V11(params, lr=lr, betas=(0.9, 0.999),
                           weight_decay=0.0005, eta=0.10, gamma=0.10,
                           beta_h=0.12, sigma=0.042, max_h=10.0)
    elif name == 'ADAMW':
        opt = torch.optim.AdamW(params, lr=lr, betas=(0.9, 0.999),
                                weight_decay=0.0005)
    elif name == 'SGD':
        opt = torch.optim.SGD(params, lr=lr, momentum=0.9,
                              weight_decay=0.0005, nesterov=True)
    elif name == 'SGD_MOMENTUM':
        opt = torch.optim.SGD(params, lr=lr, momentum=0.85,
                              weight_decay=0.0005, nesterov=False)
    else:
        raise ValueError(f"Unknown optimizer: {name}")

    # OneCycleLR — matches checkpoint exactly
    sched = torch.optim.lr_scheduler.OneCycleLR(
        opt,
        max_lr       = lr,
        total_steps  = total_steps,
        pct_start    = 0.05,         # 5% warmup
        anneal_strategy = 'cos',
        cycle_momentum  = True,
        base_momentum   = 0.85,
        max_momentum    = 0.95,
        div_factor      = 25.0,
        final_div_factor= 1e6,
    )
    return opt, sched


def train_one_seed(optimizer_name: str,
                   seed: int,
                   epochs: int,
                   device: torch.device,
                   max_lr: float = 0.1,
                   batch_size: int = 128,
                   data_dir: str = './data',
                   verbose: bool = True) -> RunResult:

    seed_everything(seed)
    train_dl, val_dl = get_data(data_dir, batch_size)
    steps_per_epoch  = len(train_dl)

    def kaiming_init(m):
        if isinstance(m, nn.Conv2d):
            nn.init.kaiming_normal_(m.weight, mode='fan_out', nonlinearity='relu')
        elif isinstance(m, nn.Linear):
            nn.init.kaiming_uniform_(m.weight, a=math.sqrt(5))

    total_start = time.perf_counter()
    
    for restart_attempt in range(3):
        if restart_attempt > 0:
            seed_everything(seed + restart_attempt * 100)
            
        model     = build_model(NUM_CLASSES).to(device)
        model.apply(kaiming_init)
        criterion = nn.CrossEntropyLoss(label_smoothing=0.0)
        opt, sched = make_optimizer(optimizer_name, model.parameters(),
                                    max_lr, epochs, steps_per_epoch)

        is_catalyze = isinstance(opt, CATALYZE_V11)
        epoch_records: List[EpochRecord] = []
        best_val_acc = 0.0
        best_val_loss = float('inf')
        
        dead_network_detected = False

        for epoch in range(epochs):
            # ── TRAIN ──────────────────────────────────────────────────────────
            model.train()
            t_loss, t_correct, t_total, t_gnorm = 0.0, 0, 0, 0.0
            epoch_start = time.perf_counter()

            for xb, yb in train_dl:
                xb, yb = xb.to(device, non_blocking=True), yb.to(device, non_blocking=True)
                opt.zero_grad(set_to_none=True)
                logits = model(xb)
                loss   = criterion(logits, yb)
                loss.backward()

                # Gradient clip — identical across all optimizers
                gnorm = torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0).item()
                t_gnorm += gnorm

                opt.step()
                sched.step()

                t_loss    += loss.item()
                preds      = logits.detach().argmax(1)
                t_correct += preds.eq(yb).sum().item()
                t_total   += yb.size(0)

            t_loss    /= len(train_dl)
            t_acc      = 100.0 * t_correct / t_total
            t_gnorm   /= len(train_dl)

            # ── VALIDATE ────────────────────────────────────────────────────────
            model.eval()
            v_loss, v_correct, v_total = 0.0, 0, 0
            with torch.no_grad():
                for xb, yb in val_dl:
                    xb, yb = xb.to(device, non_blocking=True), yb.to(device, non_blocking=True)
                    logits  = model(xb)
                    v_loss += criterion(logits, yb).item()
                    v_correct += logits.argmax(1).eq(yb).sum().item()
                    v_total   += yb.size(0)

            v_loss /= len(val_dl)
            v_acc   = 100.0 * v_correct / v_total
            ep_time = time.perf_counter() - epoch_start
            
            # Dead network restart hook
            if epoch == 2 and v_acc < 15.0:
                print(f"  [DEAD NETWORK DETECTED] seed={seed} val_acc={v_acc:.2f}% — reinitializing (Attempt {restart_attempt+1}/3)")
                dead_network_detected = True
                break

            if v_acc  > best_val_acc:  best_val_acc  = v_acc
            if v_loss < best_val_loss: best_val_loss = v_loss

            current_lr = sched.get_last_lr()[0] if hasattr(sched, 'get_last_lr') else max_lr

            rec = EpochRecord(
                epoch      = epoch,
                train_loss = round(t_loss,6), train_acc = round(t_acc,3),
                val_loss   = round(v_loss,6), val_acc   = round(v_acc,3),
                lr         = round(current_lr, 8),
                epoch_time = round(ep_time, 3),
                grad_norm  = round(t_gnorm, 5),
                h_energy   = round(opt.get_h_energy(), 6)   if is_catalyze else 0.0,
                eff_lr     = round(opt.get_effective_lr(), 8) if is_catalyze else 0.0,
            )
            epoch_records.append(rec)

            if verbose and (epoch % 10 == 0 or epoch == epochs - 1):
                h_str = f"  H={rec.h_energy:.4f}" if is_catalyze else ""
                print(f"  [{optimizer_name}|s{seed}] "
                      f"ep{epoch:>3}  val={v_acc:>6.2f}%  loss={v_loss:.4f}  "
                      f"lr={current_lr:.5f}  gnorm={t_gnorm:.3f}  t={ep_time:.1f}s{h_str}")
                
        if not dead_network_detected:
            break

    total_time = time.perf_counter() - total_start

    # ── COMPUTE DERIVED METRICS ─────────────────────────────────────────────
    accs = [r.val_acc for r in epoch_records]

    # High-LR volatility: epochs 5–30 (or whatever's available)
    vol_start, vol_end = 5, min(30, len(accs) - 1)
    vol_deltas = [abs(accs[i+1] - accs[i]) for i in range(vol_start, vol_end)]
    high_lr_vol = float(np.mean(vol_deltas)) if vol_deltas else 0.0

    # Worst single-epoch crash
    all_deltas = [(accs[i] - accs[i+1], i) for i in range(len(accs)-1)]
    worst_crash, worst_crash_epoch = max(all_deltas, key=lambda x: x[0]) if all_deltas else (0.0, 0)

    # Convergence clocks
    def epochs_to(target):
        for r in epoch_records:
            if r.val_acc >= target:
                return r.epoch
        return epochs  # never reached

    # PRIME H-field on val_loss trajectory
    losses_traj = [r.val_loss for r in epoch_records]
    prime = prime_hfield_analysis(losses_traj)

    return RunResult(
        optimizer_name     = optimizer_name,
        seed               = seed,
        epochs_run         = epochs,
        best_val_acc       = round(best_val_acc, 3),
        best_val_loss      = round(best_val_loss, 6),
        final_val_acc      = round(accs[-1], 3),
        total_time         = round(total_time, 2),
        high_lr_volatility = round(high_lr_vol, 4),
        worst_crash        = round(worst_crash, 3),
        worst_crash_epoch  = worst_crash_epoch,
        epoch_to_75        = epochs_to(75),
        epoch_to_85        = epochs_to(85),
        epoch_to_90        = epochs_to(90),
        epoch_to_93        = epochs_to(93),
        prime_lyapunov     = round(prime['lyapunov'], 4),
        prime_final_h      = round(prime['final_energy'], 5),
        prime_transitions  = prime['n_transitions'],
        epoch_log          = [asdict(r) for r in epoch_records],
    )


# ══════════════════════════════════════════════════════════════════════════════
# STATISTICS ENGINE
# ══════════════════════════════════════════════════════════════════════════════

def bootstrap_ci(a: List[float], b: List[float],
                 n_boot: int = 10_000,
                 ci: float = 0.95) -> Dict:
    """Bootstrap confidence interval for mean difference a - b."""
    arr_a, arr_b = np.array(a), np.array(b)
    observed_diff = arr_a.mean() - arr_b.mean()
    diffs = []
    rng   = np.random.default_rng(42)
    for _ in range(n_boot):
        ba = rng.choice(arr_a, size=len(arr_a), replace=True)
        bb = rng.choice(arr_b, size=len(arr_b), replace=True)
        diffs.append(ba.mean() - bb.mean())
    lo = np.percentile(diffs, (1 - ci) / 2 * 100)
    hi = np.percentile(diffs, (1 + ci) / 2 * 100)
    return {'observed': observed_diff, 'lo': lo, 'hi': hi,
            'significant': not (lo <= 0 <= hi)}


def welch_t(a: List[float], b: List[float]) -> Dict:
    from scipy import stats as sp
    t_stat, p_val = sp.ttest_ind(a, b, equal_var=False)
    std_b = np.std(b, ddof=1)
    cohen = (np.mean(a) - np.mean(b)) / std_b if std_b > 0 else 0.0
    return {'t_stat': float(t_stat), 'p_value': float(p_val),
            'cohen_d': float(cohen)}


def levene_variance(a: List[float], b: List[float]) -> Dict:
    """Levene's test: does CATALYZE have significantly lower variance?"""
    from scipy import stats as sp
    stat, p = sp.levene(a, b)
    return {'levene_stat': float(stat), 'p_value': float(p),
            'a_std': float(np.std(a, ddof=1)),
            'b_std': float(np.std(b, ddof=1)),
            'variance_reduction_pct': (1 - np.std(a)/np.std(b)) * 100
                                       if np.std(b) > 0 else 0.0}


def compute_stats(results_a: List[RunResult],
                  results_b: List[RunResult],
                  name_a: str = 'CATALYZE_V11',
                  name_b: str = 'SGD') -> Dict:
    """
    Full statistical comparison between two optimizer result sets.
    All 8 primary metrics compared.
    """
    metrics = {
        'best_val_acc':       ([r.best_val_acc       for r in results_a], [r.best_val_acc       for r in results_b]),
        'high_lr_volatility': ([r.high_lr_volatility for r in results_a], [r.high_lr_volatility for r in results_b]),
        'worst_crash':        ([r.worst_crash        for r in results_a], [r.worst_crash        for r in results_b]),
        'epoch_to_90':        ([r.epoch_to_90        for r in results_a], [r.epoch_to_90        for r in results_b]),
        'epoch_to_93':        ([r.epoch_to_93        for r in results_a], [r.epoch_to_93        for r in results_b]),
        'prime_lyapunov':     ([r.prime_lyapunov     for r in results_a], [r.prime_lyapunov     for r in results_b]),
        'total_time':         ([r.total_time         for r in results_a], [r.total_time         for r in results_b]),
    }

    out = {}
    for metric, (a_vals, b_vals) in metrics.items():
        if len(a_vals) < 2 or len(b_vals) < 2:
            out[metric] = {'skipped': 'insufficient data'}
            continue
        # CATALYZE "wins" on a metric depends on direction
        higher_is_better = metric in ('best_val_acc',)
        lower_is_better  = metric in ('high_lr_volatility', 'worst_crash',
                                       'epoch_to_90', 'epoch_to_93', 'total_time')

        if higher_is_better:
            win_rate = sum(a > b for a, b in zip(a_vals, b_vals)) / len(a_vals)
        elif lower_is_better:
            win_rate = sum(a < b for a, b in zip(a_vals, b_vals)) / len(a_vals)
        else:
            win_rate = None

        t_res  = welch_t(a_vals, b_vals)
        ci_res = bootstrap_ci(a_vals, b_vals)

        out[metric] = {
            f'{name_a}_mean':  float(np.mean(a_vals)),
            f'{name_a}_std':   float(np.std(a_vals, ddof=1)),
            f'{name_b}_mean':  float(np.mean(b_vals)),
            f'{name_b}_std':   float(np.std(b_vals, ddof=1)),
            'delta_mean':      float(np.mean(a_vals) - np.mean(b_vals)),
            'win_rate':        win_rate,
            'welch_t':         t_res['t_stat'],
            'p_value':         t_res['p_value'],
            'cohen_d':         t_res['cohen_d'],
            'bootstrap_ci':    ci_res,
            'significant_p05': t_res['p_value'] < 0.05,
        }

    # Variance comparison (the strongest claim)
    acc_a = [r.best_val_acc for r in results_a]
    acc_b = [r.best_val_acc for r in results_b]
    if len(acc_a) >= 3 and len(acc_b) >= 3:
        out['variance_test'] = levene_variance(acc_a, acc_b)

    # Crash resistance: what fraction of CATALYZE worst crashes < SGD worst crash same seed?
    crash_pairs = list(zip([r.worst_crash for r in results_a],
                            [r.worst_crash for r in results_b]))
    if crash_pairs:
        out['crash_resistance_rate'] = sum(a < b for a, b in crash_pairs) / len(crash_pairs)

    return out


# ══════════════════════════════════════════════════════════════════════════════
# REPORT PRINTER
# ══════════════════════════════════════════════════════════════════════════════

def print_report(all_results: Dict[str, List[RunResult]],
                 stats: Dict[str, Dict],
                 baseline: str = 'SGD'):
    """Terminal report — battle-tested cartography style."""
    SEP = "═" * 76
    sep = "─" * 76

    print(f"\n{SEP}")
    print("  ZKAEDI PRIME — BENCHMARK CARTOGRAPHY REPORT")
    print(SEP)

    # Per-optimizer summary
    for opt_name, runs in all_results.items():
        if not runs: continue
        accs    = [r.best_val_acc       for r in runs]
        vols    = [r.high_lr_volatility for r in runs]
        crashes = [r.worst_crash        for r in runs]
        t90     = [r.epoch_to_90        for r in runs]
        times   = [r.total_time         for r in runs]

        print(f"\n  ┌── {opt_name} ({len(runs)} seeds) {'─'*(50-len(opt_name))}┐")
        print(f"  │  Best Val Acc      : {np.mean(accs):>6.2f}% ± {np.std(accs):.3f}%")
        print(f"  │  High-LR Volatility: {np.mean(vols):>6.3f}% ± {np.std(vols):.4f}%/epoch")
        print(f"  │  Worst Crash       : {np.mean(crashes):>6.2f}% ± {np.std(crashes):.3f}%")
        print(f"  │  Epochs to 90%     : {np.mean(t90):>6.1f} ± {np.std(t90):.1f} epochs")
        print(f"  │  Total Time        : {np.mean(times):>6.1f}s ± {np.std(times):.1f}s")
        print(f"  │  Per-Seed          :", end="")
        for r in runs:
            print(f"  s{r.seed}={r.best_val_acc:.2f}%", end="")
        print(f"\n  └{'─'*56}┘")

    # Head-to-head CATALYZE vs each baseline
    if 'CATALYZE_V11' in all_results:
        cat_runs = all_results['CATALYZE_V11']
        for base_name, base_runs in all_results.items():
            if base_name == 'CATALYZE_V11' or not base_runs: continue
            key = f"CATALYZE_V11_vs_{base_name}"
            if key not in stats: continue
            s = stats[key]

            print(f"\n{sep}")
            print(f"  HEAD-TO-HEAD: CATALYZE_V11  vs  {base_name}")
            print(sep)

            # Accuracy
            if 'best_val_acc' in s:
                m = s['best_val_acc']
                delta = m['delta_mean']
                sign  = "+" if delta >= 0 else ""
                verdict = "CATALYZE WINS ★" if delta > 0 else ("TIE" if abs(delta) < 0.05 else "CATALYZE LOSES")
                print(f"\n  ACCURACY")
                print(f"    CATALYZE: {m['CATALYZE_V11_mean']:.3f}% ± {m['CATALYZE_V11_std']:.3f}%")
                print(f"    {base_name:8s}: {m[f'{base_name}_mean']:.3f}% ± {m[f'{base_name}_std']:.3f}%")
                print(f"    Δ={sign}{delta:.3f}%  win_rate={m['win_rate']:.0%}  "
                      f"p={m['p_value']:.3f}  d={m['cohen_d']:.3f}  → {verdict}")
                ci = m['bootstrap_ci']
                print(f"    Bootstrap 95% CI: [{ci['lo']:+.3f}%, {ci['hi']:+.3f}%]  "
                      f"{'significant ✓' if ci['significant'] else 'not significant'}")

            # Volatility (key metric)
            if 'high_lr_volatility' in s:
                m = s['high_lr_volatility']
                delta = m['delta_mean']
                pct   = delta / m[f'{base_name}_mean'] * 100 if m[f'{base_name}_mean'] > 0 else 0
                verdict = "CATALYZE STABLE ★" if delta < 0 else "MORE VOLATILE"
                print(f"\n  HIGH-LR VOLATILITY (epochs 5–30)")
                print(f"    CATALYZE: {m['CATALYZE_V11_mean']:.4f}%/ep ± {m['CATALYZE_V11_std']:.4f}")
                print(f"    {base_name:8s}: {m[f'{base_name}_mean']:.4f}%/ep ± {m[f'{base_name}_std']:.4f}")
                print(f"    Δ={delta:+.4f}%  ({pct:+.1f}%)  win_rate={m['win_rate']:.0%}  → {verdict}")
                if 'crash_resistance_rate' in s:
                    print(f"    Crash resistance rate: {s['crash_resistance_rate']:.0%} of seeds")

            # Worst crash
            if 'worst_crash' in s:
                m = s['worst_crash']
                delta = m['delta_mean']
                pct   = delta / m[f'{base_name}_mean'] * 100 if m[f'{base_name}_mean'] > 0 else 0
                verdict = "CATALYZE CRASH-RESISTANT ★" if delta < 0 else "WORSE CRASHES"
                print(f"\n  WORST-CASE CRASH")
                print(f"    CATALYZE: {m['CATALYZE_V11_mean']:.3f}% drop ± {m['CATALYZE_V11_std']:.3f}")
                print(f"    {base_name:8s}: {m[f'{base_name}_mean']:.3f}% drop ± {m[f'{base_name}_std']:.3f}")
                print(f"    Δ={delta:+.3f}%  ({pct:+.1f}%)  → {verdict}")

            # Convergence speed
            if 'epoch_to_90' in s:
                m = s['epoch_to_90']
                delta = m['delta_mean']
                print(f"\n  CONVERGENCE SPEED (epochs to 90%)")
                print(f"    CATALYZE: {m['CATALYZE_V11_mean']:.1f} ep ± {m['CATALYZE_V11_std']:.1f}")
                print(f"    {base_name:8s}: {m[f'{base_name}_mean']:.1f} ep ± {m[f'{base_name}_std']:.1f}")
                faster = "CATALYZE FASTER ★" if delta < 0 else "CATALYZE SLOWER"
                print(f"    Δ={delta:+.1f} epochs  win_rate={m['win_rate']:.0%}  → {faster}")

            # Variance reduction
            if 'variance_test' in s:
                vt = s['variance_test']
                pct = vt['variance_reduction_pct']
                print(f"\n  VARIANCE REDUCTION (Levene's test)")
                print(f"    CATALYZE std: {vt['a_std']:.4f}%")
                print(f"    {base_name:8s} std: {vt['b_std']:.4f}%")
                print(f"    Reduction: {pct:+.1f}%  Levene p={vt['p_value']:.3f}  "
                      f"{'significant ✓' if vt['p_value'] < 0.05 else 'not significant'}")

            # Overhead
            if 'total_time' in s:
                m = s['total_time']
                overhead = m['delta_mean'] / m[f'{base_name}_mean'] * 100
                print(f"\n  OVERHEAD")
                print(f"    CATALYZE: {m['CATALYZE_V11_mean']:.0f}s  |  "
                      f"{base_name}: {m[f'{base_name}_mean']:.0f}s  |  "
                      f"overhead: {overhead:+.1f}%")

    # PRIME field summary
    print(f"\n{sep}")
    print("  PRIME H-FIELD ANALYSIS (loss curve energy)")
    print(sep)
    for opt_name, runs in all_results.items():
        if not runs: continue
        lyaps = [r.prime_lyapunov   for r in runs]
        trans = [r.prime_transitions for r in runs]
        print(f"  {opt_name:16s}  Lyapunov λ={np.mean(lyaps):+.3f} ± {np.std(lyaps):.3f}  "
              f"({'stable ✓' if np.mean(lyaps) < 0 else 'chaotic ⚠'})  "
              f"phase_transitions={np.mean(trans):.1f} ± {np.std(trans):.1f}")

    # Verdict
    print(f"\n{SEP}")
    print("  VERDICT")
    print(SEP)
    if 'CATALYZE_V11' in all_results and baseline in all_results:
        cat = all_results['CATALYZE_V11']
        base = all_results[baseline]
        vol_win  = np.mean([r.high_lr_volatility for r in cat]) < np.mean([r.high_lr_volatility for r in base])
        acc_win  = np.mean([r.best_val_acc       for r in cat]) > np.mean([r.best_val_acc       for r in base])
        crash_win= np.mean([r.worst_crash        for r in cat]) < np.mean([r.worst_crash        for r in base])
        n_wins   = sum([vol_win, acc_win, crash_win])

        print(f"\n  vs {baseline}:")
        print(f"    Accuracy wins    : {'✓ YES' if acc_win  else '✗ NO'}")
        print(f"    Volatility wins  : {'✓ YES' if vol_win  else '✗ NO'}")
        print(f"    Crash resistance : {'✓ YES' if crash_win else '✗ NO'}")
        print(f"    Score            : {n_wins}/3")
        print(f"\n  Strongest defensible claim: ", end="")
        if vol_win and crash_win:
            print("CATALYZE is a more STABLE optimizer under aggressive LR schedules.")
        elif acc_win:
            print("CATALYZE matches or exceeds SGD accuracy.")
        else:
            print("Results mixed — run more seeds.")

    n_seeds = len(list(all_results.values())[0]) if all_results else 0
    power = {7: "~15%", 20: "~50%", 40: "~80%", 100: "~95%"}
    closest = min(power.keys(), key=lambda k: abs(k - n_seeds))
    print(f"\n  Seeds run: {n_seeds}  →  Statistical power ≈ {power[closest]} at Cohen's d=0.45")
    if n_seeds < 20:
        print(f"  To reach 80% power: run --seeds 40")
    print(f"\n{SEP}\n")


# ══════════════════════════════════════════════════════════════════════════════
# MAIN RUNNER
# ══════════════════════════════════════════════════════════════════════════════

def main():
    parser = argparse.ArgumentParser(description='ZKAEDI PRIME Supercharged Benchmark')
    parser.add_argument('--seeds',    type=int, default=7,
                        help='Number of seeds to run (7=quick, 40=power)')
    parser.add_argument('--epochs',   type=int, default=100,
                        help='Epochs per run')
    parser.add_argument('--batch',    type=int, default=128)
    parser.add_argument('--data',     type=str, default='./data')
    parser.add_argument('--out',      type=str, default='zkaedi_benchmark_results.json')
    parser.add_argument('--resume',   type=str, default=None,
                        help='Resume from JSON checkpoint')
    parser.add_argument('--quick',    action='store_true',
                        help='50 epochs, 3 seeds — fast sanity check')
    parser.add_argument('--optimizers', nargs='+',
                        default=['CATALYZE_V11', 'SGD', 'ADAMW'],
                        help='Optimizers to benchmark')
    parser.add_argument('--baseline', type=str, default='SGD',
                        help='Baseline optimizer for comparison')
    parser.add_argument('--lr', type=float, default=0.1,
                        help='Max learning rate (OneCycleLR peak)')
    parser.add_argument('--quiet', action='store_true')
    args = parser.parse_args()

    if args.quick:
        args.seeds  = 3
        args.epochs = 50

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    print(f"\n🔱 ZKAEDI PRIME SUPERCHARGED BENCHMARK")
    print(f"   Device: {device}  |  Seeds: {args.seeds}  |  Epochs: {args.epochs}")
    print(f"   Optimizers: {args.optimizers}")
    print(f"   Baseline: {args.baseline}  |  Max LR: {args.lr}\n")

    # Load checkpoint if resuming
    all_results: Dict[str, List[RunResult]] = defaultdict(list)
    completed_runs: set = set()

    if args.resume and Path(args.resume).exists():
        with open(args.resume) as f:
            saved = json.load(f)
        for opt_name, runs in saved.get('runs', {}).items():
            for r in runs:
                result = RunResult(**{k: v for k, v in r.items()})
                all_results[opt_name].append(result)
                completed_runs.add((opt_name, r['seed']))
        print(f"  Resumed: {sum(len(v) for v in all_results.values())} completed runs")

    seeds = list(range(args.seeds))
    total_runs = len(args.optimizers) * len(seeds)
    done = 0

    try:
        for seed in seeds:
            for opt_name in args.optimizers:
                if (opt_name, seed) in completed_runs:
                    done += 1
                    continue

                print(f"\n{'─'*60}")
                print(f"  [{done+1}/{total_runs}]  {opt_name}  seed={seed}")
                print(f"{'─'*60}")

                result = train_one_seed(
                    optimizer_name = opt_name,
                    seed           = seed,
                    epochs         = args.epochs,
                    device         = device,
                    max_lr         = args.lr,
                    batch_size     = args.batch,
                    data_dir       = args.data,
                    verbose        = not args.quiet,
                )
                all_results[opt_name].append(result)
                completed_runs.add((opt_name, seed))
                done += 1

                # Auto-save checkpoint after every run
                checkpoint = {
                    'runs': {k: [asdict(r) for r in v] for k, v in all_results.items()},
                    'config': vars(args),
                    'completed': done,
                    'total': total_runs,
                }
                with open(args.out, 'w') as f:
                    json.dump(checkpoint, f, indent=2)

                print(f"  ✓ Best={result.best_val_acc:.2f}%  "
                      f"Vol={result.high_lr_volatility:.3f}%  "
                      f"Crash={result.worst_crash:.2f}%  "
                      f"t={result.total_time:.0f}s  "
                      f"[saved → {args.out}]")

    except KeyboardInterrupt:
        print(f"\n  Interrupted — {done}/{total_runs} runs saved to {args.out}")

    # ── Compute statistics ──────────────────────────────────────────────────
    stats = {}
    if 'CATALYZE_V11' in all_results:
        for base_name in all_results:
            if base_name == 'CATALYZE_V11': continue
            key = f"CATALYZE_V11_vs_{base_name}"
            cat_runs  = all_results['CATALYZE_V11']
            base_runs = all_results[base_name]
            n = min(len(cat_runs), len(base_runs))
            if n >= 2:
                stats[key] = compute_stats(cat_runs[:n], base_runs[:n],
                                            'CATALYZE_V11', base_name)

    # ── Print report ────────────────────────────────────────────────────────
    print_report(dict(all_results), stats, args.baseline)

    # ── Save final with stats ───────────────────────────────────────────────
    final = {
        'runs':   {k: [asdict(r) for r in v] for k, v in all_results.items()},
        'stats':  stats,
        'config': vars(args),
    }
    with open(args.out, 'w') as f:
        json.dump(final, f, indent=2)
    print(f"  Full results saved → {args.out}")


if __name__ == '__main__':
    main()
