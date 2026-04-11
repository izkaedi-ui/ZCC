"""
ZKAEDI MEV Async Supervisor — production integration.

Drop-in: DynamicZKAEDICore (atomic hot-swap, PGO, JIT synthesis), build_supervisor wiring.
Dependencies (LoRA, Router, Executor, MempoolMonitor) can be imported from your
existing packages; stubs below allow this module to run standalone.

Run from repo root so ./zcc and zkaedi_core_fixed.c are on the path.
Block names in live_profile: use ZCC_DUMP_PGO_BLOCKS=1 ./zcc ... 2>&1 | grep PGO-BLOCK.
"""

import asyncio
import ctypes
import logging
import os
import subprocess
from typing import Dict, Tuple, Optional

try:
    from dotenv import load_dotenv
    load_dotenv()
except ImportError:
    pass

# ══════════════════════════════════════════════════════════════════
# C-TYPES MEMORY LAYOUT (Must match ZCC exact output)
# ══════════════════════════════════════════════════════════════════
class ZKAEDIEMAStateFixed(ctypes.Structure):
    _fields_ = [
        ("ema",         ctypes.c_longlong),
        ("ema_sq",      ctypes.c_longlong),
        ("alpha",       ctypes.c_longlong),
        ("one_minus",   ctypes.c_longlong),
        ("initialized", ctypes.c_int),
        ("_pad",        ctypes.c_int),
    ]

class ZKAEDIMEVScorerFixed(ctypes.Structure):
    _fields_ = [
        ("gas_ema",    ZKAEDIEMAStateFixed),
        ("val_ema",    ZKAEDIEMAStateFixed),
        ("txs_scored", ctypes.c_longlong),
    ]

# Hard layout enforcement
assert ctypes.sizeof(ZKAEDIEMAStateFixed) == 40
assert ctypes.sizeof(ZKAEDIMEVScorerFixed) == 88

