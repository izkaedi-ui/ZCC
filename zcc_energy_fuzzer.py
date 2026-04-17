#!/usr/bin/env python3
import asyncio
import json
import websockets
import time
import sys
import os
import random
import tempfile
from pathlib import Path
from colorama import init, Fore, Style
import io

# Initialize terminal colors
init()
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

# Import the core engine of the ZCC differential fuzzer directly
from zcc_fuzz import FuzzConfig, fuzz_one

# Configuration
ZCC_PATH = "./zcc2"
OUTPUT_DIR = Path("./fuzz_results_gods_eye")

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
(OUTPUT_DIR / 'crashes').mkdir(exist_ok=True)
(OUTPUT_DIR / 'mismatches').mkdir(exist_ok=True)

# The Squad Matrices
def build_agent_config(ai_temperature):
    cfg = FuzzConfig()
    
    if ai_temperature >= 1.2:
        # Matrix 3: The Critical Chaos Field
        agent = "CRITICAL CHAOS FIELD [1.2+ k_B]"
        color = Fore.RED
        cfg.max_locals = 15     # Nested dynamic allocations (OP_ALLOCA clobber attempts)
        cfg.use_structs = True
        cfg.use_pointers = True
        cfg.use_recursion = True
        cfg.max_array_size = 50 
        cfg.bias_sign_extension = 0.85
        cfg.bias_shift = 0.8
    elif ai_temperature < 0.6:
        # Matrix 1: The Stable Field
        agent = "STABLE FIELD [0.0 - 0.6 k_B]"
        color = Fore.CYAN
        cfg.max_depth = 8       # Deep recursive descent arrays
        cfg.max_stmts = 30
        cfg.bias_division = 0.6
        cfg.use_structs = False
    else:
        # Matrix 2: The Action Field
        agent = "ACTION FIELD [0.6 - 1.2 k_B]"
        color = Fore.MAGENTA
        cfg.bias_shift = 0.8
        cfg.bias_sign_extension = 0.85
        cfg.use_structs = False
        cfg.max_locals = 8
        
    return cfg, agent, color

async def squad_fuzz_loop():
    print(f"{Fore.YELLOW}======================================================{Style.RESET_ALL}")
    print(f"{Fore.YELLOW} 🌌 ZKAEDI PRIME SQUADRON: ENERGY FIELD EXCURSION{Style.RESET_ALL}")
    print(f"{Fore.YELLOW}======================================================{Style.RESET_ALL}")
    print(f"[*] Attaching to Multiverse Entropy Router (ws://127.0.0.1:8082/ws/ai_matrix)...")
    
    # Verify ZCC Stage 2 exists
    if not os.path.exists(ZCC_PATH):
        print(f"{Fore.RED}[X] ERROR: ./zcc2 not found. Cannot commence bug hunt.{Style.RESET_ALL}")
        return

    seed_counter = int(time.time()) % 100000

    try:
        async with websockets.connect('ws://127.0.0.1:8082/ws/ai_matrix') as ws:
            print(f"[+] Hardware Neural Link Established.")
            print(f"[+] Sub-Agent Squadron Standing By. Awaiting Physics Vectors...\n")
            
            while True:
                message = await ws.recv()
                data = json.loads(message)
                
                ai_temp = data.get('ai_temperature', 0.5)
                state_vector = data.get('state_vector', [0.5, 0.5])
                
                await do_fuzz_batch(ai_temp, state_vector, seed_counter)
                seed_counter += 3
                
                await asyncio.sleep(0.5)
    except (ConnectionRefusedError, websockets.exceptions.ConnectionClosedError, OSError):
        print(f"{Fore.YELLOW}[*] Hardware socket refused/disconnected. Switching to standalone Local Entropy Mode.{Style.RESET_ALL}")
        while True:
            ai_temp = random.uniform(0.1, 1.8)
            state_vector = [random.random(), random.random()]
            await do_fuzz_batch(ai_temp, state_vector, seed_counter)
            seed_counter += 3
            await asyncio.sleep(0.5)
            
    except Exception as e:
        print(f"{Fore.RED}[X] Squadron Offline: {e}{Style.RESET_ALL}")

async def do_fuzz_batch(ai_temp, state_vector, seed_counter):
    # Biome-specific Fuzz Configuration
    cfg, agent_name, agent_color = build_agent_config(ai_temp)
    
    print(f"{agent_color}[+] DEPLOYING: {agent_name} | HEAT (k_B): {ai_temp:.3f}{Style.RESET_ALL}")
    
    # Execute a swift micro-burst of fuzzing payload dictated by this temperature
    with tempfile.TemporaryDirectory(prefix='squad_fuzz_') as tmpdir:
        fails = 0
        crashes = 0
        batch_size = 3  # Fire 3 highly targeted payloads per heartbeat
        
        for i in range(batch_size):
            seed = seed_counter + i
            
            # FIRE THE DIFFERENTIAL FUZZER
            result = fuzz_one(seed, ZCC_PATH, tmpdir, cfg, timeout=5)
            
            if result.status == 'pass':
                print(f"    {Fore.GREEN}--> [Seed: {seed:6d}] Fuzz Matrix Stable (rc: {result.zcc_rc}){Style.RESET_ALL}")
            elif result.status == 'crash':
                crashes += 1
                fails += 1
                print(f"    {Fore.RED}-X- [Seed: {seed:6d}] CRITICAL RUPTURE DEDUCED (SIGSEGV). Energy Vector: {state_vector} (k_B: {ai_temp:.2f}). Snapshot Captured.{Style.RESET_ALL}")
                # Save crash to gods eye tracker
                crash_file = OUTPUT_DIR / 'crashes' / f'crash_energy{ai_temp:.2f}_seed{seed}.c'
                with open(crash_file, 'w') as f:
                    f.write(f"/* ENERGY VECTOR: {state_vector} | TEMP (k_B): {ai_temp:.2f} | SEED: {seed} */\n")
                    f.write(result.source)
            else:
                fails += 1
                if 'timeout' in result.status:
                     print(f"    {Fore.YELLOW}-?- [Seed: {seed:6d}] TIMEOUT. Snapshot Captured.{Style.RESET_ALL}")
                else:
                     print(f"    {Fore.YELLOW}-?- [Seed: {seed:6d}] {result.status.upper()}: {result.error_msg[:100]}. Snapshot Captured.{Style.RESET_ALL}")
                mm_file = OUTPUT_DIR / 'mismatches' / f'mismatch_energy{ai_temp:.2f}_seed{seed}.c'
                with open(mm_file, 'w') as f:
                    f.write(f"/* ENERGY VECTOR: {state_vector} | TEMP (k_B): {ai_temp:.2f} | SEED: {seed} | STATUS: {result.status.upper()} | ERROR: {result.error_msg} */\n")
                    f.write(result.source)
                
        if fails == 0:
            print(f"{agent_color}[*] Vector Sweep Complete. No logical decoupling found.{Style.RESET_ALL}\n")
        else:
            print(f"{agent_color}[!] Sweep Complete. {fails} DECOUPLING EVENTS FOUND. Logic logged to {OUTPUT_DIR}{Style.RESET_ALL}\n")

if __name__ == "__main__":
    try:
        asyncio.run(squad_fuzz_loop())
    except KeyboardInterrupt:
        print(f"\n{Fore.CYAN}[*] Excursion Aborted. Squad RTB.{Style.RESET_ALL}")
