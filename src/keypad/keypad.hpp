/*
   キーパッド用ライブラリ
   基本構成は Arduino 用 5つボタンキーパッドと同じ構成

   ADC info thanks to
   https://jhalfmoon.com/dbc/2020/07/12/%E9%B3%A5%E3%81%AA%E3%81%8D%E9%87%8C%E3%81%AE%E3%83%9E%E3%82%A4%E3%82%B3%E3%83%B3%E5%B1%8B91-longan-nano%E3%80%81adc-%E5%8D%98%E7%99%BA%E5%A4%89%E6%8F%9B/

  ボタン 押して無いとき 4095
  ボタン1 2985 - 2995
  ボタン2 2051 - 2065
  ボタン3 1375 - 1385
  ボタン4 582 - 592
  ボタン5 8 - 20

  回路
           3.3V
            │
          [2kΩ]
   (ADCへ)──┼───(ボタン1)── GND
          [330Ω]
            ├───(ボタン2)── GND
          [660Ω]
            ├───(ボタン3)── GND
          [1kΩ]
            ├───(ボタン4)── GND
          [3.3kΩ]
            └───(ボタン5)── GND
 */

#ifndef __KEYPAD_H
#define __KEYPAD_H

#include <Arduino.h>

#include "tick.hpp"

#define BUTTON_INTERVAL 64   // ボタン取得間隔 ms
#define REPEAT_DELAY 500     // ボタンリピート開始まで ms

class KeypadClass {
 private:
  uint32_t buttonLastTick = 0;         // 最後にボタンが押された時間
  uint32_t buttonRepeatStarted = 0;    // リピート開始時間
  byte lastAnalogInput = btnNONE;
  byte readButton();
 public:
  enum Button { btnNONE, btnRIGHT, btnUP, btnDOWN, btnLEFT, btnSELECT };
  KeypadClass();
  byte checkButton();
};

extern KeypadClass Keypad;

#endif
