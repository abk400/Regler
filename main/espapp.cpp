#include <sstream>

#include "espapp.h"
#include "esp_headers.h"
#include "esp_task_wdt.h"

using namespace std;
EspApp::EspApp()
{
    
}

EspApp::~EspApp()
{
}

void EspApp::register_map(const TObjectTransitionMap &transitions)
{
    m_transition = transitions;
}

void EspApp::register_class(EspObject *obj)
{
    m_objects[obj->type()] = obj;
}

void EspApp::setup()
{
    m_state = m_objects[INITIAL];
    m_state->enter(nullptr, nullptr);
}

void EspApp::run()
{
    nvs_flash_init(); 
    setup();

    for(;;) {
        micros(); //update overflow
        loop();
        vTaskDelay(10);
        esp_task_wdt_reset();
    }
    fflush(stdout);
    esp_restart();   
}

void EspApp::loop()
{
    EventPtr ptr = m_state->loop();
    if (ptr) {
        ObjectType objType = m_transition[m_state->type()][ptr->m_type];
        EspObject * obj = m_objects[objType];
        if (obj != m_state) {
            stringstream ss;
            ss << "Event: " << Event::objectsNames[ptr->m_type] << "\n";
            ss << EspObject::objectsNames[m_state->type()] << " ==> " << EspObject::objectsNames[obj->type()] << "\n";
            
            Serial.println(ss.str().c_str());
            
            obj->enter(m_state, &(*ptr));
            m_state = obj;
        }
    }
}

int toInt(std::string str) {
    istringstream sstr(str);
    int number = 0;
    sstr >> number;
    return number;
}
