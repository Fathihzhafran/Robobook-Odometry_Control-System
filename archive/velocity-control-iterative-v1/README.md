# Phase 1: Velocity Control Iterative (v1)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Folder ini berisi kode dasar pertama yang digunakan untuk mengontrol kecepatan motor hoverboard secara independen. Kode ini merupakan hasil iterasi dari percobaan yang dilakukan.

* **Tujuan:** Menguji respons dasar motor hoverboard secara independen tanpa beban lantai.
* **Pencapaian:** Implementasi arsitektur dual-core untuk memisahkan tugas kontrol (Core 0) dan monitoring (Core 1), serta penerapan filter Low-Pass pada pulsa Hall sensor.
* **Hasil:** Putaran motor stabil pada target RPM tertentu dalam kondisi tanpa beban (no-load).

### Fitur Teknis:
* **Dual-Core Processing:** Loop kontrol berjalan di Core 0 (120ms sampling) sementara komunikasi serial di Core 1.
* **LPF (Low Pass Filter):** Meredam noise pembacaan pulsa Hall sensor agar data RPM lebih halus.
* **PID + Feed-Forward:** Menggabungkan koreksi error (PID) dengan daya dasar (Feed-Forward) agar motor lebih responsif.
* **Cross-Coupling Sync:** Menjaga agar perbedaan kecepatan antara roda kiri dan kanan tetap minimal.

---

## 🇺🇸 Description (English)
This folder contains the first foundational code used to independently control hoverboard motor speeds. This code is the result of iterations of the experiments carried out.

* **Objective:** Testing foundational hoverboard motor responsiveness independently without floor load (bench testing).
* **Key Achievement:** Implementation of dual-core architecture to decouple control tasks (Core 0) from monitoring tasks (Core 1), along with Low-Pass filtering on Hall sensor pulses.
* **Result:** Stable motor rotation achieved at specific target RPMs under no-load conditions.

### Technical Features:
* **Dual-Core Processing:** The control loop executes on Core 0 (120ms sampling) while serial communication is handled by Core 1.
* **LPF (Low Pass Filter):** Dampens Hall sensor pulse noise to provide smoother RPM data.
* **PID + Feed-Forward:** Combines error correction (PID) with a baseline power output (Feed-Forward) for improved motor responsiveness.
* **Cross-Coupling Sync:** Ensures minimal speed deviation between the left and right wheels for better tracking.

---

## 🛠️ Configuration
* **MCU:** ESP32
* **Actuator:** Hoverboard Hub Motor (via ESC)
* **Sensor:** Internal Hall Effect Sensors (90 pulses per rev)
* **Date:** 2026-01-21
