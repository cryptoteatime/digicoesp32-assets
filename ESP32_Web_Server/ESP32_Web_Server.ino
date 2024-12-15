// Import required libraries
#include "WiFi.h"                           // Enables Wi-Fi
#include "ESPAsyncWebServer.h"              // Web server
#include <Adafruit_Sensor.h>                // DHT sensor
#include <DHT.h>                            // DHT sensor library
#include <Adafruit_GFX.h>                   // OLED graphics
#include <Adafruit_SSD1306.h>               // OLED library
#include <Wire.h>                           // I2C communication
#include <ESPmDNS.h>                        // mDNS
//#include <ArduinoOTA.h>                     // Over-the-Air updates
#include <Arduino.h>                        // Not sure what this is for
#include <Base64.h>                         // Base64 Generate Tokens
#include <map>                              // Token management
// Import User Assets
#include "data/html/dc_index.h"             // Public: Homepage HTML
#include "data/html/dc_stats.h"             // Public: Server Stats HTML
#include "data/html/dc_settings.h"          // Private: Settings HTML

std::map<String, String> currentTokens; // Map to store tokens for each location

String generateToken() {
  uint8_t randomBytes[16];
  for (int i = 0; i < 16; i++) {
    randomBytes[i] = random(0, 256); // Generate random bytes
  }
  return base64::encode((const uint8_t *)randomBytes, sizeof(randomBytes));
}

String generateTokenForLoc(const String& loc) {
  String token = generateUrlSafeToken(); // Generate a new random token
  currentTokens[loc] = token;    // Assign the token to the location
  return token;
}

String generateUrlSafeToken() {
  String token = generateToken();
  token.replace("+", "");
  token.replace("/", "");
  token.replace("=", "");
  return token;
}

bool validateToken(const String& loc, const String& token) {
  if (currentTokens.count(loc) == 0) {
    Serial.println("No token found for location: " + loc);
    return false;
  }
  if (currentTokens[loc] != token) {
    Serial.println("Token mismatch for location: " + loc);
    Serial.println("Expected: " + currentTokens[loc]);
    Serial.println("Received: " + token);
    return false;
  }
  return true;
}

String formatUptime(unsigned long milliseconds) {
  unsigned long totalSeconds = milliseconds / 1000;
  unsigned long days = totalSeconds / 86400;
  totalSeconds %= 86400;
  unsigned long hours = totalSeconds / 3600;
  totalSeconds %= 3600;
  unsigned long minutes = totalSeconds / 60;
  unsigned long seconds = totalSeconds % 60;

  String formattedUptime = String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";
  return formattedUptime;
}

String classifyWiFiSignal(int rssi) {
  if (rssi >= -50) return "Excellent";
  if (rssi >= -60) return "Good";
  if (rssi >= -70) return "Fair";
  return "Poor";
}


// Replace with your network credentials
const char* ssid = "NETGEAR46";
const char* password = "kindsea989";

// DHT Sensor Configuration
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// OLED Configuration
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Web Server Configuration
AsyncWebServer server(80);

// Global variables for sensor data
float currentTemperature = NAN;
float currentHumidity = NAN;
unsigned long lastSensorUpdate = 0;
const unsigned long sensorUpdateInterval = 5000; // 5 seconds

