#include "eventcredentials.h"
using namespace std;

EventCredentials::EventCredentials(EspObject * source, std::map<string, string> arguments) :
    Event(WIFI_CREDENTIALS, source),
    m_arguments(arguments)
{
}
