# Phase 5.5: Fine-Tuning ADRC & Motion Smoothing (29-Jan Morning)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Iterasi ini difokuskan pada "kenyamanan berkendara" Robobook. Melalui pengujian ketiga pada pagi hari tanggal 29 Januari, sistem kontrol ditingkatkan agar lebih responsif namun tetap halus.

* **Tujuan:** Menghilangkan efek overshoot saat akselerasi dan menghentikan getaran motor (jitter) pada kecepatan rendah.
* **Pencapaian:** Implementasi variabel accelRate (0.15) untuk akselerasi bertahap dan Anti-Jitter Filter untuk memastikan motor segera mendapatkan torsi yang cukup saat berada di ambang batas gerak.
* **Hasil:** Pergerakan robot jauh lebih stabil saat mulai berjalan (start-up) dan transisi kecepatan (misal dari Forward ke Boost) terasa lebih organik.

### Fitur Pemurnian Kontrol:
1. **Anti-Overshoot Logic:** Menggunakan `accelRate` untuk membatasi seberapa cepat target RPM naik. Hal ini mencegah robot meloncat terlalu keras saat perintah `FORWARD` atau `BOOST` diberikan.
2. **Anti-Jitter Filter (Anti-Kejang):** Logika khusus yang mendeteksi jika PWM berada di rentang 1005-1065ms (zona motor sering bergetar tapi tidak berputar), dan memaksanya ke 1070ms untuk torsi instan.
3. **Smooth ADRC:** Nilai `wo` (Observer Bandwidth) diturunkan ke 0.8 untuk mengurangi sensitivitas terhadap noise sensor yang bisa menyebabkan motor "kejang".
4. **Wireless PTIO:** Tetap menggunakan protokol UDP untuk navigasi jarak jauh.

---

## 🇺🇸 Description (English)
This iteration focuses on the "ride comfort" of Robobook. Through the third morning test on January 29, the control system was enhanced to be responsive yet smooth.

* **Objective:** Eliminating acceleration overshoot and preventing motor jitter at low speeds.
* **Key Achievement:** Implementation of the accelRate variable (0.15) for gradual acceleration and an Anti-Jitter Filter to ensure motors receive sufficient torque when at the movement threshold.
* **Result:** The robot's movement is significantly more stable during start-up, and velocity transitions (e.g., Forward to Boost) feel more organic.

### Control Refinement Features:
1. **Anti-Overshoot Logic:** Employs `accelRate` to limit how quickly the target RPM ramps up. This prevents the robot from jerking aggressively when `FORWARD` or `BOOST` commands are issued.
2. **Anti-Jitter Filter:** Specialized logic that detects if the PWM falls within the 1005-1065ms range (where motors often vibrate without spinning) and forces it to 1070ms for instant torque.
3. **Smooth ADRC:** The `wo` (Observer Bandwidth) value was lowered to 0.8 to reduce sensitivity to sensor noise that could cause motor jitter.
4. **Wireless PTIO:** Continues to utilize the UDP protocol for long-range navigation.
