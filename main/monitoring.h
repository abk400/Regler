#include "espobject.h"
#include "door.h"
#include "protocol.h"

class Monitoring : public EspObject
{
    typedef EspObject TBase;
public:

    Monitoring(EspApp * app);
    
    void enter(EspObject * source, Event * event) override;
    
    void queryEvents();
    void saveEvents();
    virtual EventPtr loop() override;
    
private:
    DoorAccess m_doorAccess;

    EntranceEventContainer m_ee_container;
};

