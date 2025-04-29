#include "main.cpp"

#ifdef M7

#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MPU6050.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>

// --- Constants ---
#define DHTPIN 15
#define DHTTYPE DHT11
#define ONE_WIRE_BUS 23
#define TRIG_PIN 18
#define ECHO_PIN 19
#define RXPin 4
#define TXPin 3
#define GPSBaud 9600
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define MPU_SCALE 9.80665 / 16384.0

// --- Objects ---
DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MPU6050 mpu(0x68);
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);
AsyncWebServer server(80);

// --- WiFi ---
const char* ssid = "Galaxy A52s 5G785C";
const char* password = "";

// --- Data Structures ---
struct AccelData {
  float ax, ay, az;
};

struct GPSData {
  float lat, lng, alt, speed;
  int sats;
  int hour, minute, second;
};

struct SensorData {
  float tempDHT, humidity, tempDS, distance;
  AccelData accel;
  GPSData gps;
  bool gpsValid;
};

typedef struct AccelData oldacc;
oldacc _o ={0.0f, 0.0f, 0.0f};

typedef struct AccelData delacc;
delacc _c ={0.0f, 0.0f, 0.0f};

// --- Global State ---
SensorData sensorData;
String logBuffer[50];
int bufIndex = 0, sampleIndex = 0;
unsigned long lastSample = 0, lastSave = 0;

enum class EventSeverity {
    NONE = 0,
    MILD_JERK = 1,
    MODERATE_SHAKE = 2,
    SEVERE_SHAKE = 3,
    SEVERE_WRECK = 4,
    SPEEDING = 5,
    DECELERATING = 6
};


// Constants
#define ACCEL_THRESHOLD_CRASH 1.5        // Threshold for crash classification
#define ACCEL_THRESHOLD_SPEEDING 0.5     // Threshold for speeding detection
#define ACCEL_NOISE_THRESHOLD_X 0.15    // Threshold for noise in X direction
#define ACCEL_NOISE_THRESHOLD_Y 0.15    // Threshold for noise in Y direction
#define ACCEL_NOISE_THRESHOLD_Z 2.0     // Threshold for noise in Z direction
#define ACCEL_THRESHOLD_DECELERATION 1.5  // Threshold for significant deceleration

#define TIME_INTERVAL 200               // Time interval in milliseconds (for speed calculation)

// Buffer for last three acceleration readings (magnitude and direction)
float accelBuffer[3] = {0.0f, 0.0f, 0.0f};  
float lastAccel[3] = {0.0f, 0.0f, 0.0f}; // Previous accelerations (to detect deltas)
unsigned long lastSpeedUpdate = 0;
float currentSpeed = 0.0f;              // Vehicle speed in m/s
unsigned long lastAccelTime = 0;       // Time of the last acceleration update

// === Function Prototypes ===
void readAllSensors();
void updateDisplay();
void logToBuffer();
void saveToFile();
void mpuinit();
String hashLogFile();
EventSeverity getEventSeverity(float ax, float ay, float az);

// === Initialization ===
void setup() {
  Serial.begin(9600);
  dht.begin();
  ds18b20.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("📡 Sensor system ready");
  gpsSerial.begin(GPSBaud);
  
  delay(2000);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println("❌ OLED init failed");
    //   while (true);
    }
    
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Waiting for GPS...");
  display.display();
  
  
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("WiFi Connected: " + WiFi.localIP().toString());
  
  if (!LittleFS.begin()) Serial.println("❌ LittleFS mount failed");
  
  mpuinit();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/log.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/log.txt", "text/plain");
  });

  server.on("/hashlog", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/log.sha256")) {
      request->send(LittleFS, "/log.sha256", "text/plain");
    } else {
      request->send(404, "text/plain", "Hash not found. Please generate log.");
    }
  });  

  server.begin();
}

// === Main Loop ===
void loop() {
  while (gpsSerial.available()) gps.encode(gpsSerial.read());

  volatile unsigned long now = millis();
  if (now - lastSample >= 200) {
    //   Serial.println(now);
    //   Serial.println(lastSample);
      readAllSensors();
      updateDisplay();
      logToBuffer();
      lastSample = now;
    }
    
    now = millis();
    if (now - lastSave >= 5000) {
        // Serial.println(now);
        // Serial.println(lastSave);
      saveToFile();
      lastSave = now;
      if(!sensorData.gpsValid)
        Serial.println("🔍 Waiting for GPS...");
  }
}

