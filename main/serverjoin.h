#pragma once
#include "httphelper.h"

struct EventCredentials;
class ServerJoin : public HttpHelper
{
public:
    ServerJoin(EspApp * app);
     virtual void enter(EspObject * source, Event * event);
    void handleNotFound();
    void handleServerJoin();
    void handleSubmit();
    void handleReset();
    bool connect(const std::string & server, int port, int id, int sensor_id, std::string *response);
    bool reset(std::string *response);
    void fillResponseHtml();
    void restartAP();
    
private:
    EventPtr m_eventCredentials;

    void setTime(const int64_t &date);
};
