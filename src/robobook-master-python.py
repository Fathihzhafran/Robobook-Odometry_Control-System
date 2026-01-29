"""
Copyright 2026 Fathihzafran
Licensed under the Apache License, Version 2.0 (Apache 2.0)
Project: Robobook Master GCS (Privacy Version)
"""

import socket, threading
from pynput import keyboard

# --- PRIVACY-FIRST NETWORK CONFIG ---
ROBOT_IP = "YOUR_ROBOT_IP" # Masukkan IP ESP32 dari Serial Monitor
PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
active_keys = set()

def send_cmd(msg): sock.sendto(msg.encode(), (ROBOT_IP, PORT))

def on_press(key):
    try:
        k = key.char.lower() if hasattr(key, 'char') else key
        if k not in active_keys:
            active_keys.add(k)
            if k == 'w':
                if any(s in active_keys for s in [keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r]):
                    send_cmd("BOOST")
                else: send_cmd("FORWARD")
            elif k == 'a': send_cmd("LEFT")
            elif k == 'd': send_cmd("RIGHT")
            elif k == 'q': send_cmd("ROTATE_Q")
            elif k == 'e': send_cmd("ROTATE_E")
            elif k == 's': send_cmd("STOP")
    except Exception: pass

def on_release(key):
    try:
        k = key.char.lower() if hasattr(key, 'char') else key
        if k in active_keys:
            active_keys.remove(k)
            if k == 'w': send_cmd("STOP") # Momentary Safety
    except Exception: pass

def receive():
    while True:
        try:
            data, _ = sock.recvfrom(1024)
            d = data.decode().split(',')
            if len(d) >= 7:
                print(f"\r[STATUS] RPM: {d[0]}|{d[1]} | Load: {d[2]}|{d[3]} | Head: {d[6]}°      ", end="")
        except: pass

print("=== MASTER ROBOBOOK GCS READY ===")
threading.Thread(target=receive, daemon=True).start()
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
