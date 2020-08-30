/*
 * ファイルと vgm 処理関係
 */

#include "file.hpp"

#include "SI5351/SI5351.hpp"
#include "YM2149/YM2149.h"

#define BUFFERCAPACITY 512  // VGMの読み込み単位（バイト）
#define MAXLOOP 2           // 次の曲いくループ数

boolean    mount_is_ok = false;
uint8_t    currentDir;   // 今のディレクトリインデックス
uint8_t    currentFile;  // 今のファイルインデックス
uint8_t    numDirs = 0;  // ルートにあるディレクトリ数
char       **dirs;       // ルートにあるディレクトリの配列
char       ***files;     // 各ディレクトリ内の vgm ファイル名配列
uint8_t    *numFiles;    // 各ディレクトリ内の vgm ファイル数

boolean    fileOpened = false;          // ファイル開いてるか
uint8_t    vgmBuffer1[BUFFERCAPACITY];  // バッファ
uint16_t   bufferPos = 0;               // バッファ内の位置
uint16_t   filesize = 0;                // ファイルサイズ
UINT       bufferSize = 0;              // 現在のバッファ容量

// ファイル処理用
FATFS      fs;
FIL        fil;
FILINFO    fno;
FRESULT    fr;

// VGM状態
uint32_t   vgmLoopOffset;       // ループデータの戻る場所
boolean    vgmLoaded = false;   // VGM データが読み込まれた状態か
boolean    vgmEnd = false;      // 曲終了
uint8_t    vgmLoop = 0;         // 現在のループ回数 
uint32_t   startTime;
uint32_t   duration;

//---------------------------------------------------------------
// 初期化とSDオープン
// ファイル構造の読み込み
boolean sd_init() {
  int i,j,n;

  DIR dir;
  FILINFO fno;

  currentDir = 0;
  currentFile = 0;

  // SD マウントするまで試行
  for (i = 0; i < 20; i++) {
    Tick.delay_ms(200);
    fr = f_mount(&fs, "", 1);
    LCD_ShowNum(0, 64, i, 2, CYAN);
    if (fr == 0) break;
  }

  if (fr == 0) {
    mount_is_ok = true;
    LCD_ShowString(0, 0, (u8 *)("READING DIRS..."), GRAY);

    //-------------------------------------------------------
    // ルートのフォルダ一覧を取得する
    // 数える
    fr = f_findfirst(&dir, &fno, "", "*");
    LCD_ShowString(0, 16, (u8 *)("ROOT"), CYAN);

    while (fr == FR_OK && fno.fname[0]) {  // 数える
      if (!(fno.fattrib & AM_SYS) && !(fno.fattrib & AM_HID) &&
          (fno.fattrib & AM_DIR)) {
        // システムじゃない && 隠しじゃない && ディレクトリ
        numDirs++;
      }
      f_findnext(&dir, &fno);
    }

    // ディレクトリが無い
    if (numDirs == 0) {
      LCD_ShowString(0, 0, (u8 *)("No directory."), WHITE);
      return false;
    }

    // 配列初期化
    dirs = (char **)malloc(sizeof(char *) * numDirs);
    for (i = 0; i < numDirs; i++) {
      dirs[i] = (char *)malloc(sizeof(char) * 13);  // メモリ確保
    }

    n = 0;
    fr = f_findfirst(&dir, &fno, "", "*");
    while (fr == FR_OK && fno.fname[0]) {
      if (!(fno.fattrib & AM_SYS) && !(fno.fattrib & AM_HID) &&
          (fno.fattrib & AM_DIR)) {
        // システムじゃない && 隠しじゃない && ディレクトリ
        strcpy(dirs[n++], fno.fname);
        LCD_ShowString(0, 32, (u8 *)(dirs[n - 1]), CYAN);
      }
      f_findnext(&dir, &fno);
    }

    // 各フォルダ内のファイル数に応じて確保
    files = (char ***)malloc(sizeof(char **) * numDirs);
    numFiles = new uint8_t[numDirs];

    //-------------------------------------------------------
    // フォルダ内ファイル情報取得（数える）
    LCD_ShowString(0, 0, (u8 *)("READING FILES..."), GRAY);

    for (i = 0; i < numDirs; i++) {
      LCD_ShowString(0, 16, (u8 *)("             "), CYAN);
      LCD_ShowString(0, 32, (u8 *)("             "), CYAN);
      LCD_ShowString(0, 16, (u8 *)(dirs[i]), CYAN);

      fr = f_findfirst(&dir, &fno, dirs[i], "*.VGM");
      n = 0;
      while (fr == FR_OK && fno.fname[0]) {
        n++;
        f_findnext(&dir, &fno);
      }

      // ファイル名保持用配列メモリ確保
      if (n > 0) {
        files[i] = (char **)malloc(sizeof(char *) * n);
        for (j = 0; j < n; j++) {
          files[i][j] = (char *)malloc(sizeof(char) * 13);
        }
      }
      // フォルダ内のファイル数保持配列設定
      numFiles[i] = n;
    }

    // ファイル名取得
    if (n > 0) {
      for (i = 0; i < numDirs; i++) {
        fr = f_findfirst(&dir, &fno, dirs[i], "*.VGM");
        n = 0;
        while (fr == FR_OK && fno.fname[0]) {
          strcpy(files[i][n], fno.fname);
          //LCD_ShowString(0, 32, (u8 *)(files[i][n]), CYAN);
          // Tick.delay_ms(16);
          n++;
          f_findnext(&dir, &fno);
        }
      }
    }

    return true;

  } else {
    mount_is_ok = false;
    LCD_ShowString(0, 0, (u8 *)("SD card mount Error"), WHITE);
    return false;
  }
}

