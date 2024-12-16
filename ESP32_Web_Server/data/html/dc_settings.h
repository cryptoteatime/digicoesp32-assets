const char settings_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
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
<body class="settings-page">
  <h1 class="dc-page-title">{{PAGE_HEADER}}</h1>
  <nav class="dc-header-nav">
    <a href="/">Home</a> |
    <a href="/stats">Stats</a> |
    <a href="/settings">Settings</a>
  </nav>
  <h2>General Settings</h2>
  <h3>WiFi Settings</h3>
  <form id="wifi-form" onsubmit="event.preventDefault(); testWiFi();">
    <label for="ssid">Wi-Fi SSID:</label>
    <input type="text" id="ssid" name="ssid" required><br>
    <label for="password">Password:</label>
    <input type="password" id="password" name="password" required><br>
    <button type="submit">Test & Save</button>
  </form>

  <div id="status-message"></div>

  <script>
    async function saveWiFiCredentials() {
      const ssid = document.getElementById("ssid").value;
      const password = document.getElementById("password").value;

      const token = "{{GENERATED_TOKEN}}"; // Use the generated token for validation

      document.getElementById("status-message").innerText = "Saving Wi-Fi credentials...";

      try {
        const response = await fetch("/settings", {
          method: "POST",
          headers: { "Content-Type": "application/x-www-form-urlencoded" },
          body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}&token=${encodeURIComponent(token)}`,
        });

        const data = await response.json();
        if (data.status === "success") {
          document.getElementById("status-message").innerText = data.message;
          setTimeout(() => window.location.reload(), 2000); // Reload after reboot
        } else {
          document.getElementById("status-message").innerText = data.message;
        }
      } catch (error) {
        console.error("Error saving Wi-Fi credentials:", error);
        document.getElementById("status-message").innerText = "An error occurred. Please try again.";
      }
    }

  </script>
  <script>
    // Start auto-updates for homepage
    // autoUpdate('settings', '{{GENERATED_TOKEN}}', updateSettings, 5000);
  </script>
</body>
</html>
)rawliteral";