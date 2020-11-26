/*
  Nixie Tube Driver Board (NTDB) 4-Nixie Tube Clock Example

This example demonstrates a complete clock using 4 nixie tubes, 3 tactile buttons and 2 colon bulbs.

Can drive more tubes when multiple NTDB boards connected in series.

In order to make a complete clock, you will need:
1) This KIT: https://nixie.ai/ntdb4
2) 4x nixie tubes
3) 1x NCH6300HV: https://nixie.ai/nch6300hv
4) 1x Arduino Uno board
5) 1x Real Time Clock module (DS1307, DS3231, etc)
6) Some jumper wires

 
 Circuit:
 Connect the NTDB to Arduino Uno:
 --------------------------------
    NTDB        Arduino Pins
 --------------------------------
    GND         GND
    DC5V
    DATA        11
    OE          10
    STCP        8
    SHCP        12
    COLON       5
    ON/OFF      6 (HVEnable)
 --------------------------------
Connect the 12V DC power to the NTDB board 


Nixie Tube Driver Board (NTDB) Arduino Library released at
https://github.com/omnixie-electronics/Nixie-Tube-Driver-Board-Arduino-Library
with short link: https://nixie.ai/ntdb4github

First released in public domain on Thanksgiving day, 11/26/2020

Library and example code created
 by Aiden Fang
 www.Omnixie.com

 NTDB board designed by Zeyuan, Yan
 www.Omnixie.cn

 */

#include <Wire.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <Omnixie_NTDB.h>

Omnixie_NTDB nixieClock(11, 8, 12, 10, 6, 5, 1);
// pin_DataIN, pin_STCP(latch), pin_SHCP(clock), pin_Blank(Output Enable; PWM pin preferred),
// HVEnable pin, Colon pin, number of Nixie Tube Driver Boards
// PWM Pins on Arduino Uno: 3, 5, 6, 9, 10, 11; PWM FREQUENCY 490 Hz (pins 5 and 6: 980 Hz)

bool isColonFlashing = true;
bool colonOn = true;

const int btn1Pin = 2;
const int btn2Pin = 3;
const int btn3Pin = 4;

int btn1State = 0;
int btn2State = 0;
int btn3State = 0;

int prevSec = 0;

int clockState = 0;
int clockStatePrev = 0;



//EEPROM
bool eepromIs12HFormat = true;
bool is12HFormat = true;
bool eepromIsMMDDFormat = true;
bool isMMDDFormat = true;
byte eepromBrightnessLevel;
byte brightnessLevel;

const byte brightnessLevelMin = 0x05;
const byte brightnessLevelMax = 0xff;

int counter = 0;
int nTimeOut = 1200;

//button debouce and long press
//Button Press Detection
long longPressTime = 500;
long button1TimerStart = 0;
long button2TimerStart = 0;
long button3TimerStart = 0;
boolean button1StillPressed = false;
boolean button2StillPressed = false;
boolean button3StillPressed = false;
boolean longPress1Active = false;
boolean longPress2Active = false;
boolean longPress3Active = false;

unsigned int currentFlashingNum = 0;

void setup() {
  is12HFormat = EEPROM.read(eepromIs12HFormat);
  brightnessLevel = EEPROM.read(eepromBrightnessLevel);

  nixieClock.setHVPower(true);
  if (brightnessLevel < brightnessLevelMin) brightnessLevel = brightnessLevelMin;
  nixieClock.setBrightness(brightnessLevel);


  pinMode(btn1Pin, INPUT_PULLUP);
  pinMode(btn2Pin, INPUT_PULLUP);
  pinMode(btn3Pin, INPUT_PULLUP);


  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  CathodePoisoningPrevention(3, 100);

}