# ══════════════════════════════════════════════════════════════════
# DYNAMIC ZKAEDI CORE (The Living Engine)
# ══════════════════════════════════════════════════════════════════
class DynamicZKAEDICore:
    """
    Manages the bare-metal C-extension compiled by ZCC.
    Features atomic hot-swapping, live PGO injection, and JIT synthesis.
    """
    def __init__(self, base_c_source: str = "zkaedi_core_fixed.c"):
        self.base_c_source = base_c_source
        self.version = 0
        self.lib: Optional[ctypes.CDLL] = None
        self.scorer: Optional[ZKAEDIMEVScorerFixed] = None
        self._compile_lock = asyncio.Lock()

        # Cold start compilation
        self._sync_compile_and_swap(profile_data=None)

    def _sync_compile_and_swap(
        self,
        profile_data: Optional[Dict[str, Tuple[float, float]]],
        custom_c_source: Optional[str] = None,
    ):
        """Thread-safe synchronous compile, link, and atomic memory swap."""
        next_ver = self.version + 1
        so_name = f"./libzkaedi_v{next_ver}.so"
        asm_name = f"./zkaedi_v{next_ver}.s"
        prof_name = f"./mempool_v{next_ver}.prof"

        source_to_compile = custom_c_source if custom_c_source else self.base_c_source

        pgo_flag = ""
        if profile_data:
            with open(prof_name, "w") as f:
                f.write("# ZKAEDI Live Mempool Profile\n")
                for block_name, (p0, p1) in profile_data.items():
                    f.write(f"{block_name} {p0:.4f} {p1:.4f}\n")
            pgo_flag = f"-use-profile={prof_name}"

        # Compile via ZCC (Middle-end optimizations active); fallback to gcc if zcc fails
        quiet = os.environ.get("ZCC_QUIET", "")
        devnull = subprocess.DEVNULL if quiet == "1" else None
        
        # Link our GNU asm payload together with ZCC/GCC C-engine output
        weights_asm = "zkaedi_weights.s" if os.path.exists("zkaedi_weights.s") else ""
        
        compile_cmd = f"ZCC_IR_BRIDGE=1 ./zcc {pgo_flag} {source_to_compile} -o {asm_name}"
        link_cmd = f"gcc -shared -fPIC -Wl,-z,notext {asm_name} {weights_asm} -o {so_name}"
        
        zcc_ok = (
            subprocess.run(compile_cmd, shell=True, stderr=devnull, cwd=os.getcwd()).returncode == 0
            and subprocess.run(link_cmd, shell=True, stderr=devnull).returncode == 0
        )
        if not zcc_ok:
            gcc_direct = f"gcc -shared -fPIC {source_to_compile} {weights_asm} -o {so_name}"
            if subprocess.run(gcc_direct, shell=True, stderr=devnull).returncode != 0:
                logging.error(f"[JIT] Compilation failed for v{next_ver}. Retaining current engine.")
                return
            logging.info(f"[JIT] ZCC failed; built v{next_ver} with gcc.")

        # Load new physical engine
        new_lib = ctypes.CDLL(os.path.abspath(so_name))

        # Bind standard MEV scorer; JIT strategy may export different symbols
        try:
            new_lib.zkaedi_mev_scorer_fixed_init.argtypes = [ctypes.POINTER(ZKAEDIMEVScorerFixed)]
            new_lib.zkaedi_mev_score_tx_fixed.argtypes = [
                ctypes.POINTER(ZKAEDIMEVScorerFixed), ctypes.c_longlong, ctypes.c_longlong
            ]
            new_lib.zkaedi_mev_score_tx_fixed.restype = ctypes.c_longlong
        except AttributeError:
            pass  # JIT strategy might not have the global scorer

        # Bind Neural C-Inference Engine (Phase 6 Transpilation)
        try:
            new_lib.evm_classifier_infer.argtypes = [ctypes.POINTER(ctypes.c_longlong)]
            new_lib.evm_classifier_infer.restype = ctypes.c_longlong
        except AttributeError:
            pass

        # Atomic State Transfer
        new_scorer = ZKAEDIMEVScorerFixed()
        if self.scorer is not None:
            ctypes.memmove(
                ctypes.byref(new_scorer),
                ctypes.byref(self.scorer),
                ctypes.sizeof(ZKAEDIMEVScorerFixed),
            )
        else:
            if hasattr(new_lib, "zkaedi_mev_scorer_fixed_init"):
                new_lib.zkaedi_mev_scorer_fixed_init(ctypes.byref(new_scorer))

        # Hot swap active pointers
        self.lib = new_lib
        self.scorer = new_scorer
        self.version = next_ver

        logging.info(
            f"[JIT] Active Engine: v{self.version} | Source: {source_to_compile} | PGO: {bool(profile_data)}"
        )

        # Disk cleanup (explicit paths; run from repo root)
        if self.version > 2:
            old_ver = self.version - 2
            for path in [
                f"./libzkaedi_v{old_ver}.so",
                f"./zkaedi_v{old_ver}.s",
                f"./mempool_v{old_ver}.prof",
            ]:
                try:
                    os.remove(path)
                except OSError:
                    pass

    def energy_signature(self, gas_price_gwei: float, value_eth: float) -> float:
        """The Hot Path — Called by the Router inside the Supervisor pipeline."""
        if self.lib is None or self.scorer is None:
            return 0.0
        gas_scaled = int(gas_price_gwei * 100)
        val_scaled = int(value_eth * 1000)
        mag = self.lib.zkaedi_mev_score_tx_fixed(
            ctypes.byref(self.scorer), gas_scaled, val_scaled
        )
        return float(mag) / 1024.0

    async def pgo_regime_loop(self, supervisor: "AsyncSupervisor", interval_seconds: int = 300):
        """Monitors mempool, extracts branch telemetry, and recompiles for PGO."""
        while True:
            await asyncio.sleep(interval_seconds)

            total_scored = supervisor._metrics.get("audited", 0)
            if total_scored < 50:
                continue

            anomaly_rate = supervisor._metrics.get("high_energy_txs", 0) / max(total_scored, 1)
            normal_rate = 1.0 - anomaly_rate

            # Block names from ZCC_DUMP_PGO_BLOCKS=1 on zkaedi_core_fixed.c (entry, if.merge.0)
            live_profile = {
                "entry": (anomaly_rate, normal_rate),
                "if.merge.0": (anomaly_rate, normal_rate),
            }

            logging.info(f"[PGO] Regime Shift: {anomaly_rate*100:.1f}% anomalies. Recompiling...")
            async with self._compile_lock:
                await asyncio.to_thread(self._sync_compile_and_swap, live_profile)

            supervisor._metrics["audited"] = 0
            supervisor._metrics["high_energy_txs"] = 0

    async def autonomous_synthesis_loop(
        self,
        target_contract_hex: str,
        lora_engine: "LoRAInferenceEngine",
        executor: "MEVExecutionEngine",
    ):
        """The Ultra Meta Loop: EVM logic to Native Silicon."""
        logging.info(f"[SYNTHESIS] Initiating Target Triage for {target_contract_hex}")

        # ── 1. FAST-GATE STRUCTURAL PRE-FILTER (Offensive Honeypot Evasion) ──
        if hasattr(self.lib, "evm_classifier_infer"):
            logging.info("[FAST-GATE] Triggering C-Native Ouroboros Neural Engine...")
            try:
                from etherscan_to_prime import SolidityFeatureExtractor, PRIMEEnergyMapper
                
                source_code = await executor.fetch_contract_source(target_contract_hex)
                features = SolidityFeatureExtractor.extract(source_code)
                energy_19d = PRIMEEnergyMapper.compute_energy_vector(features)
                prime_4d = PRIMEEnergyMapper.compute_prime_hamiltonian(features)
                
                x_23d = energy_19d + [
                    prime_4d['base_cost'] / 1000.0,
                    prime_4d['branch_density'],
                    prime_4d['call_density'],
                    prime_4d['prime_energy'] / 1000.0
                ]
                
                # Convert float tensor into Q16.16 64-bit bounds
                c_array_type = (ctypes.c_longlong * 23)
                q16_array = c_array_type()
                for i in range(23):
                    q16_array[i] = int(round(x_23d[i] * 65536))
                
                # Fire native silicon inference
                argmax_id = self.lib.evm_classifier_infer(q16_array)
                
                # ── SHADOW MODE LOGGING ──
                # Log the prediction, but do NOT abort execution yet.
                if argmax_id == 0:
                    logging.warning("[SHADOW-MODE] 🚨 FLAG: Reentrancy Risk (Class 0) detected! Evaluating execution passively.")
                elif argmax_id == 9:
                    logging.warning("[SHADOW-MODE] 🚨 FLAG: Flash Loan Attack (Class 9) detected! Evaluating execution passively.")
                elif argmax_id in [12, 6]:
                    logging.warning(f"[SHADOW-MODE] ⚠️ General MEV Honeypot Detected (Class: {argmax_id}). Evaluating execution passively.")
                else:
                    logging.info(f"[SHADOW-MODE] Structural topology cleared via C-Math (Class: {argmax_id})")
                
                # Intentionally bypass the previous `return` to let the execution 
                # continue. We will compare logs against actual chain states.
            except Exception as e:
                logging.error(f"[FAST-GATE] Native extraction/prediction failed: {e}")
        else:
            logging.info("[FAST-GATE] C-Native Neural Engine offline, bypassing structural check.")

        # ── 2. JIT C-Engine Math Synthesis ──

        c_source = await lora_engine.synthesize_c_model(target_contract_hex)
        strat_name = f"jit_strat_{target_contract_hex[:8]}.c"

        with open(strat_name, "w") as f:
            f.write(c_source)

        async with self._compile_lock:
            await asyncio.to_thread(
                self._sync_compile_and_swap, None, custom_c_source=strat_name
            )

        if hasattr(self.lib, "calculate_optimal_input"):
            self.lib.calculate_optimal_input.argtypes = [
                ctypes.c_longlong,
                ctypes.c_longlong,
            ]
            self.lib.calculate_optimal_input.restype = ctypes.c_longlong

            optimal_wei = self.lib.calculate_optimal_input(1000000000, 500000000)
            logging.info(f"[SYNTHESIS] Optimal Input Computed Natively: {optimal_wei} wei")
            await executor.fire_custom_yul_payload(optimal_wei)
        else:
            logging.warning("[SYNTHESIS] JIT .so did not export calculate_optimal_input")

