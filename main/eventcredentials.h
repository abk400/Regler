#include <string>
#include "event.h"

struct EventCredentials : public Event
{
    EventCredentials(EspObject *source, std::map<std::string, std::string> arguments);
    
    std::map<std::string, std::string> m_arguments;
};
