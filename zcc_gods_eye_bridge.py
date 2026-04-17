import sys
import os
import time
import subprocess
import re
import asyncio

# Sideload the Gods Eye Emitter from the Triton v5 cluster
TRITON_DIR = r"H:\agents\__tritonv5"
if TRITON_DIR not in sys.path:
    sys.path.append(TRITON_DIR)

from gods_eye_emitter import GodsEyeEmitter, TelemetryState

ZCC_BIN = "./zcc"
FUZZ_CMD = ["python3", "zcc_fuzz.py", "--zcc", ZCC_BIN, "--count", "1000000", "--timeout", "5"]

async def run_zcc_panopticon_bridge():
    print("""
    █▀▀▀ █▀▀█ █▀▀▄ █▀▀ 　 █▀▀ █░░█ █▀▀ 
    █░▀█ █░░█ █░░█ ▀▀█ 　 █▀▀ █▄▄█ █▀▀ 
    ▀▀▀▀ ▀▀▀▀ ▀▀▀░ ▀▀▀ 　 ▀▀▀ ▄▄▄█ ▀▀▀ 
    ZCC -> GODS EYE TELEMETRY BRIDGE ACTIVE
    Injecting Compiler State directly into 127.0.0.1:41337
    """)

    daemon = GodsEyeEmitter(sync_every=1)
    daemon.start()

    swarm_cycles = 0
    deadlocks = 0
    latency_ms = 0.0
    h_t_state = 1.0
    current_mode = 0
    
    # We will invoke the zcc_fuzz.py explicitly so we can parse output.
    process = await asyncio.create_subprocess_exec(
        *FUZZ_CMD,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE
    )

    pattern = re.compile(r'\[\s*(\d+)/\s*\d+\] seed=\s*\d+\s+([✓💥≠✗⊘]+)\s+([a-zA-Z_]+)')

    try:
        last_time = time.time()
        
        while True:
            line_bytes = await process.stdout.readline()
            if not line_bytes:
                break
                
            line = line_bytes.decode('utf-8').strip()
            
            # Simple match to line formats like: "[   1/1000] seed=    42  ✓ pass"
            match = pattern.search(line)
            if match:
                prog_count = int(match.group(1))
                status = match.group(3)

                now = time.time()
                latency_ms = (now - last_time) * 1000.0
                last_time = now

                swarm_cycles += 1

                if status == 'pass':
                    h_t_state = min(h_t_state + 0.01, 1.5)  # Stability approaches 1.5
                    current_mode = 0  # Mode A (Converging)
                elif status in ['crash', 'mismatch', 'zcc_fail']:
                    deadlocks += 1
                    h_t_state = max(h_t_state - 0.2, -1.5)  # Instability pushes to -1.5
                    current_mode = 1  # Mode B (Chaos)

                # Send Telemetry explicitly through sync_solve every 5 cycles
                # But we can push faster by calling `_sign_and_emit`
                with daemon.state_lock:
                    daemon.current_state.swarm_cycles = swarm_cycles
                    daemon.current_state.jit_latency_ms = latency_ms
                    daemon.current_state.deadlocks_healed = deadlocks
                    daemon.current_state.h_t_state = h_t_state
                    daemon.current_state.h_t_mode = current_mode
                    
                    # VRAM and Temp handled by idle loop inside GodEyeEmitter
                    daemon.current_state.gpu_util_pct, daemon.current_state.gpu_temp_c = daemon._get_gpu_stats()
                    daemon.current_state.vram_usage_mb = daemon._get_vram_mb()
                    
                    # We inject a pseudo VRAM pressure if no CUDA detected to animate it
                    if daemon.current_state.vram_usage_mb == 0.0:
                        daemon.current_state.vram_usage_mb = 16000 + (swarm_cycles % 1000) * 10 

                    safe_state = TelemetryState(**daemon.current_state.__dict__)
                
                daemon._sign_and_emit(safe_state)
                
                color = "\033[96m" if current_mode == 0 else "\033[95m"
                reset = "\033[0m"
                print(f"[ZCC-PANOPTICON] Cyc: {swarm_cycles:06d} | lat: {latency_ms:05.1f}ms | H_t: {h_t_state:+.2f} | Deadlocks: {deadlocks} | Status: {color}{status}{reset}")

    except KeyboardInterrupt:
        print("\n[GODS EYE] Interrupt caught. Disengaging umbilical.")
    finally:
        daemon.stop()
        if process.returncode is None:
            process.terminate()

if __name__ == "__main__":
    asyncio.run(run_zcc_panopticon_bridge())
