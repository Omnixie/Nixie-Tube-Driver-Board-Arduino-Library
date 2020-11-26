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

#include "Omnixie_NTDB.h"

Omnixie_NTDB::Omnixie_NTDB(uint8_t pin_DataIN, 
	uint8_t pin_LatchSTCP, 
	uint8_t pin_ClockSHCP, 
	uint8_t pin_BlankOE, 
	uint8_t pin_HVenable,
	uint8_t pin_colon, 
	byte ntdb_count) :
		_pin_DataIN(pin_DataIN), 
		_pin_LatchSTCP(pin_LatchSTCP), 
		_pin_ClockSHCP(pin_ClockSHCP), 
		_pin_BlankOE(pin_BlankOE), 
		_pin_HV_SHDN(pin_HVenable), 
		_pin_Colon_Ctrl(pin_colon), 
		_ntdb_count(ntdb_count)
{
	_cache_length_bytes = ntdb_count * 5;
	_data = (byte *) malloc(sizeof(byte) * ntdb_count); 

	pinMode(_pin_DataIN, OUTPUT);
	pinMode(_pin_LatchSTCP, OUTPUT);
	pinMode(_pin_ClockSHCP, OUTPUT);
	pinMode(_pin_BlankOE, OUTPUT);
	pinMode(_pin_HV_SHDN, OUTPUT);
	pinMode(_pin_Colon_Ctrl, OUTPUT);

	// this->setBrightness();

	this->clear();

	memset(_data, 0, _cache_length_bytes);// use memset to fill the "_data" array with a single value
	//Sets the first "_cache_length_bytes" bytes of the block of memory pointed by "_data" to the specified value ("0" here)
	//(interpreted as an unsigned char).
}

void Omnixie_NTDB::loadData(byte data, BitOrder bo) const
{
	switch (bo)
	{
	case LSB:
		for (byte i = 0; i < 8; i++)
		{
			digitalWrite(_pin_DataIN, bitRead(data, i));
			digitalWrite(_pin_ClockSHCP, LOW);
			digitalWrite(_pin_ClockSHCP, HIGH);
		}
		break;

	case MSB:
		for (byte i = 7; i >= 0; i--)
		{
			digitalWrite(_pin_DataIN, bitRead(data, i));
			digitalWrite(_pin_ClockSHCP, LOW);
			digitalWrite(_pin_ClockSHCP, HIGH);
		}
		break;

	default:
		break;
	}
}

void Omnixie_NTDB::display()
{
	for (byte i = 0; i < _ntdb_count * 5; i++)
	{
		this->loadData(_data[i], LSB);
	}

	digitalWrite(_pin_LatchSTCP, LOW);
	digitalWrite(_pin_LatchSTCP, HIGH);
}

void Omnixie_NTDB::setText(char charArr[]) {
	int num = 0;
	for (int i = 0; i < 4; i++) {
		if(isDigit(charArr[i])) {
			num += (charArr[i] - '0') * 1000 / (pow(10, i));
		}
	}
	this->setNumber(num, 4);
}

void Omnixie_NTDB::setNumber(unsigned int number, byte DigitDisplayMask)
{
	if (number < 0) number = 0;
	if (number > 9999) number = 9999;
	//use DigitDisplayMask to determine which tube goes dark (not displaying anything)
	//When it's set to 10, the corresponding tube goes dark.
    int digitLeft1 = (DigitDisplayMask & 0b1000) == 0b1000 ? (number / 1000) % 10 : 10;
    int digitLeft2 = (DigitDisplayMask & 0b0100) == 0b0100 ? (number / 100) % 10 : 10;
    int digitLeft3 = (DigitDisplayMask & 0b0010) == 0b0010 ? (number / 10) % 10 : 10;
    int digitLeft4 = (DigitDisplayMask & 0b0001) == 0b0001 ? number % 10 : 10;

    _data[0] = digit[digitLeft4] & 0b11111111;
    _data[1] = ((digit[digitLeft4] & 0b1100000000) >> 8) | ((digit[digitLeft3] & 0b111111) << 2);
    _data[2] = ((digit[digitLeft3] & 0b1111000000) >> 6) | ((digit[digitLeft2] & 0b1111) << 4);
    _data[3] = ((digit[digitLeft2] & 0b1111110000) >> 4) | ((digit[digitLeft1] & 0b11) << 6);
    _data[4] = ((digit[digitLeft1] & 0b1111111100) >> 2);
}

void Omnixie_NTDB::setHVPower(bool hv)
{
	digitalWrite(_pin_HV_SHDN, hv? HIGH : LOW);
}

// void Omnixie_NTDB::setColon(bool colon)
// {
// 	digitalWrite(_pin_Colon_Ctrl, colon);
// }

void Omnixie_NTDB::setColon(byte brightness)
{
	if (digitalPinToTimer(_pin_Colon_Ctrl) == NOT_ON_TIMER)
		{Serial.println("high low");
		digitalWrite(_pin_Colon_Ctrl, brightness ? HIGH : LOW);}
	else
		analogWrite(_pin_Colon_Ctrl, brightness);
}

void Omnixie_NTDB::setBrightness(byte brightness)
{
	if (digitalPinToTimer(_pin_BlankOE) == NOT_ON_TIMER)
		digitalWrite(_pin_BlankOE, brightness ? LOW : HIGH);
	else
		analogWrite(_pin_BlankOE, 0xff - brightness);
}

void Omnixie_NTDB::putWord(byte index, word value)
{
	index %= _ntdb_count;
	_data[index] = value;
}

void Omnixie_NTDB::clear(word value)
{
	for (byte i = 0; i < _ntdb_count; i++)
		this->putWord(i, value);
}

Omnixie_NTDB::~Omnixie_NTDB()
{
	free(_data);
}

