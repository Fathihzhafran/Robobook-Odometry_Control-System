# 📜 Master Development Log: Robobook Phase 1

## 🇮🇩 Deskripsi (Bahasa Indonesia)

Halaman ini berisi catatan sejarah perkembangan lengkap dari sistem kontrol Robobook Tahap 1. Direktori ini mendokumentasikan setiap langkah evolusi, mulai dari pengujian meja (*bench test*) awal hingga integrasi sistem nirkabel final.

## 🛠️ Timeline Evolusi Kontrol (Control Evolution Timeline)

### 📅 21 Januari 2026: Fondasi & Bench Testing
Fase ini berfokus pada pengujian meja tanpa beban lantai untuk menstabilkan pembacaan sensor dan menentukan arsitektur kontrol dasar.

* **Fase 1: Velocity Control Iterative (v1) - 08:00 WIB**
    * **Deskripsi:** Menguji respons dasar motor secara independen tanpa beban lantai.
    * **Pencapaian:** Implementasi arsitektur *dual-core* (Core 0: Control, Core 1: Monitoring) dan filter *Low-Pass* pada sensor Hall.
    * **Hasil:** Putaran motor stabil pada target RPM tertentu dalam kondisi *no-load*.

* **Fase 2: Bench Tuning & Optimization (v2) - 10:26 s/d 15:30 WIB**
    * **Deskripsi:** Mengurangi osilasi motor dan mempermudah tuning PID via Serial Monitor.
    * **Pencapaian:** Implementasi **Target Ramping** untuk *start* halus, **Deadzone** 1.5 RPM, dan **Extreme Smoothing Filter** (0.98).
    * **Hasil:** Kontrol jauh lebih stabil dan sinkronisasi roda (`Ksync`) meningkat hingga 8.0.

* **Fase 3: Methodology Comparison (v3) - 15:30 s/d 17:45 WIB**
    * **Deskripsi:** Membandingkan tiga filosofi kontrol: *Performance* (responsif), *Conservative* (aman), dan *Hybrid* (asimetris).
    * **Pencapaian:** Penemuan logika **Master-Follower** untuk sinkronisasi roda yang agresif.
    * **Hasil:** Pemilihan basis logika *Feed-Forward* individual untuk menangani perbedaan fisik antar motor.

---

### 📅 27 Januari 2026: Transisi ke Kontrol Modern (ADRC)
Fase ini menandai pergeseran dari kontrol PID klasik ke kontrol berbasis model yang lebih tangguh terhadap gangguan.

* **Fase 4: ADRC Transition & IMU Fusion - 11:20 s/d 17:30 WIB**
    * **Deskripsi:** Mengganti PID dengan ADRC untuk ketahanan terhadap gangguan beban dan mengunci arah robot menggunakan BNO055.
    * **Pencapaian:** Implementasi **Extended State Observer (ESO)** untuk estimasi beban real-time dan prosedur **Atomic Arming** untuk keamanan ESC.
    * **Kalibrasi:** Penemuan rentang operasional manual ESC Hobbywing (Inverse) dan Standar (Normal).

---

### 📅 28 - 29 Januari 2026: Navigasi Nirkabel & Finalisasi
Robot mulai dikendalikan sepenuhnya secara nirkabel melalui jaringan WiFi PTIO ITB.

* **Fase 5: Wireless UDP Control - 10:33 s/d 13:59 WIB**
    * **Deskripsi:** Mengaktifkan kendali jarak jauh nirkabel menggunakan protokol UDP via WiFi.
    * **Pencapaian:** Integrasi sistem ADRC dengan perintah navigasi (Forward, Stop, Rotate) dan pengiriman telemetri sensor kembali ke laptop.
    * **Hasil:** Robot dapat bermanuver secara lincah tanpa kabel dengan stabilitas tinggi terhadap gangguan lantai.

* **Fase 5.5: Motion Smoothing 29 Jan - 11:04 WIB**
    * **Deskripsi:** Menghilangkan efek *overshoot* dan menghentikan getaran motor (*jitter*) pada kecepatan rendah.
    * **Pencapaian:** Implementasi variabel `accelRate` (0.15) dan **Anti-Jitter Filter** (memaksa torsi pada zona 1005-1065ms).

* **Fase 6: Master Controller Integration 29 Jan - 15:00 WIB**
    * **Deskripsi:** Membangun antarmuka kontrol berbasis Python (GCS) dengan fitur keselamatan.
    * **Pencapaian:** Fitur **Momentary Mode** (otomatis berhenti saat tombol lepas) dan tampilan telemetri *multi-threaded*.

---

## 📈 Tabel Ringkasan Teknis (Technical Summary)

