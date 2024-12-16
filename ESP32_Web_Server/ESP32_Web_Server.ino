// Import required libraries
#include "WiFi.h"                           // Enables Wi-Fi
#include "ESPAsyncWebServer.h"              // Web server
#include <Adafruit_Sensor.h>                // DHT sensor
#include <DHT.h>                            // DHT sensor library
#include <Adafruit_GFX.h>                   // OLED graphics
#include <Adafruit_SSD1306.h>               // OLED library
#include <Wire.h>                           // I2C communication
#include <ESPmDNS.h>                        // mDNS
#include <Arduino.h>                        // Not sure what this is for
#include <Base64.h>                         // Base64 Generate Tokens
#include <map>                              // Token management
#include <Preferences.h>

// Map to store tokens for each location
std::map<String, String> currentTokens; 
// Preferences to save WIFI creds and settings. 
Preferences preferences;
// Web Server Config
AsyncWebServer server(80);
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

// Default Access Point credentials
const char* apSSID = "ESP32_Config";
const char* apPassword = "12345678";

// Global variables for sensor data
float currentTemperature = NAN;
float currentHumidity = NAN;
unsigned long lastSensorUpdate = 0;
const unsigned long sensorUpdateInterval = 5000; // 5 seconds

// Import User Assets
#include "data/assets/images/favicon.h"
#include "data/html/dc_index.h"             // Public: Homepage HTML
#include "data/html/dc_stats.h"             // Public: Server Stats HTML
#include "data/html/dc_settings.h"          // Private: Settings HTML
#include "data/assets/dc_styles.h"
#include "data/assets/dc_scripts.h"


/* Functions */
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

void saveWiFiCredentials(const String& ssid, const String& password) {
  preferences.begin("WiFiCreds", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
}

String loadWiFiSSID() {
  preferences.begin("WiFiCreds", true);
  String ssid = preferences.getString("ssid", "");
  preferences.end();
  return ssid;
}

String loadWiFiPassword() {
  preferences.begin("WiFiCreds", true);
  String password = preferences.getString("password", "");
  preferences.end();
  return password;
}

void updateOLED(float temperature, float humidity) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.printf("Temp: %.1f°F\nHum: %.1f%%", temperature, humidity);
  display.display();
}

void readAndUpdateSensorData() {
  currentTemperature = dht.readTemperature(true);
  currentHumidity = dht.readHumidity();

  if (isnan(currentTemperature) || isnan(currentHumidity)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.printf("Temperature: %.1f°F\nHumidity: %.1f%%\n", currentTemperature, currentHumidity);
    updateOLED(currentTemperature, currentHumidity);
  }
}

bool connectToWiFi(const char* ssid, const char* password) {
  Serial.printf("Attempting to connect to Wi-Fi: %s\n", ssid);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(1000);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to Wi-Fi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
    return false;
  }
}

void startAccessPoint() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);

  Serial.println("Access Point started");
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}


//* Main Setup Function *//
void setup() {
  Serial.begin(115200);

  // Initialize Preferences
  preferences.begin("WiFiCreds", false);

  // Initialize DHT Sensor
  dht.begin();

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED initialization failed!"));
    while (true);
  }
  display.clearDisplay();

  // Start Sensor Updates Immediately
  readAndUpdateSensorData();

  // Connect to Wi-Fi or Start AP Mode
  String savedSSID = loadWiFiSSID();
  String savedPassword = loadWiFiPassword();

  if (!savedSSID.isEmpty() && !savedPassword.isEmpty()) {
    if (connectToWiFi(savedSSID.c_str(), savedPassword.c_str())) {
      Serial.println("Connected to saved Wi-Fi credentials.");
    } else {
      Serial.println("Failed to connect to saved Wi-Fi. Starting Access Point...");
      startAccessPoint();
    }
  } else {
    Serial.println("No Wi-Fi credentials found. Starting Access Point...");
    startAccessPoint();
  }

  //####### Web Server Routes #######//
  // PUBLIC: Favicon
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "image/x-icon", favicon_ico, favicon_ico_len);
  });


  // Public: Homepage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String token = generateTokenForLoc("homepage"); // Generate token for homepage
    String html(index_html);

    // Replace placeholders with initial data
    html.replace("{{PAGE}}", "Home");
    html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "DigiCo Web Server");
    html.replace("{{JS_PLACEHOLDER}}", script_js);
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
    html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "Server Stats");
    html.replace("{{JS_PLACEHOLDER}}", script_js);
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
    String html(settings_html);

    // Replace placeholders with initial stats
    html.replace("{{PAGE}}", "Settings");
    html.replace("{{CSS_PLACEHOLDER}}", styles_css);
    html.replace("{{PAGE_HEADER}}", "Server Settings");
    html.replace("{{GENERATED_TOKEN}}", token); // Embed the generated token
    html.replace("{{CONNECTION_STATUS}}", WiFi.status() == WL_CONNECTED ? "Connected to Wi-Fi" : "Running in Access Point Mode");


    request->send(200, "text/html", html);
  });

  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest *request) {
      // Ensure required parameters exist
      if (!request->hasParam("ssid", true) || !request->hasParam("password", true) || !request->hasParam("token", true)) {
          request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing SSID, Password, or Token\"}");
          return;
      }

      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();
      String receivedToken = request->getParam("token", true)->value();

      // Validate token for 'settings' location
      if (!validateToken("settings", receivedToken)) {
          request->send(403, "application/json", "{\"status\":\"error\",\"message\":\"Invalid or expired token\"}");
          return;
      }

      // Save Wi-Fi credentials
      saveWiFiCredentials(ssid, password);

      // Respond with success and prepare for restart
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Credentials saved! Rebooting...\"}");
      delay(2000);
      ESP.restart();
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

  // Test Wifi Details AJAX Call
  server.on("/api/testwifi", HTTP_POST, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("ssid", true) || !request->hasParam("password", true)) {
          request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing SSID or Password\"}");
          return;
      }

      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();

      WiFi.begin(ssid.c_str(), password.c_str());
      unsigned long startAttemptTime = millis();

      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
          delay(500);  // Check connection for up to 15 seconds
      }

      if (WiFi.status() == WL_CONNECTED) {
          request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Wi-Fi Connected Successfully!\"}");
      } else {
          request->send(200, "application/json", "{\"status\":\"error\",\"message\":\"Failed to connect to Wi-Fi. Please try again.\"}");
      }
      WiFi.disconnect();  // Disconnect to ensure proper reboot handling later
  });



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
  // Periodically Update Sensor Data and OLED
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorUpdate >= sensorUpdateInterval) {
    lastSensorUpdate = currentMillis;
    readAndUpdateSensorData();
  }
}
