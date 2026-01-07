/**
 * @file WebPortalPages.h
 * @brief HTML page definitions for unified web portal
 * 
 * This file contains the HTML/CSS/JavaScript pages for the unified portal
 * with tabbed interface for both configuration and OTA updates.
 * Pages are embedded as raw string literals for serving via WebServer.
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2026-01-07
 */

#ifndef STAC_WEB_PORTAL_PAGES_H
#define STAC_WEB_PORTAL_PAGES_H


namespace Net {
namespace WebPortal {

    /**
     * @brief Common page header with CSS for tabs
     */
    const char PAGE_HEAD[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>STAC Portal</title>
  <style type="text/css">
  * {
    box-sizing: border-box;
    margin: 0;
    padding: 0;
  }
  body {
    font-family: Helvetica, Arial, sans-serif;
    text-align: center;
    background: linear-gradient(135deg, #CDD6FA 0%, #E9F6DC 100%);
    padding: 10px;
  }
  h1 {
    margin: 20px 0;
    color: #333;
  }
  h2 {
    margin: 15px 0;
    color: #555;
  }
  h3, h4 {
    margin: 10px 0;
    color: #666;
  }
  .container {
    max-width: 600px;
    margin: 0 auto;
    background: white;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
    overflow: hidden;
  }
  .device-info {
    background: #f5f5f5;
    padding: 15px;
    border-bottom: 2px solid #ddd;
  }
  .device-info p {
    margin: 5px 0;
    font-size: 14px;
    color: #666;
  }
  .tabs {
    display: flex;
    background: #e0e0e0;
    border-bottom: 2px solid #ccc;
  }
  .tab {
    flex: 1;
    padding: 15px 10px;
    background: #e0e0e0;
    border: none;
    cursor: pointer;
    font-size: 16px;
    font-weight: bold;
    color: #555;
    transition: all 0.3s;
  }
  .tab:hover {
    background: #d0d0d0;
  }
  .tab.active {
    background: white;
    color: #333;
    border-bottom: 3px solid #4CAF50;
  }
  .tab-content {
    display: none;
    padding: 20px;
    animation: fadeIn 0.3s;
  }
  .tab-content.active {
    display: block;
  }
  @keyframes fadeIn {
    from { opacity: 0; }
    to { opacity: 1; }
  }
  form {
    max-width: 400px;
    margin: 0 auto;
  }
  label {
    display: block;
    text-align: left;
    margin: 15px 0 5px 0;
    font-weight: bold;
    color: #555;
  }
  input[type="text"],
  input[type="password"],
  input[type="number"],
  select {
    width: 100%;
    padding: 10px;
    font-size: 14px;
    border: 1px solid #ccc;
    border-radius: 4px;
  }
  input[type="file"] {
    width: 100%;
    padding: 10px;
    font-size: 14px;
    border: 1px solid #ccc;
    border-radius: 4px;
    background: white;
  }
  button, input[type="submit"], input[type="reset"] {
    padding: 12px 30px;
    font-size: 16px;
    margin: 20px 5px 10px 5px;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-weight: bold;
    transition: all 0.3s;
  }
  input[type="submit"] {
    background: #4CAF50;
    color: white;
  }
  input[type="submit"]:hover:not(:disabled) {
    background: #45a049;
  }
  input[type="submit"]:disabled {
    background: #ccc;
    cursor: not-allowed;
  }
  input[type="reset"] {
    background: #f44336;
    color: white;
  }
  input[type="reset"]:hover {
    background: #da190b;
  }
  .info-text {
    margin: 15px 0;
    padding: 10px;
    background: #f0f0f0;
    border-radius: 4px;
    font-size: 14px;
    line-height: 1.6;
  }
  .section {
    margin: 20px 0;
    padding: 15px;
    background: #fafafa;
    border-radius: 4px;
  }
  </style>
</head>
<body>
  <div class="container">
    <h1>STAC Portal</h1>
)=====";

    /**
     * @brief Opening tag for device info section
     * Content is dynamically inserted between DEVICE_INFO_OPEN and DEVICE_INFO_CLOSE
     */
    const char DEVICE_INFO_OPEN[] = R"=====(    <div class="device-info">
)=====";

    /**
     * @brief Closing tag for device info section
     */
    const char DEVICE_INFO_CLOSE[] = R"=====(    </div>
)=====";

