#pragma once
#include "espobject.h"

class ReglerApp;
struct EventCredentials;
class ServerJoin : public EspObject
{
public:
    ServerJoin(EspApp * app);
     virtual void enter(EspObject * source, Event * event);
    void handleNotFound();
    void handleServerJoin();
    void handleRefresh();
    void handleSubmit();
    bool connect(const std::string & server, int port, int id, int sensor_id, std::string *response);
    void fillResponseHtml();
    void restartAP();
    
    std::string getResponseHtml();
    
private:
    ReglerApp * m_reglerApp;
    EventPtr m_eventCredentials;
};
