Nixie Tube Driver Board for Arduino library (Omnixie_NTDB)

Released at
https://github.com/omnixie-electronics/Nixie-Tube-Driver-Board-Arduino-Library
short link: https://nixie.ai/ntdb4github

This Arduino library helps you make your own nixie tube clocks with ease.
More tubes can be driven when multiple NTDB boards connected in series.

Two examples are provided:
1) A minimal 4-Nixie-tube display.
2) A complete clock using 4 nixie tubes, 3 tactile buttons and 2 colon bulbs.

![Flow Chart of the Complete 4-Nixie-tube Clock](./images/Complete-4-Nixie-tube-Clock-Flow-Chart-v1.jpg)

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
 Connect a 12V DC power to the NTDB board 

 Arduino Library and example code created
 by Aiden Fang
 www.Omnixie.com

 NTDB board designed by Zeyuan, Yan
 www.Omnixie.cn

