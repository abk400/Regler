#include "espobject.h"

class ReglerApp;
struct EventCredentials;
class WifiSpot : public EspObject
{
public:
    WifiSpot(EspApp * app);
    
    virtual void enter(EspObject * source, Event * event);
    virtual EventPtr loop();
    void handleNotFound();
    void handleRoot();
    void handleRefresh();
    void handleConnect();
//    void connectWifi();
    void fillResponseHtml();
    void restartAP();
    void refresh();
    std::string getResponseHtml();
//    void connectWifi(const std::string & ssid, const std::string & password );
    
private:
    ReglerApp * m_reglerApp;
    EventPtr m_eventCredentials;
    
};
