# Akusisi Data ESP32 dengan Modbus, WiFi, dan MQTT

Proyek ini menggunakan ESP32 untuk mengumpulkan data akselerometer melalui protokol Modbus, mengirim data melalui WiFi, dan mempublikasikannya menggunakan protokol MQTT. Proyek ini dirancang untuk implementasi sistem IoT pada perangkat keras yang terhubung dengan akselerometer berbasis Modbus.


## Fitur Utama

1. **Komunikasi Modbus:**
   - Menggunakan protokol Modbus RTU untuk membaca data akselerasi dari dua sensor akselerometer.

2. **WiFi dan MQTT:**
   - Koneksi ke jaringan WiFi.
   - Publikasi data ke broker MQTT dalam format JSON.

3. **Task Berbasis FreeRTOS:**
   - Pemrosesan data pada core ESP32 menggunakan multitasking.

4. **Pemantauan Sistem:**
   - Deteksi kesehatan sistem berdasarkan waktu respons, koneksi WiFi, dan status MQTT.
   - Penanganan kegagalan secara otomatis.


## Instalasi

### Persyaratan

- **Perangkat keras:**
  - ESP32
  - Transceiver MAX485 untuk komunikasi Modbus RTU
  - Sensor akselerometer berbasis Modbus

- **Software:**
  - Arduino IDE
  - Library berikut:
    - [PubSubClient](https://github.com/knolleary/pubsubclient)
    - [ArduinoJson](https://arduinojson.org/)
    - [ModbusMaster](https://github.com/4-20ma/ModbusMaster)

### Langkah-langkah

1. Clone repository ini:
   ```bash
   git clone https://github.com/username/akusisi_data_esp32.git
   ```

2. Buka file `akusisi_data.ino` menggunakan Arduino IDE.

3. Instal library yang diperlukan melalui Library Manager Arduino IDE.

4. Konfigurasikan parameter WiFi dan MQTT:
   ```cpp
   const char* ssid = "SSID_WIFI_ANDA";
   const char* password = "PASSWORD_WIFI_ANDA";
   const char* mqtt_broker = "ALAMAT_BROKER_MQTT";
   const int mqtt_port = 1883;
   ```

5. Upload kode ke ESP32.


## Pengaturan Hardware

1. Hubungkan MAX485 ke ESP32:
   - **DE/RE Pin**: GPIO4
   - **RX Pin**: GPIO18
   - **TX Pin**: GPIO19

2. Pastikan sensor akselerometer diatur untuk menggunakan protokol Modbus RTU dengan baud rate 9600.


## Struktur Kode

- **WiFi:**
  - Membuat koneksi ke jaringan WiFi dengan fungsi `connectWiFi()`.

- **Modbus:**
  - Konfigurasi komunikasi Modbus RTU pada `setup_modbus()`.
  - Fungsi untuk membaca data akselerasi: `readSensor1()` dan `readSensor2()`.

- **MQTT:**
  - Publikasi data ke broker MQTT pada topik `sensors/accelerometer_data`.
  - Status sistem dipublikasikan pada topik `sensors/status`.

- **Multitasking:**
  - Proses pengumpulan data dijalankan pada Core 1 ESP32 menggunakan FreeRTOS.

- **Pemantauan Kesehatan:**
  - Deteksi status sistem dan publikasi status melalui MQTT.


## Format Data MQTT

Data akselerasi dipublikasikan dalam format JSON:

### Contoh Data
```json
{
  "acceleration": 1.23,
  "x": 1.45,
  "y": 0.98,
  "z": 0.67,
  "samples": 10,
  "timestamp": 12345678
}
```

Status sistem dipublikasikan pada topik `sensors/status`:
```json
{
  "status": "healthy",
  "failures": 0,
  "lastRead": 1000
}
```


## Troubleshooting

1. **WiFi Tidak Terhubung:**
   - Periksa SSID dan password.
   - Pastikan sinyal WiFi cukup kuat.

2. **Gagal Publikasi MQTT:**
   - Periksa koneksi ke broker MQTT.
   - Pastikan topik MQTT benar.

3. **Kesalahan Modbus:**
   - Periksa koneksi MAX485.
   - Pastikan baud rate dan konfigurasi sensor sesuai.



