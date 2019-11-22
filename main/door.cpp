#include "door.h"
#include "esp_headers.h"
#include "logger.h"

#include "log.h"

#include <string>
#include <map>

using namespace std;


int freq1 = CONFIG_LED1_FREQ;     // Частота ШИМ Channel 0
int freq2 = CONFIG_LED2_FREQ;     // Частота ШИМ Channel 1
int ledChannel1 = 0;  // Канал ШИМ 0
int ledChannel2 = 1;  // Канал ШИМ 1
int resolution = 8;   // Разрешение
// byte PWM_PIN = 32;    // Вход измерения ШИМ (32 канал)
volatile int pwm_value = 0; // Длительнсть измеренного импульса
volatile int prev_time = 0; // Временая переменая для оценки времени
const byte duty = 122; // 50%

Door * Door::instance;

static std::map<DoorState, string> STATE_NAMES = {
    {DoorStateNone, "DoorStateNone"},
	{MON, "MON"},
	{TRY_ENTER1, "TRY_ENTER1"},
	{TRY_ENTER2, "TRY_ENTER2"},
	{TRY_ENTER3, "TRY_ENTER3"},
    
	{TRY_LEAVE1, "TRY_LEAVE1"},
	{TRY_LEAVE2, "TRY_LEAVE2"},
	{TRY_LEAVE3, "TRY_LEAVE3"}
};

static std::map<DoorEvent, string> EVENT_NAMES = {
	{L1_UP_DOWN, "L1_UP_DOWN"},
	{L1_DOWN_UP, "L1_DOWN_UP"},
    
	{L2_UP_DOWN, "L2_UP_DOWN"},
	{L2_DOWN_UP, "L2_DOWN_UP"}
};

Door::Door()
{
    instance = this;
    m_beeper = Beeper::getInstance();
}

void main_run(void *pvParameter) {
    Door * door = (Door*)pvParameter;

    door->setup();

    door->loop();

}

void Door::start()
{
    xTaskCreate(&main_run, "door_task", 2048, (void *)this, 5, NULL);
}

