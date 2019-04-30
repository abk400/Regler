#include "httphelper.h"

class ReglerApp;
struct EventCredentials;
class WifiSpot : public HttpHelper
{
public:
    WifiSpot(EspApp * app);
    
    virtual void enter(EspObject * source, Event * event);
    virtual EventPtr loop();
    void handleNotFound();
    void handleRoot();
    void handleRefresh();
    void handleConnect();
    void fillResponseHtml();
    void restartAP();
    void refresh();
    
private:
    EventPtr m_eventCredentials;
    
};
