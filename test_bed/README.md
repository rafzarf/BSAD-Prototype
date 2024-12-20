# Akusisi Data dan Kontrol Stepper Motor

Repositori ini berisi dua proyek utama untuk mengontrol stepper motor dan mengukur jarak menggunakan ESP32. Proyek ini mencakup fitur seperti mode otomatis, kontrol manual melalui Serial Monitor, dan penggunaan limit switch untuk batasan gerakan.


## Struktur Direktori

```
.
├── auto_runner
│   └── auto_runner.ino
├── hitung_jarak
│   └── hitung_jarak.ino
└── README.md
```


## Proyek 1: Auto Runner

### Deskripsi

File `auto_runner/auto_runner.ino` dirancang untuk:
- Mengontrol stepper motor dengan mode manual dan otomatis.
- Memanfaatkan limit switch untuk menghentikan motor secara aman.
- Menampilkan jarak tempuh dan waktu berjalan secara periodik.

### Fitur Utama

1. **Kontrol Manual:**
   - Gunakan perintah seperti `f` (maju), `b` (mundur), `s` (berhenti), `1-9` (atur kecepatan), dan `auto` (mode otomatis).

2. **Mode Otomatis:**
   - Mengatur durasi gerakan otomatis dengan memasukkan waktu dalam jam.

3. **Pemantauan Waktu dan Jarak:**
   - Menampilkan langkah, jarak tempuh, dan waktu berjalan melalui Serial Monitor.

### Cara Menggunakan

1. Upload file `auto_runner.ino` ke ESP32 menggunakan Arduino IDE.
2. Buka Serial Monitor untuk memasukkan perintah.
3. Gunakan perintah seperti berikut:
   - `f`: Motor maju secara terus-menerus.
   - `b`: Motor mundur secara terus-menerus.
   - `s`: Berhenti.
   - `auto`: Mode otomatis. Masukkan waktu dalam jam setelah perintah ini.
   - `1-9`: Mengatur kecepatan motor (1 lambat, 9 cepat).

### Perangkat Keras yang Dibutuhkan

- ESP32
- Stepper motor dan driver (misalnya, A4988 atau DRV8825)
- Limit switch untuk batasan gerakan
- Mekanisme ulir bola untuk perhitungan jarak

## Proyek 2: Hitung Jarak

### Deskripsi

File `hitung_jarak/hitung_jarak.ino` dirancang untuk:
- Mengontrol stepper motor dengan pengukuran jarak berbasis langkah.
- Memanfaatkan limit switch untuk mengatur batasan gerakan.
- Menyediakan mode otomatis untuk menggerakkan motor dalam durasi tertentu.

### Fitur Utama

1. **Pengukuran Jarak:**
   - Menghitung jarak berdasarkan jumlah langkah motor dan pitch ulir bola.

2. **Mode Manual:**
   - Kontrol maju (`f`), mundur (`b`), berhenti (`s`), dan kembali ke asal (`return`).

3. **Mode Otomatis:**
   - Menjalankan motor secara otomatis selama durasi tertentu (misalnya, `1h` untuk 1 jam).

4. **Reset Jarak:**
   - Jarak dapat direset saat limit switch ditekan.

### Cara Menggunakan

1. Upload file `hitung_jarak.ino` ke ESP32 menggunakan Arduino IDE.
2. Buka Serial Monitor untuk memasukkan perintah.
3. Gunakan perintah seperti berikut:
   - `move <distance>mm`: Menggerakkan motor untuk jarak tertentu.
   - `f`: Motor maju secara terus-menerus.
   - `b`: Motor mundur secara terus-menerus.
   - `s`: Berhenti.
   - `return`: Kembali ke posisi awal (limit switch 1).
   - `auto`: Mode otomatis. Motor bergerak maju sampai limit switch ditekan.
   - `1h`, `2h`, `3h`: Mode waktu otomatis selama 1, 2, atau 3 jam.
   - `1-9`: Mengatur kecepatan motor (1 lambat, 9 cepat).

### Perangkat Keras yang Dibutuhkan

- ESP32
- Stepper motor dan driver (misalnya, A4988 atau DRV8825)
- Limit switch untuk batasan gerakan
- Mekanisme ulir bola untuk perhitungan jarak


## Instalasi

1. Clone repository ini:
   ```bash
   git clone https://github.com/username/stepper_control.git
   ```

2. Instal library yang diperlukan melalui Library Manager di Arduino IDE:
   - [AccelStepper](https://www.airspayce.com/mikem/arduino/AccelStepper/)

3. Konfigurasikan koneksi perangkat keras sesuai kebutuhan.

4. Upload file `.ino` ke ESP32.


## Troubleshooting

1. **Motor Tidak Bergerak:**
   - Periksa koneksi stepper motor dan driver.
   - Pastikan driver motor mendapatkan suplai daya yang cukup.

2. **Limit Switch Tidak Berfungsi:**
   - Periksa koneksi limit switch.
   - Pastikan pull-up resistor diaktifkan pada pin input.

3. **Jarak Tidak Akurat:**
   - Pastikan pitch ulir bola sesuai dengan nilai `stepDistance` dalam kode.