void Door::setup()
{
    D_PRINTLN("Door::setup");

    m_leds[0].pin_frequency = 25;
    m_leds[1].pin_frequency = 27;

    m_leds[0].pin_in_level = 32;
    m_leds[1].pin_in_level = 35;

    m_leds[0].channel = 0;
    m_leds[1].channel = 1;

    fillDoorStates();

    // Channel 0
    ledcAttachPin(m_leds[0].pin_frequency, m_leds[0].channel);
    ledcSetup(m_leds[0].channel, freq1, resolution);
    ledcWrite(m_leds[0].channel, duty);
    // Channel 1
    ledcAttachPin(m_leds[1].pin_frequency, m_leds[1].channel);
    ledcSetup(m_leds[1].channel, freq2, resolution);
    ledcWrite(m_leds[1].channel, duty);

    for (Led& led : m_leds) {
        pinMode(led.pin_in_level, INPUT);
        led.pin_irq_num = digitalPinToInterrupt(led.pin_in_level);
        detachInterrupt(led.pin_irq_num);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void Door::loop()
{
    static unsigned long update = micros();

    int active_channel = 0;

    while (true) {

        unsigned long now = micros();

        int iddle_channel = active_channel ? 0 : 1;

        // Read active line and turn it off
        Led& led = m_leds[active_channel];
        led.barrier_now = gpio_get_level((gpio_num_t)led.pin_in_level);
        ledcDetachPin(led.pin_frequency);
        // Wait for the transients to complete        
        vTaskDelay(2 / portTICK_PERIOD_MS);
        // Turn on iddle line
        Led& iled = m_leds[iddle_channel];
        ledcAttachPin(iled.pin_frequency, iled.channel);

        // Analize cached active line status
        if (led.barrier_now != led.barrier_prev) {
            led.barrier_prev = led.barrier_now;
            led.stabilization_start = micros();
            led.stabilization = true;
        } else {
            if (led.barrier_now == true) {
                if (now - led.stabilization_start > 1000 * 1000 * m_beeper->getBeeperTimeout()) {
                    m_beeper->pipka();
                    if (now - update > 1000 * 1000 * 5) {
                        log_error("led.barrier %d:%d, beeper:On\n", m_leds[0].barrier_now, m_leds[1].barrier_now);
                        update = now;
                    }
                }
            }
        }
        if (led.stabilization) {
            if (now - led.stabilization_start > quarter_of_second) {
                if (led.barrier_now) {
                    handleEvent((DoorEvent)(led.channel * 2 + 1));
                } else if (NOT led.barrier_now) {
                    handleEvent((DoorEvent)(led.channel * 2 + 2));
                }
                led.stabilization = false;
            }
        }
        vTaskDelay(8 / portTICK_PERIOD_MS);
        // Switch channels
        active_channel = active_channel ? 0 : 1;
    }
}

void Door::setEnterHandler(TLeaveEnter enterHandler)
{
    m_enterHandler = enterHandler;
}

void Door::handleEvent(DoorEvent event)
{
    DoorState state = doorStateMachine[m_doorState][event];
//    D_PRINT_F("I: current state: %s, came event %s\n", STATE_NAMES[m_doorState].c_str(), EVENT_NAMES[event].c_str());
    if (state == DoorStateNone) {
        // m_beeper->start();
//        Serial.println("E: invalid transition");
        return;
    }
//    D_PRINT_F("I: state: %s ==> %s\n", STATE_NAMES[m_doorState].c_str(), STATE_NAMES[state].c_str());
    if (m_doorState == TRY_ENTER3 && state == MON) {
        // D_PRINTLN("I: ENTER");
        m_enterHandler(m_peopleCounter, 1);
        m_peopleCounter++;
        m_beeper->pipka();
        log_info("ENTER [pc:%d]",m_peopleCounter);
    } else if (m_doorState == TRY_LEAVE3 && state == MON) {
        // D_PRINTLN("I: LEAVE");
        m_enterHandler(m_peopleCounter, -1);
        m_peopleCounter--;
        m_beeper->pipka();
        log_info("LEAVE [pc:%d]",m_peopleCounter);
    }

    m_doorState = state;
}

void Door::fillDoorStates() {
    memset(doorStateMachine, DoorStateNone, sizeof(doorStateMachine));
    
    doorStateMachine[MON][L1_UP_DOWN] = TRY_ENTER1;
    doorStateMachine[MON][L2_UP_DOWN] = TRY_LEAVE1;
    
    doorStateMachine[TRY_ENTER1][L1_DOWN_UP] = MON;
    doorStateMachine[TRY_ENTER1][L2_UP_DOWN] = TRY_ENTER2;
    
    doorStateMachine[TRY_ENTER2][L1_DOWN_UP] = TRY_ENTER3;
    doorStateMachine[TRY_ENTER2][L2_DOWN_UP] = TRY_ENTER1;
    
    doorStateMachine[TRY_ENTER3][L1_UP_DOWN] = TRY_ENTER2;
    doorStateMachine[TRY_ENTER3][L2_DOWN_UP] = MON;
    
    doorStateMachine[TRY_LEAVE1][L1_UP_DOWN] = TRY_LEAVE2;
    doorStateMachine[TRY_LEAVE1][L2_DOWN_UP] = MON;
    
    doorStateMachine[TRY_LEAVE2][L1_DOWN_UP] = TRY_LEAVE1;
    doorStateMachine[TRY_LEAVE2][L2_DOWN_UP] = TRY_LEAVE3;
    
    doorStateMachine[TRY_LEAVE3][L1_DOWN_UP] = MON;
    doorStateMachine[TRY_LEAVE3][L2_UP_DOWN] = TRY_LEAVE2;
    m_doorState = MON;
}

void Door::led_event(int number, bool up)
{
//    Led & led = m_leds[number];
//    bool barrier = up;
//    if (led.barrier_now != barrier_prev) {
//        led.barrier_prev = barrier;
//        led.stabilization_start = micros();
//    }
//    led.stabilization = true;
}

void DoorAccess::checkOutFromMain(MessageQueue * result)
{
    *result = std::move(queue_main);
    
    if (lock.try_lock()) {
        queue_main = std::move(queue_from_thread);
        lock.unlock();
        result->insert(result->end(), queue_main.begin(), queue_main.end());
    }
}

void DoorAccess::startFromMain()
{
    m_door.setEnterHandler([this] (int count, int delta) {
//        D_PRINT_F("I: door handler count %d, delta %d\n", count, delta);
        //time_t seconds = time(nullptr); // time since the Epoch
        addFromThread(count, delta);
    });
    m_door.start();
}

void DoorAccess::addFromThread(int counter, int delta)
{
    DoorMessage message;
    message.counter = counter;
    message.delta = delta;
    
    lock.lock();
    queue_from_thread.push_back(message);
    lock.unlock();
}
