// #include <TinyGPSPlus.h>
// #include <SoftwareSerial.h>
// #include <Wire.h>
// #include <Adafruit_SSD1306.h>
// // #include <Fonts/FreeSans9pt7b.h>

// // OLED config
// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 64
// #define OLED_RESET     -1
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // GPS config
// static const int RXPin = 4, TXPin = 3;
// static const uint32_t GPSBaud = 9600;
// TinyGPSPlus gps;
// SoftwareSerial gpsSerial(RXPin, TXPin);

// // For timing
// unsigned long lastDisplay = 0;

// void setup() {
//   Serial.begin(9600);
//   gpsSerial.begin(GPSBaud);

//   // OLED init
//   delay(2000);
//   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//     Serial.println("❌ OLED init failed");
//     while (true);
//   }

//   display.clearDisplay();
//   display.setTextSize(2);
// //   display.setFont(&FreeSans9pt7b);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 0);
//   display.println("🔍 Waiting for GPS...");
//   display.display();

//   Serial.println("🛰️ GPS + OLED Logger Ready");
// }

// void loop() {
//   while (gpsSerial.available()) {
//     char c = gpsSerial.read();
//     gps.encode(c);
//   }

//   if (millis() - lastDisplay > 1000) {
//     lastDisplay = millis();
//     display.clearDisplay();
//     display.setCursor(0, 0);

//     if (gps.location.isValid() && gps.time.isValid()) {
//       float lat = gps.location.lat();
//       float lng = gps.location.lng();
//       float alt = gps.altitude.meters();
//       int sats = gps.satellites.value();
//       float speed = gps.speed.kmph();
//       int h = gps.time.hour();
//     //   h+=5;
//       int m = gps.time.minute();
//     //   if(m+30>=60){
//         // m=60-m;
//         // h++;
//     //   }
//     //   else m+=30;
//       int s = gps.time.second();

//       // Serial Output
//       Serial.println("===== GPS DATA =====");
//       Serial.printf("Time: %02d:%02d:%02d\n", h, m, s);
//       Serial.printf("Lat: %.6f\n", lat);
//       Serial.printf("Lng: %.6f\n", lng);
//       Serial.printf("Alt: %.2f m\n", alt);
//       Serial.printf("Speed: %.2f km/h\n", speed);
//       Serial.printf("Sats: %d\n", sats);
//       Serial.println("====================");

//       // OLED Output
//       display.printf("T:%02d:%02d:%02d \n", h, m, s);
//       display.printf("Lat: %.2f\n", lat);
//       display.printf("Lng: %.2f\n", lng);
//     //   display.printf("Alt: %d m\n", (int)alt);
//     //   display.printf("Spd: %d km/h\n", (int)speed);
//       display.printf("Sat: %d\n", sats);
//     } else {
//       Serial.println("⌛ Waiting for GPS fix or time...");
//       display.setTextSize(2);
//       display.println("Waiting for GPS fix...");
//     }

//     display.display();
//   }
// }
