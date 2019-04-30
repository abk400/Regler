#pragma once
#include "logger.h"
#include "espapp.h"
#include "esp_headers.h"
#include "protocol.h"
#include <memory>
class ReglerApp : public EspApp
{
    typedef EspApp TBase;
public:

    virtual void loop() override;
    ReglerApp();
    void serverRestart();
    const byte DNS_PORT = 53;
    IPAddress apIP;
    DNSServer dnsServer;    
    std::unique_ptr<WebServer> server;

    std::string responseHTML;

    std::string responseHTML_Status;
    
    std::string page;
    std::string page1;
    std::string page2;
    
    std::string m_name;
    std::string m_description;
    IPAddress m_local_ip;
    
    int id = -1;
    ServerCommunication communication;

};
