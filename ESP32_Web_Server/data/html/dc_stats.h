const char stats_html[] PROGMEM = R"rawliteral(
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
<body class="stats-page">
  <h1 class="dc-page-title">{{PAGE_HEADER}}</h1>
  <nav class="dc-header-nav">
    <a href="/">Home</a> |
    <a href="/stats">Stats</a> |
    <a href="/settings">Settings</a>
  </nav>
  <p>
    <b>Uptime:</b> <span id="dc-stats-uptime">{{UPTIME}}</span>
  </p>
  <p>
    <b>Heap Free:</b> <span id="dc-stats-heap">{{HEAP}}</span>
    <sup class="dc-units">bytes</sup>
  </p>
  <p>
    <b>WiFi Strength:</b> 
    <span id="dc-stats-wifi">{{WIFI}}</span> (<span id="dc-stats-wifi-quality">{{WIFI_QUALITY}}</span>)
  </p>
  <script>{{JS_PLACEHOLDER}}</script>
  <script>
    DCautoUpdate("stats", "{{GENERATED_TOKEN}}", (token, callback) => {
      DCfetchData("/api/data", token, "stats", (data, newToken) => {
        callback(newToken); // Update the token

        // Update page elements dynamically
        document.getElementById("dc-stats-uptime").innerText = data.uptime;
        document.getElementById("dc-stats-heap").innerText = data.heap;
        document.getElementById("dc-stats-wifi").innerText = data.wifi;
        document.getElementById("dc-stats-wifi-quality").innerText = data.wifi_strength;
      });
    }, 5000);
  </script>
</body>
</html>
)rawliteral";
