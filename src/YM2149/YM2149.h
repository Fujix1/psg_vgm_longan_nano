#ifndef YM2149_H
#define YM2149_H

#include <Arduino.h>

#include "tick.hpp"

//  Pins
#define WRCS_PIN    PA11
#define A0_PIN      PB10
#define RESET_PIN   PB11  // IC/RESET

class YM2149 {
 public:
  YM2149();
  void reset(void);
  void set_register(byte addr, byte value);
 private:
};

extern YM2149 SSG;

#endif
