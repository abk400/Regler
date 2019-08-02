#include "monitoring.h"
#include "esp_headers.h"
#include "reglerapp.h"
#include "logger.h"
#include "storage.h"

#define MAX_SAVED_EVENTS 240

static const char * const EE_CONTAINER_KEY     = "ee_container";

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
            ReglerApp* reglerApp = static_cast<ReglerApp*>(m_app);

            EntranceEvent entrance_in(EntranceEvent::IN, now*60);
            EntranceEvent entrance_out(EntranceEvent::OUT, now*60);

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

                if (m_ee_container.size() < MAX_SAVED_EVENTS) {
                    if (entrance_in.count > 0)
                        m_ee_container.push_back(entrance_in);
                    if (entrance_out.count > 0)
                        m_ee_container.push_back(entrance_out);
                } else {
                    D_PRINTLN("EntranceEvent container full.");
                }
            }
        }
    }
}

// sa
void Monitoring::saveEvents()
{
    static int64_t last_poll = 0;

    if (m_ee_container.empty())
        return;

    int64_t now = esp_timer_get_time() / (1000000*60);
    if (now - last_poll > 60) {
        last_poll = now;
        Storage * storage= Storage::instance();
        size_t size= m_ee_container.size() * sizeof(EntranceEvent);
        storage->write_EE(EE_CONTAINER_KEY, (void*)&m_ee_container[0], size);
    }
}

EventPtr Monitoring::loop()
{
    queryEvents();
    return TBase::loop();
}
