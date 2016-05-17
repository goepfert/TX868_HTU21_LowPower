/*
 * Arduino library to control the ELV TX868 rf transmitter module
 * to send temperature and humidity values over the air at 868.35 MHz.
 * The communication protocol is compatible to ELV sensors like the
 * S 300 and ASH 2200, therefore the data may be received by weather
 * stations like USB-WDE 1, WS 200/300, and IPWE 1 manufactured by ELV 
 * (http://www.elv.de).
 * 
 * Copyright 2015 Martin Kompf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _TempHygroTX868_h
#define _TempHygroTX868_h

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define TOTAL 2000
#define SHORT 600
#define LONG (TOTAL - SHORT)

#define NSYNC 9

#define DATATYPE 0
#define ADD 1
#define VAL1 2
#define VAL2 4
#define VAL3 6
#define LENGTH 8

/*
 * Controller for the TX868 rf transmitter. 
 */
class TempHygroTX868 {

public:
	enum datatype_t {HTV, NONE};

private:
	//void sendData(byte* data, byte length);
	void sendByte(byte value);
	void sendBit(byte value);
	//void sendNibble(byte value);

	byte m_pin;
	//byte m_addr;
	unsigned long m_nextSlope;
	byte data[8];
	//datatype_t m_datatype;

	void sendEOF();

public:
	TempHygroTX868(byte pin);

	void setAddress(byte addr);
	void setDataType(TempHygroTX868::datatype_t datatype);
	void setData(float temp, float humidity, float voltage);
	//void send(float temp, float humidity);
	void send();
	//int getPause();
};

#endif

