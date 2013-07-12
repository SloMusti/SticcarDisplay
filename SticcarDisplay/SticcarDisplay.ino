/*
SticcarDisplay.ino - Control software for interfacing a HT1632 display with hc/05 bluettoth module.

Copyright 2013 Luka Mustafa - Musti, l.mustafa@ee.ucl.ac.uk

The SticcarDisplay is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

The SticcarDisplay is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the SticcarDisplay. If not, see http://www.gnu.org/licenses/.

*/

#include "HT1632.h"

#define DATA 11
#define WR 10
#define CS1 8
#define CS2 7
#define CS3 9
#define CS4 6
#define RD 5

//Blueotooth
#define KEY 14
#define PAIR 13
#define MODE_IN 12
#define EN_BT 2

//Battery
#define BAT_sense 12 //A7

// System variables
#define hwversion 0.1 // this must equal the version printed on the board
#define fwversion 0.1 // this must equal to the version of this code

int v[32]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
int blank[32]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


//***************************************************************
// Pointers to functions called via serial port

// Command "ACOM /x calls" GETfunctions array
// ACOM /0 must always be (int)testGET, where x is a positive integer

// Command "ACOM /x /v" calls SETfunctions array, where v is an integer
// ACOM /0 v must always be (int)testSET

int GETfunctions[2]={(int)testGET,(int)voltageGET};
int* SETfunctions[6 ]={(int*)testSET,(int*)imageSET,(int*)displaySET,(int*)brightnessSET,(int*)nameSET,(int*)pinSET};

HT1632 matrix = HT1632(DATA, WR, CS1);

void setup() {

  // go to UART bluetooth mode
  pinMode(KEY, OUTPUT);
  digitalWrite(KEY,LOW);  
  // turn off bluetooth via pnp
  pinMode(EN_BT, OUTPUT);
  digitalWrite(EN_BT,HIGH);  
  
  //BT configuration
  
  bt_setup(); //this turns on the module as well

  // go to UART bluetooth mode
  //digitalWrite(KEY,LOW);  
  // turn on bluetooth via pnp
  //digitalWrite(EN_BT,HIGH);  
  
  Serial.begin(115200);
  matrix.begin(HT1632_COMMON_16NMOS);

  delay(100); 
  matrix.clearScreen();
  
  analogReference(INTERNAL1V5);
  matrix.fillScreen(&v[0]);

}

void bt_setup(){
   //turn off
  digitalWrite(EN_BT,HIGH);
  delay(50);
  // go to CONFIG bluetooth mode
  digitalWrite(KEY,HIGH); 
  delay(10); 
  // turn on bluetooth via pnp
  digitalWrite(EN_BT,LOW);  
  
  //config mode baudrate
  Serial.begin(38400);
  //test connection
  delay(500);
  Serial.println("AT");
  delay(500);
  Serial.println("AT+NAME=sticcar-2");
  delay(500);
  Serial.println("AT+PSWD=1234");
  delay(500);
  Serial.println("AT+IPSCAN=1024,16,1024,16"); //1024,512,1024,512 default
  delay(500);
  Serial.println("AT+SNIFF=1024,16,10,1024"); //0,0,0,0 default (tmax, tmin, retry, timeout)
  delay(500);
  //turn off   
  digitalWrite(EN_BT,HIGH);
  delay(50);
  // go to UART bluetooth mode
  digitalWrite(KEY,LOW); 
  delay(1); 
  // turn on bluetooth via pnp
  digitalWrite(EN_BT,LOW);  
}

// tests the communication and returns version
void testGET(){
  Serial.print("Sticcar display hw ");
  Serial.print(hwversion);
  Serial.print(" fw ");
  Serial.print(fwversion);
  Serial.print("\n\n"); //terminator  
}

void testSET(int data){
  Serial.print("test write, decimal value: ");
  Serial.print(data);
  matrix.fillScreen(&v[0]);
  Serial.print("\n\n"); //terminator
}

void voltageGET(){
  float sensorValue =0;
  for(char i=0;i<16;i++){
    sensorValue += (float)analogRead(A7)*0.0081112993;
  }
  sensorValue/=16;;
  Serial.print(sensorValue);
  Serial.print("\n\n"); //terminator  
}

void imageSET(int* data){
  matrix.fillScreen(data);
  Serial.print("OK \n\n"); //terminator
}

void nameSET(int* data){
  Serial.print("OK \n\n"); //terminator
}

void pinSET(int* data){
  Serial.print("OK \n\n"); //terminator
}

void displaySET(int* data){
  if(*data){
    matrix.onoff(HIGH);
  }
  else{
    matrix.onoff(LOW);
  }
  Serial.print("OK \n\n"); //terminator
}

void brightnessSET(int* data){
  matrix.setBrightness((uint8_t)*data);
  Serial.print("OK \n\n"); //terminator
}

void loop() {
  //digitalWrite(ledPin, digitalRead(pairPin));

 //_BIS_SR(LPM1_bits | GIE); //Turn on low power mode

  while (Serial.available() > 7) {
    int x = 0;
    // Check for "ACOM /" string
    if(Serial.find("ACOM /")){
      // parse command number after /
      x = Serial.parseInt(); 
      //if termianted, call GET
      if (Serial.read() == '\n'){
        // call GET function
        ((void (*)()) GETfunctions[x])();
      }
      //if not terminated, read value argument and call SET
      else{
        char i=0;
        do{
        v[i++] = Serial.parseInt();
        }
        while(Serial.read() != '\n');
        
        ((void (*)(int*)) SETfunctions[x])(&v[0]); 
      }
      //more arguments can be supported in the same manner
    }
  }
}
