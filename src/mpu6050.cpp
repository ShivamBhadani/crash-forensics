// #include <Wire.h>
// #include <Arduino.h>

// void setup() {
//   Wire.begin(); // Default pins: ESP32 = SDA 21, SCL 22
//   Serial.begin(115200);
//   while (!Serial); // Wait for serial monitor

//   Serial.println("\nI2C Scanner Ready");
// }

// void loop() {
//   byte error, address;
//   int nDevices = 0;

//   Serial.println("Scanning...");

//   for (address = 1; address < 127; address++) {
//     Wire.beginTransmission(address);
//     error = Wire.endTransmission();

//     if (error == 0) {
//       Serial.print("I2C device found at 0x");
//       if (address < 16) Serial.print("0");
//       Serial.print(address, HEX);
//       Serial.println("  ✔");
//       nDevices++;
//     } else if (error == 4) {
//       Serial.print("Unknown error at 0x");
//       if (address < 16) Serial.print("0");
//       Serial.println(address, HEX);
//     }
//   }

//   if (nDevices == 0)
//     Serial.println("No I2C devices found ❌");
//   else
//     Serial.println("Scan complete ✅");

//   delay(5000); // Wait before scanning again
// }
