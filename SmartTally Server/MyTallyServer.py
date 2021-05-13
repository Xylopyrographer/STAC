# Python 3 server example
from http.server import BaseHTTPRequestHandler, HTTPServer
from datetime import datetime, timedelta

# hostName = "localhost"
hostName = "192.168.1.15"
serverPort = 8080

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):

        referer = self.headers.get('Referer')

        # print( referer, self.path )  

        if ( referer == None ) & (self.path == '/' ) :
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()        
            self.wfile.write(bytes("<html><head><title>https://RolandServerTest</title></head>", "utf-8"))
            self.wfile.write(bytes("<p>Request: %s</p>" % self.path, "utf-8"))
            self.wfile.write(bytes("<body>", "utf-8"))
            self.wfile.write(bytes("<p> <a href='http://"+hostName+":8080/tally/1/status'>Channel 1</a></p>", "utf-8"))
            self.wfile.write(bytes("<p> <a href='http://"+hostName+":8080/tally/2/status'>Channel 2</a></p>", "utf-8"))
            self.wfile.write(bytes("<p> <a href='http://"+hostName+":8080/tally/3/status'>Channel 3</a></p>", "utf-8"))
            self.wfile.write(bytes("<p> </p>", "utf-8"))
            self.wfile.write(bytes("<p>This is an example SmartTally Web Service. It will cycle through ", "utf-8"))
            self.wfile.write(bytes("{OnAir, Selected and Unselected} every 20 seconds</p>", "utf-8"))
            self.wfile.write(bytes("</body></html>", "utf-8"))

        elif self.path == '/tally/1/status':    
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()
            self.wfile.write( bytes("%s" % modes( 1 ), "utf-8") )
            
##            self.wfile.write(bytes("<html><head><title>https://RolandServerTest></title></head>", "utf-8"))
##            self.wfile.write(bytes("<body>", "utf-8"))
##            self.wfile.write(bytes("<p>Channel 1</p>", "utf-8"))
##            self.wfile.write(bytes("<p></p>", "utf-8"))
##            self.wfile.write(bytes("<p> <a href='http://localhost:8080/'>Return</a></p>", "utf-8"))
##            self.wfile.write(bytes("</body></html>", "utf-8"))


        elif self.path == '/tally/2/status':    
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()        
            self.wfile.write( bytes("%s" % modes( 2 ), "utf-8") )      

##            self.wfile.write(bytes("<html><head><title>https://RolandServerTest></title></head>", "utf-8"))
##            self.wfile.write(bytes("<body>", "utf-8"))
##            self.wfile.write(bytes("<p>Channel 2</p>", "utf-8"))
##            self.wfile.write(bytes("<p></p>", "utf-8"))
##            self.wfile.write(bytes("<p> <a href='http://localhost:8080/'>Return</a></p>", "utf-8"))
##            self.wfile.write(bytes("</body></html>", "utf-8"))

        elif self.path == '/tally/3/status':    
            self.send_response(200)
            self.send_header("Content-type", "text/html")
            self.end_headers()        
            self.wfile.write( bytes("%s" % modes( 3 ), "utf-8") )      

##            self.wfile.write(bytes("<html><head><title>https://RolandServerTest></title></head>", "utf-8"))
##            self.wfile.write(bytes("<body>", "utf-8"))
##            self.wfile.write(bytes("<p>Channel 3</p>", "utf-8"))
##            self.wfile.write(bytes("<p></p>", "utf-8"))
##            self.wfile.write(bytes("<p> <a href='http://localhost:8080/'>Return</a></p>", "utf-8"))
##            self.wfile.write(bytes("</body></html>", "utf-8"))
                        
        
    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        body = self.rfile.read(content_length)
        self.send_response(200)
        self.end_headers()
        response = BytesIO()
        response.write(b'This is POST request. ')
        response.write(b'Received: ')
        response.write(body)
        self.wfile.write(response.getvalue()) 


