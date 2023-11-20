# ESP32-tugas-akhir
Source code ESP32 for my research 
- kontrol PID
- Integrasi Firebase
- Pembacaan tingkat kematangan dengan sensor pH
- Program berorientasi object

## Struktur Folder

---

```markdown
.
├── include
│   ├── /* Header Folder */
│   ├── Network.h
│   └── SensorData.h
├── lib
│   └── /* Additional lib Folder */
├── src
│   ├── /* Program Folder */
│   ├── main.cpp   /*main program*/
│   ├── Network.cpp /*Network Class*/
│   └── SensorData.cpp /*Sensor Class*/
├── test
│   └── /* Unit test folder */
└── platform.ini
    └── /* platform io configuration */
```

## API Reference

---

### Network

---

- `init_wifi()`
    
    inisialisasi jaringan wifi dan event untuk reconnect ke jaringan wifi.
    
- `init_firebase()`
    
    inisialisasi koneksi dengan firebase service.
    
- `update_data()`
    
    mengirim data sensor suhu, kelemababan, dan pH ke firestore database dengan parameter `int temp, int hum, double ph` untuk monitoring.
    
- `get_kontrol_data()`
    
    boolean data untuk kontrol aktivasi sistem.
    
- `get_setpoint()`
    
    data integer untuk setpoint pengontrolan suhu.
    
- `get_history_title()`
    
    data string untuk nama history dan menyimpan data history proses fermentasi.
    
- `append_suhu_to_history()`
    
    commit data suhu ke dokumen history sesuai nama history yang diberikan dengan parameter `int temp, String title` ( nilai suhu dan nama dokumen history )
    
- `append_kelebaban_to_ history()`
    
    commit data kelembaban ke dokumen history sesuai judul history yang diberikan dengan parameter `int hum, String title` ( nilai kelemababan dan nama dokumen history ).
    
- `update_time_history()`
    
    commit data waktu lama proses fermentasi ke dokumen history dengan parameter `String time, String title`
    
- `update_time()`
    
    update lama proses fermentasi untuk monitoring dengan parameter `String time`
    
- `notification()`
    
    mengirim notifikasi ke aplikasi dengan parameter `String date` 
    

### Sensor Data

---

untuk class sensor data dibutuhkan dua parameter yaitu `int DHTpin` dan `int pHpin` untuk menetukan pin GPIO yang digunakan pada ESP32.

- `init_dht()`
    
    inisialisasi sensor DHT22 dengan pin yang telah diberikan pada constructor SensorData.
    
- `Init_pH()`
    
    inisialisasi sensor pH dengan pin yang telah diberikan pada constructor SensorData.
    
- `get_dht_data()`
    
    pembacaan sensor DHT22 dengan tipe data `TempAndHumidity` sesuai dengan library `DHTesp`.
    
- `get_ph_data()`
    
    pembacaan dan perhitungan konduktivitas sampel pada sensor pH sehingga didapatkan pH pada sampel.
