# Sensor Data Analysis dengan Deteksi Anomali

Aplikasi **FastAPI** ini dirancang untuk menganalisis data sensor (accelerometer) menggunakan model deteksi anomali berbasis autoencoder. Fitur utama meliputi pemrosesan batch data, threshold dinamis, penyimpanan data di database PostgreSQL dan InfluxDB, serta visualisasi data secara real-time.

## Fitur Utama

1. **Pemrosesan Batch:** Memproses batch data berisi 24 titik data.
2. **Threshold Dinamis:** Menentukan anomali berdasarkan nilai error rekonstruksi.
3. **Penyimpanan Multi-DB:** Data anomali disimpan ke PostgreSQL dan InfluxDB.
4. **Visualisasi Real-Time:** Membuat grafik visualisasi setelah buffer data mencapai ambang batas tertentu.



## Instalasi dan Pengaturan

1. **Persyaratan Sistem**
    - Python 3.8 atau lebih baru
    - Database PostgreSQL dan InfluxDB
    - Dependensi Python (tertera di `requirements.txt`)

2. **Langkah-Langkah Instalasi**

    ```bash
    # Clone repositori
    git clone https://github.com/user/sensor-data-analysis.git

    # Masuk ke direktori proyek
    cd sensor-data-analysis

    # Install dependensi
    pip install -r requirements.txt
    ```

3. **Konfigurasi Lingkungan**
    Buat file `.env` dan tambahkan variabel lingkungan berikut:

    ```env
    AUTOENCODER_MODEL_PATH=path/to/autoencoder/model
    SCALER_MODEL_PATH=path/to/scaler/model

    POSTGRES_DB=nama_database
    POSTGRES_USER=username
    POSTGRES_PASSWORD=password
    POSTGRES_HOST=host

    INFLUXDB_ADMIN_TOKEN=token_influxdb
    INFLUXDB_INIT_ORG=org_influxdb
    INFLUXDB_BUCKET=bucket_influxdb
    ```

4. **Menjalankan Aplikasi**

    ```bash
    uvicorn main:app --reload
    ```

    Aplikasi akan berjalan di `http://127.0.0.1:8000`.



## API Endpoint

### 1. **Inferensi Data Sensor**
   **Endpoint:** `/infer`

   **Metode:** `POST`

   **Deskripsi:** Memproses batch data sensor untuk mendeteksi anomali.

   **Contoh Payload:**

   ```json
   {
       "data": [
           {"x_accelerometer_data": 0.12, "y_accelerometer_data": -0.03, "z_accelerometer_data": 0.98, "acceleration_accelerometer_data": 1.01},
           ...
       ]
   }
   ```

   **Respon:**

   ```json
   {
       "timestamp": "2024-12-20T10:00:00",
       "reconstruction_error": 0.01234,
       "anomaly_status": true
   }
   ```

### 2. **Menyajikan Plot Visualisasi**
   **Endpoint:** `/plot/{timestamp}`

   **Metode:** `GET`

   **Deskripsi:** Mengembalikan gambar plot untuk timestamp tertentu.

   **Respon:**
   - File gambar (format PNG)


## Struktur Proyek

```plaintext
.
├── dependencies.py
├── __init__.py
├── internal
├── main.py
├── __pycache__
│   ├── __init__.cpython-39.pyc
│   └── main.cpython-39.pyc
├── README.md
└── routers

3 directories, 6 files
```

