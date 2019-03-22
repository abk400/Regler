#include "connecting.h"
#include "event.h"
#include "esp_headers.h"
#include "eventcredentials.h"
#include "reglerapp.h"
#include "storage.h"

using namespace std;
Connecting::Connecting(EspApp *app):
    EspObject(WIFI_CONNECTING, app)
{
    
}

void Connecting::enter(EspObject */*source*/, Event *event)
{
    if (event->m_type == WIFI_CREDENTIALS) {
        EventCredentials * e = static_cast<EventCredentials*>(event);
        ReglerApp * app = static_cast<ReglerApp *> (m_app);
        app->m_name = e->m_arguments["name"];
        app->m_description = e->m_arguments["descr"];
        connectWifi(e->m_arguments["network"], e->m_arguments["pass"]);
    }
}


void Connecting::connectWifi(const string & ssid, const string & password ) {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());
    delay(3000);

    WiFi.begin(ssid.c_str(), password.c_str());

    int timeout = 50;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (--timeout <= 0) {
            Serial.println("WiFi not connected error");
            
            m_newEvent = std::make_shared<Event>(WIFI_ERROR_CONNECTION, this);
            Storage * disk = Storage::instance();
            disk->write("network", "");
            return ;
        }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    ReglerApp * app = static_cast<ReglerApp *> (m_app);
    app->m_local_ip = WiFi.localIP();
    m_newEvent = std::make_shared<Event>(WIFI_CONNECTED, this);
}
