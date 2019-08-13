#include <string.h>
#include <sstream>
#include <set>
#include <map>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds
#include <sys/time.h>
 
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

static const char * const SERVER_IP_STR     = "srv_ip";
static const char * const SERVER_PORT_STR   = "srv_port";
static const char * const SENSOR_ID_STR     = "srv_sensor_id";
static const char * const POINT_ID_STR      = "srv_point_id";
ServerJoin::ServerJoin(EspApp *app):
    HttpHelper(SERVER_CONNECTING, app)
{
}

void ServerJoin::handleServerJoin() {
  D_PRINTLN("ServerJoin::handleServerJoin");
  
  std::string html;
  getResponseHtml(html);
  
//  stringstream printss;
//  printss << "Data returned to client: \n" << html;
//  D_PRINTLN(printss.str().c_str());
  
  m_reglerApp->server->send(200, "text/html", html.c_str());
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

  D_PRINTLN("ServerJoin::handleSubmit");
  std::map<string, string> arguments;
  for (uint8_t i = 0; i < m_reglerApp->server->args(); i++) {
    arguments[string(m_reglerApp->server->argName(i).c_str()) ] = string(m_reglerApp->server->arg(i).c_str());
  }
  string response;
  /*bool result = */
  if (arguments.count(SERVER_IP_STR) &&
      arguments.count(SERVER_PORT_STR) &&
      arguments.count(POINT_ID_STR) &&
      arguments.count(SENSOR_ID_STR))
  {
    connect(arguments[SERVER_IP_STR], toInt(arguments[SERVER_PORT_STR]), toInt(arguments[POINT_ID_STR]), toInt(arguments[SENSOR_ID_STR]), &response);
  }
  response += "\n";
  m_reglerApp->server->send(200, "text/html", response.c_str());
}

//void ServerJoin::handleReset() {
//  D_PRINTLN("ServerJoin::handleReset");
//  string response;
//  reset(&response);
//  response += "\n";
//  m_reglerApp->server->send(200, "text/html", response.c_str());
//}

bool ServerJoin::connect(const std::string & server, int port, int point_id, int sensor_id, std::string * response)
{ 
  stringstream ss;
  ss << "Connecting " << server.c_str();
  D_PRINTLN(ss.str().c_str());
  
  ServerCommunication & communication = m_reglerApp->communication;
  
  JoinStatus status = communication.join(server, port, point_id, sensor_id, response);
  
  Storage * storage = Storage::instance();
  if (status) {
      m_newEvent = std::make_shared<Event>(SERVER_CONNECTED, this);
      storage->write(SERVER_IP_STR, server);
      storage->write_int(SERVER_PORT_STR, port);
      storage->write_int(POINT_ID_STR, point_id);
      storage->write_int(SENSOR_ID_STR, sensor_id);

      // set current time from server
      setTime(status.time);
  } else {
      m_newEvent = std::make_shared<Event>(SERVER_ERROR, this);
      storage->write(SERVER_IP_STR, "");
  }
  m_reglerApp->id = status.id;
  
  if (!status) {
      D_PRINTLN("");
      *response = m_reglerApp->page1 + "Server not connected!!!!" + m_reglerApp->page2;
  }
  
  return status;
}


void ServerJoin::fillResponseHtml() {
    ReglerApp * app = static_cast<ReglerApp *> (m_app);
    string ip_string = app->m_local_ip.toString().c_str();
  
    stringstream ss;

    Storage * storage= Storage::instance();
    string server_ip = storage->read(SERVER_IP_STR);
    int server_port  = storage->read_int(SERVER_PORT_STR);
    int sensor_id    = storage->read_int(SENSOR_ID_STR);
    int point_id     = storage->read_int(POINT_ID_STR);
    
    ss << "Local IP: " << ip_string << "<br>\n";
    ss << "<form action='/connect_server'>\n";
    ss << "IP: <input type=text name=" << SERVER_IP_STR;
    ss << " value=\"" << server_ip << "\"";
    ss << "><br>\n";
    ss << "port: <input type=number name=" << SERVER_PORT_STR;
    ss << " value=\"" << server_port << "\"";
    ss << "><br>\n";
    ss << "Point id: <input type=numer name=" << POINT_ID_STR;
    ss << " value=\"" << point_id << "\"";
    ss << "><br>\n";
    ss << "Sensor id: <input type=numer name=" << SENSOR_ID_STR;
    ss << " value=\"" << sensor_id << "\"";
    ss << "><br>\n";
    
    ss << "<input type=submit value=Submit>";
    ss << "</form>\n";

    // Reset button
    ss << "<form action=\"/reset\">";
    ss << "<input type=\"submit\" value=\"Reset\">";
    ss << "</form>";

    m_reglerApp->responseHTML = ss.str();
    D_PRINTLN(ss.str().c_str());
}

void ServerJoin::setTime(const int64_t &date)
{
    if (date > 0)
    {
        struct timeval tv;
        tv.tv_sec = date;
        settimeofday(&tv, NULL);
    }
}

void ServerJoin::enter(EspObject */*source*/, Event */*event*/)
{
  Storage * storage = Storage::instance();
  string server_ip = storage->read(SERVER_IP_STR);
  int server_port  = storage->read_int(SERVER_PORT_STR);
  int sensor_id    = storage->read_int(SENSOR_ID_STR);
  int point_id     = storage->read_int(POINT_ID_STR);
  
  if (NOT server_ip.empty()) {
      std::string response;
      bool result = connect(server_ip, server_port, point_id, sensor_id, &response);
      if (!result) {
          std::stringstream ss;
          ss << "Can't connect to Server: " << server_ip << ":" << server_port
             << " sensor id: " << sensor_id
             << " point id: " << point_id;
          app->lastErrMsg = ss.str();
//        return ;
      }
  }
  
  fillResponseHtml();
  m_reglerApp->server->on("/server_join", std::bind(&ServerJoin::handleServerJoin, this));
  m_reglerApp->server->on("/connect_server", std::bind(&ServerJoin::handleSubmit, this));
//  m_reglerApp->server->on("/reset", std::bind(&ServerJoin::handleReset, this));
  m_reglerApp->server->onNotFound(std::bind(&ServerJoin::handleNotFound, this));
  
}