    /**
     * @brief Tab buttons
     */
    const char TAB_BUTTONS[] = R"=====(    <div class="tabs">
      <button class="tab active" onclick="showTab('setup')" id="tab-setup">Setup</button>
      <button class="tab" onclick="showTab('update')" id="tab-update">Update</button>
    </div>
)=====";

    /**
     * @brief Setup tab content - model selection and configuration
     */
    const char TAB_SETUP[] = R"=====(    <div id="content-setup" class="tab-content active">
      <h2>Device Setup</h2>
      
      <form id="form-model" method="post" action="/config-step1">
        <label for="stModel">Select Roland Switcher Model:</label>
        <select name="stModel" id="stModel" required>
          <option value="" disabled selected>Choose model...</option>
          <option value="V-60HD">V-60HD</option>
          <option value="V-160HD">V-160HD</option>
        </select>
        <input type="submit" value="Next">
      </form>
      
      <!-- V-60HD Config Form (hidden initially) -->
      <form id="form-v60hd" method="post" action="/config" style="display:none;">
        <input type="hidden" name="stModel" value="V-60HD">
        
        <div class="section">
          <h3>WiFi Settings</h3>
          <label for="SSID">Network Name (SSID):</label>
          <input type="text" id="SSID" name="SSID" maxlength="32" required>
          
          <label for="pwd">Password:</label>
          <input type="password" id="pwd" name="pwd" maxlength="64" required>
        </div>
        
        <div class="section">
          <h3>V-60HD Settings</h3>
          <label for="stIP">V-60HD IP Address:</label>
          <input type="text" id="stIP" name="stIP" placeholder="192.168.1.100" required>
          
          <label for="stPort">Port:</label>
          <input type="number" id="stPort" name="stPort" value="8080" min="1" max="65535" required>
          
          <label for="stChan">Max HDMI Channel (1-8):</label>
          <input type="number" id="stChan" name="stChan" value="8" min="1" max="8" required>
          
          <label for="pollTime">Poll Interval (ms):</label>
          <input type="number" id="pollTime" name="pollTime" value="250" min="175" max="2000" required>
        </div>
        
        <input type="submit" value="Save Configuration">
        <input type="reset" value="Reset">
        <button type="button" onclick="showModelSelect()">Back</button>
      </form>
      
      <!-- V-160HD Config Form (hidden initially) -->
      <form id="form-v160hd" method="post" action="/config" style="display:none;">
        <input type="hidden" name="stModel" value="V-160HD">
        
        <div class="section">
          <h3>WiFi Settings</h3>
          <label for="SSID2">Network Name (SSID):</label>
          <input type="text" id="SSID2" name="SSID" maxlength="32" required>
          
          <label for="pwd2">Password:</label>
          <input type="password" id="pwd2" name="pwd" maxlength="64" required>
        </div>
        
        <div class="section">
          <h3>V-160HD Settings</h3>
          <label for="stIP2">V-160HD IP Address:</label>
          <input type="text" id="stIP2" name="stIP" placeholder="192.168.1.100" required>
          
          <label for="stPort2">Port:</label>
          <input type="number" id="stPort2" name="stPort" value="80" min="1" max="65535" required>
          
          <label for="stnetUser">LAN Username:</label>
          <input type="text" id="stnetUser" name="stnetUser" maxlength="32" required>
          
          <label for="stnetPW">LAN Password:</label>
          <input type="password" id="stnetPW" name="stnetPW" maxlength="32" required>
          
          <label for="stChanHDMI">Max HDMI Channel (1-8):</label>
          <input type="number" id="stChanHDMI" name="stChanHDMI" value="8" min="1" max="8" required>
          
          <label for="stChanSDI">Max SDI Channel (1-8):</label>
          <input type="number" id="stChanSDI" name="stChanSDI" value="8" min="1" max="8" required>
          
          <label for="pollTime2">Poll Interval (ms):</label>
          <input type="number" id="pollTime2" name="pollTime" value="250" min="175" max="2000" required>
        </div>
        
        <input type="submit" value="Save Configuration">
        <input type="reset" value="Reset">
        <button type="button" onclick="showModelSelect()">Back</button>
      </form>
    </div>
)=====";

    /**
     * @brief Update tab content - OTA firmware upload
     */
    const char TAB_UPDATE[] = R"=====(    <div id="content-update" class="tab-content">
      <h2>Firmware Update</h2>
      
      <div class="info-text">
        <strong>Instructions:</strong><br>
        1. Select a .bin firmware file below<br>
        2. Click "Update Firmware"<br>
        3. Wait for upload and installation to complete<br>
        4. STAC will restart automatically<br>
        <br>
        <strong>Note:</strong> To cancel, press the STAC reset button now.
      </div>
      
      <form method="post" enctype="multipart/form-data" action="/update">
        <label for="update">Select Firmware File (.bin):</label>
        <input type="file" name="update" id="update" accept=".bin" required>
        <br>
        <input type="submit" value="Update Firmware" id="update-btn" disabled>
      </form>
    </div>
)=====";

    /**
     * @brief JavaScript for tab switching and form handling
     */
    const char PAGE_SCRIPT[] = R"=====(    <script>
    // Tab switching
    function showTab(tabName) {
      // Hide all tab content
      document.querySelectorAll('.tab-content').forEach(el => {
        el.classList.remove('active');
      });
      // Deactivate all tabs
      document.querySelectorAll('.tab').forEach(el => {
        el.classList.remove('active');
      });
      // Show selected tab content
      document.getElementById('content-' + tabName).classList.add('active');
      // Activate selected tab
      document.getElementById('tab-' + tabName).classList.add('active');
    }
    
    // Handle model selection in Setup tab
    document.getElementById('form-model').addEventListener('submit', function(e) {
      e.preventDefault();
      const model = document.getElementById('stModel').value;
      
      // Hide model selection form
      document.getElementById('form-model').style.display = 'none';
      
      // Show appropriate config form
      if (model === 'V-60HD') {
        document.getElementById('form-v60hd').style.display = 'block';
      } else if (model === 'V-160HD') {
        document.getElementById('form-v160hd').style.display = 'block';
      }
    });
    
    // Back button - return to model selection
    function showModelSelect() {
      document.getElementById('form-v60hd').style.display = 'none';
      document.getElementById('form-v160hd').style.display = 'none';
      document.getElementById('form-model').style.display = 'block';
    }
    
    // Enable update button when file is selected
    document.getElementById('update').addEventListener('change', function(e) {
      document.getElementById('update-btn').disabled = !e.target.value;
    });
    </script>
)=====";

    /**
     * @brief Page footer
     */
    const char PAGE_FOOTER[] = R"=====(  </div>
