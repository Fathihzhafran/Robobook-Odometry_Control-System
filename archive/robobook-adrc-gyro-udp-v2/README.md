# Phase 5: Wireless UDP Control & ADRC Maneuvers (28-Jan Iteration)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Iterasi ini menandai tahap operasional nirkabel pertama Robobook. Sistem bermigrasi dari Serial monitoring ke **UDP Protocol** yang berjalan di atas jaringan **PTIO ITB**. 

### Fitur Navigasi & Kontrol:
1. **ADRC-ESO Core:** Kontrol model-based yang kebal terhadap gangguan fisik lantai.
   - **Persamaan ESO:** `z1 += (z2 + b0 * last_u + 2 * wo * err_obs) * dt`
2. **Wireless Telemetry:** Data RPM dan orientasi dikirim kembali ke Master (Laptop) secara real-time.
3. **Maneuver Baru:** Implementasi mode **Pivot Kiri (Q)** dan **Pivot Kanan (E)** untuk rotasi di tempat.
4. **Wireless Connectivity:** Menggunakan WiFi PTIO ITB (Password: `wific1t4`) pada port UDP `4210`.

### Catatan Kalibrasi (Referensi 27-Jan):
Sebagai dasar operasional, sistem ini tetap menggunakan hasil kalibrasi dari fase sebelumnya:
- **PWM Minimum:** `1055` us.
- **Jump Start:** `28 RPM` untuk menembus inersia statis.
- **Resolusi PWM:** 14-bit (0-16383) pada frekuensi 50Hz.

---

## 🇺🇸 Description (English)
This iteration marks the first wireless operational stage of Robobook. The system migrated from Serial monitoring to **UDP Protocol** running on the **PTIO ITB** network.

### Navigation & Control Features:
1. **ADRC-ESO Core:** Model-based control robust against physical floor disturbances.
   - **ESO Equation:** `z1 += (z2 + b0 * last_u + 2 * wo * err_obs) * dt`
2. **Wireless Telemetry:** RPM and orientation data are transmitted back to the Master (Laptop) in real-time.
3. **New Maneuvers:** Implementation of **Pivot Left (Q)** and **Pivot Right (E)** modes for stationary rotation.
4. **Wireless Connectivity:** Utilizes PTIO ITB WiFi (Password: `wific1t4`) on UDP port `4210`.

### Calibration Notes (27-Jan Reference):
The system continues to utilize calibration results from the previous phase as its operational foundation:
- **Minimum PWM:** `1055` us.
- **Jump Start:** `28 RPM` to overcome static inertia.
- **PWM Resolution:** 14-bit (0-16383) at 50Hz frequency.

# Robobook Master Controller (Python Interface)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Folder ini berisi program Python yang berjalan di Master (Laptop) untuk mengendalikan Robobook. Program ini berkomunikasi via **UDP Protocol** dan dirancang untuk memberikan kendali yang responsif serta monitoring data sensor secara langsung.

### Fitur Utama:
1. **Momentary Mode:** Robot hanya akan bergerak jika tombol 'W' ditahan. Melepaskan tombol akan memicu perintah `STOP` secara otomatis demi keamanan.
   > **Kutipan Kode:**
   > ```python
   > if k == 'w':
   >     send_cmd("STOP") # Berhenti otomatis saat dilepas
   > ```
2. **Shift-to-Boost:** Menggunakan logika kombinasi tombol untuk mengganti target RPM secara instan (Normal: 32 RPM, Boost: 45-50 RPM).
3. **Multi-threaded Telemetry:** Program dapat menerima dan menampilkan data RPM serta Heading dari robot tanpa menghentikan proses pembacaan keyboard.
4. **Keyboard Integration:** Menggunakan library `pynput` untuk menangkap input keyboard secara non-blocking.

---

## 🇺🇸 Description (English)
This folder contains Python programs running on the Master (Laptop) to control Robobook. The programs communicate via **UDP Protocol** and are designed to provide responsive control and live sensor monitoring.

### Key Features:
1. **Momentary Mode:** The robot only moves as long as the 'W' key is held down. Releasing the key triggers an automatic `STOP` command for safety.
   > **Code Excerpt:**
   > ```python
   > if k == 'w':
   >     send_cmd("STOP") # Automatic stop on release
   > ```
2. **Shift-to-Boost:** Utilizes key combination logic to instantly switch target RPM (Normal: 32 RPM, Boost: 45-50 RPM).
3. **Multi-threaded Telemetry:** The program receives and displays RPM and Heading data from the robot without interrupting keyboard polling.
4. **Keyboard Integration:** Employs the `pynput` library for non-blocking keyboard input capture.

---

## ⚙️ Requirements & Usage
1. Install dependencies: `pip install pynput`
2. Ensure Laptop and Robot are on the same network (**PTIO ITB**).
3. Update `ROBOT_IP` in the script to match the ESP32 IP address.
4. Run: `python robobook-master-v1.py`
