#include "espobject.h"

EspObject::EspObject(ObjectType type, EspApp *app):
    m_type(type),
    m_app(app)
{
}

EventPtr EspObject::loop()
{
    if (m_newEvent && m_newEvent->m_type != EMPTY_EVENT) {
        return m_newEvent;
    } else {
        return EventPtr();
    }
}

std::map<ObjectType, std::string> EspObject::objectsNames = {
    {EMPTY_OBJECT, "EMPTY"},
    {INITIAL, "INITIAL"}, 
    {WIFI_SPOT, "WIFI_SPOT"},
    {WIFI_CONNECTING, "WIFI_CONNECTING"},
    {SERVER_CONNECTING, "SERVER_CONNECTING"},
    {MONITORING, "MONITORING"},
    {OBJECT_COUNT, "OBJECT_COUNT"}
};

ObjectType EspObject::type() const
{

    return m_type;
}

Initial::Initial(EspApp *app):
    EspObject(INITIAL, app)
{
}

EventPtr Initial::loop()
{
    return std::make_shared<Event>(INITIAL_TRANSITION, this);
}
