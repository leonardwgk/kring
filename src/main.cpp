#include <Arduino.h>
#include <DHT.h>

// Konfigurasi Pin berdasarkan skema Wokwi
#define TRIG_PIN 5
#define ECHO_PIN 18
#define DHT_PIN 15
#define RELAY_PIN 4
#define DHT_TYPE DHT22

// Ambang Batas Logika
const float DISTANCE_THRESHOLD = 15.0; // cm
const float HUMIDITY_START = 65.0;     // %
const float HUMIDITY_STOP = 55.0;      // %

DHT dht(DHT_PIN, DHT_TYPE);
bool isDrying = false; // Flag status kipas

float getDistance();

void setup() {
  Serial.begin(115200);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Pastikan kipas mati di awal

  dht.begin();
  Serial.println("Sistem Smart Helmet Standby...");
}

void loop() {
  float distance = getDistance();
  
  if (distance > 0 && distance < DISTANCE_THRESHOLD) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Gagal membaca dari sensor DHT!");
      delay(2000);
      return;
    }

    Serial.printf("Helm Terdeteksi | Suhu: %.1f °C | Kelembapan: %.1f %%\n", temperature, humidity);

    // Logika Histeresis
    if (!isDrying && humidity > HUMIDITY_START) {
      Serial.println(">>> Kelembapan tinggi. Memulai pengeringan (Relay ON)");
      digitalWrite(RELAY_PIN, HIGH);
      isDrying = true;
    } 
    else if (isDrying && humidity <= HUMIDITY_STOP) {
      Serial.println(">>> Target kelembapan tercapai. Pengeringan selesai (Relay OFF)");
      digitalWrite(RELAY_PIN, LOW);
      isDrying = false;
    }
    else if (isDrying) {
      Serial.println(">>> Sedang mengeringkan...");
    }

  } else {
    // Jika helm diangkat tiba-tiba saat proses
    if (isDrying) {
      digitalWrite(RELAY_PIN, LOW);
      isDrying = false;
      Serial.println("!!! Helm diangkat sebelum kering! Kipas dimatikan.");
    }
    Serial.println("Status: Standby (Menunggu Helm)");
  }

  Serial.println("-----------------------------------");
  delay(2000); // Tampilkan pembaruan setiap 2 detik di Serial Monitor
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;
  
  return (duration * 0.0343) / 2.0;
}