/*
  Nixie Tube Driver Board (NTDB) Arduino library (Omnixie_NTDB)


Released at
https://github.com/omnixie-electronics/Nixie-Tube-Driver-Board-Arduino-Library
short link: https://nixie.ai/ntdb4github

This Arduino library helps you make your own nixie tube clocks with ease.
More tubes can be driven when multiple NTDB boards connected in series.


In order to make a complete clock, you will need:
1) This KIT: https://nixie.ai/ntdb4
2) 4x nixie tubes
3) 1x NCH6300HV: https://nixie.ai/nch6300hv
4) 1x Arduino Uno board
5) 1x Real Time Clock module (DS1307, DS3231, etc)
6) Some jumper wires

 How to use this library:

1) include the header file in your Arduino code: 
#include <Omnixie_NTDB.h>

2) define and pass the pin numbers in use:
Omnixie_NTDB nixieClock(11, 8, 12, 10, 6, 5, 1);

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

3) turn on HV power in your setup() routine
nixieClock.setHVPower(true);

4) specify what number to display, and which tubes to display
nixieClock.setNumber(0259, 0b1111);

5) Light up the tubes.
nixieClock.display();


Library and example code created
 by Aiden Fang
 www.Omnixie.com

 NTDB board designed by Zeyuan, Yan
 www.Omnixie.cn


Updates:
1) 11/26/2020
First released in public domain on Thanksgiving day 


 */

#ifndef Omnixie_NTDB_H_
#define Omnixie_NTDB_H_

#include "Arduino.h"

enum Colon
{
	None, Upper, Lower, Both
};

enum BitOrder
{
	LSB, MSB
};

class Omnixie_NTDB
{

public:
	Omnixie_NTDB(uint8_t pin_DataIN, 
		uint8_t pin_LatchSTCP, 
		uint8_t pin_ClockSHCP, 
		uint8_t pin_BlankOE, 
		uint8_t pin_HVenable,
		uint8_t pin_colon, 
		byte ntdb_count = 1 // qty of NTDB boards
		);
	
	virtual ~Omnixie_NTDB();

	void clear(word value = 0x7000); //16-bit word

	void putWord(byte index, word value = 0x7000);

	void setBrightness(byte brightness = 0x40);
	void display();

	void setHVPower(boolean hv = false);

	void setNumber(unsigned int num, byte DigitDisplayMask);
	void Omnixie_NTDB::setText(char charArr[]);

	// void setColon(boolean colonOn = false);
	void Omnixie_NTDB::setColon(byte brightness);

private:

	// word *_buff; //store an unsigned number of at least 16 bits (from 0 to 65535).
	// char * _cache; //8 bit
	byte * _data; // 8 bit array

	const uint8_t _pin_DataIN; // Data In
	const uint8_t _pin_LatchSTCP; // Latch Pin (STCP)
	const uint8_t _pin_ClockSHCP; // Clock Pin (SHCP)
	const uint8_t _pin_BlankOE; // Blank Pin (OE). Although any I/O can be used, a PWM pin is preferred to manipulate brightness.
	const uint8_t _pin_HV_SHDN; // HV SHDN Pin
	const uint8_t _pin_Colon_Ctrl; // Colon Ctrl Pin
	const byte _ntdb_count;

	byte _cache_length_bytes;

	void loadData(byte data, BitOrder order) const;

	// from left 9 to right 0; 7618325409; check the board schematic to find out why
	const int digit[11] =
	{
	  0b0000000010, //0
	  0b0010000000, //1
	  0b0000010000, //2
	  0b0000100000, //3
	  0b0000000100, //4
	  0b0000001000, //5
	  0b0100000000, //6
	  0b1000000000, //7
	  0b0001000000, //8
	  0b0000000001, //9
	  0b0000000000  //display off
	};
};

#endif /* Omnixie_NTDB_H_ */
