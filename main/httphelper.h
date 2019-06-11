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
protected:
    ReglerApp * m_reglerApp;
};

#endif // HTTPHELPER_H
