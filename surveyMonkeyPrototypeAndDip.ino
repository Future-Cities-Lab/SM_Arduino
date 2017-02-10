
#include "FastLED.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

/* MUX SWITCHES */
#define s0 20
#define s1 21
#define s2 22

/* LED */
#define LED_1_DATA 14 //2
#define LED_1_CLOCK 15 //3

#define LED_2_DATA 16 //4
#define LED_2_CLOCK 17 //5

#define LED_3_DATA 18 //6
#define LED_3_CLOCK 19 //7

#define LED_4_DATA 2 //8
#define LED_4_CLOCK 3 //9

#define LED_5_DATA 4 //14
#define LED_5_CLOCK 5 //15

#define LED_6_DATA 6 //16
#define LED_6_CLOCK 7 //17

#define LED_7_DATA 8 //18
#define LED_7_CLOCK 9 //19

/* GLOBALS */
#define LEDS_PER_COLUMN 32
#define TOTAL_LEDS 224

int test_c = 0;
int test_v = 0;
int fadeValue = 3;

/* POT */
int sigPin = A9;
int brightnessTestMode = 0;

//// State of each switch (0 or 1)
//int s1state;
//int s2state;
//int s3state;
//int s4state;

// Our number
int x;

EthernetUDP Udp;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

const int LED_IN_BUFFER_SIZE = TOTAL_LEDS*3;
char ledBuffer[LED_IN_BUFFER_SIZE];

int currentModule;

IPAddress ip;
unsigned int localPort = 8888;
//char replyBuffer[] = {'0'};

const unsigned int resetPin = 23;
CRGB leds[TOTAL_LEDS];

int state = 1;
byte mac[] = { 0x04, 0xE9, 0xE5, 0x03, 0x18, 0x01 };

//Functions
void testLEDs();
int readDIPAddress(); 
void resetEthernet();
void getMAC();

void setup() {
  delay(2000);
  // pins for Mux are inputs
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  delay(20);

//Reset Ethernet Module
resetEthernet();

  //currentModule = readDIPAddress();
  currentModule = 0;

  //mac[5] = macs[currentModule];
  ip = IPAddress(192, 168, 1, currentModule + 100);
  //ip = IPAddress(192, 168, 2, 101);
  //getMAC();

  Ethernet.begin(mac, ip);
  delay(200);

  Udp.begin(localPort);
  Serial.begin(9600);

  LEDS.addLeds<WS2801, LED_1_DATA, LED_1_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 0, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_2_DATA, LED_2_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 1, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_3_DATA, LED_3_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 2, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_4_DATA, LED_4_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 3, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_5_DATA, LED_5_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 4, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_6_DATA, LED_6_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 5, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2801, LED_7_DATA, LED_7_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN * 6, LEDS_PER_COLUMN);

//  for (int i = 0; i < TOTAL_LEDS; i++) {
//    leds[i] = CRGB(255, 255, 255);
//  }
//  FastLED.show();

//Set Mux to Trimmer Pot Channel
 digitalWrite(s0, HIGH);
 digitalWrite(s1, HIGH);
 digitalWrite(s2, HIGH);
 

}

void loop() {
  //Serial.println(currentModule);
  brightnessTestMode = map(constrain(analogRead(sigPin), 100, 1023), 100, 1023, 0, 255);
  //Serial.println(brightnessTestMode);
  FastLED.setBrightness(brightnessTestMode);
   if (currentModule == 0) {
     testLEDs();
    } else {
     int packetSize = Udp.parsePacket();
     if (packetSize == 2) {
       Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
       Udp.write((char)(currentModule+100));
       Udp.endPacket();
       state = 2;
     } else if (packetSize == 864) {
       Udp.read((char*)ledBuffer, 864);
       memcpy(leds, ledBuffer, 864);
     } else if (packetSize == 1 || (state == 1 && packetSize != 2)) {
       FastLED.show();
     }
    }
  //testLEDs();
  
}

void testLEDs() {
  CRGB current_value;
      current_value = CRGB(255, 255, 255);

//  switch (test_c) {
//    case 0:
//      current_value = CRGB(test_v, 0, 0);
//      break;
//    case 1:
//      current_value = CRGB(0, test_v, 0);
//      break;
//    case 2:
//      current_value = CRGB(0, 0, test_v);
//      break;
//    case 3:
//      current_value = CRGB(test_v, test_v, test_v);
//      break;
//  }

  for (int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = current_value;
  }

  FastLED.show();
delay(10);
  test_v = test_v + fadeValue;

  if (test_v >= 255) {
    test_v = 255;
    fadeValue = fadeValue * -1;
  } else if (test_v <= 0) {
    test_v = 0;
    fadeValue = fadeValue * -1;
    test_c++;
  }

  if (test_c > 3) {
    test_c = 0;
  }


}

int readDIPAddress() {
  int address = 0;
  int r0[] = {0, 1, 0, 1, 0, 1, 0, 1}; //value of select pin at the 4051 (s0)
  int r1[] = {0, 0, 1, 1, 0, 0, 1, 1}; //value of select pin at the 4051 (s1)
  int r2[] = {0, 0, 0, 0, 1, 1, 1, 1}; //value of select pin at the 4051 (s2)


  for (int i = 0; i < 5; i++) {

    digitalWrite(s0, r0[i]);
    digitalWrite(s1, r1[i]);
    digitalWrite(s2, r2[i]);
    delay(100);

    if ((i == 0) && (analogRead(sigPin) > 512)) {
      address += 2;
    }
    if ((i == 1) && (analogRead(sigPin) > 512)) {
      address += 4;
    }
    if ((i == 2) && (analogRead(sigPin) > 512)) {
      address += 8;
    }
    if ((i == 3) && (analogRead(sigPin) > 512)) {
      address += 1;
    }
    if ((i == 4) && (analogRead(sigPin) > 512)) {
      address += 16;
    }


  }
  Serial.print("DIP Address =>  ");
  Serial.println(address);
  //delay(2000);
  //    Serial.println(muxValue);

 digitalWrite(s0, HIGH);
 digitalWrite(s1, HIGH);
 digitalWrite(s2, HIGH);
return address;

}


//Reset Ethernet Module
void resetEthernet() {

  //set MUX to resetpin channel 6

 digitalWrite(s0, LOW);
 digitalWrite(s1, HIGH);
 digitalWrite(s2, HIGH);
 delay(100);
  
  digitalWrite(resetPin, LOW);
  pinMode(resetPin, OUTPUT);
  delayMicroseconds(10);
  pinMode(resetPin, INPUT);

}

void getMAC(){
   delay(1000);
  
  Serial.println("Reading MAC from hardware...");
  //read_mac();
  
  Serial.print("MAC: ");
  //print_mac();
  Serial.println();

  
  Serial.println("Finished.");
  
}

