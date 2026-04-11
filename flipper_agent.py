import sys
import threading
import time
import argparse
import datetime
import os

try:
    import serial # type: ignore
    import serial.tools.list_ports # type: ignore
except ImportError:
    print("FATAL: Pyserial framework missing. Run 'pip install pyserial'")
    sys.exit(1)

# ANSI Architecture Styling
CYAN = '\033[96m'
RED = '\033[91m'
GREEN = '\033[92m'
RESET = '\033[0m'
BOLD = '\033[1m'

class ZkaediFlipperNode:
    """Bare-metal Python reciprocal agent for interfacing with Physical Flipper MCU arrays."""
    def __init__(self, port: str = "auto", baud: int = 115200, vault_log: bool = True):
        self.baud = baud
        self.vault_log = vault_log
        self.running = True
        self.ser = None
        self.log_file = None
        
        if self.vault_log:
            os.makedirs("vault", exist_ok=True)
            self.log_file = open(f"vault/flipper_telemetry_{int(time.time())}.log", "w", encoding="utf-8")
            
        if port.lower() == "auto":
            self.port = self._discover_flipper()
        else:
            self.port = port
            
    def _log(self, msg: str, stream: str = "SYS"):
        """Vaults persistent data for machine learning or protocol reviewing."""
        if self.log_file and msg:
            timestamp = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
            # Replace carriage returns for cleaner file logging
            msg_clean = msg.replace("\r", "")
            if msg_clean.strip():
                self.log_file.write(f"[{timestamp}] [{stream}] {msg_clean.strip()}\n")
                self.log_file.flush()

    def _discover_flipper(self) -> str:
        """Autonomously maps COM ports searching for specific STMicroelectronics hardware signatures."""
        print(f"{CYAN}{BOLD}[ZKAEDI SCANNER] Searching hardware matrix for Flipper Zero signatures...{RESET}")
        ports = serial.tools.list_ports.comports()
        for p in ports:
            # Flipper Zero universally uses STMicroelectronics Virtual COM Port (VID: 0483, PID: 5740)
            if p.vid == 0x0483 and p.pid == 0x5740:
                print(f"{GREEN}[ZKAEDI SCANNER] Physical Target Locked: {p.device} ({p.description}){RESET}")
                return p.device
                
            # Fallback heuristic mapping
            if "Flipper" in str(p.description) or "Virtual COM Port" in str(p.description):
                print(f"{GREEN}[ZKAEDI SCANNER] Heuristic Target Locked: {p.device}{RESET}")
                return p.device
        
        print(f"{RED}[ZKAEDI SCANNER] No matching hardware signatures found. Defaulting structural mapping to COM3.{RESET}")
        return "COM3"

    def _reader_daemon(self):
        """Asynchronous telemetry extraction from Flipper MCU."""
        while self.running:
            try:
                if self.ser and self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting).decode('utf-8', errors='replace')
                    sys.stdout.write(data)
                    sys.stdout.flush()
                    
                    # Capture the raw stream for the GEMS Vault
                    self._log(data, stream="FLIPPER")
                time.sleep(0.01)
            except Exception as e:
                if self.running:
                    print(f"\n{RED}[!] Hardware interface severed or buffer underrun: {e}{RESET}")
                    self.running = False
                break

    def _writer_daemon(self):
        """Asynchronous payload ingestion into the Flipper CLI processor."""
        while self.running:
            try:
                line = sys.stdin.readline()
                if not line:
                    break
                if not self.running:
                    break
                
                payload = line.strip()
                self._log(payload, stream="ZKAEDI_PAYLOAD")
                
                if payload.lower() in ("exit", "quit", "disconnect"):
                    print(f"{CYAN}\n[ZKAEDI PRIME] Jacking out. Terminating hardware handshake...{RESET}")
                    self.running = False
                    break
                    
                if self.ser and self.ser.is_open:
                    self.ser.write((payload + "\r\n").encode('utf-8', errors='ignore'))
            except Exception as e:
                if self.running:
                    print(f"\n{RED}[!] Payload injection structural failure: {e}{RESET}")
                    self.running = False
                break

    def ignite(self):
        print(f"\n{BOLD}{CYAN}=== ZKAEDI BARE-METAL AGENT: FLIPPER ZERO RECIPROCAL ==={RESET}")
        print(f"{CYAN} > Target Node: {self.port}{RESET}")
        print(f"{CYAN} > Frequency:   {self.baud} Baud{RESET}")
        if self.vault_log:
            print(f"{CYAN} > Telemetry:   Active (Routing I/O securely to /vault/){RESET}")
        
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=1)
            time.sleep(0.5) 
            
            # Wakes up the Flipper terminal by spamming carriage lines mathematically
            self.ser.write(b"\r\n\r\n") 
            
            reader = threading.Thread(target=self._reader_daemon, daemon=True)
            writer = threading.Thread(target=self._writer_daemon, daemon=True)
            
            reader.start()
            writer.start()
            
            # Main thread operates as a spin-lock so we catch Ctrl+C aborts smoothly
            while self.running:
                time.sleep(0.1)
                
        except serial.SerialException as e:
            print(f"\n{RED}[FATAL ERROR] Structural kernel rejected connection on {self.port}:{RESET}")
            print(f"{RED} -> {e}{RESET}")
            print(f"{RED} (Ensure hardware is plugged into the mainframe and NO other tool (like qFlipper) is locking the serial bus!){RESET}")
        except KeyboardInterrupt:
            print(f"\n\n{CYAN}[ZKAEDI PRIME] Manual override (Ctrl+C). Severing physical connection.{RESET}")
        finally:
            self.running = False
            if self.ser and self.ser.is_open:
                try:
                    self.ser.write(b"\r\n")
                    self.ser.close()
                except Exception:
                    pass
            if self.log_file:
                self.log_file.close()
            print(f"{CYAN}[ZKAEDI PRIME] Agent slumber. Return to base.{RESET}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ZKAEDI BARE-METAL AGENT: FLIPPER ZERO RECIPROCAL")
    # 'auto' replaces hardcoded COM3 so it automatically scans for the device!
    parser.add_argument("--port", type=str, default="auto", help="Hardware node (use 'auto' to scan).")
    parser.add_argument("--baud", type=int, default=115200, help="Symbol rate.")
    parser.add_argument("--no-log", action="store_true", help="Disable structural background vault logging.")
    args = parser.parse_args()
    
    node = ZkaediFlipperNode(port=args.port, baud=args.baud, vault_log=not args.no_log)
    node.ignite()