// === Read All Sensors ===
void readAllSensors() {
  sensorData.humidity = dht.readHumidity();
  sensorData.tempDHT = dht.readTemperature();

  ds18b20.requestTemperatures();
  sensorData.tempDS = ds18b20.getTempCByIndex(0);

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  sensorData.distance = duration * 0.0343 / 2;

  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  _o.ax=sensorData.accel.ax;
  _o.ay=sensorData.accel.ay;
  _o.az=sensorData.accel.az;
  sensorData.accel.ax = ax * MPU_SCALE;
  sensorData.accel.ay = ay * MPU_SCALE;
  sensorData.accel.az = az * MPU_SCALE;
  _c.ax = round((sensorData.accel.ax - _o.ax) * (1000.0 / (millis() - lastSample)) * 10.0) / 10.0;
  _c.ay = round((sensorData.accel.ay - _o.ay) * (1000.0 / (millis() - lastSample)) * 10.0) / 10.0;
  _c.az = round((sensorData.accel.az - _o.az) * (1000.0 / (millis() - lastSample)) * 10.0) / 10.0;
  

//   Serial.printf("Ax: %.2f, Ay: %.2f, Az: %.2f\n", sensorData.accel.ax, sensorData.accel.ay, sensorData.accel.az);
//   Serial.printf("Prev Ax: %.2f, Ay: %.2f, Az: %.2f\n", _o.ax, _o.ay, _o.az);
  
  // Then print the delta accelerations
//   Serial.printf("Delta Ax: %.6f, Ay: %.6f, Az: %.6f\n", _c.ax, _c.ay, _c.az);
  

  if (gps.location.isValid() && gps.time.isValid()) {
    sensorData.gpsValid = true;
    sensorData.gps.lat = gps.location.lat();
    sensorData.gps.lng = gps.location.lng();
    sensorData.gps.alt = gps.altitude.meters();
    sensorData.gps.sats = gps.satellites.value();
    sensorData.gps.speed = gps.speed.kmph();

    int h = gps.time.hour();
    int m = gps.time.minute();
    int s = gps.time.second();
    m += 30; if (m >= 60) { m -= 60; h++; }
    h += 5; if (h >= 24) h -= 24;
    Serial.printf("T:%02d:%02d:%02d\n",h,m,s);
    sensorData.gps.hour = h;
    sensorData.gps.minute = m;
    sensorData.gps.second = s;
  } else {
    sensorData.gpsValid = false;
  }
}

// === OLED Display Update ===
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  if (sensorData.gpsValid)
    display.printf("T:%02d:%02d:%02d\n", sensorData.gps.hour, sensorData.gps.minute, sensorData.gps.second);
  else
    display.println("No GPS");

  display.printf("Ax: %.2f m/s^2\n", sensorData.accel.ax);
  display.printf("Ay: %.2f m/s^2\n", sensorData.accel.ay);
  display.printf("Az: %.2f m/s^2\n", sensorData.accel.az);

  display.display();
}

// === Save Buffer to File ===
void saveToFile() {
    File file = LittleFS.open("/log.txt", "w");
    if (file) {
      for (int i = 0; i < 50; ++i) {
        int index = (bufIndex + i) % 50;
        if (logBuffer[index].length()) file.println(logBuffer[index]);
      }
      file.close();
      Serial.println("💾 log.txt saved");
  
      // Generate and save hash to /log.sha256
      String hash = hashLogFile();
      Serial.println("✅ SHA-256 hash updated in /log.sha256:");
      Serial.println(hash);
    }
}  

void mpuinit() {
    Wire.begin(21, 22);
    mpu.initialize();
    if (!mpu.testConnection()) {
      Serial.println("MPU6050 connection failed");
    } else {
      Serial.println("MPU6050 connected");
    }
}

String hashLogFile() {
    File file = LittleFS.open("/log.txt", "r");
    if (!file) {
      Serial.println("❌ Failed to open log.txt");
      return "ERROR: Unable to read log.txt";
    }
  
    // Initialize SHA-256 context
    mbedtls_md_context_t ctx;
    const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!info) return "ERROR: SHA256 info unavailable";
  
    mbedtls_md_init(&ctx);
    if (mbedtls_md_setup(&ctx, info, 0) != 0) {
      mbedtls_md_free(&ctx);
      return "ERROR: SHA256 setup failed";
    }
  
    mbedtls_md_starts(&ctx);
  
    // Read file in chunks
    const size_t bufSize = 512;
    uint8_t buffer[bufSize];
    while (file.available()) {
      size_t len = file.read(buffer, bufSize);
      mbedtls_md_update(&ctx, buffer, len);
    }
  
    unsigned char hash[32]; // SHA256 = 32 bytes
    mbedtls_md_finish(&ctx, hash);
    mbedtls_md_free(&ctx);
    file.close();
  
    // Convert hash to hex string
    String hashString = "";
    for (int i = 0; i < sizeof(hash); i++) {
      if (hash[i] < 0x10) hashString += "0";
      hashString += String(hash[i], HEX);
    }
  
    // Save the hash to /log.sha256
    File hashFile = LittleFS.open("/log.sha256", "w");
    if (hashFile) {
      hashFile.println(hashString);
      hashFile.close();
    } else {
      Serial.println("❌ Failed to write /log.sha256");
    }
  
    return hashString;
}

// Function to update speed based on acceleration
void updateVehicleSpeed(float ax, float ay, float az, unsigned long currentTime) {
    // Calculate the magnitude of the current acceleration
    float magnitude = sqrt(ax * ax + ay * ay + az * az);
  
    // Update speed if enough time has passed since last update
    if (currentTime - lastSpeedUpdate >= TIME_INTERVAL) {
        float deltaTime = (currentTime - lastSpeedUpdate) / 1000.0f; // Convert to seconds
        if (magnitude > ACCEL_NOISE_THRESHOLD_Z) {  // Ignore noise or gravitational forces
            currentSpeed += magnitude * deltaTime;  // Speed = acceleration * time (simplified)
        }
        lastSpeedUpdate = currentTime;
    }
}

