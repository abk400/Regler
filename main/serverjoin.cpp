#include <string.h>
#include <sstream>
#include <set>
#include <map>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
 
#include <FreeRTOS.h>

#include "esp_headers.h"
#include "wifispot.h"
#include "reglerapp.h"
#include "eventcredentials.h"
#include "reglerapp.h"
#include "serverjoin.h"
#include "protocol.h"
#include "storage.h"

using namespace std;

static const char * const SERVER_IP_STR = "server_ip";
static const char * const SENSOR_ID_STR= "server_sensor_id";
static const char * const POINT_ID_STR= "server_point_id";
ServerJoin::ServerJoin(EspApp *app):
    EspObject(SERVER_CONNECTING, app)
{
    m_reglerApp = static_cast<ReglerApp*>(app);
}


string ServerJoin::getResponseHtml() {
  return "<html><body>" + m_reglerApp->responseHTML + "</html></body>";
}


void ServerJoin::handleServerJoin() {
  Serial.println("ServerJoin::handleServerJoin");
  
  stringstream ss;
  ss << getResponseHtml() << endl;            
  
  stringstream printss;
  printss << "Data returned to client: \n" << ss.str();
  Serial.println(printss.str().c_str());
  
  m_reglerApp->server->send(200, "text/html", ss.str().c_str());
}

void ServerJoin::handleNotFound() {
  
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


void ServerJoin::handleSubmit() {

  Serial.println("ServerJoin::handleSubmit");
  std::map<string, string> arguments;
  for (uint8_t i = 0; i < m_reglerApp->server->args(); i++) {
    arguments[string(m_reglerApp->server->argName(i).c_str()) ] = string(m_reglerApp->server->arg(i).c_str());
  }
  string response;
  /*bool result = */
  connect(arguments[SERVER_IP_STR], toInt(arguments[POINT_ID_STR]), toInt(arguments[SENSOR_ID_STR]), &response);
  response += "\n";
  m_reglerApp->server->send(200, "text/html", response.c_str());
}

bool ServerJoin::connect(const std::string & server, int id, int sensor_id, std::string * response)
{ 
  
  
  stringstream ss;
  ss << "Connecting " << server.c_str();
  Serial.println(ss.str().c_str());
  
  ServerCommunication & communication = m_reglerApp->communication;
  
  JoinStatus status = communication.join(server, 54115, id, sensor_id, response);
  
  Storage * storage = Storage::instance();
  if (status) {
      m_newEvent = std::make_shared<Event>(SERVER_CONNECTED, this);
      storage->write(SERVER_IP_STR, server);
      storage->write_int(POINT_ID_STR, id);
      storage->write_int(SENSOR_ID_STR, sensor_id);
  } else {
      m_newEvent = std::make_shared<Event>(SERVER_ERROR, this);
      storage->write(SERVER_IP_STR, "");
  }
  m_reglerApp->id = status.id;
  
  if (!status) {
      Serial.println("");
      *response = m_reglerApp->page1 + "Server not connected!!!!" + m_reglerApp->page2;
  }
  
  return status;
}


void ServerJoin::fillResponseHtml() {
    ReglerApp * app = static_cast<ReglerApp *> (m_app);
    string ip_string = app->m_local_ip.toString().c_str();
  
    stringstream ss;
    
    ss << "Local IP: " << ip_string << "<br>\n";
    ss << "<form action='/connect_server'>\n";
    ss << "server: <input type=text name=" << SERVER_IP_STR << "><br>\n";
    ss << "point id number: <input type=numer name=" << POINT_ID_STR << "><br>\n";
    ss << "sensor id number: <input type=numer name=" << SENSOR_ID_STR << " ><br>\n";
    
    ss << "<input type=submit value=Submit>";
    ss << "</form>\n";

    m_reglerApp->responseHTML = ss.str();
    Serial.print(ss.str().c_str());
    Serial.println("");
}

void ServerJoin::enter(EspObject */*source*/, Event */*event*/)
{
  Storage * storage = Storage::instance();
  string server_ip = storage->read("server_ip");
  int sensor_id    = storage->read_int(SENSOR_ID_STR);
  int point_id     = storage->read_int(POINT_ID_STR);
  
  if (NOT server_ip.empty()) {
      std::string response;
      bool result = connect(server_ip, point_id, sensor_id, &response);
      if (result) {
        return ;
      }
  }
  
  fillResponseHtml();
  m_reglerApp->server->on("/server_join", std::bind(&ServerJoin::handleServerJoin, this));
  m_reglerApp->server->on("/connect_server", std::bind(&ServerJoin::handleSubmit, this));
  m_reglerApp->server->onNotFound(std::bind(&ServerJoin::handleNotFound, this));
  
}
