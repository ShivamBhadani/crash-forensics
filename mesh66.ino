#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wi-Fi credentials for ESP8266 Access Point (AP)
// const char* apSSID = "ESP8266-Car-Control"; // Access Point SSID
// const char* apPassword = "123456789"; // Password for the AP


// Wi-Fi credentials for your local Wi-Fi network
const char* ssid = "Galaxy A52s 5G785C";
const char* password = "";


// Motor control pins (updated for DRV8833)
const int IN1 = D3;
const int IN2 = D4;
const int IN3 = D5;
const int IN4 = D6;
const int Nsleep = D7;

// Default motor speed (not needed for DRV8833)
int motorSpeed = 1023; // No speed control for DRV8833

// Web server setup
ESP8266WebServer server(80);

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
  delay(100); // Turn for 400ms
  motorStop(); // Stop after turning
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(100); // Turn for 400ms
  motorStop(); // Stop after turning
}

void handleRoot();

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

  // Start ESP8266 as an Access Point (AP)
  // WiFi.softAP(apSSID, apPassword);

  // Serial.print("ESP8266 is in AP Mode. IP Address: ");
  // Serial.println(WiFi.softAPIP());

   WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected. IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the web server
  server.on("/", HTTP_GET, handleRoot);

  server.on("/forward", HTTP_GET, []() {
    moveForward();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/backward", HTTP_GET, []() {
    moveBackward();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/left", HTTP_GET, []() {
    turnLeft();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/right", HTTP_GET, []() {
    turnRight();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  server.on("/stop", HTTP_GET, []() {
    motorStop();
    server.sendHeader("Location", "/");
    server.send(303);
  });
  // Start the server
  server.begin();
}

void loop() {
  // Handle the HTTP requests for the web server
  server.handleClient();
}

void handleRoot() {
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
  html += "@media (max-width: 600px) { .btn { width: 100px; height: 100px; font-size: 24px; } .btn-stop { width: 120px; height: 120px; font-size: 28px; }";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>Control Your Car</h1>";
  // Buttons for movement control
  html += "<div class='container'>";
  html += "<button class='btn' onclick='location.href=\"/backward\"'>Forward</button>";
  html += "<button class='btn' onclick='location.href=\"/forward\"'>Backward</button><br>";
  html += "<button class='btn' onclick='location.href=\"/left\"'>Right</button>";
  html += "<button class='btn' onclick='location.href=\"/right\"'>Left</button>";
  html += "</div>";
  // Stop button
  html += "<button class='btn-stop' onclick='location.href=\"/stop\"'>Stop</button>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}
