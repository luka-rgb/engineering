/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - DHT-11 Library, by Charalampos Andrianakis on 18/12/11
*/

#ifndef DHT_H_
#define DHT_H_


void get_temp_hum(void);
void compare_temp(void);

#include <stdio.h>
#include <avr/io.h>

//setup port

#define DHT_DDR DDRB
#define DHT_PORT PORTB
#define DHT_PIN PINB
#define DHT_INPUTPIN PB0

//timeout retries

#define DHT_TIMEOUT 200

//functions

extern int8_t dht_gettemperature(int8_t *temperature);
extern int8_t dht_gethumidity(int8_t *humidity);
extern int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity);

#endif