boolean closeFile() {
  FRESULT fr;

  if (!mount_is_ok) return false;

  if (fileOpened) {
    fr = f_close(&fil);
    if (fr == FR_OK) {
      fileOpened = false;
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------
// 1バイト返す
uint8_t getByte() {

  if (bufferPos == bufferSize) {  // バッファの終端を超えた
    if (bufferSize == BUFFERCAPACITY) {
      bufferPos = 0;
      f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
    } else {
      LCD_ShowString(0, 64, (u8 *)("EOF"), WHITE);
      return 0;
    }
  }
  return vgmBuffer1[ bufferPos++ ];
}

//----------------------------------------------------------------------
// 16 bit 返す
uint16_t read16() {
  return getByte() + (getByte() << 8);
}

//----------------------------------------------------------------------
// 指定場所の 8 bit 返す
uint8_t read8at(FSIZE_t pos) {
  FSIZE_t currentPos = fil.fptr; // 一時退避
  uint8_t buffer[1];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 1, &br);
  f_lseek(&fil, currentPos);

  return buffer[0];
}

//----------------------------------------------------------------------
// 指定場所の 16 bit 返す
uint16_t read16at(FSIZE_t pos) {
  FSIZE_t currentPos = fil.fptr;  // 一時退避
  uint8_t buffer[2];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 2, &br);
  f_lseek(&fil, currentPos);

  return (uint32_t(buffer[0])) + (uint32_t(buffer[1]) << 8);
}

//----------------------------------------------------------------------
// 指定場所の 32 bit 返す
uint32_t read32at(FSIZE_t pos) {
  FSIZE_t currentPos = fil.fptr;  // 一時退避
  uint8_t buffer[4];
  UINT br;

  f_lseek(&fil, pos);
  f_read(&fil, buffer, 4, &br);
  f_lseek(&fil, currentPos);

  return (uint32_t(buffer[0])) + (uint32_t(buffer[1]) << 8) +
         (uint32_t(buffer[2]) << 16) + (uint32_t(buffer[3]) << 24);
}

//----------------------------------------------------------------------
// VGM のポーズ命令
void pause(uint16_t samples) {
  startTime = Tick.micros2();
  duration = ((1000.0 / (44100.0 / (float)samples)) * 1000);
}


//----------------------------------------------------------------------
// ファイルを開く
boolean openFile(char *path) {

  if (!mount_is_ok) return false;
  
  fileOpened = false;
  filesize = 0;

  LCD_Clear(BLACK);
  LCD_ShowString(0, 0, (u8 *)(path), RED);

  fr = f_open(&fil, path, FA_READ);
  if (fr == FR_OK) {
    fr = f_stat(path, &fno);  // ファイルサイズ取得 (外すとバグるかも)
    filesize = fno.fsize;  

    fileOpened = true;
    return true;
  } else {
    return false;
  }
}

//----------------------------------------------------------------------
// オープンした VGM ファイルを解析して再生準備する
void vgmReady() {

  uint8_t gd3buffer[512];
  UINT i, p;
  char c[2];
  UINT br;
  String gd3[10];

  vgmLoaded = false;
  vgmEnd = false;
  vgmLoop = 0;

  // VGM Version
  uint32_t vgm_version = read32at(8);

  // VGM Loop offset
  vgmLoopOffset = read32at(0x1c);

  // VGM gd3 offset
  uint32_t vgm_gd3_offset = read32at(0x14) + 0x14;
  //LCD_ShowNum(0,32, vgm_gd3_offset, 6,RED);

  // GD3
  if (vgm_gd3_offset != 0) {
    f_lseek(&fil, vgm_gd3_offset + 12);
    f_read(&fil, gd3buffer, 512, &br);

    p = 0;
    i = 0;
    c[1] = '\0';
    for (p = 0; p < br; p += 2) {
      if (gd3buffer[p] == 0 && gd3buffer[p+1] == 0) {
        i++;
        if (i == 10) break;
      } else {
        c[0] = gd3buffer[p];
        gd3[i].concat(c);
      }
    }

    /* GD3 の情報配列
      0 "Track name (in English characters)\0"
      1 "Track name (in Japanese characters)\0"
      2 "Game name (in English characters)\0"
      3 "Game name (in Japanese characters)\0"
      4 "System name (in English characters)\0"
      5 "System name (in Japanese characters)\0"
      6 "Name of Original Track Author (in English characters)\0"
      7 "Name of Original Track Author (in Japanese characters)\0"
      8 "Date of game's release written in the form yyyy/mm/dd, or just yyyy/mm or yyyy if month and day is not known\0"
      9 "Name of person who converted it to a VGM file.\0"
      10 "Notes\0"
    */

    gd3[0].concat(" / ");
    gd3[0].concat(gd3[2]);
    gd3[0] = gd3[0].substring(0, 39);

    gd3[4].concat(" / ");
    gd3[4].concat(gd3[8]);
    gd3[4] = gd3[4].substring(0, 39);

    LCD_ShowString(0, 16, (u8 *)(gd3[0].c_str()), WHITE);
    LCD_ShowString(0, 48, (u8 *)(gd3[4].c_str()), CYAN);
 }

  // Data offset
  // v1.50未満は 0x40、v1.50以降は 0x34 からの相対位置
  uint32_t vgm_data_offset =
      (vgm_version >= 0x150) ? read32at(0x34) + 0x34 : 0x40;
  f_lseek(&fil, vgm_data_offset);

  // Clock
  uint32_t vgm_ay8910_clock =
      (vgm_version >= 0x151 && vgm_data_offset >= 0x78) ? read32at(0x74) : 0;
  if (vgm_ay8910_clock) {
    switch (vgm_ay8910_clock) {
      case 1789772:
      case 1789773:  // ばらついてる
        SI5351.setFreq(SI5351_3579);
        break;
      case 2000000:
        SI5351.setFreq(SI5351_4000);
        break;
      default:
        SI5351.setFreq(SI5351_3579);
        break;
    }
  }
  SI5351.enableOutputs(true); // クロック出力開始
  
  // 初期バッファ補充
  fr = f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
  bufferPos = 0;
  vgmEnd = false;
  vgmLoaded = true;

}


void vgmProcess() {

  if ((Tick.micros2() - startTime) <= duration ) {
    return;
  }
  
  uint8_t reg;
  uint8_t dat;
  byte command = getByte();

  switch (command) {
    case 0xa0: // AY8910, YM2149, YM2203 PSG; val dd, reg aa
      dat = getByte();
      reg = getByte();
      //FM.write(dat, reg);
      SSG.set_register(dat, reg);
      break;
    case 0x61:
      pause (read16());
      break;
    case 0x62:
      pause(735);
      break;
    case 0x63:
      pause(882);
      break;
    case 0x66:
      if (!vgmLoopOffset) {  // ループしない曲
        vgmEnd = true;
        //onButtonDown(btnDOWN);
        if (numFiles[currentDir] - 1 == currentFile)
          openDirectory(1); else playNext(1);

      } else {
        vgmLoop++;
        if (vgmLoop == MAXLOOP) {  // 既定ループ数なら次の曲
          if (numFiles[currentDir] - 1 == currentFile)
          openDirectory(1); else playNext(1);
        } else {
          f_lseek(&fil, vgmLoopOffset + 0x1C);  // ループする曲
          bufferPos = 0;
          f_read(&fil, vgmBuffer1, BUFFERCAPACITY, &bufferSize);
        }
      }
      break;
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
    case 0x76:
    case 0x77:
    case 0x78:
    case 0x79:
    case 0x7A:
    case 0x7B:
    case 0x7C:
    case 0x7D:
    case 0x7E:
    case 0x7F:
      pause((command & 0x0f) + 1);
      break;
    default:
      // Serial.println("unknown command");
      break;
  }

}

//----------------------------------------------------------------------
// ディレクトリ番号＋ファイル番号で VGM を開く
void vgmOpen(int d, int f) {
  char st[64];
  sprintf(st, "%s/%s", dirs[d], files[d][f]);

  if (openFile(st)) {
    vgmReady();
  }
}

//----------------------------------------------------------------------
//  ディレクトリ内の count 個あとの曲再生
void playNext(int count) {
  currentFile = mod(currentFile + count, numFiles[currentDir]);
  vgmOpen(currentDir, currentFile);
}

//----------------------------------------------------------------------
// ディレクトリ開いて最初のファイルを再生
void openDirectory(int count) {
  currentFile = 0;
  currentDir = mod(currentDir + count, numDirs);
  vgmOpen(currentDir, currentFile);
}

int mod(int i, int j) {
  return (i % j) < 0 ? (i % j) + 0 + (j < 0 ? -j : j) : (i % j + 0);
}