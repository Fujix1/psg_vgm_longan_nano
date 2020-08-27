#include "YM2149.h"

YM2149::YM2149(){

  // Data Pins
  //
  // D0 - PC15
  // D1 - PC14
  // D2 - PC13
  // D3 - PA0
  // D4 - PA1
  // D5 - PA2
  // D6 - PA3
  // D7 - PA8 (元PA4)

  //  Pins
  // WRCS_PIN PA11
  // A0_PIN PB10 -> 
  // RESET_PIN PB11  ->IC

  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
            GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_8);
  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,
            GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13);

  pinMode(WRCS_PIN,   OUTPUT);
  pinMode(A0_PIN,     OUTPUT);
  pinMode(RESET_PIN,  OUTPUT);
}

void YM2149::reset(void) {
  uint8_t i;
  for (i = 0; i <= 0x0d; i++) {
    set_register(i, 0);
  }

  digitalWrite(WRCS_PIN,  HIGH);
  digitalWrite(A0_PIN,    LOW);
  digitalWrite(RESET_PIN, LOW);
  Tick.delay_ms(10);
  digitalWrite(RESET_PIN, HIGH);
}


void YM2149::set_register(byte addr, byte data) {
  uint32_t addr_bits = 0;
  uint32_t data_bits = 0;

  //---------------------------------------
  // address

  // digitalWrite(WRCS_PIN, HIGH);
  // digitalWrite(A0_PIN, HIGH);
  GPIO_BOP(GPIOA) = GPIO_PIN_11;
  GPIO_BOP(GPIOB) = GPIO_PIN_10;

  if (addr & 0b00000001) {
    addr_bits += GPIO_PIN_15;
  }
  if (addr & 0b00000010) {
    addr_bits += GPIO_PIN_14;
  }
  if (addr & 0b00000100) {
    addr_bits += GPIO_PIN_13;
  }
  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_BOP(GPIOC) = addr_bits;

  GPIO_BC(GPIOA) = 0b100001111;  // LOW
  GPIO_BOP(GPIOA) = (addr & 0b01111000) >> 3;
  if (addr & 0b10000000) {
    GPIO_BOP(GPIOA) = 0b100000000;
  }

  //digitalWrite(WRCS_PIN, LOW);
  //digitalWrite(A0_PIN, LOW);
  //Tick.delay_us(1);
  GPIO_BC(GPIOA) = GPIO_PIN_11;  // LOW
  GPIO_BC(GPIOB) = GPIO_PIN_10;  // LOW

  //---------------------------------------
  // value
  //digitalWrite(WRCS_PIN, HIGH);
  GPIO_BOP(GPIOA) = GPIO_PIN_11;

  if (data & 0b00000001) {
    data_bits += GPIO_PIN_15;
  }
  if (data & 0b00000010) {
    data_bits += GPIO_PIN_14;
  }
  if (data & 0b00000100) {
    data_bits += GPIO_PIN_13;
  }
  GPIO_BC(GPIOC) = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;  // LOW
  GPIO_BOP(GPIOC) = data_bits;                               // HIGH

  GPIO_BC(GPIOA) = 0b100001111;                 // LOW
  GPIO_BOP(GPIOA) = (data & 0b01111000) >> 3;

  if (data & 0b10000000) {
    GPIO_BOP(GPIOA) = 0b100000000;
  }

  //Tick.delay_us(1);
  //digitalWrite(WRCS_PIN, LOW);
  GPIO_BC(GPIOA) = GPIO_PIN_11;  // LOW
  
}

YM2149 SSG;