const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>DigiCo ESP32 - {{PAGE}}</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <!-- <style>{{CSS_PLACEHOLDER}}</style> -->
  <!-- External Stylesheet -->
  <link rel="stylesheet" href="https://raw.githubusercontent.com/cryptoteatime/digicoesp32-assets/main/ESP32_Web_Server/data/assets/dc_styles.css">
  <!-- Favicon -->
  <link rel="icon" type="image/x-icon" href="https://raw.githubusercontent.com/cryptoteatime/digicoesp32-assets/main/ESP32_Web_Server/data/assets/images/favicon.ico">
  <script src="https://raw.githubusercontent.com/cryptoteatime/digicoesp32-assets/main/ESP32_Web_Server/data/assets/dc_scripts.js" defer></script>
</head>
<body class="settings-page">
  <h1 class="dc-page-title">{{PAGE_HEADER}}</h1>
  <nav class="dc-header-nav">
    <a href="/">Home</a> |
    <a href="/stats">Stats</a> |
    <a href="/settings">Settings</a>
  </nav>
  <h2>General Settings</h2>
  <h3>WiFi Settings</h3>
  <form action="/update" method="POST">
    <label for="dc-wifi-ssid">Wi-Fi SSID:</label><br>
    <input type="text" id="dc-wifi-ssid" name="wifi_ssid"><br>
    <label for="dc-wifi-password">Wi-Fi Password:</label><br>
    <input type="text" id="dc-wifi-password" name="wifi_password"><br>
    <input type="hidden" name="update_token" id="dc-update-token" value="[GENERATED_TOKEN]"><br>
    <input type="submit" value="Save">
  </form>
  <h3>Sensor Settings</h3>

  <!-- <script>{{JS_PLACEHOLDER}}</script> -->
  <script src="https://raw.githubusercontent.com/cryptoteatime/digicoesp32-assets/main/ESP32_Web_Server/data/assets/dc_scripts.js"></script>
  <script>
    // Start auto-updates for homepage
    // autoUpdate('settings', '{{GENERATED_TOKEN}}', updateSettings, 5000);
  </script>
</body>
</html>
)rawliteral";