# ══════════════════════════════════════════════════════════════════
# BUILD SUPERVISOR (Wire DynamicZKAEDICore + PGO worker)
# ══════════════════════════════════════════════════════════════════
def build_supervisor(
    dry_run: bool = True,
    lora_sidecar: bool = False,
    model_path: str = "./models/qwen25-7b-solidity-lora",
    rpc_url: str = os.getenv("ALCHEMY_RPC_URL", os.getenv("ETH_RPC_URL", "http://localhost:8545")),
    ws_url: str = os.getenv("ALCHEMY_WS_URL", os.getenv("ETH_WS_URL", "ws://localhost:8546")),
    pipeline_workers: int = 4,
    pgo_interval_seconds: int = 300,
) -> "AsyncSupervisor":
    core = DynamicZKAEDICore(base_c_source="zkaedi_core_fixed.c")
    lora = LoRAInferenceEngine(model_path=model_path, use_sidecar=lora_sidecar)
    router = ZKAEDIPRIMERouter(core=core)
    executor = MEVExecutionEngine(rpc_url=rpc_url, dry_run=dry_run)
    mempool = MempoolMonitor(ws_url=ws_url)

    supervisor = AsyncSupervisor(
        lora=lora,
        core=core,
        router=router,
        executor=executor,
        mempool=mempool,
        workers=pipeline_workers,
    )
    # Give supervisor direct link to core hot-swapping
    supervisor.core = core

    supervisor.register_worker(
        "PGO-Regime-Tracker", core.pgo_regime_loop, supervisor, pgo_interval_seconds
    )

    return supervisor


