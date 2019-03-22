#pragma once
#include <string>
#include <WiFiClient.h>
#include <list>

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
    };
    EntranceEvent(Direction dir, int64_t stamp);
    int64_t id;
    Direction dir;
    int64_t stamp;
};



struct Protocol
{
public:
    static std::string registrationMessage(int point, int sensor);
    static std::string entranceEventMessage(const std::list<EntranceEvent> & entr);
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
    ReceiveStatus entranceEventMessage(const std::list<EntranceEvent> & entr);
    ResponseBlock receiveResponse(std::string * debug);
   
    WiFiClient client;
    int64_t begin_stamp;
};




