// HTML page definitions for the setup/configuration routines

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
  <p class="c2"><strong>Configuration received.</strong></p>
  <p class="c3">Close this window and reconnect<br>
  to your regular WiFi network.<br>
  <br>
  Consult the manual<br>
  if you need to reconfigure this device.</p>
</body>
</html>
)=====";

const char suFormOpen[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
  <title>STACConfig</title>
  <style type="text/css">
  form.c2 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  p.c4 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  span.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  body {background-color:#CDD6FA;}
  </style>
</head>
<body>
  <h1 class="c1">STAC Setup</h1>
  <form class="c2" method="post" action="/parse">
    <label for="SSID:">Network SSID:</label> <input id="SSID" name="SSID" required="" type="text" maxlength="32"><br>
    <br>
    <label for="Password:">Password:</label> <input id="pwd" name="pwd" type="password" size="20" maxlength="63"><br>
    <br>
    <label for="Smart Tally IP:">Smart Tally IP:</label> <input id="stIP" name="stIP" size="15" pattern="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" required="" type="text" inputmode="decimal"><br>
    <br>
    <label for="stPort">port #:</label> <input id="stPort" name="stPort" size="5" min="0" max="65353" required="" type="number" value="80" inputmode="numeric"><br>
    <br>
    <label for="stChan"># of channels:</label> <input id="stChan" name="stChan" size="3" min="1" max="8" required="" type="number" value="6" inputmode="numeric"><br>
    <br>
    <label for="pollTime">Polling interval (ms):</label> <input id="pollTime" name="pollTime" size="6" min="175" max="2000" required="" type="number" value="300" inputmode="numeric"><br>
    <br>
    <input value="Submit" type="submit"> &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; <input type="reset">
  </form><br>
  <script>
  stIP.addEventListener("input", ev => {
  const selStart = stIP.selectionStart, selEnd = stIP.selectionEnd;
  stIP.value = stIP.value.replace(/,/g, ".");
  stIP.selectionStart = selStart;
  stIP.selectionEnd = selEnd;
  });
  </script>
  <p class="c4">Unit ID: )=====";

const char suFormClose[] = R"=====(  </p>
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
  <title>notfound</title>
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
  <p class="c6"><span class="c5"><em><strong><span class="c4">This
  is not the page you are looking
  for.</span></strong></em></span></p>
  <p class="c6"><span class="c7">Move along.</span></p>
  <br>
  <form class="c1" action="/">
    <button type="submit">Moving along...</button>
  </form>
</body>
</html>
)=====";
