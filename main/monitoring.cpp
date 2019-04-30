#include "monitoring.h"
#include "esp_headers.h"
#include "protocol.h"
#include "reglerapp.h"

Monitoring::Monitoring(EspApp *app):
    EspObject(MONITORING, app)
{
    
}

void Monitoring::enter(EspObject */*source*/, Event */*event*/)
{
    m_doorAccess.startFromMain();
}

void Monitoring::queryEvents()
{
    static int64_t last_poll = 0;
    int64_t now = esp_timer_get_time() / (1000000*60);
    
    if (now - last_poll > 5) {
        last_poll = now;
        MessageQueue queue;
        m_doorAccess.checkOutFromMain(&queue);
            
        if (NOT queue.empty()) {
//            std::list<EntranceEvent> toSend;

            EntranceEvent entrance_in(EntranceEvent::IN, now*60);
            EntranceEvent entrance_out(EntranceEvent::OUT, now*60);

            for (DoorMessage & message : queue) {
                //EntranceEvent entrance(message.delta == -1 ? EntranceEvent::OUT : EntranceEvent::IN, message.timestamp);
                //toSend.push_back(entrance);
                if (message.delta == -1) {
                    entrance_out.count+= 1;
                } else {
                    entrance_in.count+= 1;
                }

            }
            ReglerApp* reglerApp = static_cast<ReglerApp*>(m_app);
            reglerApp->communication.entranceEventMessage(entrance_in, entrance_out);
        }
    }
}

EventPtr Monitoring::loop()
{
    queryEvents();
    return TBase::loop();
}
