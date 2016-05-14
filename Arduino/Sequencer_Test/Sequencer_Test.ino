// Sequncer_Test
//
// 周波数をSPIで送信するテスト
//
// PSoC 4 Slave
//      SCK    13
//      MISO   12
//      MOSI   11
//      CS      9
//
// LCD5110 
//      SCK  - Pin 13
//      MOSI - Pin 11
//      DC   - Pin 2
//      RST  - Pin 3
//      CS   - Pin 10
//
// 2016.05.09
//
#include <SPI.h>
#include <MsTimer2.h>
#include <LCD5110_Graph.h>
#include <stdio.h>
#include "scaleTable10.h"

#define SEQUENCE_RATE   500  // ms
#define PIN_PSOC_SS     9
#define TX_PACKET_SIZE  3

#define CMDM_BASE       (0b00000000)
#define CMDM_FREQ_DECI  (CMDM_BASE|0x01)
#define CMDM_PALS_WIDTH (CMDM_BASE|0x02)
#define CMDM_WAV_FORM   (CMDM_BASE|0x03)

LCD5110 myGLCD(13,11,2,3,10);  // SCK, MOSI, DC, RST, CS
extern uint8_t SmallFont[];

// チューリップ
const byte score[] = {
 60, 62, 64, 64,
 60, 62, 64, 64,
 67, 64, 62, 60,
 62, 64, 62, 62
};

byte txBuffer[TX_PACKET_SIZE];

char strBuffer[17];

void sendScore() {
  static int cnt;
  int32_t freq;
  byte freqHi, freqLo;
  char strBuffer[20];
  
  freq = scaleTable10[score[cnt]];
  freqHi = (freq >> 8) & 0xff;
  freqLo = freq & 0xff;

  sprintf(strBuffer, "Note: %d", score[cnt]);
  myGLCD.print(strBuffer, 0, 10);
  sprintf(strBuffer, "FREQ: %d", freq);
  myGLCD.print(strBuffer, 0, 20);
  myGLCD.update();
  
  cnt++;
  if (cnt == sizeof(score)) {
    cnt = 0;
  }
  
  SPI.begin();
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_PSOC_SS, LOW);
  SPI.transfer(CMDM_FREQ_DECI);
  SPI.transfer(freqHi);
  SPI.transfer(freqLo);
  digitalWrite(PIN_PSOC_SS, HIGH);
  SPI.endTransaction();
  SPI.end();
}

void setup() {
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  
  myGLCD.clrScr();
  myGLCD.print("Sequncer Test", 0, 0);
  myGLCD.update();
  
  delay(1000);
  
  pinMode(PIN_PSOC_SS, OUTPUT);
  
  // Timerの初期化
  MsTimer2::set(SEQUENCE_RATE, sendScore);
  MsTimer2::start();
}

void loop() {
  // put your main code here, to run repeatedly:

}