### Perbandingan Metodologi (Phase 3 Comparison)
| Feature | Performance (Final) | Conservative | Hybrid |
| :--- | :--- | :--- | :--- |
| **Interval** | 100ms | 300ms | 200ms |
| **Sync Logic** | Follower + PID | Parallel | Follower (Asymmetric) |
| **Safety** | Ramping | Clamping | Clamping |

### Hasil Kalibrasi ESC (Phase 4 Calibration)
| Component | Signal Logic | Arming Pulse | Max Limit |
| :--- | :--- | :--- | :--- |
| **Motor A (Hobbywing)** | Inverse | 1500 us | 1100 us |
| **Motor B (Standard)** | Normal | 1000 us | 1350 us |

## 🇺🇸 Description (English)
This page contains the complete historical log of the Robobook Phase 1 control system development. This directory documents every evolutionary step, from initial bench testing to final wireless system integration.

## 🛠️ Control Evolution Timeline

### 📅 January 21, 2026: Foundation & Bench Testing
This phase focused on bench testing without floor load to stabilize sensor readings and determine the foundational control architecture.

* **Phase 1: Velocity Control Iterative (v1) - 08:00 WIB**
    * **Description:** Testing foundational motor response independently under no-load conditions.
    * **Achievement:** Implementation of dual-core architecture (Core 0: Control, Core 1: Monitoring) and a Low-Pass Filter on Hall sensor pulses.
    * **Result:** Stable motor rotation achieved at specific target RPMs.

* **Phase 2: Bench Tuning & Optimization (v2) - 10:26 to 15:30 WIB**
    * **Description:** Reducing motor oscillations and streamlining PID tuning via the Serial Monitor.
    * **Achievement:** Implementation of **Target Ramping** for smooth starts, a **1.5 RPM Deadzone**, and an **Extreme Smoothing Filter** (0.98).
    * **Result:** Significantly more stable control; synchronization gain (`Ksync`) increased to 8.0.

* **Phase 3: Methodology Comparison (v3) - 15:30 to 17:45 WIB**
    * **Description:** Comparing three control philosophies: *Performance* (responsive), *Conservative* (safe), and *Hybrid* (asymmetric).
    * **Achievement:** Development of **Master-Follower** logic for aggressive wheel synchronization.
    * **Result:** Selection of an individual **Feed-Forward** logic baseline to handle physical motor differences.

---

### 📅 January 27, 2026: Transition to Modern Control (ADRC)
This phase marked the shift from classical PID to model-based control for superior disturbance rejection.

* **Phase 4: ADRC Transition & IMU Fusion - 11:20 to 17:30 WIB**
    * **Description:** Replacing PID with ADRC for robust load resistance and locking the robot's heading via the BNO055 sensor.
    * **Achievement:** Implementation of the **Extended State Observer (ESO)** for real-time load estimation and **Atomic Arming** procedures for ESC safety.
    * **Calibration:** Manual discovery of operational ranges for Hobbywing (Inverse) and Standard (Normal) ESCs.

---

### 📅 January 28 - 29, 2026: Wireless Navigation & Finalization
The robot transitioned to full wireless operation via the PTIO ITB WiFi network.

* **Phase 5: Wireless UDP Control - 10:33 to 13:59 WIB**
    * **Description:** Enabling remote wireless control using the UDP protocol via WiFi.
    * **Achievement:** Integration of the ADRC system with navigation commands (Forward, Stop, Rotate) and wireless telemetry transmission back to the laptop.
    * **Result:** Agile wireless maneuvering with high stability against physical floor disturbances.

* **Phase 5.5: Motion Smoothing (Jan 29) - 11:04 WIB**
    * **Description:** Eliminating acceleration overshoot and stopping low-speed motor jitter.
    * **Achievement:** Implementation of the `accelRate` (0.15) variable and an **Anti-Jitter Filter** (forcing torque in the 1005–1065ms zone).

* **Phase 6: Master Controller Integration (Jan 29) - 15:00 WIB**
    * **Description:** Developing a Python-based Ground Control Station (GCS) with integrated safety features.
    * **Achievement:** Implementation of **Momentary Mode** (automatic stop on key release) and a multi-threaded telemetry display.

---

## 📈 Technical Summary

### Methodology Comparison (Phase 3 Comparison)
| Feature | Performance (Final) | Conservative | Hybrid |
| :--- | :--- | :--- | :--- |
| **Interval** | 100ms | 300ms | 200ms |
| **Sync Logic** | Follower + PID | Parallel | Follower (Asymmetric) |
| **Safety** | Ramping | Clamping | Clamping |

### ESC Calibration Results (Phase 4 Calibration)
| Component | Signal Logic | Arming Pulse | Max Limit |
| :--- | :--- | :--- | :--- |
| **Motor A (Hobbywing)** | Inverse | 1500 us | 1100 us |
| **Motor B (Standard)** | Normal | 1000 us | 1350 us |


---
*Developed by Fathihzafran © 2026*
