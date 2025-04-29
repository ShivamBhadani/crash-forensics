#include "main.cpp"

#ifdef M1

#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu(0x68);  // Force I2C address

typedef struct{
    int16_t ax;
    int16_t ay;
    int16_t az;
}axx;

void mpuinit(){
  Wire.begin(21, 22); // Adjust if using other pins

  
  mpu.initialize();

  // Optional check
  uint8_t whoami = mpu.getDeviceID();
  Serial.print("WHO_AM_I = 0x");
  Serial.println(whoami, HEX);
}


void acc(axx *_a) {
  int16_t ax, ay, az;
  // int16_t gx, gy, gz;

  mpu.getAcceleration(&ax, &ay, &az);
  _a->ax=ax;
  _a->ay=ay;
  _a->az=az;
  return;
  // mpu.getRotation(&gx, &gy, &gz);

//   Serial.print("Accel => X: "); Serial.print(ax);
//   Serial.print(" | Y: "); Serial.print(ay);
//   Serial.print(" | Z: "); Serial.print(az);
//   Serial.print("\n");

  // Serial.print(" || Gyro => X: "); Serial.print(gx);
  // Serial.print(" | Y: "); Serial.print(gy);
  // Serial.print(" | Z: "); Serial.println(gz);

//   delay(5000);
}

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "painlessMesh.h"

// Wi-Fi credentials for ESP32 Access Point (AP)
const char* apSSID = "ESP32-Car-Control"; // Access Point SSID
const char* apPassword = "123456789"; // Password for the AP

// Motor control pins (updated for ESP32)
const int IN1 = 32;
const int IN2 = 33;
const int IN3 = 25;
const int IN4 = 26;
const int Nsleep = 27;

// Accelerometer data variables
float ax_g = 0.0, ay_g = 0.0, az_g = 0.0;

// Threshold for considering acceleration
const float accelerationThreshold = 0.06;  // in g
const float broadcastThreshold = 0.9; // Threshold to trigger the mesh broadcast

// Mesh network setup
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

// Web server setup
AsyncWebServer server(80);

// Motor control variables
float carSpeed = 0.0;
unsigned long lastTime = 0;

// Default motor speed (not needed for DRV8833)
int motorSpeed = 1023; // No speed control for DRV8833

void sendMessage() {  // Function for mesh broadcast
  String msg = "Car is moving fast!";
  mesh.sendBroadcast(msg);
}

