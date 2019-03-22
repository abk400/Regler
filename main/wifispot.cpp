#include <string.h>
#include <sstream>
#include <set>
#include <map>

#include <FreeRTOS.h>

#include "esp_headers.h"
#include "wifispot.h"
#include "reglerapp.h"
#include "eventcredentials.h"
#include "storage.h"

using namespace std;
//https://dl.espressif.com/doc/esp-idf/latest/get-started/linux-setup.html
//https://docs.espressif.com/projects/esp-idf/en/stable/get-started/
//https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md




WifiSpot::WifiSpot(EspApp *app):
    EspObject(WIFI_SPOT, app)
{
    m_reglerApp = static_cast<ReglerApp*>(app);
}


string WifiSpot::getResponseHtml() {
  return "<html><body>" + m_reglerApp->responseHTML + "</html></body>";
}

void WifiSpot::refresh() {
  Serial.println("refresh");
  fillResponseHtml();
  Serial.println("Exit scan mode");

  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.reconnect();
  
  Serial.println("AP mode enter");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(m_reglerApp->apIP, m_reglerApp->apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ReglerAcessPoint");
  
  m_reglerApp->dnsServer.start(m_reglerApp->DNS_PORT, "*", m_reglerApp->apIP);
}

void WifiSpot::handleRefresh() {
  Serial.println("REFRESH");
  m_reglerApp->server->send(200, "text/html", m_reglerApp->page.c_str());
  delay(3000);
  refresh();
  
  //refresh();
}

void WifiSpot::handleRoot() {
  Serial.println("ROOT");
  
  stringstream ss;
  ss << getResponseHtml() << endl;            
  
  stringstream printss;
  printss << "Data returned to client: \n" << ss.str();
  Serial.println(printss.str().c_str());
  
  m_reglerApp->server->send(200, "text/html", ss.str().c_str());
}

void WifiSpot::handleNotFound() {
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += m_reglerApp->server->uri();
  message += "\nMethod: ";
  message += (m_reglerApp->server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += m_reglerApp->server->args();
  message += "\n";
  for (uint8_t i = 0; i < m_reglerApp->server->args(); i++) {
    message += " " + m_reglerApp->server->argName(i) + ": " + m_reglerApp->server->arg(i) + "\n";
  }
  m_reglerApp->server->send(404, "text/plain", message);
}


void WifiSpot::handleConnect() {

  std::map<string, string> arguments;
  for (uint8_t i = 0; i < m_reglerApp->server->args(); i++) {
    arguments[string(m_reglerApp->server->argName(i).c_str()) ] = string(m_reglerApp->server->arg(i).c_str());
  }

  string ssid = arguments["network"];
  string password = arguments["pass"];  
  Storage * disk = Storage::instance();
  
  disk->write("network", ssid);
  disk->write("pass", password);
  
  stringstream ss;
  
  ss << "ssid " << ssid << "\npassword " << password;
  Serial.println(ss.str().c_str());

  m_eventCredentials = make_shared<EventCredentials>(this, arguments);
}


void WifiSpot::fillResponseHtml() {
  stringstream ss;
  Serial.println("W-1");
  
  TaskStatus_t status[10];
  uint32_t runTime;
  int count = uxTaskGetSystemState( status, 10, &runTime );
  
  stringstream ta;
  for (int i = 0; i < count; i++) {
      
      ta << "task[" << i << "].usStackHighWaterMark=" << status[i].usStackHighWaterMark << "\n";
  }
  Serial.println(ta.str().c_str());
  
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("W0");
  Serial.flush();
  WiFi.disconnect();
  delay(100);
  Serial.println("W1");
  Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    ss << "<h1>Available networks:</h1>\n";
    
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        set<string> networks;
        for (int i = 0; i < n; ++i) {
          networks.insert(WiFi.SSID(i).c_str());
        }
        int i = 0; 
        ss << "<form action='/connect'>\n";
        ss << "<input type=\"button\" value=\"Refresh Page\" onClick=\"window.location='/refresh'\"><br>\n";
        
        for (auto it = networks.begin();  it != networks.end(); it++) {
          ss << "<input type=\"radio\" name=\"network\" value=\"" << *it << "\">" << *it << "<br>\n";
          i++;
        }
        ss << "password: <input type=text name=pass>";
        ss << "<input type=submit value=Submit>";
        ss << "</form>\n";
    }
    m_reglerApp->responseHTML = ss.str();
    Serial.print(ss.str().c_str());
    Serial.println("");
}

void WifiSpot::restartAP(){
  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  delay(100);

}
 
EventPtr WifiSpot::loop() {
    
    if (m_eventCredentials) {
        Serial.println("####Generating credentials");
        auto res = m_eventCredentials;
        m_eventCredentials = EventPtr();
        return res;
    }
    return EventPtr();
}


void WifiSpot::enter(EspObject */*source*/, Event */*event*/)
{
  
  Serial.println("Starting..");

  refresh();

  m_reglerApp->server->on("/", std::bind(&WifiSpot::handleRoot, this));
  m_reglerApp->server->on("/refresh", std::bind(&WifiSpot::handleRefresh, this));
  m_reglerApp->server->on("/connect", std::bind(&WifiSpot::handleConnect, this));
  m_reglerApp->server->onNotFound(std::bind(&WifiSpot::handleNotFound, this));

  m_reglerApp->server->begin();
  Serial.println("HTTP server started");
  
  Storage * disk = Storage::instance();
  string ssid = disk->read("network");
  string pass = disk->read("pass");
  if (NOT ssid.empty()) {
    std::map<string, string> args;
    args["network"] = ssid;
    args["pass"] = pass;
    m_eventCredentials = make_shared<EventCredentials>(this, args);
  }
}