</body>
</html>
)=====";

    /**
     * @brief Configuration received confirmation page
     */
    const char CONFIG_RECEIVED[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Configuration Saved</title>
  <style type="text/css">
  body {
    font-family: Helvetica, Arial, sans-serif;
    text-align: center;
    background: #CDD6FA;
    padding: 20px;
  }
  .container {
    max-width: 500px;
    margin: 50px auto;
    background: white;
    padding: 30px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
  }
  h1 {
    color: #4CAF50;
  }
  p {
    font-size: 16px;
    line-height: 1.6;
    color: #555;
  }
  </style>
</head>
<body>
  <div class="container">
    <h1>✓ Configuration Saved</h1>
    <p>Your STAC configuration has been saved successfully.</p>
    <p>The device will now restart and connect to your WiFi network.</p>
    <p><strong>Please wait...</strong></p>
  </div>
</body>
</html>
)=====";

    /**
     * @brief OTA update result page - opening
     */
    const char OTA_PAGE_OPEN[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Update Result</title>
  <style type="text/css">
  body {
    font-family: Helvetica, Arial, sans-serif;
    text-align: center;
    background: #E9F6DC;
    padding: 20px;
  }
  .container {
    max-width: 500px;
    margin: 50px auto;
    background: white;
    padding: 30px;
    border-radius: 8px;
    box-shadow: 0 2px 10px rgba(0,0,0,0.1);
  }
  h1 {
    margin-bottom: 20px;
  }
  .success { color: #4CAF50; }
  .error { color: #f44336; }
  p {
    font-size: 16px;
    line-height: 1.6;
    color: #555;
  }
  .filename {
    font-family: monospace;
    background: #f0f0f0;
    padding: 5px 10px;
    border-radius: 3px;
  }
  </style>
</head>
<body>
  <div class="container">
)=====";

    /**
     * @brief OTA update result page - closing
     */
    const char OTA_PAGE_CLOSE[] = R"=====(  </div>
</body>
</html>
)=====";

    /**
     * @brief 404 Not Found page
     */
    const char NOT_FOUND[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Page Not Found</title>
  <style type="text/css">
  body {
    font-family: Helvetica, Arial, sans-serif;
    text-align: center;
    background: #f0f0f0;
    padding: 20px;
  }
  h1 { color: #f44336; }
  </style>
</head>
<body>
  <h1>404 - Page Not Found</h1>
  <p><a href="/">Return to STAC Portal</a></p>
</body>
</html>
)=====";

} // namespace WebPortal
} // namespace Net

#endif // STAC_WEB_PORTAL_PAGES_H


//  --- EOF --- //
