#pragma once
#include <map>
#include <memory>

#include "event.h"

class EspApp;
struct Event;

enum ObjectType{
    EMPTY_OBJECT = 0,
    INITIAL,
    WIFI_SPOT,
    WIFI_CONNECTING,
    SERVER_CONNECTING,
    MONITORING,
    OBJECT_COUNT
};

typedef std::shared_ptr<Event> EventPtr;

class EspObject
{
public:
    static std::map<ObjectType, std::string> objectsNames;
    EspObject(ObjectType type, EspApp * app);
    
    virtual void enter(EspObject * source, Event * event) {}
    virtual EventPtr loop();
    ObjectType type() const;
    
protected:
    ObjectType m_type;
    EspApp * m_app = nullptr;
    EventPtr m_newEvent;
};

class Initial : public EspObject
{
public:
    Initial(EspApp * app);
    EventPtr loop() override;
};
