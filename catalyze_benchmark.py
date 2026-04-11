import torch
import torch.nn as nn
import torchvision
import torchvision.transforms as T
import random
import numpy as np
import time

class CATALYZE_V11(torch.optim.Optimizer):
    """ZKAEDI CATALYZE V11 — Hamiltonian-Modulated AdamW"""
    def __init__(self, params, lr=1e-3, betas=(0.9, 0.999), eps=1e-8,
                 weight_decay=0.01,
                 eta=0.4, gamma=0.3, beta_h=0.1, sigma=0.05,
                 max_h=10.0):

        if not 0.0 <= lr:               raise ValueError(f"lr={lr}")
        if not 0.0 <= eps:              raise ValueError(f"eps={eps}")
        if not 0.0 <= betas[0] < 1.0:  raise ValueError(f"beta1={betas[0]}")
        if not 0.0 <= betas[1] < 1.0:  raise ValueError(f"beta2={betas[1]}")

        defaults = dict(lr=lr, betas=betas, eps=eps, weight_decay=weight_decay,
                        eta=eta, gamma=gamma, beta_h=beta_h, sigma=sigma,
                        max_h=max_h)
        super().__init__(params, defaults)

        for group in self.param_groups:
            for p in group["params"]:
                self.state[p]["noise_buf"] = torch.zeros_like(p.data)

    @torch.no_grad()
    def step(self, closure=None):
        loss = None
        if closure is not None:
            with torch.enable_grad():
                loss = closure()

        for group in self.param_groups:
            lr      = group["lr"]
            b1, b2  = group["betas"]
            eps     = group["eps"]
            wd      = group["weight_decay"]
            eta     = group["eta"]
            gamma   = group["gamma"]
            beta_h  = group["beta_h"]
            sigma   = group["sigma"]
            max_h   = group["max_h"]

            for p in group["params"]:
                if p.grad is None:
                    continue
                if p.grad.is_sparse:
                    raise RuntimeError("CATALYZE does not support sparse gradients")

                grad  = p.grad
                state = self.state[p]

                if "step" not in state:
                    state["step"]        = 0
                    state["exp_avg"]     = torch.zeros_like(p)
                    state["exp_avg_sq"]  = torch.zeros_like(p)
                    state["h_field"]     = torch.zeros_like(p)
                    state["backup"]      = p.data.clone()

                m   = state["exp_avg"]
                v   = state["exp_avg_sq"]
                h   = state["h_field"]
                buf = state["noise_buf"]
                state["step"] += 1
                t = state["step"]

                if wd != 0:
                    p.data.mul_(1.0 - lr * wd)

                m.mul_(b1).add_(grad, alpha=1.0 - b1)
                v.mul_(b2).addcmul_(grad, grad, value=1.0 - b2)

                bc1   = 1.0 - b1 ** t
                bc2   = 1.0 - b2 ** t
                m_hat = m / bc1
                v_hat = v / bc2

                h_base     = grad.pow(2)
                sigmoid_h  = torch.sigmoid(gamma * h.clamp(-max_h, max_h))
                h_rec      = eta * h * sigmoid_h

                buf.normal_(0.0, 1.0)
                noise_std = (1.0 + beta_h * h.abs().clamp(0, max_h))
                noise     = sigma * buf * noise_std

                h.copy_((h_base + h_rec + noise).clamp(-max_h, max_h))

                h_norm  = h / (h.abs().mean() + eps)
                h_scale = torch.sigmoid(h_norm) + 0.5

                denom     = v_hat.sqrt().add_(eps)
                step_size = lr * h_scale.mean().item()
                p.data.addcdiv_(m_hat, denom, value=-step_size)

                if torch.isnan(p.data).any() or torch.isinf(p.data).any():
                    p.data.copy_(state["backup"])
                    h.zero_(); m.zero_(); v.zero_()
                    state["step"] = 0
                else:
                    state["backup"].copy_(p.data)

        return loss