void loop()
{
  if (brightnessLevel < brightnessLevelMin) brightnessLevel = brightnessLevelMin;
  nixieClock.setBrightness(brightnessLevel);  // brightness control, 0x00(off)-0xff

  tmElements_t tm;
  if (RTC.read(tm)) {

  } else {
    if (RTC.chipPresent()) {
      Serial.println("DS1307 stopped. Please run the SetTime to initialize");
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
    }
  }

  // read the state of the pushbutton value:
  btn1State = digitalRead(btn1Pin);
  btn2State = digitalRead(btn2Pin);
  btn3State = digitalRead(btn3Pin);
  //======================================================= BUTTON 1 ========================================
  if (btn1State == LOW) { //btn1 goes LOW when pressed
    if (button1StillPressed == false) {
      Serial.println("===============///////////////    SHORT PRESS 1 START ++++  ");
      button1StillPressed = true;
      button1TimerStart = millis();
    }
    if ((millis() - button1TimerStart > longPressTime) && (longPress1Active == false)) { // this is long press
      longPress1Active = true;

      Serial.println("===============///////////////    LONG PRESS 1 START ++++++++++++++++  ");
      if (clockState < 10) {
        clockState = 10;
      } else {
        clockState = 0;
      }
      Serial.print("clockstate = "); Serial.println(clockState);

    }
  } else { // when btn1 is released

    if (button1StillPressed == true) {
      if (longPress1Active == true) { //released after long press
        Serial.println("===============///////////////    LONG PRESS 1 release ++++++++++++++++  ");
        longPress1Active = false;

      } else { //released after short press
        Serial.println("===============///////////////    SHORT PRESS 1 release ----  ");

        switch (clockState) {
          case 0: // HH:MM Display
          case 1: // :SS Display
          case 2: // MMDD Display
          case 3: // YYYY Display
            clockState ++;
            if (clockState == 4) clockState = 0;
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 10: // Hours Flashing (colon stay on)
            clockState ++;
            tm.Hour = currentFlashingNum;
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 11: // Minutes Flashing (colon stay on)
            clockState ++;
            tm.Minute = currentFlashingNum;
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 12: // Seconds Flashing and running (colon stay on)
            clockState ++;
            tm.Second = currentFlashingNum;
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 13: // Month Flashing (colon stay off)
            clockState ++;
            tm.Month = currentFlashingNum;
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 14: // Date Flashing (colon stay off)
            clockState ++;
            tm.Day = currentFlashingNum;
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
          case 15: // Year Flashing (colon stay off)
            clockState = 10;
            Serial.println(currentFlashingNum);
            tm.Year = CalendarYrToTm(currentFlashingNum);
            RTC.write(tm);
            Serial.print("clockstate = "); Serial.println(clockState);
            break;
        }

      }
      button1StillPressed = false;
    }
  }
  //======================================================= BUTTON 2 ========================================
  if (btn2State == LOW) { //btn2 goes LOW when pressed
    if (button2StillPressed == false) {
      Serial.println("===============///////////////    SHORT PRESS 2 START ++++  ");
      button2StillPressed = true;
      button2TimerStart = millis();
    }
    if ((millis() - button2TimerStart > longPressTime) && (longPress2Active == false)) { // this is long press
      longPress2Active = true;
      Serial.println("===============///////////////    LONG PRESS 2 START ++++++++++++++++  ");
      brightnessLevel += 25;
      if (brightnessLevel > brightnessLevelMax) brightnessLevel = brightnessLevelMin;
      nixieClock.setBrightness(brightnessLevel);
    }
  } else { // when btn2 is released

    if (button2StillPressed == true) {
      if (longPress2Active == true) { //released after long press
        Serial.println("===============///////////////    LONG PRESS 2 release ++++++++++++++++  ");
        longPress2Active = false;

      } else { //released after short press
        Serial.println("===============///////////////    SHORT PRESS 2 release ----  ");
        switch (clockState) {
          case 0: // HH:MM Display
            is12HFormat = ! is12HFormat;
            EEPROM.write(eepromIs12HFormat, is12HFormat);
            break;
          case 1: // :SS Display
            break;
          case 2: // MMDD Display
            break;
          case 3: // YYYY Display
            break;
          case 10: // Hours Flashing (colon stay on)
            currentFlashingNum ++;
            if (currentFlashingNum > 23) currentFlashingNum = 0;
            break;
          case 11: // Minutes Flashing (colon stay on)
            currentFlashingNum ++;
            if (currentFlashingNum > 59) currentFlashingNum = 0;
            break;
          case 12: // Seconds Flashing and running (colon stay on)
            currentFlashingNum ++;
            if (currentFlashingNum > 59) currentFlashingNum = 0;
            break;
          case 13: // Month Flashing (colon stay off)
            currentFlashingNum ++;
            if (currentFlashingNum > 12) currentFlashingNum = 1;
            break;
          case 14: // Date Flashing (colon stay off)
            currentFlashingNum ++;
            if (currentFlashingNum > 31) currentFlashingNum = 1;
            break;
          case 15: // Year Flashing (colon stay off)
            currentFlashingNum ++;
            if (currentFlashingNum > 2099) currentFlashingNum = 2000;
            break;
        }
      }
      button2StillPressed = false;
    }
  }
  //======================================================= BUTTON 3 ========================================
  if (btn3State == LOW) { //btn3 goes LOW when pressed
    if (button3StillPressed == false) {
      Serial.println("===============///////////////    SHORT PRESS 3 START ++++  ");
      button3StillPressed = true;
      button3TimerStart = millis();
    }
    if ((millis() - button3TimerStart > longPressTime) && (longPress3Active == false)) { // this is long press
      longPress3Active = true;
      Serial.println("===============///////////////    LONG PRESS 3 START ++++++++++++++++  ");
      brightnessLevel -= 25;
      if (brightnessLevel < brightnessLevelMin ) brightnessLevel = brightnessLevelMax;
      nixieClock.setBrightness(brightnessLevel);
    }
  } else { // when btn3 is released

    if (button3StillPressed == true) {
      if (longPress3Active == true) { //released after long press
        Serial.println("===============///////////////    LONG PRESS 3 release ++++++++++++++++  ");
        longPress3Active = false;
        //real code starts below

      } else { //released after short press
        Serial.println("===============///////////////    SHORT PRESS 3 release ----  ");
        switch (clockState) {
          case 0: // HH:MM Display
            is12HFormat = ! is12HFormat;
            EEPROM.write(eepromIs12HFormat, is12HFormat);
            break;
          case 1: // :SS Display
            break;
          case 2: // MMDD Display
            break;
          case 3: // YYYY Display
            break;
          case 10: // Hours Flashing (colon stay on)
            currentFlashingNum --;
            if (currentFlashingNum == 65535) currentFlashingNum = 23; // 0 minus 1 is 65535
            break;
          case 11: // Minutes Flashing (colon stay on)
            currentFlashingNum --;
            if (currentFlashingNum == 65535) currentFlashingNum = 59;
            break;
          case 12: // Seconds Flashing and running (colon stay on)
            currentFlashingNum --;
            if (currentFlashingNum == 65535) currentFlashingNum = 59;
            break;
          case 13: // Month Flashing (colon stay off)
            currentFlashingNum --;
            if (currentFlashingNum < 1) currentFlashingNum = 12;
            break;
          case 14: // Date Flashing (colon stay off)
            currentFlashingNum --;
            if (currentFlashingNum < 1) currentFlashingNum = 31;
            break;
          case 15: // Year Flashing (colon stay off)
            currentFlashingNum --;
            if (currentFlashingNum < 2000) currentFlashingNum = 2099;
            break;
        }

      }
      button3StillPressed = false;
    }
  }

  //============================================= clockState ================================================
  switch (clockState) {
    case 0: // HH:MM
      colonOn = true;
      counter = 0;
      if (tm.Hour > 12 && is12HFormat)  {
        nixieClock.setNumber((tm.Hour - 12) * 100 + tm.Minute, 0b1111);
      } else {
        nixieClock.setNumber(tm.Hour * 100 + tm.Minute, 0b1111);
      }
      nixieClock.display();
      break;

    case 1: // :SS
      colonOn = true;
      counter = 0;
      nixieClock.setNumber(tm.Second, 0b0011);
      nixieClock.display();
      break;

    case 2: // MM DD
      colonOn = false;
      counter ++;
      if (clockState != clockStatePrev) {
        counter = 0;
        clockStatePrev = 2;
      }
      nixieClock.setColon(false);
      nixieClock.setNumber(tm.Month * 100 + tm.Day, 0b1111);
      nixieClock.display();
      if (counter > nTimeOut) {
        counter = 0;
        clockState = 0;
      }
      break;

    case 3: // YEAR
      colonOn = false;
      counter ++;
      if (clockState != clockStatePrev) {
        counter = 0;
        clockStatePrev = 3;
      }
      nixieClock.setNumber(tmYearToCalendar(tm.Year), 0b1111);
      nixieClock.display();
      if (counter > nTimeOut) {
        counter = 0;
        clockState = 0;
      }
      break;

    case 10: // Hours Flashing (colon stay on)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tm.Hour;
        clockStatePrev = clockState;
      }
      nixieClock.setColon(brightnessLevel);
      nixieClock.setNumber(currentFlashingNum * 100, 0b1100);
      nixieClock.display();
      break;
    case 11: // Minutes Flashing (colon stay on)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tm.Minute;
        clockStatePrev = clockState;
      }
      nixieClock.setColon(brightnessLevel);
      nixieClock.setNumber(currentFlashingNum, 0b0011);
      nixieClock.display();
      break;
    case 12: // Seconds Flashing and running (colon stay on)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tm.Second;
        clockStatePrev = clockState;
      }
      nixieClock.setColon(brightnessLevel);
      nixieClock.setNumber(currentFlashingNum, 0b0011);
      nixieClock.display();
      break;
    case 13: // Month Flashing (colon stay off)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tm.Month;
        clockStatePrev = clockState;
      }
      nixieClock.setColon(false);
      nixieClock.setNumber(currentFlashingNum * 100, 0b1100);
      nixieClock.display();
      break;
    case 14: // Date Flashing (colon stay off)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tm.Day;
        clockStatePrev = clockState;
      }
      nixieClock.setColon(false);
      nixieClock.setNumber(currentFlashingNum, 0b0011);
      nixieClock.display();
      break;
    case 15: // Year Flashing (colon stay off)
      if (clockState != clockStatePrev) {
        currentFlashingNum = tmYearToCalendar(tm.Year);
        Serial.print("year - ");
        Serial.print(tmYearToCalendar(tm.Year));
        clockStatePrev = clockState;
      }
      nixieClock.setColon(false);
      nixieClock.setNumber(currentFlashingNum, 0b1111);
      nixieClock.display();
      break;

    default:
      break;
  }

  if (prevSec != tm.Second && colonOn) {
    isColonFlashing = !isColonFlashing;
    byte bLevel = isColonFlashing ? brightnessLevel : 0;
    nixieClock.setColon(bLevel);
    prevSec = tm.Second;
  }
  EEPROM.write(eepromBrightnessLevel, brightnessLevel);
  delay(1);
}

void CathodePoisoningPrevention(unsigned int num, int msDelay) {
  if (num < 1) exit;
  for (byte n = 0; n < num; n++) {
    Serial.println("Running Cathode Poisoning Prevention ... ");
    for (byte i = 0; i < 10; i++) {
      nixieClock.setNumber(i * 1111, 0b1111);
      nixieClock.display();
      delay(msDelay);
    }
  }
  delay(1000);
}