//* Main Setup Function *//
void setup() {

  // Serial Debugging
  Serial.begin(115200);

  /*
  // Setup Over-the-Air Updates
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate End");
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  */

  // Initialize DHT Sensor
  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed!"));
    while (true);
  }
  display.clearDisplay();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected. IP address: " + WiFi.localIP().toString());

  //####### Web Server Routes #######//
  // Public: Homepage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String token = generateTokenForLoc("homepage"); // Generate token for homepage
    String html(index_html);

    // Replace placeholders with initial data
    html.replace("{{PAGE}}", "Home");
    //html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "DigiCo Web Server");
    //html.replace("{{JS_PLACEHOLDER}}", script_js);
    html.replace("{{GENERATED_TOKEN}}", token); // Embed the generated token
    html.replace("{{TEMPERATURE}}", String(currentTemperature, 1));
    html.replace("{{HUMIDITY}}", String(currentHumidity, 1));

    request->send(200, "text/html", html);
  });

  // Public: Server Stats Page
  server.on("/stats", HTTP_GET, [](AsyncWebServerRequest *request) {
    String token = generateTokenForLoc("stats"); // Generate token for stats page
    String html(stats_html);

    // Replace placeholders with initial stats
    html.replace("{{PAGE}}", "Stats");
    //html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "Server Stats");
    //html.replace("{{JS_PLACEHOLDER}}", script_js);
    html.replace("{{GENERATED_TOKEN}}", token); // Embed the generated token
    html.replace("{{UPTIME}}", String(millis() / 1000));
    html.replace("{{HEAP}}", String(ESP.getFreeHeap()));
    html.replace("{{WIFI}}", String(WiFi.RSSI()));

    request->send(200, "text/html", html);
  });

  // Private: Server Settings Page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("admin", "password")) {
      return request->requestAuthentication();
    }
    String token = generateTokenForLoc("settings");
    String html(stats_html);

    // Replace placeholders with initial stats
    html.replace("{{PAGE}}", "Settings");
    //html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "Server Settings");
    //html.replace("{{JS_PLACEHOLDER}}", script_js);
    html.replace("{{GENERATED_TOKEN}}", token); // Embed the generated token

    request->send(200, "text/html", html);
  });

  // API Endpoint for AJAX calls
  // Frontend Data Call
  server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("data_token") || !request->hasParam("loc")) {
      Serial.println("Missing token or location");
      request->send(403, "text/plain", "Missing token or location");
      return;
    }

    String receivedToken = request->getParam("data_token")->value();
    String loc = request->getParam("loc")->value();

    Serial.println("Received Token: " + receivedToken);
    Serial.println("Location: " + loc);

    if (!validateToken(loc, receivedToken)) {
      Serial.println("Invalid token for location: " + loc);
      request->send(403, "text/plain", "Invalid token");
      return;
    }

    // Generate and store a new token for the location
    String newToken = generateTokenForLoc(loc);

    // Prepare response
    String json = "{";
    if (loc == "homepage") {
      json += "\"temperature\":" + String(currentTemperature) + ",";
      json += "\"humidity\":" + String(currentHumidity) + ",";
    } else if (loc == "stats") {
      json += "\"uptime\":\"" + formatUptime(millis()) + "\",";
      json += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
      json += "\"wifi\":" + String(WiFi.RSSI()) + ",";
      json += "\"wifi_strength\":\"" + classifyWiFiSignal(WiFi.RSSI()) + "\",";

    }
    json += "\"new_token\":\"" + newToken + "\"";
    json += "}";

    Serial.println("Response JSON: " + json);
    request->send(200, "application/json", json);
  });


  // Protected: Backend Update AJAX Call
  /*
  server.on("/api/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    const String authToken = "my_secure_token";
    if (!request->hasParam("update_token") || request->getParam("update_token")->value() != authToken) {
      request->send(403, "text/plain", "Forbidden");
      return;
    }
    if (request->hasParam("wifi_ssid", true) && request->hasParam("wifi_password", true)) {
      String ssid = request->getParam("wifi_ssid", true)->value();
      String password = request->getParam("wifi_password", true)->value();
      Serial.println("New SSID: " + ssid);
      Serial.println("New Password: " + password);
      request->send(200, "text/html", "Settings updated!");
    } else {
      request->send(400, "text/html", "Missing parameters.");
    }
  });
  */


  // Start Web Server
  server.begin();

  // Setup custom domain for easier access (http://digicoesp32.local)
  if (!MDNS.begin("digicoesp32")) {
    Serial.println("Error setting up mDNS responder!");
  } else {
    Serial.println("mDNS responder started. Access via http://digicoesp32.local");
  }
}

void loop() {
  // Update sensor data and OLED every interval
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = currentMillis;

    // Read sensor data
    currentTemperature = dht.readTemperature(true);
    currentHumidity = dht.readHumidity();

    if (isnan(currentTemperature) || isnan(currentHumidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.printf("Temperature: %.1f°F\nHumidity: %.1f%%\n", currentTemperature, currentHumidity);

      // Update OLED display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0, 10);
      display.printf("Temp: %.1f°F\nHum: %.1f%%", currentTemperature, currentHumidity);
      display.display();
    }
  }
  //ArduinoOTA.handle();
}