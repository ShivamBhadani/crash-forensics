// #include <Arduino.h>

// #define TRIG_PIN 18
// #define ECHO_PIN 19

// void setup() {
//   Serial.begin(115200);

//   // Initialize HC-SR04 pins
//   pinMode(TRIG_PIN, OUTPUT);
//   pinMode(ECHO_PIN, INPUT);

//   Serial.println("HC-SR04 Distance Sensor Initialized");
// }

// void loop() {
//   // Make sure the trigger pin is LOW before sending the pulse
//   digitalWrite(TRIG_PIN, LOW);
//   delayMicroseconds(2);
  
//   // Send a 10us pulse to trigger the HC-SR04
//   digitalWrite(TRIG_PIN, HIGH);
//   delayMicroseconds(10);
//   digitalWrite(TRIG_PIN, LOW);
  
//   // Read the echo pulse duration
//   long duration = pulseIn(ECHO_PIN, HIGH);

//   // Calculate the distance (in cm)
//   float distance = duration * 0.0343 / 2;  // Speed of sound: 343m/s or 0.0343cm/µs

//   // Check if the distance is zero or unrealistically small, which might indicate an issue
//   if (distance <= 0 || distance >= 400) {
//     Serial.println("Invalid distance reading, check the sensor setup!");
//   } else {
//     Serial.printf("📏 Distance: %.2f cm\n", distance);
//   }

//   delay(1000); // Delay to give time between readings
// }
