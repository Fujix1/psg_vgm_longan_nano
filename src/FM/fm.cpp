/*
 * FM
 */

#include "fm.hpp"

FM_class::FM_class() {

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(IC, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(WR, OUTPUT);

}

FM_class::~FM_class() {
}

void FM_class::begin() { // ICリセット
  WR_HIGH;
  A0_LOW;
  IC_LOW;
  Tick.delay_ms(100);
  IC_HIGH;
  Tick.delay_ms(100);
}

void FM_class::write(uint8_t addr, uint8_t value) {
  uint32_t addr_bits = 0;
  uint32_t value_bits = 0;

  //---------------------------------------
  // addr
  A0_LOW;
  WR_LOW;

  GPIO_BC(GPIOA) = 0b00011111;
  GPIO_BOP(GPIOA) = (addr & 0b11111000) >> 3;

  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  if (addr & 0b00000001) {
    addr_bits += GPIO_PIN_15;
  }
  if (addr & 0b00000010) {
    addr_bits += GPIO_PIN_14;
  }
  if (addr & 0b00000100) {
    addr_bits += GPIO_PIN_13;
  }
  GPIO_BOP(GPIOC) = addr_bits;

  WR_HIGH;
  Tick.delay_us(6); // 書き込みピンに応じて調整必要

  //---------------------------------------
  // value
  WR_LOW;
  A0_HIGH;

  GPIO_BC(GPIOA) = 0b00011111;                  // LOW
  GPIO_BOP(GPIOA) = (value & 0b11111000) >> 3;  // HIGH

  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;  // LOW

  if (value & 0b00000001) {
    value_bits += GPIO_PIN_15;
  }
  if (value & 0b00000010) {
    value_bits += GPIO_PIN_14;
  }
  if (value & 0b00000100) {
    value_bits += GPIO_PIN_13;
  }
  GPIO_BOP(GPIOC) = value_bits;  // HIGH

  WR_HIGH;
  Tick.delay_us(6);  // 書き込みピンに応じて調整必要
}

void FM_class::wait(uint16_t us) {
  Tick.delay_ms(us);
}

FM_class FM;