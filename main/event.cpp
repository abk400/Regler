#include "event.h"


Event::Event(EventType t):
    m_type(t)
{
}
Event::Event(EventType t, EspObject * source):
    m_source(source),
    m_type(t)
{

}

std::map<EventType, std::string> Event::objectsNames = {
    {EMPTY_EVENT, "EMPTY_EVENT"},
    {INITIAL_TRANSITION, "INITIAL_TRANSITION"}, 
    {WIFI_CREDENTIALS, "WIFI_CREDENTIALS"},
    {WIFI_ERROR_CONNECTION, "WIFI_ERROR_CONNECTION"},
    {WIFI_CONNECTED, "WIFI_CONNECTED"},
    {SERVER_CONNECTED, "SERVER_CONNECTED"},
    {SERVER_ERROR,     "SERVER_ERROR"},
    {EVENT_COUNT, "EVENT_COUNT"}
};
