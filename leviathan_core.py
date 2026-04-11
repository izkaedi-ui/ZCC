# leviathan_core.py
# ZKAEDI PRIME - Async Mempool Threat Interceptor (v2 MAXED)

import asyncio
import ctypes
import numpy as np
import time
import sys

print("--- BOOTING ZKAEDI PRIME LEVIATHAN CORE ---")

# =====================================================================
# 1. C-CORE INJECTION (Bypassing the GIL)
# =====================================================================
try:
    # Load the compiled shared library (.so)
    # Ensure hamiltonian.so is in the same directory!
    leviathan_c_core = ctypes.CDLL('./hamiltonian.so')
    print("[+] hamiltonian.so loaded successfully. Hardware acceleration active.")
except Exception as e:
    print(f"[-] FATAL: C-Core load failed. Did you compile hamiltonian.c?\nError: {e}")
    sys.exit(1)

# Map the C function signature
# long compute_hamiltonian(int n, long *q, long *p, long *m)
leviathan_c_core.compute_hamiltonian.argtypes = [
    ctypes.c_int,
    np.ctypeslib.ndpointer(dtype=np.int64, ndim=1, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, ndim=1, flags='C_CONTIGUOUS'),
    np.ctypeslib.ndpointer(dtype=np.int64, ndim=1, flags='C_CONTIGUOUS')
]
leviathan_c_core.compute_hamiltonian.restype = ctypes.c_long

def scan_mempool_batch(q_batch, p_batch, m_batch):
    """
    Formats the async python arrays into raw C-contiguous memory blocks
    and fires them into the compiled Hamiltonian physics engine.
    """
    n = len(q_batch)
    
    # Cast to strictly 64-bit contiguous memory
    q_c = np.ascontiguousarray(q_batch, dtype=np.int64)
    p_c = np.ascontiguousarray(p_batch, dtype=np.int64)
    m_c = np.ascontiguousarray(m_batch, dtype=np.int64)
    
    # Hit the bare metal
    start_time = time.perf_counter()
    threat_energy = leviathan_c_core.compute_hamiltonian(n, q_c, p_c, m_c)
    calc_time = time.perf_counter() - start_time
    
    return threat_energy, calc_time

# =====================================================================
# 2. ASYNC MEMPOOL ARCHITECTURE
# =====================================================================

BATCH_SIZE = 50000
POLL_RATE = 0.05  # 50ms polling rate

async def simulate_mempool_stream(tx_queue):
    """
    Simulates a firehose of incoming pending transactions from a Web3 node.
    """
    print("[*] Web3 Mempool listener active. Routing signal to buffer...")
    try:
        while True:
            # Simulate a burst of transactions hitting the mempool
            burst_size = np.random.randint(500, 5000)
            
            for _ in range(burst_size):
                # (q: State Risk, p: Tx Velocity, m: Gas Limit)
                await tx_queue.put((
                    np.random.randint(0, 1000), 
                    np.random.randint(10, 510), 
                    1000
                ))
                
            # Yield control back to the event loop so the analyzer can breathe
            await asyncio.sleep(0.01) 
    except asyncio.CancelledError:
        print("[!] Mempool stream disconnected.")

async def leviathan_threat_analyzer(tx_queue):
    """
    The master compressor. Buffers transactions and slams them into the 
    C-core in massive parallel batches.
    """
    print("[*] Leviathan Threat Analyzer spinning up. Awaiting signal...")
    
    try:
        while True:
            batch_q, batch_p, batch_m = [], [], []
            
            # Fill the buffer until we hit BATCH_SIZE or the stream pauses
            while len(batch_q) < BATCH_SIZE:
                try:
                    q, p, m = tx_queue.get_nowait()
                    batch_q.append(q)
                    batch_p.append(p)
                    batch_m.append(m)
                except asyncio.QueueEmpty:
                    if len(batch_q) > 0:
                        # Flush the buffer if we have data but the queue is temporarily empty
                        break 
                    else:
                        await asyncio.sleep(POLL_RATE)
                        continue

            if not batch_q:
                continue
                
            # Route the batch to the C-Core
            energy, calc_time = scan_mempool_batch(batch_q, batch_p, batch_m)
            
            print(f"[>] BATCH CLEARED | Txs: {len(batch_q):05d} | "
                  f"Threat Energy: {energy:09d} | "
                  f"C-Core Latency: {calc_time:.6f}s")
            
            # Non-linear threat spike detection
            if energy > (len(batch_q) * 800): 
                print("    [!!!] LEVIATHAN ALERT: CRITICAL THREAT SPIKE DETECTED IN MEMPOOL.")
                
    except asyncio.CancelledError:
        print("[!] Threat Analyzer shutting down.")

# =====================================================================
# 3. MASTER BUS (Execution Entry Point)
# =====================================================================

async def run_interceptor():
    tx_queue = asyncio.Queue()
    
    print("--- ZKAEDI PRIME INTERCEPTOR ONLINE ---")
    
    # Run the listener and the analyzer concurrently
    stream_task = asyncio.create_task(simulate_mempool_stream(tx_queue))
    analyzer_task = asyncio.create_task(leviathan_threat_analyzer(tx_queue))
    
    try:
        # Run indefinitely (or until you hit Ctrl+C)
        await asyncio.gather(stream_task, analyzer_task)
    except KeyboardInterrupt:
        print("\n[!] Manual override. Shutting down Leviathan.")
        stream_task.cancel()
        analyzer_task.cancel()

if __name__ == '__main__':
    # If running as a standard python script:
    try:
        asyncio.run(run_interceptor())
    except KeyboardInterrupt:
        pass
        
    # NOTE: If you are running this inside a Jupyter Notebook (like Untitled44.ipynb),
    # the event loop is already running. You must comment out the `if __name__ == '__main__':` 
    # block above, and simply run this at the bottom of the cell instead:
    #
    # await run_interceptor()