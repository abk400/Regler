#pragma once
#include <string>
#include <WiFiClient.h>
#include <vector>

struct ResponseBlock
{
    int result = -1;
    int id = -1;
    std::string error_message;
    int64_t begin;
};

enum Direction {
    IN = 0,
    OUT = 1,
}uint8_t;

struct EntranceEvent {
    EntranceEvent(Direction dir, int64_t stamp);
    int64_t stamp;
    uint16_t id;
    uint8_t count;
    Direction dir;
};

typedef std::vector<EntranceEvent> EntranceEventContainer;



struct Protocol
{
public:
    static std::string registrationMessage(int point, int sensor);
    static std::string entranceEventMessage(const EntranceEventContainer &entr);
    static std::string entranceEventMessage(const EntranceEvent &entr_in, const EntranceEvent &entr_out);
    static bool parseResponseMessage(const std::string & message, ResponseBlock * block);
};

typedef bool ReceiveStatus;

struct JoinStatus {
    int id = -1;
    
    operator bool() const {
        return id != -1;
    }
};

struct ServerCommunication
{
public:
    JoinStatus join(std::string ip_str,  int port, int point, int sensor, std::string * debug);
    ReceiveStatus entranceEventMessage(const EntranceEventContainer &entr);
    ReceiveStatus entranceEventMessage(const EntranceEvent &entr_in, const EntranceEvent &entr_out);
    ResponseBlock receiveResponse(std::string * debug);
   
    WiFiClient client;
    int64_t begin_stamp;

private:
    IPAddress m_ip;
    int m_port;
};




