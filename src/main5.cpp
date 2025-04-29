// #include <DHT.h>
// #include <OneWire.h>
// #include <DallasTemperature.h>

// // ---- DHT11 ----
// #define DHTPIN 15
// #define DHTTYPE DHT11
// DHT dht(DHTPIN, DHTTYPE);

// // ---- DS18B20 ----
// #define ONE_WIRE_BUS 23
// OneWire oneWire(ONE_WIRE_BUS);
// DallasTemperature ds18b20(&oneWire);

// // ---- HC-SR04 ----
// #define TRIG_PIN 18
// #define ECHO_PIN 19

// void setup() {
//   Serial.begin(115200);

//   // Initialize sensors
//   dht.begin();
//   ds18b20.begin();
//   pinMode(TRIG_PIN, OUTPUT);
//   pinMode(ECHO_PIN, INPUT);

//   Serial.println("📡 Sensor system ready");
// }

// void loop() {
//   // ----- Read DHT11 -----
//   float humidity = dht.readHumidity();
//   float tempDHT = dht.readTemperature();

//   // ----- Read DS18B20 -----
//   ds18b20.requestTemperatures();
//   float tempDS = ds18b20.getTempCByIndex(0);

//   // ----- Read HC-SR04 -----
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);
//   long duration = pulseIn(ECHO_PIN, HIGH);
//   float distance = duration * 0.0343 / 2;

//   // ----- Output -----
//   Serial.println("🔎 Sensor Readings:");
//   if (!isnan(tempDHT)) {
//     Serial.printf("🌡️ DHT11 Temp: %.1f°C\n", tempDHT);
//   } else {
//     Serial.println("❌ DHT11 Temp read failed");
//   }

//   if (!isnan(humidity)) {
//     Serial.printf("💧 Humidity: %.1f%%\n", humidity);
//   } else {
//     Serial.println("❌ Humidity read failed");
//   }

//   if (tempDS > -127.0 && tempDS < 125.0) {
//     Serial.printf("🌡️ DS18B20 Temp: %.2f°C\n", tempDS);
//   } else {
//     Serial.println("❌ DS18B20 Temp read failed");
//   }

//   Serial.printf("📏 Distance: %.2f cm\n", distance);
//   Serial.println("========================\n");

//   delay(2000);
// }
