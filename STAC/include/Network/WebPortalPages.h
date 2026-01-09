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
    cursor: pointer;
    display: block;
    -webkit-appearance: none;
    -moz-appearance: none;
    appearance: none;
  }
  input[type="file"]::-webkit-file-upload-button {
    padding: 8px 16px;
    background: #2196F3;
    color: white;
    border: none;
    border-radius: 4px;
    cursor: pointer;
    font-size: 14px;
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
  .landing {
    display: flex;
    flex-direction: column;
    align-items: center;
    justify-content: center;
    padding: 60px 20px;
    min-height: 300px;
  }
  .landing h2 {
    color: #4CAF50;
    margin-bottom: 20px;
  }
  .landing p {
    color: #666;
    margin: 10px 0;
    font-size: 16px;
  }
  .landing .btn-continue {
    margin-top: 30px;
    padding: 15px 40px;
    background: #4CAF50;
    color: white;
    border: none;
    border-radius: 4px;
    font-size: 18px;
    font-weight: bold;
    cursor: pointer;
    transition: all 0.3s;
    text-decoration: none;
    display: inline-block;
  }
  .landing .btn-continue:hover {
    background: #45a049;
  }
  .spinner {
    border: 4px solid #f3f3f3;
    border-top: 4px solid #4CAF50;
    border-radius: 50%;
    width: 40px;
    height: 40px;
    animation: spin 1s linear infinite;
    margin: 20px auto;
  }
  @keyframes spin {
    0% { transform: rotate(0deg); }
    100% { transform: rotate(360deg); }
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
     * @brief Landing page with auto-redirect to full browser
     */
    const char LANDING_PAGE[] = R"=====(    <div id="landing" class="landing">
      <h2>Welcome to STAC Setup</h2>
      <div class="spinner"></div>
      <p id="landing-message">Opening in browser...</p>
      <a href="http://192.168.6.14?open=1" target="_blank" class="btn-continue" id="btn-continue" style="display:none;">
        Continue to Setup
      </a>
    </div>
)=====";

    /**
     * @brief Tab buttons
     */
    const char TAB_BUTTONS[] = R"=====(    <div class="tabs" id="tabs" style="display:none;">
      <button class="tab active" onclick="showTab('setup')" id="tab-setup">Setup</button>
      <button class="tab" onclick="showTab('maintenance')" id="tab-maintenance">Maintenance</button>
    </div>
    <div id="portal-notice" class="info-text" style="display:none; margin: 15px 20px; background: #fff3cd; border: 1px solid #ffc107;">
      <strong>Note:</strong> For firmware updates, please:<br>
      1. Stay connected to STAC WiFi<br>
      2. Open a full browser (<strong>Safari</strong>, <strong>Chrome</strong>, or <strong>Firefox</strong>)<br>
      3. Go to: <strong>http://stac.local</strong> or <strong>http://192.168.6.14</strong>
    </div>
)=====";

    /**
     * @brief Setup tab content - model selection and configuration
     */
    const char TAB_SETUP[] = R"=====(    <div id="content-setup" class="tab-content active" style="display:none;">
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
          <input type="password" id="pwd" name="pwd" maxlength="63">
        </div>
        
        <div class="section">
          <h3>V-60HD Settings</h3>
          <label for="stIP">V-60HD IP Address:</label>
          <input type="text" id="stIP" name="stIP" placeholder="192.168.1.100" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" inputmode="decimal" required>
          
          <label for="stPort">Port:</label>
          <input type="number" id="stPort" name="stPort" value="80" min="1" max="65535" inputmode="numeric" pattern="[0-9]*" required>
          
          <label for="stChan">Max HDMI Channel (1-8):</label>
          <input type="number" id="stChan" name="stChan" value="6" min="1" max="8" inputmode="numeric" pattern="[0-9]*" required>
          
          <label for="pollTime">Poll Interval (ms):</label>
          <input type="number" id="pollTime" name="pollTime" value="300" min="175" max="2000" inputmode="numeric" pattern="[0-9]*" required>
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
          <input type="password" id="pwd2" name="pwd" maxlength="63">
        </div>
        
        <div class="section">
          <h3>V-160HD Settings</h3>
          <label for="stIP2">V-160HD IP Address:</label>
          <input type="text" id="stIP2" name="stIP" placeholder="192.168.1.100" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" inputmode="decimal" required>
          
          <label for="stPort2">Port:</label>
          <input type="number" id="stPort2" name="stPort" value="80" min="1" max="65535" inputmode="numeric" pattern="[0-9]*" required>
          
          <label for="stnetUser">LAN Username:</label>
          <input type="text" id="stnetUser" name="stnetUser" value="admin" maxlength="32" required>
          
          <label for="stnetPW">LAN Password:</label>
          <input type="password" id="stnetPW" name="stnetPW" value="admin" maxlength="32" required>
          
          <label for="stChanHDMI">Max HDMI Channel (1-8):</label>
          <input type="number" id="stChanHDMI" name="stChanHDMI" value="8" min="1" max="8" inputmode="numeric" pattern="[0-9]*" required>
          
          <label for="stChanSDI">Max SDI Channel (1-8):</label>
          <input type="number" id="stChanSDI" name="stChanSDI" value="8" min="1" max="8" inputmode="numeric" pattern="[0-9]*" required>
          
          <label for="pollTime2">Poll Interval (ms):</label>
          <input type="number" id="pollTime2" name="pollTime" value="300" min="175" max="2000" inputmode="numeric" pattern="[0-9]*" required>
        </div>
        
        <input type="submit" value="Save Configuration">
        <input type="reset" value="Reset">
        <button type="button" onclick="showModelSelect()">Back</button>
      </form>
    </div>
)=====";

    /**
     * @brief Maintenance tab content - OTA firmware upload and factory reset
     */
    const char TAB_MAINTENANCE[] = R"=====(    <div id="content-maintenance" class="tab-content" style="display:none;">
      <h2>Maintenance</h2>
      
      <!-- Firmware Update Section -->
      <div class="section">
        <h3>Firmware Update</h3>
        <div class="info-text">
          <strong>Instructions:</strong><br>
          1. Select a .bin firmware file below<br>
          2. Click "Update Firmware"<br>
          3. Wait for upload and installation to complete<br>
          4. STAC will restart automatically<br>
          <br>
          <strong>Note:</strong> To cancel, press the STAC reset button.
        </div>
        
        <form method="post" enctype="multipart/form-data" action="/update" id="upload-form">
          <label for="update">Select Firmware File (.bin):</label>
          <input type="file" name="update" id="update" accept=".bin" required>
          <br>
          <input type="submit" value="Update Firmware" id="upload-btn">
          <div id="upload-progress" style="display:none; margin-top: 15px;">
            <div style="background: #f0f0f0; border-radius: 10px; height: 24px; overflow: hidden;">
              <div id="progress-bar" style="background: #4CAF50; height: 100%; width: 0%; transition: width 0.3s;"></div>
            </div>
            <p id="progress-text" style="margin-top: 10px; font-weight: bold;">Uploading: 0%</p>
          </div>
        </form>
      </div>
      
      <!-- Factory Reset Section -->
      <div class="section" style="margin-top: 30px;">
        <h3 style="color: #f44336;">Factory Reset</h3>
        <div class="info-text" style="background: #ffebee; border-left: 4px solid #f44336;">
          <strong style="color: #f44336;">&#9888; Warning:</strong><br>
          Factory Reset will erase all settings including:<br>
          • WiFi credentials<br>
          • Roland switcher configuration<br>
          • Display brightness settings<br>
          • Channel assignments<br>
          <br>
          <strong>This action cannot be undone!</strong><br>
          The STAC will restart and require re-configuration.
        </div>
        
        <form id="factory-reset-form" method="post" action="/factory-reset">
          <input type="submit" id="reset-btn" value="Perform Factory Reset" style="background: #f44336; margin-top: 15px; padding: 12px 30px; font-size: 16px; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; color: white;">
        </form>
      </div>
    </div>
)=====";

    /**
     * @brief JavaScript for tab switching and form handling
     */
    const char PAGE_SCRIPT[] = R"=====(    <script>
    // Auto-redirect from captive portal to full browser
    function openInBrowser() {
      window.location.href = 'http://192.168.6.14?open=1';
    }
    
    // Tab switching
    function showTab(tabName) {
      // Hide all tab content - remove inline styles and active class
      document.querySelectorAll('.tab-content').forEach(el => {
        el.classList.remove('active');
        el.style.display = 'none';
      });
      // Deactivate all tabs
      document.querySelectorAll('.tab').forEach(el => {
        el.classList.remove('active');
      });
      // Show selected tab content
      const selectedContent = document.getElementById('content-' + tabName);
      selectedContent.style.display = 'block';
      selectedContent.classList.add('active');
      // Activate selected tab
      document.getElementById('tab-' + tabName).classList.add('active');
    }
    
    // Check if we should auto-redirect (detect captive portal environment)
    function checkCaptivePortal() {
      // Check if we've already been redirected (look for ?open=1 parameter)
      const urlParams = new URLSearchParams(window.location.search);
      const alreadyOpened = urlParams.get('open') === '1';
      
      // Detect captive portal environments
      const isIOSCaptivePortal = /CaptiveNetworkSupport/i.test(navigator.userAgent) && /iPhone|iPad|iPod/i.test(navigator.userAgent);
      const isMacOSCaptivePortal = /CaptiveNetworkSupport/i.test(navigator.userAgent) && /Mac OS X|Macintosh/i.test(navigator.userAgent);
      const isAndroid = /Android/i.test(navigator.userAgent);
      const hasCaptiveNetworkSupport = /CaptiveNetworkSupport/i.test(navigator.userAgent);
      
      // Check if we're in a full browser (has Chrome, Firefox, Safari in UA) vs captive portal WebView
      const isFullBrowser = /Chrome|Firefox|Safari|Edge|OPR/i.test(navigator.userAgent) && 
                           !/wv|WebView/i.test(navigator.userAgent) &&
                           !hasCaptiveNetworkSupport;
      
      // Show both tabs if: redirected, full browser, or iOS captive portal (file uploads work on iOS)
      if (alreadyOpened || isFullBrowser || isIOSCaptivePortal) {
        document.getElementById('landing').style.display = 'none';
        document.getElementById('tabs').style.display = 'flex';
        document.getElementById('tab-maintenance').style.display = 'inline-block';
        showTab('setup');
        return;
      }
      
      // macOS captive portal, Android captive portal, or any other restricted environment
      // Show Setup only, hide Maintenance (file inputs may not work)
      document.getElementById('landing').style.display = 'none';
      document.getElementById('tabs').style.display = 'flex';
      document.getElementById('tab-maintenance').style.display = 'none';
      document.getElementById('portal-notice').style.display = 'block';
      showTab('setup');
    }
    
    // Initialize page after DOM is loaded
    function initializePage() {
      checkCaptivePortal();
      
      // Handle firmware upload with progress
      const uploadForm = document.getElementById('upload-form');
      if (uploadForm) {
        uploadForm.addEventListener('submit', function(e) {
          e.preventDefault();
          
          const fileInput = document.getElementById('update');
          const file = fileInput.files[0];
          
          if (!file) {
            alert('Please select a firmware file');
            return;
          }
          
          // Show progress, hide submit button
          document.getElementById('upload-btn').disabled = true;
          document.getElementById('upload-progress').style.display = 'block';
          
          const xhr = new XMLHttpRequest();
          
          // Track upload progress
          xhr.upload.addEventListener('progress', function(e) {
            if (e.lengthComputable) {
              const percent = Math.round((e.loaded / e.total) * 100);
              document.getElementById('progress-bar').style.width = percent + '%';
              document.getElementById('progress-text').textContent = 'Uploading: ' + percent + '%';
            }
          });
          
          // Handle completion
          xhr.addEventListener('load', function() {
            if (xhr.status === 200) {
              document.getElementById('progress-text').textContent = 'Upload complete! Processing...';
              // The response is the result page - display it
              document.open();
              document.write(xhr.responseText);
              document.close();
            } else {
              document.getElementById('progress-text').textContent = 'Upload failed: ' + xhr.status;
              document.getElementById('upload-btn').disabled = false;
            }
          });
          
          // Handle errors
          xhr.addEventListener('error', function() {
            document.getElementById('progress-text').textContent = 'Upload failed - connection error';
            document.getElementById('upload-btn').disabled = false;
          });
          
          // Send the file
          const formData = new FormData();
          formData.append('update', file);
          xhr.open('POST', '/update');
          xhr.send(formData);
        });
      }
      
      // Handle model selection in Setup tab
      const formModel = document.getElementById('form-model');
      if (formModel) {
        formModel.addEventListener('submit', function(e) {
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
      }
    }
    
    // Back button - return to model selection
    function showModelSelect() {
      document.getElementById('form-v60hd').style.display = 'none';
      document.getElementById('form-v160hd').style.display = 'none';
      document.getElementById('form-model').style.display = 'block';
    }
    
    // Run on page load
    window.addEventListener('load', initializePage);
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
    <h1>Configuration Saved</h1>
    <p>Your STAC configuration has been saved successfully.</p>
    <p>The device will now restart and connect to your WiFi network.</p>
    <p><strong>Please wait...</strong></p>
  </div>
</body>
</html>
)=====";

    /**
     * @brief Factory reset confirmation page
     */
    const char FACTORY_RESET_RECEIVED[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Factory Reset</title>
  <style type="text/css">
  body {
    font-family: Helvetica, Arial, sans-serif;
    text-align: center;
    background: #ffebee;
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
    color: #f44336;
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
    <h1>Factory Reset Complete</h1>
    <p>All configuration data has been erased.</p>
    <p>The STAC will now restart with factory default settings.</p>
    <p><strong>Please wait for restart...</strong></p>
    <p style="margin-top: 20px; font-size: 14px; color: #999;">You will need to re-configure the device after restart.</p>
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
