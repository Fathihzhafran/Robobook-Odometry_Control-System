# ESP-NOW dan Dual ADRC (9-12 Feb)

Iterasi ini difokuskan pada penggunaan protokol ESP-NOW untuk komunikasi controller dan pada kemampuan roda menyesuaikan kecepatan masing-masing untuk beradaptasi pada situasi sisi kanan dan kiri robot memiliki beban yang berbeda.

* **Tujuan:** Membuat jalan robot presisi dalam keadaan distirbusi massa tidak merata dan menghilangkan ketergantungan WIFI eksternal dalam melakukan testing.
* **Pencapaian:** Implementasi protokol ESP-NOW menghasilkan jalur komunikasi nirkabel yang cukup jauh, di mana mencapai sekitar 24 meter. Masing-masing roda dapat memiliki kecepatan yang berbeda berdasarkan terdeteksinya perubahan sudut oleh BNO055 dan dikontrol oleh ADRC yang berbeda untuk kedua roda. Controller didesain untuk dapat menaik-turunkan nilai RPM untuk dapat mencari nilai RPM minimum untuk robot berjalan dalam keadaan berbeban.
* **Hasil:** Pergerakan robot lebih stabil.
