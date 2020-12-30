/*
 DHT Library 0x03

 copyright (c) Davide Gironi, 2012

 Released under GPLv3.
 Please refer to LICENSE file for licensing information.
 */

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "../LCD/lcd44780.h"
#include "../MENU/menu.h"
#include "dht.h"

extern uint8_t temperature_temp;	//czy bez tego bêdzie widzia³o t¹ zmienn¹

int8_t temperature = 0;
int8_t humidity = 0;

char printbuff[100];

/*
 * get data from sensor
 */

int8_t dht_getdata(int8_t *temperature, int8_t *humidity) {

	uint8_t bits[5];
	uint8_t i, j = 0;

	memset(bits, 0, sizeof(bits));

	//reset port
	DHT_DDR |= (1 << DHT_INPUTPIN); //output
	DHT_PORT |= (1 << DHT_INPUTPIN); //high
	_delay_ms(100);

	//send request
	DHT_PORT &= ~(1 << DHT_INPUTPIN); //low
	_delay_us(500);
	DHT_PORT |= (1 << DHT_INPUTPIN); //high
	DHT_DDR &= ~(1 << DHT_INPUTPIN); //input
	_delay_us(40);

	//check start condition 1
	if ((DHT_PIN & (1 << DHT_INPUTPIN))) {
		return -1;
	}
	_delay_us(80);
	//check start condition 2
	if (!(DHT_PIN & (1 << DHT_INPUTPIN))) {
		return -1;
	}
	_delay_us(80);

	//read the data
	uint16_t timeoutcounter = 0;
	for (j = 0; j < 5; j++) { //read 5 byte
		uint8_t result = 0;
		for (i = 0; i < 8; i++) { //read every bit
			timeoutcounter = 0;
			while (!(DHT_PIN & (1 << DHT_INPUTPIN))) { //wait for an high input (non blocking)
				timeoutcounter++;
				if (timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
			_delay_us(30);
			if (DHT_PIN & (1 << DHT_INPUTPIN)) //if input is high after 30 us, get result
				result |= (1 << (7 - i));
			timeoutcounter = 0;
			while (DHT_PIN & (1 << DHT_INPUTPIN)) { //wait until input get low (non blocking)
				timeoutcounter++;
				if (timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
		}
		bits[j] = result;
	}

	//reset port
	DHT_DDR |= (1 << DHT_INPUTPIN); //output
	DHT_PORT |= (1 << DHT_INPUTPIN); //low
	_delay_ms(100);

	//check checksum
	if ((uint8_t) (bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		//return temperature and humidity

		uint16_t rawhumidity = bits[0] << 8 | bits[1];
		uint16_t rawtemperature = bits[2] << 8 | bits[3];
		if (rawtemperature & 0x8000) {
			*temperature = (float) ((rawtemperature & 0x7FFF) / 10.0) * -1.0;
		} else {
			*temperature = (float) (rawtemperature) / 10.0;
		}
		*humidity = (float) (rawhumidity) / 10.0;		//dlaczego tutaj jest float

		return 0;
	}

	return -1;
}

/*
 * get temperature
 */

int8_t dht_gettemperature(int8_t *temperature) {
	int8_t humidity = 0;
	return dht_getdata(temperature, &humidity);
}

/*
 * get humidity
 */

int8_t dht_gethumidity(int8_t *humidity) {
	int8_t temperature = 0;
	return dht_getdata(&temperature, humidity);
}

/*
 * get temperature and humidity
 */

int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity) {
	return dht_getdata(temperature, humidity);
}

void get_temp_hum(void) {
	if (dht_gettemperaturehumidity(&temperature, &humidity) != -1) {

		itoa(temperature, printbuff, 10);

		lcd_locate(0, 0);
		lcd_str("temp: ");
		lcd_str(printbuff);
		lcd_str(" oC");

		itoa(humidity, printbuff, 10);

		lcd_locate(1, 0);
		lcd_str("hum: ");
		lcd_str(printbuff);
		lcd_str(" %RH");
//_delay_ms(1000);	//niepotrzebny delay tylko muszê raz wywo³ywac funkcjê
	} else {
		lcd_locate(0, 0);
		lcd_str("error");
	}
}

void compare_temp(void) { //na warunku z tego bêdzie realizowane wietrzenie i grzanie
	uint8_t comp_temp;
	if (temperature == temperature_temp) {
		comp_temp = 1;
		lcd_locate(0, 0);
		lcd_str("temp zgodne");
	} else {
		comp_temp = 0;
		lcd_locate(0, 0);
		lcd_str("temp niezgodne");
	}
}