def set_seed(seed: int):
    random.seed(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    if torch.cuda.is_available():
        torch.cuda.manual_seed_all(seed)
    torch.backends.cudnn.deterministic = True
    torch.backends.cudnn.benchmark = False

def run_attractor_benchmark():
    DEVICE = "cuda" if torch.cuda.is_available() else "cpu"
    print(f"[*] IGNITING CATALYZE V11 BENCHMARK on {DEVICE.upper()} (Seed 4)")
    
    set_seed(4)
    
    CIFAR10_NORM = dict(mean=(0.4914, 0.4822, 0.4465), std=(0.2023, 0.1994, 0.2010))
    tr = T.Compose([T.RandomCrop(32, padding=4), T.RandomHorizontalFlip(),
                    T.ToTensor(), T.Normalize(**CIFAR10_NORM)])
    te = T.Compose([T.ToTensor(), T.Normalize(**CIFAR10_NORM)])
    
    train_ds = torchvision.datasets.CIFAR10("/tmp/cifar10_eval", train=True, download=True, transform=tr)
    test_ds = torchvision.datasets.CIFAR10("/tmp/cifar10_eval", train=False, download=False, transform=te)
    
    trainloader = torch.utils.data.DataLoader(train_ds, batch_size=128, shuffle=True, pin_memory=True)
    testloader = torch.utils.data.DataLoader(test_ds, batch_size=128, shuffle=False)
    
    from model import CompactCNN
    model = CompactCNN().to(DEVICE)
    
    optimizer = CATALYZE_V11(model.parameters(), lr=1e-3, weight_decay=0.01, eta=0.4, gamma=0.3, beta_h=0.1, sigma=0.05)
    
    scheduler = torch.optim.lr_scheduler.OneCycleLR(
        optimizer, max_lr=1e-3, epochs=100,
        steps_per_epoch=len(trainloader), pct_start=0.05,
        anneal_strategy="cos", div_factor=25, final_div_factor=1e4,
    )
    
    criterion = nn.CrossEntropyLoss()
    epoch_accs = []
    
    print("[*] Entering The Crucible (Epochs 0 to 30) to evaluate Volatility...")
    
    start_time = time.time()
    for epoch in range(31):
        model.train()
        for x, y in trainloader:
            x, y = x.to(DEVICE), y.to(DEVICE)
            optimizer.zero_grad()
            loss = criterion(model(x), y)
            loss.backward()
            torch.nn.utils.clip_grad_norm_(model.parameters(), 1.0)
            optimizer.step()
            scheduler.step()
            
        model.eval()
        va_corr = va_tot = 0
        with torch.no_grad():
            for x, y in testloader:
                x, y = x.to(DEVICE), y.to(DEVICE)
                out = model(x)
                va_corr += out.argmax(1).eq(y).sum().item()
                va_tot += y.size(0)
                
        va_acc = 100.0 * va_corr / va_tot
        epoch_accs.append(va_acc)
        
        delta_acc = abs(epoch_accs[-1] - epoch_accs[-2]) if epoch > 0 else 0.0
        print(f"   [Epoch {epoch:2d}] Acc: {va_acc:.2f}% | Δacc: {delta_acc:.2f}%")
        
    end_time = time.time()
    
    # Analysis 5-30
    deltas = [abs(epoch_accs[i] - epoch_accs[i-1]) for i in range(5, 31)]
    mean_delta = sum(deltas) / len(deltas)
    
    print("\n" + "="*50)
    print("ZKAEDI PRIME CATALYZE ATTRACTOR REPORT")
    print("="*50)
    print(f"Time Taken       : {end_time - start_time:.1f}s")
    print(f"Goal Constraint  : mean |Δacc| < 1.88% (SGD Baseline Volatility)")
    print(f"CATALYZE Result  : mean |Δacc| = {mean_delta:.2f}%")
    if mean_delta < 1.88:
        print(f"Status           : ✓ PASSED. Attractor field successfully suppresses chaotic variance!")
    else:
        print(f"Status           : ✗ FAILED. H-field instability bleeding through.")

if __name__ == '__main__':
    run_attractor_benchmark()
