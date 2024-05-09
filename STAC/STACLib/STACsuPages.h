// HTML page definitions for the setup/configuration routines

const char suFormOpen[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>stSelect</title>
  <style type="text/css">
  form.c2 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h2.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h3.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h4.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  p.c4 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  span.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  body {background-color:#CDD6FA;}
  </style>
</head>
<body>
  <h1 class="c1">STAC Setup</h1>
  <h2 class="c1">Select Model</h2>
  <form class="c2" method="post" action="/">
    <select name="stModel" id="stModel" size="1" autofocus="">
      <option value="V-60HD" selected>V-60HD</option>
      <option value="V-160HD">V-160HD</option>
    </select><br>
    <br>
    <input type="submit" value=" Next "><br>
  </form>
  <p class="c4">
)=====";

const char suFormClose[] = R"=====(</p>
</body>
</html>
)=====";

const char suConfigV60[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>STACSetup</title>
  <style type="text/css">
  form.c2 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h2.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h3.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h4.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  p.c4 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  span.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  body {background-color:#CDD6FA;}
  </style>
</head>
<body>
  <h1 class="c1">STAC Setup</h1>
  <h2 class="c1">&bull; V-60HD &bull;</h2>
  <h4 class="c1">WiFi Settings</h4>
  <form class="c2" method="post" action="/parse">
    <label for="SSID:">WiFi Network SSID:</label> <input id="SSID" name="SSID" required="" type="text" maxlength="32"><br>
    <br>
    <label for="Password:">WiFi Password:</label> <input id="pwd" name="pwd" type="password" size="20" maxlength="63"><br>
    <br>
    <h4 class="c1">V-60HD Network Settings</h4>
    <label for="Smart Tally IP:">V-60HD IP Address:</label> <input id="stIP" name="stIP" size="15" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" required="" type="text" inputmode="decimal"><br>
    <br>
    <label for="stPort">V-60HD Port #:</label> <input id="stPort" name="stPort" size="5" min="0" max="65353" required="" type="number" value="80" inputmode="numeric"><br>
    <br>
    <h4 class="c1">STAC Settings</h4>
    <label for="stChan">HDMI channel maximum:</label> <input id="stChan" name="stChan" size="4" min="1" max="8" required="" type="number" value="6" inputmode="numeric"><br>
    <br>
    <label for="pollTime">Polling interval (ms):</label> <input id="pollTime" name="pollTime" size="6" min="175" max="2000" required="" type="number" value="300" inputmode="numeric"><br>
    <br>
    <button onclick="document.location='/'">Back</button>&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<input value="Submit" type="submit"> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; <input type="reset">
  </form><br>
  <script>
  stIP.addEventListener("input", ev => {
  const selStart = stIP.selectionStart, selEnd = stIP.selectionEnd;
  stIP.value = stIP.value.replace(/,/g, ".");
  stIP.selectionStart = selStart;
  stIP.selectionEnd = selEnd;
  });
  </script>
)=====";

const char suConfigV160[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>STACSetup</title>
  <style type="text/css">
  form.c2 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h2.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h3.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h4.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  p.c4 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  span.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  body {background-color:#CDD6FA;}
  </style>
</head>
<body>
  <h1 class="c1">STAC Setup</h1>
  <h2 class="c1">&bull; V-160HD &bull;</h2>
  <form class="c2" method="post" action="/parse2">
  <h4 class="c1">WiFi Settings</h4>
    <label for="SSID:">WiFi Network SSID:</label> <input id="SSID" name="SSID" required="" type="text" maxlength="32"><br>
    <br>
    <label for="Password:">WiFi Password:</label> <input id="pwd" name="pwd" type="password" size="20" maxlength="63"><br>
    <br>
  <h4 class="c1">V-160HD Switch Settings</h4>
    <label for="Smart Tally IP:">V-160HD IP Address:</label> <input id="stIP" name="stIP" size="15" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" required="" type="text" inputmode="decimal"><br>
    <br>
    <label for="stPort">V-160HD Port #:</label> <input id="stPort" name="stPort" size="5" min="0" max="65353" required="" type="number" value="80" inputmode="numeric"><br>
    <br>
     <label for="stnetUser">V-160HD Network User ID:</label> <input id="stnetUser" name="stnetUser" size="8" required="" type="text" value="user" maxlength="8"><br>
    <br>
    <label for="stnetPW">V-160HD Network Password:</label> <input id="stnetPW" name="stnetPW" size="6" min="0000" max="9999" required="" type="number" value="0000" inputmode="numeric"><br>
    <br>
  <h4 class="c1">STAC Settings</h4>
    <label for="stChanHDMI">HDMI channel maximum:</label> <input id="stChanHDMI" name="stChanHDMI" size="4" min="0" max="8" required="" type="number" value="8" inputmode="numeric"><br>
    <br>
    <label for="stChanSDI">SDI channel maximum:</label> <input id="stChanSDI" name="stChanSDI" size="4" min="0" max="8" required="" type="number" value="8" inputmode="numeric"><br>
    <br>
    <label for="pollTime">Polling interval (ms):</label> <input id="pollTime" name="pollTime" size="6" min="175" max="5000" required="" type="number" value="300" maxlength="8"><br>
    <br>
    <br>
    <button onclick="document.location='/'">Back</button>&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<input value="&nbsp Submit &nbsp" type="submit">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<input type="reset">
  </form>
  <script>
  stIP.addEventListener("input", ev => {
  const selStart = stIP.selectionStart, selEnd = stIP.selectionEnd;
  stIP.value = stIP.value.replace(/,/g, ".");
  stIP.selectionStart = selStart;
  stIP.selectionEnd = selEnd;
  });
  </script>
)=====";

const char suReceived[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=0.86, maximum-scale=1.0, minimum-scale=0.86">
  <title>STACConfirm</title>
  <style type="text/css">
  body {background-color:#CDD6FA;}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  p.c2 {font-family: Helvetica, Arial, sans-serif; text-align: center; font-size: 20px; color: blue}
  p.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  </style>
</head>
<body>
  <h1 class="c1">STAC Setup</h1>
  <p class="c2"><strong>Setup information received.</strong></p>
  <p class="c3">Close this window and reconnect<br>
  to your regular WiFi network.<br>
  <br>
  Consult the manual<br>
  if you need to reconfigure this device.</p>
</body>
</html>
)=====";

const char suNotFound[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content=
  "width=device-width, initial-scale=1.0, maximum-scale=1.5, user-scalable=yes">
  <meta http-equiv="content-type" content=
  text/html; charset=utf-8">
  <title>STAC404</title>
  <style type="text/css">
  body {background-color:#2A2C32;}
  form.c1 {text-align:center;}
  h1.c2 {text-align:center;}
  h2.c3 {text-align:center;}
  p.c6 {text-align:center;}
  span.c1 {color: #DCDCDC; font-family: arial,helvetica,sans-serif}
  span.c4 {color: #00BFFF; font-size: 18px}
  span.c5 {font-family: arial,helvetica,sans-serif; font-size: 16px}
  span.c7 {color: #DCDCDC; font-family: arial,helvetica,sans-serif; font-size: 18px; text-align: center}
  </style>
</head>
<body>
  <h1 class="c2"><span class="c1">STAC Setup</span></h1>
  <h2 class="c3"><span class="c1">404</span></h2>
  <p class="c6"><span class="c5"><em><strong><span class="c4">
  This is not the page you are looking for.
  </span></strong></em></span></p>
  <p class="c6"><span class="c7">Move along.</span></p>
  <br>
  <form class="c1" action="/">
    <button type="submit">Moving along...</button>
  </form>
</body>
</html>
)=====";


// --- EOF ---
