#include "reglerapp.h"
#include "wifispot.h"
#include "connecting.h"
#include "monitoring.h"
#include "serverjoin.h"
//https://dl.espressif.com/doc/esp-idf/latest/get-started/linux-setup.html
//https://docs.espressif.com/projects/esp-idf/en/stable/get-started/
//https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md
//https://github.com/lucadentella/esp32-tutorial

extern "C" void app_main()
{
    Serial.begin(115200);
    D_PRINTLN("P1");
    Serial.flush();
    delay(1000);

    ReglerApp app;
    Serial.flush();
    D_PRINTLN("P2");
    TObjectTransitionMap transitions(OBJECT_COUNT);
    for (std::vector<ObjectType> & obj: transitions) {
        obj.resize(EVENT_COUNT);
    }
    D_PRINTLN("-P3");
    
    transitions[INITIAL][INITIAL_TRANSITION] = WIFI_SPOT;
    transitions[WIFI_SPOT][WIFI_CREDENTIALS] = WIFI_CONNECTING;
    transitions[WIFI_CONNECTING][WIFI_ERROR_CONNECTION] = WIFI_SPOT;
    transitions[WIFI_CONNECTING][WIFI_CONNECTED] = SERVER_CONNECTING;
    transitions[SERVER_CONNECTING][SERVER_CONNECTED] = MONITORING;
    transitions[SERVER_CONNECTING][SERVER_ERROR] = SERVER_CONNECTING;
    
    D_PRINTLN("P4");
    app.register_map(transitions);
    app.register_class(new WifiSpot(&app));
    app.register_class(new Connecting(&app));
    app.register_class(new Initial(&app));
    app.register_class(new Monitoring(&app));
    app.register_class(new ServerJoin(&app));
    D_PRINTLN("P5");
    app.run();
}
