#pragma once
#include <functional>
#include <queue>
#include <list>

#include <mutex>

#include "beeper.h"
// level up   -- means barrier
// level down -- means no barrier


//UP   -- means MISSING barrier
//DOWN -- means barrier
enum DoorState {
    DoorStateNone = 0,
    MON = 1,        /// L1 -- UP,   L2 -- UP 
    TRY_ENTER1 = 2, /// L1 -- DOWN, L2 -- UP 
    TRY_ENTER2 = 3, /// L1 -- DOWN, L2 -- DOWN
    TRY_ENTER3 = 4, /// L1 -- UP,   L2 -- DOWN
    
    TRY_LEAVE1 = 5, /// L1 -- UP,   L2 -- DOWN
    TRY_LEAVE2 = 6, /// L1 -- DOWN, L2 -- DOWN
    TRY_LEAVE3 = 7, /// L1 -- DOWN, L2 -- UP
    DoorStatesCount = TRY_LEAVE3 + 1,
};

enum DoorEvent {
    L1_UP_DOWN = 1,
    L1_DOWN_UP = 2,
    
    L2_UP_DOWN = 3,
    L2_DOWN_UP = 4,
    DoorEventsCount = L2_DOWN_UP + 1,
};
typedef std::function<void (int , int )> TLeaveEnter; // counter, delta
struct DoorMessage {
    int counter = 0;
    int delta = 0;
//    int64_t timestamp = 0;
};
typedef std::list<DoorMessage> MessageQueue;

struct Led {
    bool stabilization = false;
    bool barrier_prev = false;
    bool barrier_now = false;
    unsigned long stabilization_start = 0;
    
    int pin_frequency;
    int pin_in_level;
    int channel;
};
class Door
{
public:
    Door();
    void setup();
    void loop();
    void start();
    
    void setEnterHandler(TLeaveEnter enterHandler);
    
    static Door * instance;
    
    void handleEvent(DoorEvent event);

    Beeper m_beeper;
private:
    void fillDoorStates();

    void led_event(int number, bool up);
    void ledPoll(int number);

    Led m_leds[2];
    
    static const unsigned long quarter_of_second = 1000 * 1000 * 0.05;
    
    DoorState doorStateMachine[DoorStatesCount][DoorEventsCount];
    
    TLeaveEnter m_enterHandler;
    
    DoorState m_doorState = DoorStateNone;
    int m_peopleCounter = 0;
    friend void rising(int );
    friend void falling(int );

};


class DoorAccess {
public:
    void startFromMain();
    void checkOutFromMain(MessageQueue *result );
    void addFromThread(int counter, int delta);
    
private:
    Door m_door;
    MessageQueue queue_main;
    MessageQueue queue_from_thread;
    std::mutex lock;
};

