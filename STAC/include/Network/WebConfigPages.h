/**
 * @file WebConfigPages.h
 * @brief HTML page definitions for web configuration interface
 * 
 * This file contains the HTML/CSS/JavaScript pages for the web-based
 * configuration interface with tabbed layout for setup and maintenance.
 * Pages are embedded as raw string literals for serving via WebServer.
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2026-01-07
 */

#ifndef STAC_WEB_CONFIG_PAGES_H
#define STAC_WEB_CONFIG_PAGES_H


namespace Net {
namespace WebConfig {

    /**
     * @brief Common page header with CSS for tabs
     */
    const char PAGE_HEAD[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>STAC Setup</title>
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
  .modal {
    display: none;
    position: fixed;
    z-index: 1000;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0,0,0,0.4);
  }
  .modal-content {
    background-color: white;
    margin: 15% auto;
    padding: 20px;
    border-radius: 8px;
    width: 80%;
    max-width: 500px;
    box-shadow: 0 4px 6px rgba(0,0,0,0.1);
  }
  .modal-text {
    margin: 20px 0;
    padding: 15px;
    background: #f5f5f5;
    border-radius: 4px;
    font-family: monospace;
    font-size: 14px;
    line-height: 1.6;
    white-space: pre-wrap;
  }
  .modal-buttons {
    display: flex;
    gap: 10px;
    margin-top: 20px;
  }
  </style>
</head>
<body>
  <div class="container">
    <h1>STAC Setup</h1>
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
     * @brief Landing page with browser escape options
     */
    const char LANDING_PAGE[] = R"=====(    <div id="landing" class="landing">
      <h2>Welcome to STAC Setup</h2>
      <p style="margin: 20px 0; font-size: 16px; color: #555;">
        Configure your STAC or update firmware using the tabs below.
      </p>
      
      <!-- Access info -->
      <div style="background: #e3f2fd; padding: 15px; border-radius: 8px; margin: 20px 0; text-align: left;">
        <h3 style="margin-top: 0; color: #1976d2;">📡 Access Information</h3>
        <p style="margin: 5px 0; color: #555;">
          <strong>Hostname:</strong> stac.local<br>
          <strong>IP Address:</strong> 192.168.6.14
        </p>
      </div>
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
      
      <!-- Quick Setup Section -->
      <div class="section" style="background: #e8f5e9; padding: 15px; margin-bottom: 20px;">
        <h3>&#9889; Quick Setup</h3>
        <p style="margin: 10px 0;">Load configuration from a file or you can paste the settings from the clipboard.</p>
        
        <!-- Import Methods -->
        <div id="import-section">
          <label for="config-file">Use settings file:</label>
          <input type="file" id="config-file" accept=".json,.txt" onchange="loadFromFile()">
          
          <div style="margin-top: 10px;">
            <button type="button" onclick="loadFromClipboard()" style="background: #4caf50; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer;">
              &#128203; Paste Settings
            </button>
          </div>
        </div>
      </div>
      
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
        
        <input type="submit" value="Configure STAC">
        <br>
        <input type="reset" value="Reset">
        <button type="button" onclick="showModelSelect()">Back</button>
        
        <!-- Export buttons below Save Configuration -->
        <div style="margin-top: 15px; padding-top: 15px; border-top: 1px solid #ddd;">
          <button type="button" onclick="copyExportJSON()" style="background: #4caf50; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer; margin-right: 8px;">
            &#128203; Copy Settings
          </button>
          <button type="button" onclick="downloadExportJSON()" style="background: #2196f3; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer;">
            &#128190; Save Settings
          </button>
          <small style="display:block; margin-top:8px; color:#666;">
            Copy to paste into another STAC, or Save as file for backup.
          </small>
        </div>
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
        
        <input type="submit" value="Configure STAC">
        <br>
        <input type="reset" value="Reset">
        <button type="button" onclick="showModelSelect()">Back</button>
        
