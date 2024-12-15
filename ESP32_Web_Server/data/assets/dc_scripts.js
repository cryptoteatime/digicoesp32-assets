// Tokens stored for each location
const DCtokens = {}; // A map of tokens for each location
// Fetch data and update elements dynamically
function DCfetchData(url, token, loc, callback) {
  fetch(`${url}?data_token=${encodeURIComponent(token)}&loc=${encodeURIComponent(loc)}`)
    .then((response) => {
      if (!response.ok) {
        throw new Error("Invalid token or location");
      }
      return response.json();
    })
    .then((data) => {
      if (data.new_token) {
        console.log(`Token updated for ${loc}: ${data.new_token}`);
        callback(data, data.new_token); // Pass both the data and the new token
      }
    })
    .catch((error) => {
      console.error("Error fetching data:", error);
    });
}

// Auto-update logic
function DCautoUpdate(loc, initialToken, updateFunction, interval) {
  let currentToken = initialToken; // Store the initial token for the location
  let isRequestPending = false;   // Track if a request is in progress

  function makeRequest() {
    if (isRequestPending) return; // Skip if the previous request hasn't completed

    isRequestPending = true; // Mark request as in progress

    updateFunction(currentToken, (newToken) => {
      // Callback to handle token update
      currentToken = newToken; // Update the token with the new one
      isRequestPending = false; // Mark request as completed
    });
  }

  // Schedule periodic updates
  setInterval(makeRequest, interval);

  // Immediately make the first request
  makeRequest();
}