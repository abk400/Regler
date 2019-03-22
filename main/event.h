#pragma once
#include <string>
#include "espobject.h"

class EspObject;
enum EventType{
    EMPTY_EVENT = 0,
    INITIAL_TRANSITION,
    WIFI_CREDENTIALS,
    WIFI_ERROR_CONNECTION,
    WIFI_CONNECTED,
    SERVER_CONNECTED,
    SERVER_ERROR,
    EVENT_COUNT
};

struct Event
{
    static std::map<EventType, std::string> objectsNames;

    EspObject * m_source;
    EventType m_type;
    Event(EventType t);
    Event(EventType t, EspObject * source);
};