// Needed for painless library callbacks
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("startHere: Received msg=%s\n", msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void mesh_init(void){
    // Initialize mesh network
  mesh.setDebugMsgTypes(ERROR | STARTUP);  // Debugging options
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void mesh_deinit() {
  // Stop the mesh network
  WiFi.softAP(apSSID, apPassword);
  Serial.println(WiFi.softAPIP());
  server.begin();
  Serial.println("Mesh network deinitialized.");
  Serial.println("Server restarted");
}

// Motor control functions
void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(400); // Turn for 400ms
  motorStop(); // Stop after turning
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(400); // Turn for 400ms
  motorStop(); // Stop after turning
}

void setup() {
  Serial.begin(115200);


  // Initialize motor control pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(Nsleep, OUTPUT);
  
  // Set the Nsleep pin HIGH to wake up the motor driver
  digitalWrite(Nsleep, HIGH);

  // Start ESP32 as an Access Point (AP)
  WiFi.softAP(apSSID, apPassword);

  Serial.print("ESP32 is in AP Mode. IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Start the web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head><title>Car Control</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background: #F8E1D4; margin: 0; padding: 0; height: 100vh; display: flex; justify-content: center; align-items: center; flex-direction: column; }";
  html += "h1 { font-size: 2.5rem; color: #4B4B4B; margin-bottom: 20px; }";
  html += ".container { display: flex; flex-wrap: wrap; justify-content: center; align-items: center; gap: 30px; margin-top: 20px; }";
  html += ".btn { background-color: #A5D8FF; color: black; padding: 30px; border-radius: 50%; width: 200px; height: 200px; font-size: 25px; border: none; cursor: pointer; transition: 0.3s; box-shadow: 0 6px 10px rgba(0, 0, 0, 0.1); }";
  html += ".btn:hover { background-color: #78C5D6; }";
  html += ".btn-stop { background-color: #FF8B8B; width: 140px; height: 140px; font-size: 32px; border-radius: 50%; }";
  html += ".slider { width: 100%; max-width: 500px; margin-top: 30px; }";
  html += ".slider-container { text-align: center; margin-top: 40px; }";
  
  // Improved accelerometer styling
  html += ".accel-values { font-size: 1.5rem; margin-top: 30px; background-color: #fff; border-radius: 15px; padding: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2); display: inline-block; max-width: 400px; }";
  html += ".accel-values h2 { color: #4B4B4B; font-size: 1.8rem; margin-bottom: 10px; }";
  html += ".accel-value { margin: 10px 0; font-size: 1.2rem; color: #4B4B4B; }";
  html += ".accel-value span { font-weight: bold; color: #007BFF; }";
  
  html += "#speed { font-size: 2rem; font-weight: bold; color: #28a745; margin-top: 20px; background-color: #F1F1F1; padding: 10px 20px; border-radius: 10px; }";
  html += "@media (max-width: 600px) { .btn { width: 100px; height: 100px; font-size: 24px; } .btn-stop { width: 120px; height: 120px; font-size: 28px; } .accel-values { max-width: 90%; } }";
  html += "</style>";
  html += "<script>";
  html += "function fetchAccelData() {";
  html += "fetch('/getAccelData').then(response => response.json()).then(data => {";
  html += "document.getElementById('ax').innerText = 'x: ' + data.ax + ' g';";
  html += "document.getElementById('ay').innerText = 'y: ' + data.ay + ' g';";
  html += "document.getElementById('az').innerText = 'z: ' + data.az + ' g';";
  html += "document.getElementById('speed').innerText = 'Speed: ' + data.speed.toFixed(2) + ' m/s';";
  html += "});";
  html += "setTimeout(fetchAccelData, 100);"; // Update every 100ms for real-time speed
  html += "}"; 
  html += "window.onload = fetchAccelData;";
  html += "</script>";
  html += "</head><body>";
  html += "<h1>Control Your Car</h1>";

  // Buttons for movement control
  html += "<div class='container'>";
  html += "<button class='btn' onclick='location.href=\"/backward\"'>Backward</button>";
  html += "<button class='btn' onclick='location.href=\"/forward\"'>Forward</button><br>";
  html += "<button class='btn' onclick='location.href=\"/left\"'>Left</button>";
  html += "<button class='btn' onclick='location.href=\"/right\"'>Right</button>";
  html += "</div>";

  // Stop button
  html += "<button class='btn-stop' onclick='location.href=\"/stop\"'>Stop</button>";

  // Display speed value above accelerometer values
  html += "<div id='speed'>Speed: " + String(carSpeed, 2) + " m/s</div>";

  // Display accelerometer values dynamically
  html += "<div class='accel-values'>";
  html += "<h2>Accelerometer (in g)</h2>";
  html += "<p class='accel-value' id='ax'>x: " + String(ax_g, 4) + " g</p>";
  html += "<p class='accel-value' id='ay'>y: " + String(ay_g, 4) + " g</p>";
  html += "<p class='accel-value' id='az'>z: " + String(az_g, 4) + " g</p>";
  html += "</div>";

  html += "</body></html>";
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", html);
  request->send(response);
  });

  server.on("/getAccelData", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"ax\": " + String(ax_g, 4) + ", \"ay\": " + String(ay_g, 4) + ", \"az\": " + String(az_g, 4) + ", \"speed\": " + String(carSpeed, 2) + "}";
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    request->send(response);
  });

  server.on("/forward", HTTP_GET,[](AsyncWebServerRequest *request) {
    moveForward();
    AsyncWebServerResponse *response = request->beginResponse(303, "text/html", "");
    response->addHeader("Location", "/"); // Redirect to root

    // Send the response
    request->send(response);
    
  });
  server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request) {
    moveBackward();
    AsyncWebServerResponse *response = request->beginResponse(303, "text/html", "");
    response->addHeader("Location", "/"); // Redirect to root

    // Send the response
    request->send(response);
    
  });
  server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request) {
    turnLeft();
    AsyncWebServerResponse *response = request->beginResponse(303, "text/html", "");
    response->addHeader("Location", "/"); // Redirect to root

    // Send the response
    request->send(response);
    
  });
  server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request) {
    turnRight();
    AsyncWebServerResponse *response = request->beginResponse(303, "text/html", "");
    response->addHeader("Location", "/"); // Redirect to root

    // Send the response
    request->send(response);
    
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    motorStop();
    AsyncWebServerResponse *response = request->beginResponse(303, "text/html", "");
    response->addHeader("Location", "/"); // Redirect to root

    // Send the response
    request->send(response);
    
  });
  mpuinit();

  
  delay(2000);
  // Start the server
  server.begin();
}

void loop() {
  axx _a;
  acc(&_a);
  // int16_t ax, ay, az;

  // Read accelerometer data from the MPU6050
  // mpu.getAcceleration(&ax, &ay, &az);

  // Convert the raw accelerometer data to 'g' (gravitational units)
  ax_g = _a.ax / 16384.0;
  ay_g = _a.ay / 16384.0;
  az_g = _a.az / 16384.0;

  delay(1000);

  // Check if either X or Y acceleration exceeds the threshold (for mesh broadcast)
  // if (fabs(ax_g) > broadcastThreshold || fabs(ay_g) > broadcastThreshold) {
  //   mesh_init();
  //   delay(200);
  //   // Trigger the mesh broadcast if the threshold is exceeded
  //   sendMessage();  // Broadcast mesh message
  //   mesh_deinit();
  // }

  // Only consider acceleration in the X-axis if it exceeds the threshold
  // if (fabs(ax_g) > accelerationThreshold) {
    // Calculate acceleration in m/s^2
    // float ax_mps2 = ax_g * 9.81;

    // Calculate the time difference (in seconds)
    // unsigned long currentTime = millis();
    // float deltaTime = (currentTime - lastTime) / 1000.0;  // in seconds

    // Update speed: v = u + at (initial speed u = 0, so v = at)
    // carSpeed += ax_mps2 * deltaTime;  // m/s

    // Limit speed to prevent it from becoming unreasonably large
    // if (carSpeed < 0) carSpeed = 0;

    // lastTime = currentTime;  // Update the time for the next loop
  // }

  // Handle the HTTP requests for the web server
  // server.handleClient();
}

#endif
