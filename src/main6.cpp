#include "main.cpp"

#ifdef M6
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

// ---- DHT11 ----
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---- DS18B20 ----
#define ONE_WIRE_BUS 23
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// ---- HC-SR04 ----
#define TRIG_PIN 18
#define ECHO_PIN 19

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "Galaxy A52s 5G785C";
const char* password = "";

// MPU6050 setup
MPU6050 mpu(0x68);
typedef struct {
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t h;
  int16_t m;
  int16_t s;

} axx;

// Web server
AsyncWebServer server(80);

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial gpsSerial(RXPin, TXPin);

// Circular buffer
const int bufferSize = 50;
String mpuBuffer[bufferSize];
int bufIndex = 0;
unsigned long lastSampleTime = 0;

void mpuinit() {
  Wire.begin(21, 22);
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
  } else {
    Serial.println("MPU6050 connected");
  }
}

void acc(axx* _a) {
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);
  _a->ax = ax;
  _a->ay = ay;
  _a->az = az;
}

void tim(axx* _a){
    if(gps.time.isValid()){
        int h = gps.time.hour();
        int m = gps.time.minute();
        int s = gps.time.second();
        _a->h=h;
        _a->m=m;
        _a->s=s;
    }
    
}

void oledShowData(axx* d) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  float ax_ms2 = (d->ax / 16384.0) * 9.80665;
  float ay_ms2 = (d->ay / 16384.0) * 9.80665;
  float az_ms2 = (d->az / 16384.0) * 9.80665;
  
  display.printf("T:%02d:%02d:%02d \n", d->h, d->m, d->s);

  display.print("AX: ");
  display.print(ax_ms2, 2);
  display.println(" m/s^2");

  display.print("AY: ");
  display.print(ay_ms2, 2);
  display.println(" m/s^2");

  display.print("AZ: ");
  display.print(az_ms2, 2);
  display.println(" m/s^2");
  


  display.display();
}

int x=0;

void setup() {
    Serial.begin(9600);

    dht.begin();
    ds18b20.begin();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.println("📡 Sensor system ready");

    gpsSerial.begin(GPSBaud);
  
   // OLED init
    delay(2000);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
     Serial.println("❌ OLED init failed");
     while (true);
    }
 
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("🔍 Waiting for GPS...");
    display.display();

  // Wi-Fi connect
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // FS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  // MPU6050 init
  mpuinit();

  // Serve index
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      request->send(404, "text/plain", "index.html not found");
    }
  });

  // Serve log
  server.on("/log.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/log.txt")) {
      request->send(LittleFS, "/log.txt", "text/plain");
    } else {
      request->send(404, "text/plain", "log not found");
    }
  });

  server.begin();
}

void loop() {
  unsigned long now = millis();

  float humidity = dht.readHumidity();
  float tempDHT = dht.readTemperature();

  // ----- Read DS18B20 -----
  ds18b20.requestTemperatures();
  float tempDS = ds18b20.getTempCByIndex(0);

  // ----- Read HC-SR04 -----
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.0343 / 2;

  char buff[128]={0};

  
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    // Serial.write(c);
    gps.encode(c);
  }
  
  
  // Log every 200ms
  if (now - lastSampleTime >= 200) {
    axx data ={0};
    acc(&data);
    tim(&data);
    
    snprintf(buff,sizeof(buff),"\nDHT11 Temp: %.1f°C\nHumidity: %.1f%%\nDS18B20 Temp: %.2f°C\nDistance: %.2f cm\n",tempDHT,humidity,tempDS,distance);
    String logLine = String(x++%bufferSize)+"\n";

    if (gps.location.isValid() && gps.time.isValid()) {
        float lat = gps.location.lat();
        float lng = gps.location.lng();
        float alt = gps.altitude.meters();
        int sats = gps.satellites.value();
        float speed = gps.speed.kmph();
        int h = gps.time.hour();
        int m = gps.time.minute();
        int s = gps.time.second();

        m += 30;
        if (m >= 60) {
          m -= 60;
          h += 1;
        }
        h += 5;
        if (h >= 24) {
          h -= 24;
        }

        Serial.printf("T:%02d:%02d:%02d\n",h,m,s);

        logLine += "Lat: " + String(lat);
        logLine += ",Lng: " + String(lng);
        logLine += ",Alt: " + String(alt);
        logLine += "\n";
        logLine += "Sats: " + String(sats);
        logLine += ",Speed: " + String(speed);
        logLine += "\nTime: ";
        logLine += String(h);
        logLine += ":" + String(m);
        logLine += ":" + String(s);
        logLine += "\n";

    }
    else{
        logLine += "waiting for GPS\n";
    }
    // Log data
    logLine += "AX=" + String((data.ax / 16384.0) * 9.80665, 2);
    logLine += "m/s^2";
    logLine += ", AY=" + String((data.ay / 16384.0) * 9.80665, 2);
    logLine += "m/s^2";
    logLine += ", AZ=" + String((data.az / 16384.0) * 9.80665, 2);
    logLine += "m/s^2";
    logLine += buff;
    mpuBuffer[bufIndex] = logLine;
    bufIndex = (bufIndex + 1) % bufferSize;
    lastSampleTime = now;

    Serial.println("🔎 Sensor Readings:");
  if (!isnan(tempDHT)) {
    Serial.printf("🌡️ DHT11 Temp: %.1f°C\n", tempDHT);
  } else {
    Serial.println("❌ DHT11 Temp read failed");
  }

  if (!isnan(humidity)) {
    Serial.printf("💧 Humidity: %.1f%%\n", humidity);
  } else {
    Serial.println("❌ Humidity read failed");
  }

  if (tempDS > -127.0 && tempDS < 125.0) {
    Serial.printf("🌡️ DS18B20 Temp: %.2f°C\n", tempDS);
  } else {
    Serial.println("❌ DS18B20 Temp read failed");
  }

  Serial.printf("📏 Distance: %.2f cm\n", distance);
  Serial.println("========================\n");


    // Show on OLED
    oledShowData(&data);
  }

  // Save every 5 seconds
  static unsigned long lastWrite = 0;
  if (now - lastWrite >= 5000) {
    File f = LittleFS.open("/log.txt", "w");
    if (f) {
      for (int i = 0; i < bufferSize; i++) {
        int index = (bufIndex + i) % bufferSize;
        if (mpuBuffer[index].length())
          f.println(mpuBuffer[index]);
      }
      f.close();
      Serial.println("Saved log.txt");
    }
    lastWrite = now;
  }
}

#endif