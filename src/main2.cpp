#ifdef M2
// Wi-Fi credentials
const char* ssid = "Galaxy A52s 5G785C";
const char* password = "";


#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>


// Async Web Server on port 80
AsyncWebServer server(80);

// Password Hashing Stuff
const char* plainPassword = "user1234";
unsigned char salt[16] = {
  0x12, 0x34, 0x56, 0x78,
  0x9A, 0xBC, 0xDE, 0xF0,
  0x12, 0x34, 0x56, 0x78,
  0x9A, 0xBC, 0xDE, 0xF0
};
unsigned char hashedPassword[32]; // SHA256 = 32 bytes

void hashPassword() {
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);

  const mbedtls_md_info_t* info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (!info) {
    Serial.println("Failed to get SHA256 info.");
    return;
  }

  if (mbedtls_md_setup(&ctx, info, 1) != 0) {
    Serial.println("Failed to setup md context.");
    return;
  }

  int ret = mbedtls_pkcs5_pbkdf2_hmac(
    &ctx,
    (const unsigned char*)plainPassword, strlen(plainPassword),
    salt, sizeof(salt),
    1000,                    // Iterations
    sizeof(hashedPassword), // Output length
    hashedPassword
  );

  if (ret != 0) {
    Serial.printf("Hashing failed: %d\n", ret);
  } else {
    Serial.print("Password hash: ");
    for (int i = 0; i < sizeof(hashedPassword); i++) {
      if (hashedPassword[i] < 0x10) Serial.print("0");
      Serial.print(hashedPassword[i], HEX);
    }
    Serial.println();
  }

  mbedtls_md_free(&ctx);
}

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  // Mount LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed!");
    return;
  }

  // Hash the password
  hashPassword();

  // Serve index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
    } else {
      request->send(404, "text/plain", "index.html not found");
    }
  });

  server.begin();
}

void loop() {
  // Nothing here
}

#endif