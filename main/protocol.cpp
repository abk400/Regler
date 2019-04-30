#define ARDUINOJSON_ENABLE_STD_STREAM 1
#define ARDUINOJSON_ENABLE_STD_STRING 1

#include "logger.h"
#include "protocol.h"
#include "ArduinoJson.h"
#include "ArduinoJson/Serialization/StreamPrintAdapter.hpp"
#include "esp_headers.h"

#include <thread>
#include <sstream>
#include <string>
#include <stdio.h>
using namespace std;
std::string Protocol::registrationMessage(int point, int sensor)
{
    char char_buffer[1024];
    char_buffer[sizeof(char_buffer) - 1] = '/0';
    DynamicJsonBuffer buffer;
    JsonObject& root = buffer.createObject();
    root["type"] = "registration";
    root["point"] = point;
    root["sensor"] = sensor;
    root.printTo(char_buffer, sizeof(char_buffer) - 1);
    printf("%s\n", char_buffer);
    return char_buffer;
    
}

string Protocol::entranceEventMessage(const std::list<EntranceEvent> &entr)
{
    string str;
    
    DynamicJsonBuffer buffer;
    JsonObject& root = buffer.createObject();
    root["type"] = "event";
    
    JsonArray & array= root.createNestedArray("data");
    for (const EntranceEvent & event : entr) {
        JsonObject& nested = array.createNestedObject();
        nested["id"] = (int) event.id;
        nested["time"] = (long)event.stamp;
        nested["direction"] = (event.dir == EntranceEvent::IN) ? 0 : 1;
    }
    
    root.printTo(str);
    printf("entranceEventMessage %s\n", str.c_str());
    return str;
}

string Protocol::entranceEventMessage(const EntranceEvent &entr_in, const EntranceEvent &entr_out)
{
    string str;

    DynamicJsonBuffer buffer;
    JsonObject& root = buffer.createObject();
    root["type"] = "event";

    JsonArray & array= root.createNestedArray("data");
//    for (const EntranceEvent & event : entr) {
    if (entr_in.count > 0) {
        JsonObject& nested = array.createNestedObject();
        nested["id"] = (int) entr_in.id;
        nested["time"] = (long)entr_in.stamp;
        nested["direction"] = entr_in.dir;
        nested["count"] = entr_out.count;
    }

    if (entr_out.count > 0) {
        JsonObject& nested = array.createNestedObject();
        nested["id"] = (int) entr_out.id;
        nested["time"] = (long)entr_out.stamp;
        nested["direction"] = entr_out.dir;
        nested["count"] = entr_out.count;
    }

    root.printTo(str);
    printf("entranceEventMessage %s\n", str.c_str());
    return str;
}

bool Protocol::parseResponseMessage(const std::string &message, ResponseBlock *block)
{
    DynamicJsonBuffer buffer;
    JsonObject& root = buffer.parseObject(message.c_str());    
    
    if (NOT root.success()) return false;
    
    block->id = root["id"];
    const char * error = root.get<const char *>("errorMsg");
    block->error_message =  error ? error : "";
    block->result = root["result"];
    const char * time = ((const char *) root["time"]) ? ((const char *) root["time"]) : "";
    block->begin = strtoull(time, 0, 0);
    
    return true;
}

JoinStatus ServerCommunication::join(std::string ip_str, int port, int point, int sensor, std::string * debug)
{
  m_ip.fromString(ip_str.c_str());
  m_port = port;
  JoinStatus status;
  if (client.connect(m_ip, m_port)) {
      D_PRINTLN("Connected");
      stringstream ss2;
      ss2 << "Connecting " << ip_str << " succeeded";
      string message = Protocol::registrationMessage(point, sensor);
      client.println(message.c_str());
      ResponseBlock response = receiveResponse(debug);
      status.id = response.id;
      ss2 << "id: " << status.id;
      client.stop();
  }

  return status;
}

ReceiveStatus ServerCommunication::entranceEventMessage(const std::list<EntranceEvent> &entr)
{
    std::string eventsBuffer = Protocol::entranceEventMessage(entr);
    
    client.println(eventsBuffer.c_str());
    std::string debug;
    ResponseBlock response = receiveResponse(&debug);

    return response.result == 0;
}

ReceiveStatus ServerCommunication::entranceEventMessage(const EntranceEvent &entr_in, const EntranceEvent &entr_out)
{
    bool result = false;
    std::string eventsBuffer = Protocol::entranceEventMessage(entr_in, entr_out);

    if (client.connect(m_ip, m_port)) {
        D_PRINTLN("Connected");
        client.println(eventsBuffer.c_str());
        std::string debug;
        ResponseBlock response = receiveResponse(&debug);
        client.stop();
        result = response.result==0;
    } else {
        D_PRINTLN("Can't connect");
    }

    //! @todo if result not ok, save data

    return result;
}

//#define MINUTES_15 (15*60)

//void saveEntranceEvents(const std::list<EntranceEvent> &entr) {
////    std::list<EntranceEvent> toSave;
//    std::map<int,EntranceEvent> toSave[2];

//    for (const EntranceEvent & event : entr) {
//        JsonObject& nested = array.createNestedObject();
//        nested["id"] = (int) event.id;
//        nested["time"] = (long)event.stamp;
//        nested["direction"] = (event.dir == EntranceEvent::IN) ? 0 : 1;

//        toSave[event.dir][(long)(event.stamp/MINUTES_15)] =  ;
//    }

//}

ResponseBlock ServerCommunication::receiveResponse(std::string * debug)
{
    ResponseBlock block;

    unsigned char buffer[1024] = "";
    int size = -1;
    for (int timeout = 5; timeout > 0; timeout--) {
        if (client.available() > 0) {
            size = client.read(buffer, sizeof(buffer) - 1);
            break;
        }

        D_PRINTLN("Sleeping 1 sec...");
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }
    Serial.printf("Available %d\n", client.available());

//    int size = (client.available() > 0) ? client.read(buffer, sizeof(buffer) - 1) : -1;
    stringstream ss3;
    if (size != -1) {
      buffer[size] = '\0';
      ss3 << buffer << "\n";
      Protocol::parseResponseMessage((const char *)buffer, &block);
      if (block.result != 0) {
          ss3 << "Server error " << block.result << " " << block.error_message;
      }
    } else {
      ss3 << "Nothing responded !!!\n";
    }
    
    *debug = ss3.str();
    return block;
}


EntranceEvent::EntranceEvent(EntranceEvent::Direction dir, int64_t stamp):
    dir(dir),
    stamp(stamp)
{
    static int id_counter = 0;
    
    id_counter ++;
    count = 0;
}
