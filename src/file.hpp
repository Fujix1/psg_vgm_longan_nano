/*
 * ファイル関係
 */

#ifndef __FILER_H
#define __FILER_H

#include <Arduino.h>
extern "C" {
  #include "fatfs/tf_card.h"
  #include "lcd/lcd.h"
}
#include "tick.hpp"

extern boolean vgmLoaded;  // VGM データが読み込まれた状態か
extern boolean vgmEnd;     // 曲終了

boolean     sd_init();
boolean     closeFile();
uint8_t     getByte();
uint16_t    read16();
uint8_t     read8at(FSIZE_t pos);
uint16_t    read16at(FSIZE_t pos);
uint32_t    read32at(FSIZE_t pos);
void        pause(uint16_t samples);

boolean     openFile(char *);
void        vgmReady();
void        vgmProcess();
void        vgmOpen(int, int);
void        playNext(int);
void        openDirectory(int);
int         mod(int i, int j);
#endif