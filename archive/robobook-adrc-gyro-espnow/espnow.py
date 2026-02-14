import serial
import tkinter as tk
import time

# --- KONFIGURASI ---
COM_PORT = 'COM15'  # <--- PASTIKAN SESUAI
BAUD_RATE = 115200

print("Menghubungkan ke Transmitter...")
try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(2)
    print(f"BERHASIL Terhubung ke {COM_PORT}")
except Exception as e:
    print(f"GAGAL Konek ke {COM_PORT}.")
    exit()

keys_pressed = set()
last_command = ""

# --- VARIABEL RPM ---
current_rpm_setting = 20  # Nilai awal


def send_command(cmd):
    global last_command
    # Kita izinkan kirim ulang agar responsif saat ganti RPM sambil jalan
    if ser.is_open:
        try:
            ser.write((cmd + '\n').encode())
            # print(f"Mengirim: {cmd}") # Debug
            last_command = cmd
        except:
            pass


def on_key_press(event):
    global current_rpm_setting
    key = event.keysym.lower()

    # --- LOGIKA UBAH RPM (PANAH ATAS/BAWAH) ---
    if key == 'up':
        current_rpm_setting += 1
        update_gui_label()
    elif key == 'down':
        current_rpm_setting -= 1
        if current_rpm_setting < 0: current_rpm_setting = 0
        update_gui_label()
    else:
        # Simpan tombol huruf
        # Gunakan event.char untuk huruf biasa
        if event.char:
            keys_pressed.add(event.char.lower())


def on_key_release(event):
    if event.char:
        key = event.char.lower()
        if key in keys_pressed:
            keys_pressed.remove(key)


def update_gui_label():
    label_rpm.config(text=f"TARGET RPM: {current_rpm_setting}")


# --- GAME LOOP (LOGIKA PENGIRIMAN) ---
def game_loop():
    command = "STOP"

    # Kita format perintah jadi "KODE:RPM"

    # 1. MAJU (W) -> Kirim "M:angka"
    if 'w' in keys_pressed:
        command = f"M:{current_rpm_setting}"

    # 2. BELOK KIRI JALAN (A) -> Kirim "CL:angka"
    elif 'd' in keys_pressed:
        command = f"CL:{current_rpm_setting}"

    # 3. BELOK KANAN JALAN (D) -> Kirim "CR:angka"
    elif 'a' in keys_pressed:
        command = f"CR:{current_rpm_setting}"

    # 4. PIVOT KIRI (Q) -> Kirim "RL:angka"
    elif 'e' in keys_pressed:
        command = f"RL:{current_rpm_setting}"

    # 5. PIVOT KANAN (E) -> Kirim "RR:angka"
    elif 'q' in keys_pressed:
        command = f"RR:{current_rpm_setting}"

    elif 's' in keys_pressed:
        command = "STOP"

    send_command(command)
    root.after(50, game_loop)


# --- GUI ---
root = tk.Tk()
root.title(f"Universal RPM Controller - {COM_PORT}")
root.geometry("400x350")
root.focus_set()

label_info = tk.Label(root,
                      text="KONTROL PENUH:\n\n[UP/DOWN] Atur Kecepatan\n[W] Maju\n[A/D] Belok Sambil Jalan\n[Q/E] Putar di Tempat (Pivot)",
                      font=("Arial", 11))
label_info.pack(pady=20)

label_rpm = tk.Label(root, text=f"TARGET RPM: {current_rpm_setting}", font=("Arial", 24, "bold"), fg="blue")
label_rpm.pack(pady=20)

label_warning = tk.Label(root, text="Hati-hati saat Pivot (Q/E) dengan RPM tinggi!", fg="red")
label_warning.pack(pady=5)

root.bind('<KeyPress>', on_key_press)
root.bind('<KeyRelease>', on_key_release)

game_loop()
root.mainloop()

if ser.is_open:
    ser.close()