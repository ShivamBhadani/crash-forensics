// #include <WiFi.h>
// #include <LittleFS.h>
// #include <ESPAsyncWebServer.h>
// #include <Wire.h>
// #include <MPU6050.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 32
// #define OLED_RESET    -1
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// // WiFi credentials
// const char* ssid = "Galaxy A52s 5G785C";
// const char* password = "";

// // MPU6050 setup
// MPU6050 mpu(0x68);
// typedef struct {
//   int16_t ax;
//   int16_t ay;
//   int16_t az;
// } axx;

// // Web server
// AsyncWebServer server(80);

// // Circular buffer
// const int bufferSize = 50;
// String mpuBuffer[bufferSize];
// int bufIndex = 0;
// unsigned long lastSampleTime = 0;

// void mpuinit() {
//   Wire.begin(21, 22);
//   mpu.initialize();
//   if (!mpu.testConnection()) {
//     Serial.println("MPU6050 connection failed");
//   } else {
//     Serial.println("MPU6050 connected");
//   }
// }

// void acc(axx* _a) {
//   int16_t ax, ay, az;
//   mpu.getAcceleration(&ax, &ay, &az);
//   _a->ax = ax;
//   _a->ay = ay;
//   _a->az = az;
// }

// void oledShowData(axx* d) {
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(WHITE);
//   display.setCursor(0, 0);

//   float ax_ms2 = (d->ax / 16384.0) * 9.80665;
//   float ay_ms2 = (d->ay / 16384.0) * 9.80665;
//   float az_ms2 = (d->az / 16384.0) * 9.80665;

//   display.print("AX: ");
//   display.print(ax_ms2, 2);
//   display.println(" m/s^2");

//   display.print("AY: ");
//   display.print(ay_ms2, 2);
//   display.println(" m/s^2");

//   display.print("AZ: ");
//   display.print(az_ms2, 2);
//   display.println(" m/s^2");

//   display.display();
// }

// void setup() {
//   Serial.begin(115200);

//   // OLED init
//   if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//     Serial.println("SSD1306 allocation failed");
//     while (true);
//   }
//   display.clearDisplay();
//   display.display();

//   // Wi-Fi connect
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nConnected to WiFi");
//   Serial.println(WiFi.localIP());

//   // FS
//   if (!LittleFS.begin()) {
//     Serial.println("LittleFS mount failed!");
//     return;
//   }

//   // MPU6050 init
//   mpuinit();

//   // Serve index
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//     if (LittleFS.exists("/index.html")) {
//       request->send(LittleFS, "/index.html", "text/html");
//     } else {
//       request->send(404, "text/plain", "index.html not found");
//     }
//   });

//   // Serve log
//   server.on("/mpu_log.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
//     if (LittleFS.exists("/mpu_log.txt")) {
//       request->send(LittleFS, "/mpu_log.txt", "text/plain");
//     } else {
//       request->send(404, "text/plain", "MPU log not found");
//     }
//   });

//   server.begin();
// }

// void loop() {
//   unsigned long now = millis();

//   // Log every 200ms
//   if (now - lastSampleTime >= 200) {
//     axx data;
//     acc(&data);

//     // Log data
//     String logLine = String(now) + ",";
//     logLine += "AX=" + String((data.ax / 16384.0) * 9.80665, 2);
//     logLine += ", AY=" + String((data.ay / 16384.0) * 9.80665, 2);
//     logLine += ", AZ=" + String((data.az / 16384.0) * 9.80665, 2);
//     mpuBuffer[bufIndex] = logLine;
//     bufIndex = (bufIndex + 1) % bufferSize;
//     lastSampleTime = now;

//     // Show on OLED
//     oledShowData(&data);
//   }

//   // Save every 5 seconds
//   static unsigned long lastWrite = 0;
//   if (now - lastWrite >= 5000) {
//     File f = LittleFS.open("/mpu_log.txt", "w");
//     if (f) {
//       for (int i = 0; i < bufferSize; i++) {
//         int index = (bufIndex + i) % bufferSize;
//         if (mpuBuffer[index].length())
//           f.println(mpuBuffer[index]);
//       }
//       f.close();
//       Serial.println("Saved mpu_log.txt");
//     }
//     lastWrite = now;
//   }
// }
