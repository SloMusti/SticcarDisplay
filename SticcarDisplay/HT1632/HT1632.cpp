#include "HT1632.h"


#define swap(a, b) { uint16_t t = a; a = b; b = t; }

//char image[48]={0b00000000, 0b10000000, 0b00000000, 0b01000000, 0b00000000, 0b01100000, 0b00000000, 0b01010000, 0b00000000, 0b01011000, 0b00000000, 0b00011000, 0b00000000, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00011110, 0b00000000, 0b00010000, 0b11111111, 0b11111111, 0b00000000, 0b00010000, 0b00000000, 0b00010000, 0b00000000, 0b00011110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000};

HT1632::HT1632(int8_t data, int8_t wr, int8_t cs, int8_t rd) {
  _data = data;
  _wr = wr;
  _cs = cs;
  _rd = rd;

  for (uint8_t i=0; i<48; i++) {
    ledmatrix[i] = 0;
  }
}

void HT1632::begin(uint8_t type) {
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);
  pinMode(_wr, OUTPUT);
  digitalWrite(_wr, HIGH);
  pinMode(_data, OUTPUT);
  
  if (_rd >= 0) {
    pinMode(_rd, OUTPUT);
    digitalWrite(_rd, HIGH);
  }

  sendcommand(HT1632_SYS_EN);
  sendcommand(HT1632_LED_OFF);
  sendcommand(HT1632_BLINK_OFF);
  sendcommand(HT1632_MASTER_MODE);
  sendcommand(HT1632_INT_RC);
  sendcommand(type);
  sendcommand(HT1632_PWM_CONTROL | 0xF);
  
  WIDTH = 24;
  HEIGHT = 16;
}

void HT1632::setBrightness(uint8_t pwm) {
  if (pwm > 15) pwm = 15;
  sendcommand(HT1632_PWM_CONTROL | pwm);
}

void HT1632::blink(boolean blinky) {
  if (blinky) 
    sendcommand(HT1632_BLINK_ON);
  else
    sendcommand(HT1632_BLINK_OFF);
}

void HT1632::onoff(boolean onoff) {
  if (onoff) 
    sendcommand(HT1632_LED_ON);
  else
    sendcommand(HT1632_LED_OFF);
}

void HT1632::dumpScreen() {
  Serial.println("---------------------------------------");

  for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i++) {
    Serial.print("0x");
    Serial.print(ledmatrix[i], HEX);
    Serial.print(" ");
    if (i % 3 == 2) Serial.println();
  }

  Serial.println("\n---------------------------------------");
}

void HT1632::writeScreen() {

  digitalWrite(_cs, LOW);

  writedata(HT1632_WRITE, 3);
  // send with address 0
  writedata(0, 7);

  for (uint16_t i=0; i<(WIDTH*HEIGHT/8); i+=2) {
    uint16_t d = ledmatrix[i];
     d <<= 8;
     d |= ledmatrix[i+1];
    
    writedata(d, 16);
  }
  digitalWrite(_cs, HIGH);
}


void HT1632::clearScreen() {
  for (uint8_t i=0; i<(WIDTH*HEIGHT/8); i++) {
    ledmatrix[i] = 0;
  }
  writeScreen();
}


void HT1632::writedata(uint16_t d, uint8_t bits) {
  pinMode(_data, OUTPUT);
  for (uint8_t i=bits; i > 0; i--) {
    digitalWrite(_wr, LOW);
   if (d & (0x01<<(i-1))) {
     digitalWrite(_data, HIGH);
   } else {
     digitalWrite(_data, LOW);
   }
  digitalWrite(_wr, HIGH);
  }
  pinMode(_data, INPUT);
}




void HT1632::writeRAM(uint8_t addr, uint8_t data) {
  //Serial.print("Writing 0x"); Serial.print(data&0xF, HEX);
  //Serial.print(" to 0x"); Serial.println(addr & 0x7F, HEX);

  uint16_t d = HT1632_WRITE;
  d <<= 7;
  d |= addr & 0x7F;
  d <<= 4;
  d |= data & 0xF;
 
  digitalWrite(_cs, LOW);
  writedata(d, 14);
  digitalWrite(_cs, HIGH);
}


void HT1632::sendcommand(uint8_t cmd) {
  uint16_t data = 0;
  data = HT1632_COMMAND;
  data <<= 8;
  data |= cmd;
  data <<= 1;
  
  digitalWrite(_cs, LOW);
  writedata(data, 12);
  digitalWrite(_cs, HIGH);  
}


void HT1632::fillScreen(int* matrix) {
  digitalWrite(_cs, LOW);

  writedata(HT1632_WRITE, 3);
  // send with address 0
  writedata(0, 7);
  
  //reverse order
  matrix+=23;
  
  for (uint8_t i=0; i<24; i++) {
    writedata(*matrix, 16);
    matrix--;//order reversed
  }
  digitalWrite(_cs, HIGH);  
}