def modes( tally ):
    # returns value based on time of day to simulate tally data
    # /tally/[address]/status

    curr_time = datetime.now()

    if tally == 2 :
        curr_time = curr_time + timedelta(seconds=20)
    elif tally == 3 :
        curr_time = curr_time + timedelta(seconds=40)
      
    seconds = int( curr_time.strftime( '%S' ))

    # print( seconds )

    if seconds < 20 :
        # return '<html lang="en"><head><meta http-equiv="Content-Type" content="text/html; charset=windows-1252"><style type="text/css">html,body{width:100%;height:100%;margin:0;padding:0}html{display:table}body{display:table-cell;text-align:center;vertical-align:middle;font-size:160mm}</style><script type="text/javascript">var xhr=new XMLHttpRequest();var ch_status="";xhr.onreadystatechange=function(){var color;switch(xhr.readyState){case 0:break;case 1:break;case 2:break;case 3:break;case 4:if(xhr.status==200){if(ch_status!=xhr.responseText){switch(xhr.responseText){case"onair":color="#FF1111";ch_status="onair";break;case"selected":color="#11FF11";ch_status="selected";break;case"unselected":color="#777777";ch_status="unselected";break;default:color="#FFFFFF";ch_status="";break;}}}else{color="#111111";ch_status="";}document.getElementById(''main'').style.backgroundColor=color;break;}};window.onload=function(){document.getElementById(''main'').style.backgroundColor="#111111";var path=window.location.pathname;path=path+"/status";(function loop(){xhr.open(''GET'',path,false);xhr.send();xhr.abort();requestAnimationFrame(loop);})();};</script><title>Roland</title></head><body text="#FFFFFF" id="main" style="background-color: rgb(119, 119, 119);">5</body></html>'
        return "unselected"
    elif seconds >= 20 and seconds < 40:
        # return '<html lang="en"><head><meta http-equiv="Content-Type" content="text/html; charset=windows-1252"><style type="text/css">html,body{width:100%;height:100%;margin:0;padding:0}html{display:table}body{display:table-cell;text-align:center;vertical-align:middle;font-size:160mm}</style><script type="text/javascript">var xhr=new XMLHttpRequest();var ch_status="";xhr.onreadystatechange=function(){var color;switch(xhr.readyState){case 0:break;case 1:break;case 2:break;case 3:break;case 4:if(xhr.status==200){if(ch_status!=xhr.responseText){switch(xhr.responseText){case"onair":color="#FF1111";ch_status="onair";break;case"selected":color="#11FF11";ch_status="selected";break;case"unselected":color="#777777";ch_status="unselected";break;default:color="#FFFFFF";ch_status="";break;}}}else{color="#111111";ch_status="";}document.getElementById(''main'').style.backgroundColor=color;break;}};window.onload=function(){document.getElementById(''main'').style.backgroundColor="#111111";var path=window.location.pathname;path=path+"/status";(function loop(){xhr.open(''GET'',path,false);xhr.send();xhr.abort();requestAnimationFrame(loop);})();};</script><title>Roland</title></head><body text="#FFFFFF" id="main" style="background-color: rgb(17, 255, 17);">1</body></html>'
        return "selected"
    else:
        # return '<html lang="en"><head><meta http-equiv="Content-Type" content="text/html; charset=windows-1252"><style type="text/css">html,body{width:100%;height:100%;margin:0;padding:0}html{display:table}body{display:table-cell;text-align:center;vertical-align:middle;font-size:160mm}</style><script type="text/javascript">var xhr=new XMLHttpRequest();var ch_status="";xhr.onreadystatechange=function(){var color;switch(xhr.readyState){case 0:break;case 1:break;case 2:break;case 3:break;case 4:if(xhr.status==200){if(ch_status!=xhr.responseText){switch(xhr.responseText){case"onair":color="#FF1111";ch_status="onair";break;case"selected":color="#11FF11";ch_status="selected";break;case"unselected":color="#777777";ch_status="unselected";break;default:color="#FFFFFF";ch_status="";break;}}}else{color="#111111";ch_status="";}document.getElementById(''main'').style.backgroundColor=color;break;}};window.onload=function(){document.getElementById(''main'').style.backgroundColor="#111111";var path=window.location.pathname;path=path+"/status";(function loop(){xhr.open(''GET'',path,false);xhr.send();xhr.abort();requestAnimationFrame(loop);})();};</script><title>Roland</title></head><body text="#FFFFFF" id="main" style="background-color: rgb(255, 17, 17);">2</body></html>'
        return "onair"

    
if __name__ == "__main__":        
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")
    
