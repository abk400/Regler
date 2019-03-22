#include "espobject.h"

class Connecting : public EspObject
{
public:
    Connecting(EspApp * app);
    
    void enter(EspObject * source, Event * event) override;
    
private:
    void connectWifi(const std::string &ssid, const std::string &password);
};

