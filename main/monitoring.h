#include "espobject.h"
#include "door.h"

class Monitoring : public EspObject
{
    typedef EspObject TBase;
public:

    Monitoring(EspApp * app);
    
    void enter(EspObject * source, Event * event) override;
    
    void queryEvents();
    virtual EventPtr loop() override;
    
private:
    DoorAccess m_doorAccess;
};