# ══════════════════════════════════════════════════════════════════
# STUBS (Replace with real imports from your AsyncSupervisor package)
# ══════════════════════════════════════════════════════════════════

class EVMStructuralPrefilter:
    """
    23-D PRIME Energy Pre-Filter.
    Intercepts the target contract routing, extracts structural AST features,
    and runs the PyTorch classifier to actively avoid MEV honeypots (Flash-loans/Reentrancy).
    """
    def __init__(self, model_path: str = "evm_classifier_v2.pt"):
        self.model_path = model_path
        self.model_loaded = False
        self.classifier = None
        self.class_names = []
        
        try:
            import torch
            from train_evm_v2 import EVMEnergyClassifier
            
            if os.path.exists(model_path):
                checkpoint = torch.load(model_path, map_location='cpu')
                self.class_names = checkpoint.get('class_names', [])
                input_dim = checkpoint.get('feature_dim', 23)
                
                self.classifier = EVMEnergyClassifier(input_dim=input_dim, num_classes=len(self.class_names))
                self.classifier.load_state_dict(checkpoint['model_state_dict'])
                self.classifier.eval()
                self.model_loaded = True
                logging.info(f"[FAST-GATE] PyTorch Structural EVM Classifier active ({input_dim}-D -> {len(self.class_names)} classes).")
            else:
                logging.warning(f"[FAST-GATE] Model '{model_path}' not found. Triaging offline.")
        except Exception as e:
            logging.error(f"[FAST-GATE] Failed to load PyTorch structural integration: {e}")
            
    def analyze_vulnerability_manifold(self, source_code: str) -> dict:
        """Runs the Etherscan 23-D extraction and PyTorch scoring."""
        if not self.model_loaded or not source_code:
            return {"safe": True, "energy": 0.0, "reason": "No model or source"}
            
        try:
            import torch
            from etherscan_to_prime import SolidityFeatureExtractor, PRIMEEnergyMapper
            
            features = SolidityFeatureExtractor.extract(source_code)
            energy_19d = PRIMEEnergyMapper.compute_energy_vector(features)
            prime_4d = PRIMEEnergyMapper.compute_prime_hamiltonian(features)
            
            x_23d = energy_19d + [
                prime_4d['base_cost'] / 1000.0,
                prime_4d['branch_density'],
                prime_4d['call_density'],
                prime_4d['prime_energy'] / 1000.0
            ]
            
            tensor_x = torch.tensor([x_23d], dtype=torch.float32)
            with torch.no_grad():
                probs = torch.softmax(self.classifier.forward(tensor_x), dim=-1)[0]
                
            max_idx = torch.argmax(probs).item()
            dominant_category = self.class_names[max_idx] if max_idx < len(self.class_names) else "unknown"
            confidence = probs[max_idx].item()
            
            # FAST-GATE LOGIC: Determine honeypot risk
            is_honeypot = False
            # If the classifier finds defensive patterns mixed with explosive energy density, flag it
            if dominant_category in ["flash_loan_attack", "reentrancy", "dos_unbounded"] and confidence > 0.65:
                is_honeypot = True
                
            return {
                "safe": not is_honeypot,
                "dominant_vuln": dominant_category,
                "confidence": confidence,
                "prime_energy": prime_4d['prime_energy'],
                "reason": f"Contract flagged as {dominant_category} ({confidence:.1%})"
            }
                
        except Exception as e:
            logging.error(f"[FAST-GATE] Pre-filter mapping failed: {e}")
            return {"safe": True, "energy": 0.0, "reason": "Error parsing structural AST."}

