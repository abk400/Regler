#pragma once

#include "sdkconfig.h"

class Beeper {
public:
  static Beeper* getInstance();
  void start();
  void stop();
  int getBeeperTimeout();
  bool isOn();

  void pipka();
  
private:
  static Beeper* m_instance;
  Beeper();

  bool m_isOn;
  static constexpr float m_freq = CONFIG_SPEAKER_FREQ;  // Hz
  static const int m_timeout = CONFIG_SPEAKER_TIMEOUT; // sec
  static const int m_pipkatimeout = CONFIG_PIPKA_TIMEOUT; // sec
};
