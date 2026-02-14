# 🚧 Robobook: Odometry & Control System

<div align="center">
  <img src="media/Vid 20260128 134622.gif" alt="Robobook Phase 1 Overview" width="600">
  <p><i>Visualisasi performa Stage 1: ADRC Stability & Wireless Control</i></p>
</div>

**Status: Work In Progress (Stage 1 Completed)**

---

### 🇮🇩 Deskripsi Singkat
Repositori ini berisi sistem kontrol robot hoverboard menggunakan algoritma **ADRC (Active Disturbance Rejection Control)** dan sensor fusion **BNO055**. Pengembangan **Tahap 1 (Bench Testing & Wireless UDP Control)** telah selesai dan teruji secara fungsional.

* **Core Logic:** ADRC dengan Extended State Observer (ESO) untuk kompensasi beban secara real-time.
* **Connectivity:** Kendali nirkabel via UDP Protocol di jaringan PTIO ITB.
* **Safety:** Fitur *Momentary Drive* (Auto-Stop) dan *Emergency Stop* terintegrasi.

---

### 🇺🇸 Brief Description
This repository contains a hoverboard robot control system utilizing **ADRC** algorithms and **BNO055** sensor fusion. **Phase 1 (Bench Testing & Wireless UDP Control)** development is now complete and functionally verified.

* **Core Logic:** ADRC with Extended State Observer (ESO) for real-time load disturbance rejection.
* **Connectivity:** Wireless control via UDP Protocol over PTIO ITB network.
* **Safety:** Integrated *Momentary Drive* (Auto-Stop) and *Emergency Stop* features.

---

### 📂 Repository Structure
* **`/src`**: Production-ready source code for ESP32 Slave and Python Master interfaces.
* **`/archive`**: Comprehensive chronological logs and historical development iterations.

---
*Developed by Fathihzafran and Gabriela Heidy © 2026*
