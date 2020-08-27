/*
 * FM
 */

#ifndef __FM_H
#define __FM_H

#include <Arduino.h>
extern "C" {
  #include "lcd/lcd.h"
}
#include "tick.hpp"

#define D0 PC15
#define D1 PC14
#define D2 PC13
#define D3 PA0
#define D4 PA1
#define D5 PA2
#define D6 PA3
#define D7 PA4

#define WR PA6
#define A0 PB10
#define IC PB11 // リセット

#define WR_HIGH (GPIO_BOP(GPIOA)  = GPIO_PIN_6)
#define WR_LOW  (GPIO_BC(GPIOA)   = GPIO_PIN_6)

#define A0_HIGH (GPIO_BOP(GPIOB)  = GPIO_PIN_10)
#define A0_LOW  (GPIO_BC(GPIOB)   = GPIO_PIN_10)

#define IC_HIGH (GPIO_BOP(GPIOB)  = GPIO_PIN_11)
#define IC_LOW  (GPIO_BC(GPIOB)   = GPIO_PIN_11)

class FM_class {
 private:
  void wait(uint16_t);

 public:
  FM_class();
  ~FM_class();
  void begin();
  void write(uint8_t addr, uint8_t value);
};

extern FM_class FM;

#endif