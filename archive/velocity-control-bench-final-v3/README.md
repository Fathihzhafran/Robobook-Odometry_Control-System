# Phase 3: Final Bench Test & Methodology Comparison

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Iterasi terakhir pada 21 Januari 2026 ini membandingkan tiga filosofi kontrol berbeda untuk menentukan basis terbaik bagi sistem Robobook.

* **Tujuan:** Menemukan keseimbangan antara kecepatan respons dan keamanan sistem (safety) melalui tiga pendekatan logika kontrol yang berbeda.
* **Pencapaian:** Implementasi logika Master-Follower, Asymmetric Feed-Forward, dan teknik Extreme Smoothing (0.98) untuk menstabilkan telemetry pada interval cepat (100ms).
* **Hasil:** Kode versi final berhasil mengunci sinkronisasi roda dengan Ksync 8.0 dan mencegah lonjakan RPM mendadak melalui fitur Target Ramping.

### 1. Pendekatan Performa (User's Final)
Berfokus pada responsivitas tinggi dengan interval **100ms**. Menggunakan "Follower Logic" di mana roda kiri (`motorL`) mencoba mengejar roda kanan (`motorR`) secara agresif.
> **Kutipan Kode:**
> ```cpp
> float pOut2 = pOut1 + (errSync * Ksync) + (integralSync * 2.0); // Follower logic
> ```

### 2. Pendekatan Konservatif (Colleague's Version)
Mengutamakan keamanan dan akurasi sensor dengan interval **300ms**. Membatasi output PWM secara ketat (max 1300) untuk mencegah "Runaway RPM".
> **Kutipan Kode:**
> ```cpp
> i1 = constrain(i1 + err1, -50, 50); // Clamping ketat
> m1.writeMicroseconds((int)constrain(out1, 1000, 1300));
> ```

### 3. Pendekatan Hybrid (Individual Feed-Forward)
Mencoba mengatasi masalah asimetri fisik motor dengan memberikan tenaga dasar (*Feed-Forward*) yang berbeda untuk tiap motor.
> **Kutipan Kode:**
> ```cpp
> float FF1 = 1085; // Untuk motor licin
> float FF2 = 1105; // Untuk motor berat (tenaga lebih besar)
> ```

---

## 🇺🇸 Description (English)
The final iteration on January 21, 2026, compared three different control philosophies to determine the best foundation for the Robobook system.

* **Objective:** Finding the balance between response speed and system safety through three distinct control logic approaches.
* **Key Achievement:** Implementation of Master-Follower logic, Asymmetric Feed-Forward, and Extreme Smoothing (0.98) techniques to stabilize telemetry at fast (100ms) intervals.
* **Result:** The final version successfully locked wheel synchronization with Ksync 8.0 and prevented sudden RPM spikes using the Target Ramping feature.

### 1. Performance Approach (User's Final)
Focuses on high responsiveness with a **100ms** interval. Utilizes "Follower Logic" where the left wheel (`motorL`) aggressively pursues the right wheel's (`motorR`) state.
> **Code Excerpt:**
> ```cpp
> float pOut2 = pOut1 + (errSync * Ksync) + (integralSync * 2.0); // Follower logic
> ```

### 2. Conservative Approach (Colleague's Version)
Prioritizes safety and sensor accuracy with a **300ms** interval. Strictly limits PWM output (max 1300) to prevent "Runaway RPM" scenarios.
> **Code Excerpt:**
> ```cpp
> i1 = constrain(i1 + err1, -50, 50); // Tight Clamping
> m1.writeMicroseconds((int)constrain(out1, 1000, 1300));
> ```

### 3. Hybrid Approach (Individual Feed-Forward)
Attempts to solve physical motor asymmetry by assigning different baseline power (*Feed-Forward*) to each individual motor.
> **Code Excerpt:**
> ```cpp
> float FF1 = 1085; // For smooth motor
> float FF2 = 1105; // For heavy motor (higher baseline)
> ```

---

## 📈 Final Bench Comparison Table
| Feature | Performance (Final) | Conservative | Hybrid |
| :--- | :--- | :--- | :--- |
| **Interval** | 100ms | 300ms | 200ms |
| **Smoothing** | 0.98 (Extreme) | None | 0.80 (Balanced) |
| **Sync Logic** | Follower + PID | Parallel | Follower (Asymmetric) |
| **Safety** | Ramping | Clamping | Clamping |