class LoRAInferenceEngine:
    def __init__(self, model_path: str = "", use_sidecar: bool = False):
        self.model_path = model_path
        self.use_sidecar = use_sidecar

    async def synthesize_c_model(self, target_contract_hex: str) -> str:
        """Return ZCC-compatible fixed-point C source for the contract math."""
        return "/* Placeholder: implement via Qwen 2.5 LoRA */"

class ZKAEDIPRIMERouter:
    def __init__(self, core: DynamicZKAEDICore):
        self.core = core

class MEVExecutionEngine:
    def __init__(self, rpc_url: str = "", dry_run: bool = True):
        self.rpc_url = rpc_url
        self.dry_run = dry_run

    async def fire_custom_yul_payload(self, optimal_wei: int):
        """Submit bundle with the computed payload."""
        pass
        
    async def fetch_contract_source(self, address_hex: str) -> str:
        """Fetch decompiled or verified source from Etherscan proxy for Fast-Gate."""
        return "/* Placeholder: Structural source code for PyTorch analysis */"

class MempoolMonitor:
    def __init__(self, ws_url: str = ""):
        self.ws_url = ws_url

class AsyncSupervisor:
    def __init__(
        self,
        lora: LoRAInferenceEngine,
        core: DynamicZKAEDICore,
        router: ZKAEDIPRIMERouter,
        executor: MEVExecutionEngine,
        mempool: MempoolMonitor,
        workers: int = 4,
    ):
        self.lora = lora
        self.core = core
        self.router = router
        self.executor = executor
        self.mempool = mempool
        self.workers = workers
        self._metrics: Dict[str, int] = {"audited": 0, "high_energy_txs": 0}
        self._background_tasks: list = []

    def register_worker(self, name: str, coro, *args):
        """Register a background task (e.g. PGO regime loop)."""
        self._background_tasks.append((name, coro, args))

    async def run(self):
        """Start supervisor and all registered workers."""
        for name, coro, args in self._background_tasks:
            asyncio.create_task(coro(*args))
        # Add your main loop (mempool ingest, pipeline, etc.) here
        await asyncio.Future()


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    sup = build_supervisor(dry_run=True, pipeline_workers=2)
    asyncio.run(sup.run())
