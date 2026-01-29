# Phase 2: Velocity Control Bench Tuning (v2) - Advanced Optimization

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Iterasi ini mencakup penyempurnaan dari sesi siang hingga sore pada hari pertama. Fokus utama beralih dari sekadar memutar motor ke arah kenyamanan dan presisi kontrol (Smoothness).

* **Tujuan:** Mengurangi osilasi motor dan mempermudah pencarian konstanta PID secara langsung via Serial Monitor.
* **Pencapaian:** Implementasi fitur Target Ramping untuk start yang halus, Deadzone untuk mencegah osilasi di sekitar target, serta Extreme Smoothing Filter (0.98) untuk kestabilan grafik.
* **Hasil:** Kontrol jauh lebih "jinak" dan sinkronisasi antar roda jauh lebih kuat (Ksync meningkat hingga 8.0).

### Pembaruan Teknis:
* **Real-time Tuning:** Perintah Serial (P, I, D, T, S, F) memungkinkan tuning tanpa harus re-upload kode berkali-kali.
* **Target Ramping:** Mencegah lonjakan torsi mendadak saat target RPM berubah drastis (Soft-Start).
* **Error Deadzone:** Menambahkan toleransi error 1.5 RPM agar motor tidak terus bergetar saat sudah sangat dekat dengan target.
* **Asymmetric Sync (Ksync 8.0):** Penguatan sinkronisasi untuk memaksa kedua roda berputar pada kecepatan yang sama meski karakteristik motor berbeda.

---

## 🇺🇸 Description (English)
This iteration covers the refinements made during the afternoon and evening sessions of Day 1. The focus shifted from basic motor rotation to control comfort and precision (Smoothness).

* **Objective:** Reducing motor oscillations and streamlining PID tuning via real-time Serial commands.
* **Key Achievement:** Implementation of Target Ramping for soft starts, Deadzone to prevent micro-oscillations, and Extreme Smoothing Filter (0.98) for consistent telemetry.
* **Result:** Much smoother control response and significantly stronger inter-wheel synchronization (Ksync increased to 8.0).

### Technical Updates:
* **Real-time Tuning:** Serial commands (P, I, D, T, S, F) allow for instantaneous tuning without constant code re-uploads.
* **Target Ramping:** Prevents sudden torque spikes when the target RPM changes drastically (Soft-Start functionality).
* **Error Deadzone:** Introduced a 1.5 RPM error tolerance to prevent motor jitter when close to the target setpoint.
* **Asymmetric Sync (Ksync 8.0):** Strengthened synchronization to force both wheels into alignment despite differing motor characteristics.

## 🛠️ System Info
* **Hardware:** ESP32 + Hoverboard Hub Motors
* **Test Date:** 2026-01-21
* **Environment:** Bench Test (No Floor Load)
