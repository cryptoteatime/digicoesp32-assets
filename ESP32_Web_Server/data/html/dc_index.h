const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>DigiCo ESP32 - {{PAGE}}</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <style>{{CSS_PLACEHOLDER}}</style> 
  <!-- External Stylesheet -->
  <!-- <link rel="stylesheet" href="https://cryptoteatime.github.io/digicoesp32-assets/ESP32_Web_Server/data/assets/dc_styles.css"> -->
  <!-- Favicon -->
  <!-- <link rel="icon" type="image/x-icon" href="https://cryptoteatime.github.io/digicoesp32-assets/ESP32_Web_Server/data/assets/images/favicon.ico"> -->
  <!-- <script src="https://cryptoteatime.github.io/digicoesp32-assets/ESP32_Web_Server/data/assets/dc_scripts.js"></script> -->
</head>
<body class="index-page">
  <h1 class="dc-page-title">{{PAGE_HEADER}}</h1>
  <nav class="dc-header-nav">
    <a href="/">Home</a> |
    <a href="/stats">Stats</a> |
    <a href="/settings">Settings</a>
  </nav>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dc-dht-labels">Temperature:</span> 
    <span id="dc-temperature">{{TEMPERATURE}}</span>
    <sup class="dc-units">&deg;F</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dc-dht-labels">Humidity:</span>
    <span id="dc-humidity">{{HUMIDITY}}</span>
    <sup class="dc-units">&percnt;</sup>
  </p>
  <script>{{JS_PLACEHOLDER}}</script>
  <script>
    DCautoUpdate("homepage", "{{GENERATED_TOKEN}}", (token, callback) => {
      DCfetchData("/api/data", token, "homepage", (data, newToken) => {
        callback(newToken); // Update the token

        // Update page elements dynamically
        document.getElementById("dc-temperature").innerText = data.temperature;
        document.getElementById("dc-humidity").innerText = data.humidity;
      });
    }, 5000);
  </script>
</body>
</html>
)rawliteral";