// Function to determine the event severity (Crash, Speeding, Deceleration)
EventSeverity getEventSeverity(float ax, float ay, float az) {
    // Calculate the magnitude of the current acceleration
    float magnitude = sqrt(ax * ax + ay * ay + az * az);

    // Update the acceleration buffer with the current magnitude
    accelBuffer[0] = accelBuffer[1];
    accelBuffer[1] = accelBuffer[2];
    accelBuffer[2] = magnitude;

    // If the magnitude is significantly above the threshold, consider it a crash
    if (magnitude >= ACCEL_THRESHOLD_CRASH && accelBuffer[0] >= ACCEL_THRESHOLD_CRASH && accelBuffer[1] >= ACCEL_THRESHOLD_CRASH) {
        return EventSeverity::SEVERE_WRECK;  // Severe crash
    }

    // Check if it's a sudden change (one-off event) indicating a crash or jerk
    if (magnitude >= ACCEL_THRESHOLD_CRASH && (accelBuffer[0] < ACCEL_THRESHOLD_CRASH || accelBuffer[1] < ACCEL_THRESHOLD_CRASH)) {
        return EventSeverity::SEVERE_SHAKE;  // Sudden jerk (crash)
    }

    // Speeding classification based on persistent acceleration
    if (magnitude >= ACCEL_THRESHOLD_SPEEDING) {
        // If the vehicle has been accelerating for a while, classify as speeding
        if (fabs(ax - lastAccel[0]) > ACCEL_THRESHOLD_SPEEDING || fabs(ay - lastAccel[1]) > ACCEL_THRESHOLD_SPEEDING || fabs(az - lastAccel[2]) > ACCEL_THRESHOLD_SPEEDING) {
            return EventSeverity::SPEEDING;  // Persisting acceleration, it's speeding
        }
    }

    // Check for deceleration
    if (magnitude < ACCEL_NOISE_THRESHOLD_X && magnitude < ACCEL_NOISE_THRESHOLD_Y && magnitude < ACCEL_NOISE_THRESHOLD_Z) {
        return EventSeverity::DECELERATING;  // The vehicle is slowing down
    }

    return EventSeverity::NONE;
}

// Function to log data and event to buffer
void logToBuffer() {
    String log = String(sampleIndex++ % 50) + "\n";

    if (sensorData.gpsValid) {
        log += "Lat: " + String(sensorData.gps.lat) + ",Lng: " + String(sensorData.gps.lng) + "\n";
        log += "Alt: " + String(sensorData.gps.alt) + ",Speed: " + String(sensorData.gps.speed) + "\n";
        log += "Time: " + String(sensorData.gps.hour) + ":" + sensorData.gps.minute + ":" + sensorData.gps.second + "\n";
    } else {
        log += "Waiting for GPS\n";
    }

    log += "AX=" + String(sensorData.accel.ax, 2) + " AY=" + String(sensorData.accel.ay, 2) + " AZ=" + String(sensorData.accel.az, 2) + "\n";
    log += "Cx=" + String(_c.ax, 1) + " Cy=" + String(_c.ay, 1) + " Cz=" + String(_c.az, 1) + "\n";
    log += "DHT Temp: " + String(sensorData.tempDHT) + "C, Humidity: " + String(sensorData.humidity) + "%\n";
    log += "DS18B20 Temp: " + String(sensorData.tempDS) + "C\n";
    log += "Distance: " + String(sensorData.distance) + "cm\n";

    // Get event severity (Crash, Speeding, Deceleration)
    EventSeverity severity = getEventSeverity(_c.ax, _c.ay, _c.az);
    String severityLabel;

    switch (severity) {
        case EventSeverity::NONE:
            severityLabel = "No Event";
            break;
        case EventSeverity::MILD_JERK:
            severityLabel = "Mild Jerk";
            break;
        case EventSeverity::MODERATE_SHAKE:
            severityLabel = "Moderate Shake";
            break;
        case EventSeverity::SEVERE_SHAKE:
            severityLabel = "Severe Shake (Crash)";
            break;
        case EventSeverity::SEVERE_WRECK:
            severityLabel = "Severe Wreck (Crash)";
            break;
        case EventSeverity::SPEEDING:
            severityLabel = "Speeding";
            break;
        case EventSeverity::DECELERATING:
            severityLabel = "Decelerating";
            break;
    }

    log += "Event Severity: " + severityLabel + "\n";
    log += "Vehicle Speed: " + String(currentSpeed, 2) + " m/s\n";  // Log the vehicle speed

    logBuffer[bufIndex] = log;
    bufIndex = (bufIndex + 1) % 50;

    // Save the last acceleration values
    lastAccel[0] = _c.ax;
    lastAccel[1] = _c.ay;
    lastAccel[2] = _c.az;
}


#endif