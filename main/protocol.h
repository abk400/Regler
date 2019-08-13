#pragma once
#include <string>
#include <WiFiClient.h>
#include <list>

class ReglerApp;

struct ResponseBlock
{
    int result = -1;
    int id = -1;
    std::string error_message;
    int64_t begin;
};

struct EntranceEvent {
    enum Direction {
        IN = 0,
        OUT = 1,
    }uint8_t;

    EntranceEvent(Direction dir, int64_t stamp);
    int64_t stamp;
    uint16_t id;
    unsigned char count;
    Direction dir;
};

typedef std::list<EntranceEvent> EntranceEventContainer;



struct Protocol
{
public:
    static std::string registrationMessage(int point, int sensor);
    static std::string entranceEventMessage(const int id_sensor, const std::list<EntranceEvent> & entr);
    static std::string entranceEventMessage(const int id_sensor, const EntranceEvent &entr_in, const EntranceEvent &entr_out);
    static bool parseResponseMessage(const std::string & message, ResponseBlock * block);
};

typedef bool ReceiveStatus;

struct JoinStatus {
    int id = -1;
    int64_t time;
    
    operator bool() const {
        return id != -1;
    }
};

struct ServerCommunication
{
public:
    ServerCommunication(ReglerApp *app) : m_app(app) {}

    JoinStatus join(std::string ip_str,  int port, int point, int sensor, std::string * debug);
    ReceiveStatus entranceEventMessage(const std::list<EntranceEvent> & entr);
    ReceiveStatus entranceEventMessage(const EntranceEvent &entr_in, const EntranceEvent &entr_out);
    ResponseBlock receiveResponse(std::string * debug);
   
    WiFiClient client;
    int64_t begin_stamp;

private:
    ReglerApp *m_app;
    IPAddress m_ip;
    int m_port;
};




