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
    HttpHelper(WIFI_SPOT, app)
{
}

void WifiSpot::refresh() {
  D_PRINTLN("refresh");
  fillResponseHtml();
  D_PRINTLN("Exit scan mode");

  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.reconnect();
  
  D_PRINTLN("AP mode enter");
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(m_reglerApp->apIP, m_reglerApp->apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("ReglerAcessPoint");
  
  m_reglerApp->dnsServer.start(m_reglerApp->DNS_PORT, "*", m_reglerApp->apIP);

  lastWifiPageHTML = m_reglerApp->responseHTML;
}

void WifiSpot::handleRefresh() {
  D_PRINTLN("REFRESH");
  m_reglerApp->server->send(200, "text/html", m_reglerApp->page.c_str());
  delay(3000);
  refresh();
}

void WifiSpot::handleRoot() {
  D_PRINTLN("ROOT");
  
  std::string html;
  getResponseHtml(html);
  
//  stringstream printss;
//  printss << "Data returned to client: \n" << html;
//  D_PRINTLN(printss.str().c_str());
  
  m_reglerApp->server->send(200, "text/html", html.c_str());
}

void WifiSpot::handleWifi()
{
    D_PRINTLN("WifiSpot::handleWifi");
    string response;
    m_reglerApp->responseHTML = lastWifiPageHTML;
    getResponseHtml(response);
    m_reglerApp->server->send(200, "text/html", response.c_str());
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
  D_PRINTLN(ss.str().c_str());

  m_eventCredentials = make_shared<EventCredentials>(this, arguments);
}

void WifiSpot::handleReset() {
    D_PRINTLN("WifiSpot::handleReset");
    string response;
    reset(&response);
    response += "\n";
    m_reglerApp->server->send(200, "text/html", response.c_str());
}

void WifiSpot::handleStatus() {
    D_PRINTLN("WifiSpot::handleStatus");
    string response;
    fillStatusResponseHtml();
    getResponseHtml(response);
    m_reglerApp->server->send(200, "text/html", response.c_str());
}

void WifiSpot::fillResponseHtml() {
  stringstream ss;
  D_PRINTLN("W-1");
  
  TaskStatus_t status[10];
  uint32_t runTime;
  int count = uxTaskGetSystemState( status, 10, &runTime );
  
  stringstream ta;
  for (int i = 0; i < count; i++) {
      
      ta << "task[" << i << "].usStackHighWaterMark=" << status[i].usStackHighWaterMark << "\n";
  }
  D_PRINTLN(ta.str().c_str());
  
  WiFi.mode(WIFI_STA);
  delay(100);
  D_PRINTLN("W0");
  Serial.flush();
  WiFi.disconnect();
  delay(100);
  D_PRINTLN("W1");
  D_PRINTLN("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    D_PRINTLN("scan done");
    ss << "<h1>Available networks:</h1>\n";
    
    if (n == 0) {
        D_PRINTLN("no networks found");
    } else {
        Serial.print(n);
        D_PRINTLN(" networks found");
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
        // Reset button
        ss << "<form action=\"/reset\">";
        ss << "<input type=\"submit\" value=\"Reset\">";
        ss << "</form>\n";
    }
    m_reglerApp->responseHTML = ss.str();
    Serial.print(ss.str().c_str());
    D_PRINTLN("");
}

void WifiSpot::restartAP(){
  WiFi.softAPdisconnect();
  WiFi.disconnect(true, true);
  WiFi.mode(WIFI_STA);
  delay(100);

}
 
EventPtr WifiSpot::loop() {
    
    if (m_eventCredentials) {
        D_PRINTLN("####Generating credentials");
        auto res = m_eventCredentials;
        m_eventCredentials = EventPtr();
        return res;
    }
    return EventPtr();
}


void WifiSpot::enter(EspObject */*source*/, Event */*event*/)
{
  D_PRINTLN("Starting..");

  refresh();

  m_reglerApp->server->on("/", std::bind(&WifiSpot::handleRoot, this));
  m_reglerApp->server->on("/status", std::bind(&WifiSpot::handleStatus, this));
  m_reglerApp->server->on("/wifi", std::bind(&WifiSpot::handleWifi, this));
  m_reglerApp->server->on("/refresh", std::bind(&WifiSpot::handleRefresh, this));
  m_reglerApp->server->on("/connect", std::bind(&WifiSpot::handleConnect, this));
  m_reglerApp->server->on("/reset", std::bind(&WifiSpot::handleReset, this));
  m_reglerApp->server->onNotFound(std::bind(&WifiSpot::handleNotFound, this));

  m_reglerApp->server->begin();
  D_PRINTLN("HTTP server started");
  
  Storage * disk = Storage::instance();
  string ssid = disk->read("network");
  string pass = disk->read("pass");
  if (NOT ssid.empty() && NOT pass.empty() && ssid != "null") {
    std::map<string, string> args;
    args["network"] = ssid;
    args["pass"] = pass;
    m_eventCredentials = make_shared<EventCredentials>(this, args);

    fillStatusResponseHtml();
  }
  else
  {
      m_reglerApp->lastErrMsg = "No WiFi point selected.";
//      refresh();
  }
}
