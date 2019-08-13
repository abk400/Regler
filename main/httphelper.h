#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include "espobject.h"

class ReglerApp;
class HttpHelper : public EspObject
{
public:
    HttpHelper(ObjectType type, EspApp *app);
    virtual void enter(EspObject * source, Event * event) = 0;

    void getResponseHtml(std::string &str);

    std::string getStatisticHtml();
    std::string getCurrentTimeString();


    bool reset(std::string *response);
    void fillStatusResponseHtml();
protected:
    ReglerApp * m_reglerApp;


    static const char * const SERVER_IP_STR;
    static const char * const SERVER_PORT_STR;
    static const char * const SENSOR_ID_STR;
    static const char * const POINT_ID_STR;
};

#endif // HTTPHELPER_H
