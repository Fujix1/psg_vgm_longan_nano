/*
 *  VGM player for YM2149 PSG on Longan Nano
 *  Fujix
 */

#include <Arduino.h>
extern "C" {
  #include "fatfs/tf_card.h"
  #include "lcd/lcd.h"
}
#include "SI5351/SI5351.hpp"
#include "YM2149/YM2149.h"
#include "file.hpp"
#include "tick.hpp"
#include "keypad/keypad.hpp"

void setup() {

  Tick.delay_ms(100);           // SDカード安定用
  Lcd_Init();
  LCD_Clear(BLACK);
  Tick.delay_ms(100);

  Wire.begin();                 // I2C 起動
  Tick.delay_ms(100);

  SI5351.begin();               // SI5351 起動
  Tick.delay_ms(100);
  SSG.reset();                  // YM2149リセット

  sd_init();    // ファイル初期化
  playNext(0);  // 再生開始

}

void loop() {
  byte lastButton;

  // VGMの稼働
  if (!vgmEnd && vgmLoaded) {
    vgmProcess();
  }

  lastButton = Keypad.checkButton();

  if (lastButton != Keypad.btnNONE) {
    switch( lastButton ) {
      case Keypad.btnLEFT:
        openDirectory(+1);
        break;
      case Keypad.btnRIGHT:
        break;
      case Keypad.btnUP:
        playNext(-1);
        break;
      case Keypad.btnDOWN:
        playNext(+1);
        break;
      case Keypad.btnSELECT:
        openDirectory(-1);
        break;
    }
  }

}