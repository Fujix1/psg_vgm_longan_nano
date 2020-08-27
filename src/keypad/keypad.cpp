
#include "keypad.hpp"

KeypadClass::KeypadClass() {
  rcu_periph_clock_enable(RCU_ADC0);
  rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8);
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);

  adc_deinit(ADC0);
  adc_mode_config(ADC_MODE_FREE);
  adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
  adc_channel_length_config(ADC0, ADC_INSERTED_CHANNEL, 1);
  adc_inserted_channel_config(ADC0, 0, ADC_CHANNEL_4, ADC_SAMPLETIME_239POINT5);
  adc_external_trigger_source_config(ADC0, ADC_INSERTED_CHANNEL,
                                     ADC0_1_EXTTRIG_INSERTED_NONE);
  adc_external_trigger_config(ADC0, ADC_INSERTED_CHANNEL, ENABLE);
  adc_enable(ADC0);
  Tick.delay_ms(1);
  adc_calibration_enable(ADC0);
}

//----------------------------------------------------------------------
// ボタンの状態取得
byte KeypadClass::readButton() {
  /*
  ボタン 押して無いとき 4095
  ボタン1 2985 - 2995
  ボタン2 2051 - 2065
  ボタン3 1375 - 1385
  ボタン4 582 - 592
  ボタン5 8 - 20
  */
  adc_software_trigger_enable(ADC0, ADC_INSERTED_CHANNEL);
  int adcData = ADC_IDATA0(ADC0);

  if (adcData > 4080) return btnNONE;       // 4095, 戻り値5
  if (adcData < 40 && adcData >=0) return btnRIGHT;  // 0  , 戻り値0
  if (adcData < 620 && adcData >= 550)  return btnUP;         // 585 - 595 , 戻り値1
  if (adcData < 1400 && adcData >= 1350) return btnDOWN;       // 1375 - 1385 , 戻り値2
  if (adcData < 2100 && adcData >= 2000 ) return btnLEFT;       // 2051 - 2065 , 戻り値3
  if (adcData < 3100 && adcData >= 2900 ) return btnSELECT;     // 2985 - 2995 , 戻り値4
  return btnNONE;
}

byte KeypadClass::checkButton() {
  byte anaInput;
  uint32_t ms = Tick.millis2();
  if (ms > buttonLastTick + BUTTON_INTERVAL) {  // 最後に押したときから
                                                // BUTTON_INTERVAL 経過してるとき
    anaInput = readButton();                   // ボタン取得
    if (lastAnalogInput != anaInput) {         // 前回のボタンと違えば
      lastAnalogInput = anaInput;
      if (anaInput != btnNONE) {
        buttonRepeatStarted = ms;
        //onButtonDown(anaInput);  // ボタン押された処理
        return anaInput;
      }
    } else if (anaInput != 0 && lastAnalogInput == anaInput &&
               ms - buttonRepeatStarted > REPEAT_DELAY) {
      // 同じキーの場合はリピートディレイを過ぎているとき
      //onButtonDown(anaInput);  // ボタン押された処理
      return anaInput;
    }
    buttonLastTick = ms;
  }
  return btnNONE;
}

KeypadClass Keypad;
