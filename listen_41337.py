import socket
import json

UDP_IP = "127.0.0.1"
UDP_PORT = 41337

sock = socket.socket(socket.AF_INET, socket.socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.settimeout(5)

print("Listening on 41337...", flush=True)

packets = []
try:
    while True:
        data, addr = sock.recvfrom(65535)
        text = data.decode('utf-8')
        try:
            j = json.loads(text)
            if 'event' in j and j['event'] == 0x5000:
                packets.append(j)
                if j.get('func') == 'sqlite3VdbeExec':
                    print(f"Captured sqlite3VdbeExec tuple: {j}", flush=True)
        except:
            pass
except socket.timeout:
    print("Done listening.", flush=True)

print(f"Total 0x5000 packets: {len(packets)}")
with open('telemetry_0x5000.jsonl', 'w') as f:
    for p in packets:
        f.write(json.dumps(p) + '\n')