        <!-- Export buttons below Save Configuration -->
        <div style="margin-top: 15px; padding-top: 15px; border-top: 1px solid #ddd;">
          <button type="button" onclick="copyExportJSON()" style="background: #4caf50; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer; margin-right: 8px;">
            &#128203; Copy Settings
          </button>
          <button type="button" onclick="downloadExportJSON()" style="background: #2196f3; color: white; padding: 8px 16px; border: none; border-radius: 4px; cursor: pointer;">
            &#128190; Save Settings
          </button>
          <small style="display:block; margin-top:8px; color:#666;">
            Copy to paste into another STAC, or Save as file for backup.
          </small>
        </div>
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
        
        <form id="factory-reset-form" method="post" action="/factory-reset" onsubmit="return confirmFactoryReset();">
          <input type="submit" id="reset-btn" value="Perform Factory Reset" style="background: #f44336; margin-top: 15px; padding: 12px 30px; font-size: 16px; border: none; border-radius: 4px; cursor: pointer; font-weight: bold; color: white;">
        </form>
      </div>
      
      <!-- Geek Info Button -->
      <div style="margin-top: 30px; text-align: center;">
        <button type="button" onclick="showGeekInfo()" style="background: #607d8b; color: white; padding: 8px 20px; border: none; border-radius: 4px; cursor: pointer; font-size: 14px;">
          Show Geek Info
        </button>
      </div>
    </div>
)=====";

    /**
     * @brief JavaScript for tab switching and form handling
     */
    const char PAGE_SCRIPT[] = R"=====(    <script>
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
    
    // Initialize page after DOM is loaded
    function initializePage() {
      // Show tabs immediately (no captive portal)
      document.getElementById('landing').style.display = 'none';
      document.getElementById('tabs').style.display = 'flex';
      document.getElementById('tab-maintenance').style.display = 'inline-block';
      showTab('setup');
      
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
            attachFormListeners('form-v60hd');
          } else if (model === 'V-160HD') {
            document.getElementById('form-v160hd').style.display = 'block';
            attachFormListeners('form-v160hd');
          }
          
