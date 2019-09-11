#pragma once

#include "sdkconfig.h"

class Beeper {
public:
  Beeper();
  void start();
  void stop();
  int getBeeperTimeout();
  bool isOn();

private:
  bool m_isOn;
  static constexpr float m_freq = CONFIG_SPEAKER_FREQ;  // Hz
  static const int m_timeout = CONFIG_SPEAKER_TIMEOUUT; // sec
};
