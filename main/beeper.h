#pragma once

#include "sdkconfig.h"

class Beeper {
public:
  Beeper();
  void start();
  void stop();
  int getBeeperTimeout();

private:
  bool isOn;
  static constexpr float freq = CONFIG_SPEAKER_FREQ;  // Hz
  static const int timeout = CONFIG_SPEAKER_TIMEOUUT; // sec
};
