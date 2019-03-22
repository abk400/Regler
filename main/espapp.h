#pragma once
#include <list>
#include <vector>
#include "espobject.h"
#include "event.h"

typedef std::vector<std::vector<ObjectType> > TObjectTransitionMap;

class EspApp
{
public:
    EspApp();
    virtual ~EspApp();
    
    void register_map(const TObjectTransitionMap & transitions);
    void register_class(EspObject * obj);
    
    void setup();
    void run();
    
    virtual void loop();
    
private:
    std::map<ObjectType, EspObject *> m_objects;
    TObjectTransitionMap m_transition;
    
    EspObject * m_state;
};

int toInt(std::string str);
