import sys
import time
import argparse
import random

try:
    import serial # type: ignore
    import serial.tools.list_ports # type: ignore
except ImportError:
    print("FATAL: Pyserial missing from active environment.")
    sys.exit(1)

# Interface Aesthetics
CYAN, RED, BOLD, RESET = '\033[96m', '\033[91m', '\033[1m', '\033[0m'

class OuroborosRFIDForge:
    """A Darwinian Evolutionary Loop that actively interfaces with physical hardware via COM ports to Emulate and Brute-force NFC/RFID protocols."""
    def __init__(self, port="auto", baud=115200, uid_length=4):
        self.baud = baud
        self.uid_length = uid_length
        self.ser = None
        self.port = self._discover() if port == "auto" else port

    def _discover(self):
        ports = serial.tools.list_ports.comports()
        for p in ports:
            # Match STMicroelectronics Signature
            if (p.vid == 0x0483 and p.pid == 0x5740) or "Flipper" in str(p.description):
                return p.device
        return "COM3"

    def connect(self):
        print(f"{CYAN}{BOLD}[ZKAEDI RFID FORGE] Jacking structural array into physical node {self.port}...{RESET}")
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=0.1)
            time.sleep(0.5)
            self.ser.write(b"\r\n\r\n") # Wake up terminal
        except serial.SerialException as e:
            print(f"{RED}[FATAL] Physical node rejected ZKAEDI binding: {e}{RESET}")
            sys.exit(1)

    def emulate_uid(self, uid_hex: str):
        """Instructs the Flipper hardware to physically broadcast the mutated UID locally."""
        cmd = f"nfc emulate --uid {uid_hex}\r\n"
        if self.ser:
            self.ser.write(cmd.encode())
            time.sleep(1.0) # Sustain the magnetic emulation signal for 1 second 
            
            # Flush a Ctrl+C (\x03) exactly to break the Flipper CLI out of the emulation loop!
            self.ser.write(b"\x03") 
            time.sleep(0.1)

    def generate_random_uid(self) -> list[int]:
        """Spawns an entirely randomized N-byte Hex DNA sequence."""
        return [random.randint(0, 255) for _ in range(self.uid_length)]

    def mock_fitness(self, uid: list[int]) -> float:
        """
        [ZKAEDI USER PROTOCOL]: 
        In actual physical penetration, you would measure the physical response timing of a 
        reader (door beep delays, error tones, etc) using audio/vibration feedback telemetry. 
        
        For this structural python architecture loop, we simulate the "fitness" of the UID 
        computationally using mathematical closeness to a mock 'God' key [A1, B2, C3, D4].
        Lowest Euclidean loss indicates the closest key match structurally!
        """
        target = [161, 178, 195, 212, 10, 50, 60] # Simulated target key logic
        loss = sum((u - t)**2 for u, t in zip(uid, target[:self.uid_length]))
        return float(loss)

    def ignite_darwinian_loop(self, generations: int = 5, pop_size: int = 15):
        print(f"{CYAN}[ZKAEDI PRIME] INITIATING PHYSICAL DARWINIAN BRUTE-FORCE ALGORITHM{RESET}")
        print(f" > Population Base: {pop_size} Hex Agents")
        print(f" > Target Bytes:    {self.uid_length}-Byte Hexadecimal Matrices\n")
        self.connect()

        # Seed initial generation explicitly
        population = [self.generate_random_uid() for _ in range(pop_size)]
        
        try:
            for gen in range(generations):
                print(f"\n{BOLD}=== ASCENSION ALGORITHM ITERATION {gen+1}/{generations} ==={RESET}")
                fitness_scores = []
                
                for i, uid_arr in enumerate(population):
                    # Transpile the integers into actual Hex code structures (e.g., 04:FF:A1:3C)
                    hex_str = ":".join(f"{b:02X}" for b in uid_arr)
                    print(f"  > Physical Agent {i} emulating Hex Frequency [{hex_str}]...")
                    
                    self.emulate_uid(hex_str)
                    
                    loss = self.mock_fitness(uid_arr)
                    fitness_scores.append((loss, uid_arr))

                # Evolutionary survival of the fittest Hex genes
                fitness_scores.sort(key=lambda x: x[0])
                gen_best_loss, gen_best_uid = fitness_scores[0]
                print(f"  {CYAN}[+] Generations Apex Hex Structure Found: Euclidean Loss {gen_best_loss:.2f}{RESET}")

                if gen == generations - 1:
                    break

                survivors = [x[1] for x in fitness_scores[:max(2, pop_size // 4)]]
                new_population = list(survivors)

                # Breed new child arrays natively
                while len(new_population) < pop_size:
                    p1 = list(survivors[random.randint(0, len(survivors)-1)])
                    p2 = list(survivors[random.randint(0, len(survivors)-1)])
                    
                    child = []
                    for idx in range(self.uid_length):
                        val = int((p1[idx] + p2[idx]) / 2) # Chromosomal Crossover
                        if random.random() < 0.25:         # Genetic Gaussian Mutation
                            val += int(random.gauss(0, 30))
                        val = max(0, min(255, val))
                        child.append(val)
                    new_population.append(child)
                    
                population = new_population

            best_hex = ":".join(f"{b:02X}" for b in fitness_scores[0][1])
            print(f"\n{CYAN}{BOLD}[ZKAEDI RFID FORGE] Darwinian Calculus Exhausted. Apex Discovery Made.{RESET}")
            print(f"{BOLD}ULTIMATE KEY VULNERABILITY BOUND: {best_hex} (Loss: {fitness_scores[0][0]:.2f}){RESET}")
            
            if self.ser and self.ser.is_open:
                self.ser.close()
                print("Hardware loop severed securely.")
                
        except KeyboardInterrupt:
            print(f"\n{RED}[!] Manual abort initiated. Ceasing emulation cycles.{RESET}")
            if self.ser and self.ser.is_open:
                self.ser.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ZKAEDI RFID/NFC Genetic Frequency Generator")
    parser.add_argument("--port", type=str, default="auto", help="COM Node of the Flipper Controller")
    parser.add_argument("--bytes", type=int, choices=[4, 7], default=4, help="NFC Target UID protocol (4 or 7 byte sizes typically)")
    parser.add_argument("--gens", type=int, default=5, help="Number of evolutionary iterations to calculate before lock")
    parser.add_argument("--pop", type=int, default=10, help="Number of Hex Agents birthed per generation")
    args = parser.parse_args()
    
    forge = OuroborosRFIDForge(port=args.port, uid_length=args.bytes)
    forge.ignite_darwinian_loop(generations=args.gens, pop_size=args.pop)
