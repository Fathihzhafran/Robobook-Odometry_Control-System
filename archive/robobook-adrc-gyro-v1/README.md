# Phase 4: ADRC Transition & IMU Heading Lock (27-Jan Iteration)

## 🇮🇩 Deskripsi (Bahasa Indonesia)
Iterasi ini menandai pergeseran dari kontrol klasik (PID) ke kontrol modern berbasis model melalui **ADRC (Active Disturbance Rejection Control)**. Fokus utama adalah stabilitas navigasi dan penanganan ketidakseimbangan motor secara atomik.

* **Tujuan:** Mengganti PID dengan kontrol ADRC untuk ketahanan terhadap gangguan beban dan mengunci heading robot menggunakan BNO055.
* **Fitur Utama:** Implementasi Jump-Start Logic (langsung melompat ke 28 RPM untuk mengatasi inersia statis) dan prosedur Atomic Arming untuk keamanan ESC.
* **Kalibrasi:** Melakukan kalibrasi manual Microseconds ke Duty Cycle 14-bit untuk presisi torsi yang lebih tinggi.

### Catatan Penting: Kalibrasi ESC
Karena dokumentasi kalibrasi terlewat pada tanggal 21 Januari, fase ini mencakup kode kalibrasi manual untuk menentukan rentang operasional ESC Hobbywing dan standar.
* **Prosedur Arming:** Menggunakan sinyal 1000us-1500us selama 5 detik untuk mengaktifkan ESC agar masuk ke mode operasional.
* **Mapping 14-bit:** PWM dikonversi dari Microseconds ke resolusi 14-bit (0-16383) untuk kontrol torsi yang jauh lebih halus.

### Fitur Teknis:
1. **ADRC Core (ESO):** Menggunakan *Extended State Observer* untuk mengestimasi dan melawan gangguan beban secara real-time.
2. **Jump-Start Logic:** Sistem secara otomatis melompat ke **28 RPM** saat menerima input target untuk memberikan torsi awal yang cukup guna menembus gesekan statis lantai.
3. **Asymmetric PWM Mixer:** Menangani perbedaan logika antara Motor A (Inverse) dan Motor B (Normal) dalam satu mixer terintegrasi.
4. **BNO055 Fusion:** Mengunci sudut (yaw) robot agar tetap berjalan lurus meskipun terdapat gangguan fisik pada salah satu roda.

---

## 🇺🇸 Description (English)
This iteration marks the shift from classical control (PID) to model-based modern control via **ADRC**. The main focus is navigational stability and atomic handling of motor asymmetry.

* **Objective:** Replacing PID with ADRC for robust disturbance rejection and locking the robot's heading via BNO055.
* **Key Features:** Implementation of Jump-Start Logic (instant jump to 28 RPM to overcome static inertia) and Atomic Arming procedures for ESC safety.
* **Calibration:** Performed manual Microseconds to 14-bit Duty Cycle calibration for higher torque precision.

### Important Note: ESC Calibration
As calibration documentation was missed on January 21st, this phase includes manual calibration code to define the operational range of Hobbywing and standard ESCs.
* **Arming Procedure:** Utilizes 1000us-1500us signals for 5 seconds to initialize ESCs into operational mode.
* **14-bit Mapping:** PWM is converted from Microseconds to 14-bit resolution (0-16383) for finer torque control.

### Technical Features:
1. **ADRC Core (ESO):** Employs an *Extended State Observer* to estimate and reject load disturbances in real-time.
2. **Jump-Start Logic:** The system automatically jumps to **28 RPM** upon receiving target input to provide sufficient initial torque to break static floor friction.
3. **Asymmetric PWM Mixer:** Manages the differing logic between Motor A (Inverse) and Motor B (Normal) within a single integrated mixer.
4. **BNO055 Fusion:** Locks the robot's heading (yaw) to maintain a straight path despite physical wheel disturbances.

---

## 🛠️ Calibration Table (27-Jan Results)
| Component | Signal Logic | Arming Pulse | Max Limit |
| :--- | :--- | :--- | :--- |
| **Motor A (Hobbywing)** | Inverse | 1500 us | 1100 us |
| **Motor B (Standard)** | Normal | 1000 us | 1350 us |