          // Update export data in background (buttons are always visible)
          updateExportJSON();
        });
      }
    }
    
    // ===== Configuration Import/Export Functions =====
    
    // Attach input listeners to form fields to auto-update JSON
    function attachFormListeners(formId) {
      const form = document.getElementById(formId);
      if (!form) return;
      
      const inputs = form.querySelectorAll('input[type="text"], input[type="password"], input[type="number"]');
      inputs.forEach(function(input) {
        input.addEventListener('input', updateExportJSON);
        input.addEventListener('change', updateExportJSON);
      });
    }
    
    // Update export textarea with current form data
    function updateExportJSON() {
      const model = document.getElementById('stModel').value;
      if (!model) return;
      
      // Build config object from form
      const config = {
        model: model,
        wifi: {
          ssid: document.getElementById(model === 'V-60HD' ? 'SSID' : 'SSID2').value,
          password: document.getElementById(model === 'V-60HD' ? 'pwd' : 'pwd2').value
        },
        switch: {
          ip: document.getElementById(model === 'V-60HD' ? 'stIP' : 'stIP2').value,
          port: parseInt(document.getElementById(model === 'V-60HD' ? 'stPort' : 'stPort2').value),
          pollInterval: parseInt(document.getElementById(model === 'V-60HD' ? 'pollTime' : 'pollTime2').value)
        }
      };
      
      // Add model-specific fields
      if (model === 'V-60HD') {
        config.switch.maxChannel = parseInt(document.getElementById('stChan').value);
      } else {
        config.switch.lanUsername = document.getElementById('stnetUser').value;
        config.switch.lanPassword = document.getElementById('stnetPW').value;
        config.switch.maxHDMI = parseInt(document.getElementById('stChanHDMI').value);
        config.switch.maxSDI = parseInt(document.getElementById('stChanSDI').value);
      }
      
      // Store export data (no textarea anymore, used by copy/download buttons)
      window.currentConfigJSON = JSON.stringify(config, null, 2);
    }
    
    // Copy JSON to clipboard
    function copyExportJSON() {
      if (!window.currentConfigJSON) {
        alert('Please configure settings first');
        return;
      }
      
      // Create temporary textarea to copy from
      const textarea = document.createElement('textarea');
      textarea.value = window.currentConfigJSON;
      textarea.style.position = 'fixed';
      textarea.style.opacity = '0';
      document.body.appendChild(textarea);
      textarea.select();
      
      try {
        document.execCommand('copy');
        document.body.removeChild(textarea);
        alert('Settings copied to clipboard!');
      } catch (error) {
        document.body.removeChild(textarea);
        alert('Copy failed: ' + error.message);
      }
    }
    
    // Download JSON as file
    function downloadExportJSON() {
      if (!window.currentConfigJSON) {
        alert('Please configure settings first');
        return;
      }
      
      const model = document.getElementById('stModel').value;
      
      try {
        const blob = new Blob([window.currentConfigJSON], {type: 'application/json'});
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = 'stac-' + model.toLowerCase() + '-config.json';
        a.click();
        URL.revokeObjectURL(url);
      } catch (error) {
        alert('Save failed. Please use Copy Settings instead.');
      }
    }
    
    // Load from uploaded file
    function loadFromFile() {
      const fileInput = document.getElementById('config-file');
      const file = fileInput.files[0];
      if (!file) return;
      
      const reader = new FileReader();
      reader.onload = function(e) {
        try {
          const config = JSON.parse(e.target.result);
          applyConfig(config);
          fileInput.value = ''; // Clear file input
        } catch (error) {
          alert('Error reading file: ' + error.message);
        }
      };
      reader.readAsText(file);
    }
    
    // Load from clipboard with validation (using textarea approach for HTTP compatibility)
    function loadFromClipboard() {
      // Create temporary textarea for paste
      const pasteArea = document.createElement('textarea');
      pasteArea.style.position = 'fixed';
      pasteArea.style.opacity = '0';
      document.body.appendChild(pasteArea);
      pasteArea.focus();
      
      // Brief delay to ensure focus, then try paste
      setTimeout(function() {
        // Attempt to trigger paste event
        document.execCommand('paste');
        
        // Small delay to let paste complete
        setTimeout(function() {
          const jsonStr = pasteArea.value.trim();
          document.body.removeChild(pasteArea);
          
          if (!jsonStr) {
            alert('Clipboard is empty or paste failed.\n\nTip: After clicking Paste Settings, use Ctrl+V (or Cmd+V on Mac) to paste.');
            return;
          }
          
          try {
            const config = JSON.parse(jsonStr);
            
            // Validate structure
            if (!config.model || !config.wifi || !config.switch) {
              alert('Clipboard does not contain valid STAC configuration');
              return;
            }
            
            if (config.model !== 'V-60HD' && config.model !== 'V-160HD') {
              alert('Clipboard configuration is for unknown model: ' + config.model);
              return;
            }
            
            applyConfig(config);
          } catch (error) {
            alert('Clipboard does not contain valid JSON: ' + error.message);
          }
        }, 100);
      }, 50);
    }
    
    // Apply configuration to form
    function applyConfig(config) {
      // Validate
      if (!config.model || !config.wifi || !config.switch) {
        alert('Invalid configuration format');
        return;
      }
      
      if (config.model !== 'V-60HD' && config.model !== 'V-160HD') {
        alert('Unknown model: ' + config.model);
        return;
      }
      
      // Check if a different model form is already displayed
      const v60hdVisible = document.getElementById('form-v60hd').style.display === 'block';
      const v160hdVisible = document.getElementById('form-v160hd').style.display === 'block';
      
      if (v60hdVisible && config.model === 'V-160HD') {
        if (!confirm('This is a V-160HD configuration, but you have V-60HD selected.\n\nSwitch to V-160HD and load this configuration?')) {
          return;
        }
      } else if (v160hdVisible && config.model === 'V-60HD') {
        if (!confirm('This is a V-60HD configuration, but you have V-160HD selected.\n\nSwitch to V-60HD and load this configuration?')) {
          return;
        }
      }
      
      // Show appropriate form
      document.getElementById('stModel').value = config.model;
      document.getElementById('form-model').style.display = 'none';
      
      // Hide both forms first
      document.getElementById('form-v60hd').style.display = 'none';
      document.getElementById('form-v160hd').style.display = 'none';
      
      if (config.model === 'V-60HD') {
        document.getElementById('form-v60hd').style.display = 'block';
        document.getElementById('SSID').value = config.wifi.ssid || '';
        document.getElementById('pwd').value = config.wifi.password || '';
        document.getElementById('stIP').value = config.switch.ip || '';
        document.getElementById('stPort').value = config.switch.port || 80;
        document.getElementById('stChan').value = config.switch.maxChannel || 6;
        document.getElementById('pollTime').value = config.switch.pollInterval || 300;
        attachFormListeners('form-v60hd');
      } else {
        document.getElementById('form-v160hd').style.display = 'block';
        document.getElementById('SSID2').value = config.wifi.ssid || '';
        document.getElementById('pwd2').value = config.wifi.password || '';
        document.getElementById('stIP2').value = config.switch.ip || '';
        document.getElementById('stPort2').value = config.switch.port || 80;
        document.getElementById('stnetUser').value = config.switch.lanUsername || 'admin';
        document.getElementById('stnetPW').value = config.switch.lanPassword || 'admin';
        document.getElementById('stChanHDMI').value = config.switch.maxHDMI || 8;
        document.getElementById('stChanSDI').value = config.switch.maxSDI || 8;
        document.getElementById('pollTime2').value = config.switch.pollInterval || 300;
        attachFormListeners('form-v160hd');
      }
      
      // Update export data with loaded config
      updateExportJSON();
      
      alert('Settings loaded! Review and click Configure STAC.');
    }
    
    // Back button - return to model selection
    function showModelSelect() {
      document.getElementById('form-v60hd').style.display = 'none';
      document.getElementById('form-v160hd').style.display = 'none';
      document.getElementById('form-model').style.display = 'block';
      window.currentConfigJSON = null;
    }
    
    // Factory reset confirmation
    function confirmFactoryReset() {
      return confirm('Are you sure you want to perform a Factory Reset?\n\nThis will erase ALL settings including WiFi credentials, switcher configuration, brightness, and channel assignments.\n\nThis action cannot be undone!');
    }
    
    // Show geek info modal
    function showGeekInfo() {
      const geekText = document.getElementById('geek-info').innerHTML;
      document.getElementById('geek-text').innerHTML = geekText;
      document.getElementById('geek-modal').style.display = 'block';
    }
    
    // Close geek info modal
    function closeGeekInfo() {
      document.getElementById('geek-modal').style.display = 'none';
    }
    
    // Copy geek info to clipboard
    function copyGeekInfo() {
      const geekHTML = document.getElementById('geek-text').innerHTML;
      // Convert <br> tags to newlines for clipboard
      const geekText = geekHTML.replace(/<br\s*\/?>/gi, '\n').replace(/<[^>]+>/g, '');
      const textarea = document.createElement('textarea');
      textarea.value = geekText;
      textarea.style.position = 'fixed';
      textarea.style.opacity = '0';
      document.body.appendChild(textarea);
      textarea.select();
      
      try {
        document.execCommand('copy');
        document.body.removeChild(textarea);
        alert('Geek info copied to clipboard!');
      } catch (error) {
        document.body.removeChild(textarea);
        alert('Copy failed: ' + error.message);
      }
    }
    
    // Run on page load
    window.addEventListener('load', initializePage);
    </script>
    
    <!-- Geek Info Modal -->
    <div id="geek-modal" class="modal">
      <div class="modal-content">
        <h3 style="margin-top: 0; color: #607d8b;">Geek Info</h3>
        <div id="geek-text" class="modal-text"></div>
        <div class="modal-buttons">
          <button onclick="copyGeekInfo()" style="background: #4caf50; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; flex: 1;">
            Copy
          </button>
          <button onclick="closeGeekInfo()" style="background: #666; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; flex: 1;">
            OK
          </button>
        </div>
      </div>
    </div>
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
  <p><a href="/">Return to STAC Setup</a></p>
</body>
</html>
)=====";

} // namespace WebConfig
} // namespace Net

#endif // STAC_WEB_CONFIG_PAGES_H


//  --- EOF --- //
