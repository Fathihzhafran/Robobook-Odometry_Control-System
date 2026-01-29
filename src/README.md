# Robobook Source Code (Stage 1 Final)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Folder ini berisi kode produksi (production-ready) untuk sistem Robobook Tahap 1. Sistem ini mengintegrasikan kontrol modern ADRC dengan navigasi nirkabel untuk performa robot yang tangguh.

* **Tujuan:** Menyatukan seluruh modul kontrol (ADRC, IMU, UDP) ke dalam satu sistem operasional yang stabil dan aman untuk digunakan di lantai.
* **Pencapaian:** Implementasi Dual-ESO ADRC (setiap motor memiliki observer mandiri), sinkronisasi Master-Follower yang kuat, dan fitur Momentary Drive pada sisi Master untuk keamanan operasional.
* **Hasil:** Robot dapat dikendalikan sepenuhnya secara nirkabel dengan pergerakan yang sangat halus, mampu mempertahankan lintasan lurus secara otomatis, dan memiliki mode rotasi pivot yang cukup presisi.

### Fitur Unggulan:
1. **Dual-ESO ADRC:** Menggunakan dua *Extended State Observer* independen untuk memantau beban masing-masing motor hoverboard secara real-time.
   > **Kutipan Logika:**
   > `z1_r += (z2_r + b0 * last_u_r + 2 * wo * eObsR) * dt;`
2. **Anti-Jitter & Smooth Ramping:** Menghilangkan getaran motor pada kecepatan rendah dengan memaksa torsi ke ambang batas aman (1200ms) dan membatasi akselerasi (`accelRate: 0.12`).
3. **BNO055 Heading Lock:** Menjaga arah robot tetap lurus menggunakan sensor fusion, mengompensasi perbedaan mekanis antar roda secara otomatis.
4. **Momentary UDP Control:** Sistem keamanan nirkabel yang memastikan robot berhenti seketika saat tombol kendali di laptop dilepas.

---

## 🇺🇸 Description (English)
This folder contains the production-ready source code for the Robobook Phase 1 system. It integrates modern ADRC control with wireless navigation for robust robot performance.

* **Objective:** Unifying all control modules (ADRC, IMU, UDP) into a single stable and safe operational system for floor testing.
* **Key Achievement:** Implementation of Dual-ESO ADRC (independent observers for each motor), robust Master-Follower synchronization, and Momentary Drive functionality on the Master side for operational safety.
* **Result:** The robot is fully controllable wirelessly with ultra-smooth movement, automatic straight-line tracking, and precise pivot rotation modes.

### Key Features:
1. **Dual-ESO ADRC:** Employs two independent *Extended State Observers* to monitor the load of each hoverboard motor in real-time.
   > **Logic Excerpt:**
   > `z1_r += (z2_r + b0 * last_u_r + 2 * wo * eObsR) * dt;`
2. **Anti-Jitter & Smooth Ramping:** Eliminates low-speed motor vibration by forcing torque to a safe threshold (1200ms) and limiting acceleration (`accelRate: 0.12`).
3. **BNO055 Heading Lock:** Maintains a straight heading using sensor fusion, automatically compensating for mechanical differences between wheels.
4. **Momentary UDP Control:** A wireless safety system ensuring the robot stops immediately when the control key on the laptop is released.

---

## 🛠️ Setup & Privacy Note
Untuk alasan keamanan, kredensial jaringan telah diganti dengan *placeholder*. 
1. Buka `robobook-slave-esp32.ino`, perbarui bagian `ssid` dan `password`.
2. Buka `robobook-master-python.py`, perbarui `ROBOT_IP` sesuai dengan IP yang muncul di Serial Monitor ESP32.

| Parameter | Value | Unit |
| :--- | :--- | :--- |
| **PWM Floor R/L** | 1200 / 1220 | microseconds |
| **Sampling Rate** | 100 | ms |
| **UDP Port** | 4210 | - |
