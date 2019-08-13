#include "monitoring.h"
#include "esp_headers.h"
#include "reglerapp.h"
#include "logger.h"

#define MAX_SAVED_EVENTS 240

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
    
    if (now - last_poll > 1) {
        last_poll = now;
        MessageQueue queue;
        m_doorAccess.checkOutFromMain(&queue);
            
        if (NOT queue.empty()) {
            ReglerApp* reglerApp = static_cast<ReglerApp*>(m_app);

            time_t seconds = time(nullptr); // time since the Epoch

            EntranceEvent entrance_in(EntranceEvent::IN, seconds);
            EntranceEvent entrance_out(EntranceEvent::OUT, seconds);

            for (DoorMessage & message : queue) {
                if (message.delta == -1) {
                    entrance_out.count+= 1;
                    reglerApp->statistic.out++;
                } else {
                    entrance_in.count+= 1;
                    reglerApp->statistic.in++;
                }
            }
            bool result = reglerApp->communication.entranceEventMessage(entrance_in, entrance_out);
            reglerApp->statistic.active = result;

            if (result) {
                reglerApp->statistic.sended++;
                if(!m_ee_container.empty())
                    result = reglerApp->communication.entranceEventMessage(m_ee_container);
                if (result)
                    m_ee_container.clear();
            } else {
                reglerApp->statistic.failed_req++;
                if (entrance_in.count > 0)
                    m_ee_container.push_back(entrance_in);
                if (entrance_out.count > 0)
                    m_ee_container.push_back(entrance_out);

                if (m_ee_container.size() > MAX_SAVED_EVENTS) {
                    D_PRINTLN("EntranceEvent container full.");
                    m_ee_container.pop_front();
                    m_ee_container.pop_front();
                }
            }
        }
    }
}

EventPtr Monitoring::loop()
{
    queryEvents();
    return TBase::loop();
}
