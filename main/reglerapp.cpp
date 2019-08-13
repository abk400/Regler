#include "reglerapp.h"

ReglerApp::ReglerApp():
    apIP(192, 168, 1, 1), communication(this)
{
    server.reset(new WebServer(80));
    page1 = R"#(
<html>
<head>
<script type="text/javascript">
var timer=0;
function startTimer()
{
  setInterval("timerUp()",1000);
}


function timerUp()
{
  timer++;
  var resetat=12; //change this number to adjust the length of time in seconds
  if(timer==resetat)
  {
    window.location = '/';
  }
  var tleft=resetat-timer;
  document.getElementById('timer').innerHTML=tleft;
}
  
</script>
</head>
<body onload="startTimer()">
)#";

    page2 = R"#(
Redirecting after 
<div id="timer">
</div>
</body>
</html>
)#";
    page = page1 + page2;

}

void ReglerApp::serverRestart()
{
    server.reset(new WebServer(80));
}

void ReglerApp::loop()
{
    dnsServer.processNextRequest();
    
    server->handleClient();
    
    TBase::loop();
}
