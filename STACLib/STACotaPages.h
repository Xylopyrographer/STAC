// HTML page definitions for the OTA firmware update routines

const char udIndex[] = R"=====(<!DOCTYPE HTML>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content=
  "width=device-width, initial-scale=1.0, maximum-scale=1.5, user-scalable=yes">
  <link rel="icon" href="data:,">
  <style type="text/css">
  body.c4 {font-family: Helvetica,sans-serif;text-align: center}
  form.c2 {font-family: Helvetica, Arial, sans-serif; margin-bottom: 8px; text-align: center; width: 100%}
  h1.c1 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  span.c3 {font-family: Helvetica, Arial, sans-serif; text-align: center}
  body {background-color:#E9F6DC;}
  </style>
  <title>STACUpdater</title>
</head>
<body class="c4">
  <h1 class="c1">STAC Updater</h1>
  <div>
    <form method="post" enctype="multipart/form-data" action="/update" class="c2">
      <input type="file" name="update" accept=".bin"><br>
      <br>
      <input type="submit" value="Update" disabled="true">
    </form><span class="c3">
    <script>
    document.querySelector("input[type=file]").onchange = ({
    target: { value },
    }) => {
    document.querySelector("input[type=submit]").disabled = !value;
    };
    </script></span>
  </div>
  <div>
    <span class="c3">After clicking "Update", the STAC will<br>
    install the new firmware and restart.<br>
    <br>
    <b>To cancel the update,</b><br>
    press the STAC Reset button now.<br>
    <br>
    See the <i>"Updating Firmware"</i> section<br>
    of the manual for more information.<br></span>
  </div>
</body>
</html>
)=====";

const char udPageOpen[] = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.5, user-scalable=yes">
  <title>udSuccess</title>
  <style type="text/css">
  h1.c2 {text-align:center}
  p.c5 {text-align:center}
  span.c1 {font-family: Helvetica,Arial,sans-serif}
  span.c3 {color:blue}
  span.c4 {font-family:  Helvetica,Arial,sans-serif; font-size: 16px}
  span.c6 {color:red}
  body {background-color:#E9F6DC;}
  </style>
</head>
<body>
  <h1 class="c2"><span class="c1">STAC Updater</span></h1>
)=====";

const char udGood[] = R"=====(<p class="c5"><span class="c4"><strong><span class="c3">Update Success</span></strong></span></p>
<p class="c5"><span class="c1">New firmware file:<br>)=====";

const char udFail[] = R"=====(<p class="c5"><span class="c4"><strong><span class="c6">Update Failed</span></strong></span></p>
<p class="c5"><span class="c1">Reason:<br>)=====";

const char udPageClose[] = R"=====(<br>
<br>
<strong>Restarting the STAC.</strong><br>
Close this window and reconnect<br>
to your regular WiFi network.</span></p>
</body>
</html>
)=====";

const char udNotFound[] = R"=====(<!DOCTYPE html>
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
  <h1 class="c2"><span class="c1">STAC Updater</span></h1>
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
