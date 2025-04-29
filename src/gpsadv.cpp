// #include <TinyGPSPlus.h>
// #include <SoftwareSerial.h>
// #include <Wire.h>
// #include <Adafruit_SSD1306.h>
// #include <WiFi.h>
// #include <ESPAsyncWebServer.h>

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

// // Wi-Fi config
// const char* ssid = "Galaxy A52s 5G785C";  // Replace with your Wi-Fi SSID
// const char* password = "";  // Replace with your Wi-Fi password

// // Web server config
// AsyncWebServer server(80);

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
//   display.setTextColor(WHITE);
//   display.setCursor(0, 0);
//   display.println("🔍 Waiting for GPS...");
//   display.display();

//   // Connect to Wi-Fi
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi...");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nConnected to WiFi");
//   Serial.println(WiFi.localIP());

//   // Serve the web page
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//     String html = "<html><body>";
//     html += "<h1>GPS Data</h1>";
//     if (gps.location.isValid() && gps.time.isValid()) {
//       float lat = gps.location.lat();
//       float lng = gps.location.lng();
//       float alt = gps.altitude.meters();
//       int sats = gps.satellites.value();
//       float speed = gps.speed.kmph();
//       int h = gps.time.hour();
//       int m = gps.time.minute();
//       int s = gps.time.second();

//       html += "<p><strong>Time:</strong> " + String(h) + ":" + String(m) + ":" + String(s) + "</p>";
//       html += "<p><strong>Latitude:</strong> " + String(lat, 6) + "</p>";
//       html += "<p><strong>Longitude:</strong> " + String(lng, 6) + "</p>";
//       html += "<p><strong>Altitude:</strong> " + String(alt, 2) + " m</p>";
//       html += "<p><strong>Speed:</strong> " + String(speed, 2) + " km/h</p>";
//       html += "<p><strong>Satellites:</strong> " + String(sats) + "</p>";
//     } else {
//       html += "<p>Waiting for GPS data...</p>";
//     }
//     html += "</body></html>";
//     request->send(200, "text/html", html);
//   });

//   // Start the server
//   server.begin();
//   Serial.println("Server started");
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
//       int m = gps.time.minute();
//       int s = gps.time.second();

//       // OLED Output
//       display.printf("T:%02d:%02d:%02d \n", h, m, s);
//       display.printf("Lat: %.2f\n", lat);
//       display.printf("Lng: %.2f\n", lng);
//       display.printf("Sat: %d\n", sats);
//     } else {
//       display.setTextSize(2);
//       display.println("Waiting for GPS fix...");
//     }

//     display.display();
//   }
// }
