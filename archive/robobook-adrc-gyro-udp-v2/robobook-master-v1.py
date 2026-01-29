"""
Copyright 2026 Fathihzafran
Licensed under the Apache License, Version 2.0 (Apache 2.0)

Project: Robobook Ground Control Station
Features: UDP Communication, Momentary Drive, Live Telemetry
"""

import socket
import threading
from pynput import keyboard

# --- NETWORK CONFIGURATION ---
ROBOT_IP = "192.168.100.22" # Matches ESP32 IP from 28-Jan Iteration
PORT = 4210
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Track active keys to prevent key-repeat spam
active_keys = set()

def send_cmd(msg):
    """Sends encoded UDP packets to the robot."""
    sock.sendto(msg.encode(), (ROBOT_IP, PORT))

def on_press(key):
    try:
        # Normalize key character
        k = key.char.lower() if hasattr(key, 'char') else key
        
        if k not in active_keys:
            active_keys.add(k)
            
            # Forward & Boost Logic
            if k == 'w':
                if any(s in active_keys for s in [keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r]):
                    send_cmd("BOOST")
                else:
                    send_cmd("FORWARD")
            
            # Navigation Commands
            elif k == 'a': send_cmd("LEFT")
            elif k == 'd': send_cmd("RIGHT")
            elif k == 'q': send_cmd("ROTATE_Q") # Pivot Left
            elif k == 'e': send_cmd("ROTATE_E") # Pivot Right
            elif k == 's': send_cmd("STOP")
            
            # Handle Shift press while W is already held
            elif k in [keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r]:
                if 'w' in active_keys:
                    send_cmd("BOOST")

    except Exception:
        pass

def on_release(key):
    try:
        k = key.char.lower() if hasattr(key, 'char') else key
        if k in active_keys:
            active_keys.remove(k)
            
            # SAFETY FEATURE: Auto-stop when W is released
            if k == 'w':
                send_cmd("STOP")
                print("\n[EVENT] Key W released: Robot STOP initiated.")
            
            # Return to normal speed when Shift is released but W remains held
            elif k in [keyboard.Key.shift, keyboard.Key.shift_l, keyboard.Key.shift_r]:
                if 'w' in active_keys:
                    send_cmd("FORWARD")
                    print("\n[EVENT] Shift released: Returning to normal speed.")
                    
    except Exception:
        pass

def telemetry_receiver():
    """Background thread to receive and display robot data."""
    print(">>> Telemetry Stream Started. Receiving data...")
    while True:
        try:
            data, _ = sock.recvfrom(1024)
            # Format: RPM_R, RPM_L, Load_Z2, Heading
            print(f"\r[TELEMETRY] {data.decode()}           ", end="")
        except:
            pass

# --- MAIN EXECUTION ---
print("========================================")
print("   ROBOBOOK MASTER CONTROLLER V1.0")
print("========================================")
print("Hold W          : Move Forward")
print("Hold SHIFT + W  : High-Speed Boost")
print("Release W       : Automatic Stop")
print("A / D           : Adjust Heading")
print("Q / E           : Pivot Rotation")
print("S               : Emergency Stop")
print("========================================")

# Start telemetry thread as a daemon
threading.Thread(target=telemetry_receiver, daemon=True).start()

# Start keyboard listener
with keyboard.Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
