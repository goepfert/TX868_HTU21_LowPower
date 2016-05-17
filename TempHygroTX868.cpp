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

#include "TempHygroTX868.h"


TempHygroTX868::TempHygroTX868(byte pin) {
	m_pin = pin;
	pinMode(m_pin, OUTPUT);
	digitalWrite(m_pin, LOW);
	m_nextSlope = 0;
}


void TempHygroTX868::setAddress(byte addr) {
	data[ADD] = addr;
}


void TempHygroTX868::setDataType(TempHygroTX868::datatype_t datatype) {
	data[DATATYPE] = datatype;
}


void TempHygroTX868::setData(float temp, float humidity, float voltage) {

	//prepare data
	int16_t itemp = int16_t((temp+50) * 100. + 0.5);
	int16_t ihum = int16_t(humidity * 100. + 0.5);
	int16_t ivol = int16_t(voltage * 100. + 0.5);

	// to get it back: temp = data[VAL1] | (data[VAL1+1]<<8)
	data[VAL1] = itemp & 0xFF;
	data[VAL1+1] = (itemp >> 8) & 0xFF;

	data[VAL2] = ihum & 0xFF;
	data[VAL2+1] = (ihum >> 8) & 0xFF;

	data[VAL3] = ivol & 0xFF;
	data[VAL3+1] = (ivol >> 8) & 0xFF;
}


void TempHygroTX868::send() {

	int sum = 0;

	// sync
	sendBit(1);
	for (byte i = 0; i < NSYNC; ++i) {
		sendBit(0);
	}
	// start bit
	sendBit(1);

	// data
	for (byte pos = 0; pos < LENGTH; ++pos) {
		sum += data[pos];
		sendByte(data[pos]);
	}

	// check sum
	sendByte(sum + 5);

	sendEOF();
	return;
}


void TempHygroTX868::sendByte(byte value) {

	for (byte pos = 0; pos < 8; ++pos) {
		sendBit(value & 0x01);
		value >>= 1;
	}

	sendBit(1);
}


void TempHygroTX868::sendBit(byte value) {

	if (m_nextSlope > micros()) {
		delayMicroseconds(m_nextSlope - micros());
	}

	digitalWrite(m_pin, HIGH);
	m_nextSlope = micros() + TOTAL;

	if (value) {
		delayMicroseconds(SHORT);
	} else {
		delayMicroseconds(LONG);
	}
	digitalWrite(m_pin, LOW);
}


void TempHygroTX868::sendEOF() {

	if (m_nextSlope > micros()) {
		delayMicroseconds(m_nextSlope - micros());
	}
	digitalWrite(m_pin, HIGH);

	m_nextSlope = micros() + TOTAL;

	delayMicroseconds(2*TOTAL);
	sendBit(1);

	digitalWrite(m_pin, LOW);
}

