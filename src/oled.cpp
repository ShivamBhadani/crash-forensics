
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// #define SCREEN_WIDTH 128
// #define SCREEN_HEIGHT 32 // for 0.91" OLED

// #define OLED_RESET     -1 // No reset pin
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// void setup() {
//     Serial.begin(115200);
//     delay(2000);  // Let everything settle (power, I2C, display)
  
//     Wire.begin(21, 22); // Or your custom I2C pins
  
//     for (int i = 0; i < 3; i++) {
//       if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
//         Serial.println("✅ OLED initialized");
//         display.clearDisplay();
//         display.setTextSize(1);
//         display.setTextColor(WHITE);
//         display.setCursor(0, 0);
//         display.println("OLED is alive!");
//         display.display();
//         break;
//       } else {
//         Serial.println("⚠️ OLED failed to init, retrying...");
//         delay(500);
//       }
//     }
//   }
  
// void loop(){
//     while(1);
// }