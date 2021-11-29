/*
 * main.c
 *
 *  Created on: 17 gru 2020
 *      Author: £ukasz ¯ukowski
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include "LCD/lcd44780.h"
#include "MENU/menu.h"
#include "I2C/I2C.h"
#include "DHT/dht.h"
#include "INTERRUPTS/interrupts.h"

#define REF_256 (1<<REFS1)|(1<<REFS0)	//makro na napiêcie odniesienia wewnêtrzne 2,56
#define REF_VCC (1<<REFS0)		//makro na napiêcie odniesienia VCC

char ADC_pomiar[17];


void initialization(void) {
	lcd_init();

	ADCSRA = (1 << ADEN); 	//w³¹czenie przetwornika ADC

	/*USTAWIAMY prêdkoœæ 100 kHz na magistrali I2C*/
	i2cSetBitrate(100);

	/*ustawianie portów na wejœcie/wyjœcie dla czujnika poziomu wody*/
	DDRA |= (1 << PA2);
	DDRA &= ~(1 << PA3);

	PORTA |= (1 << PA2);

	read_parameters();
	if (program_saved == 1) {
		current_menu = 18;
		change_menu();

	} else if (program_saved == 0) {
		current_menu = 0;
		change_menu();
	}

}

void pomiar(void) {
	/*inicjalizacja ADC*/

	ADMUX = (ADMUX & 0xF8) | 0;	//zerowanie pozosta³ych miejsc rejestru poza wybranym pinem
	_delay_us(250);		//zw³oka czasowa, ¿eby dobrze wyszed³ pomiar
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ustawienie przeskalera na 64
	ADMUX |= REF_256;//wybór napiêcia odniesienia z wczeœniej zdefiniowanych makr

	ADCSRA |= (1 << ADSC);	//start konwersji
	while ((ADCSRA & (1 << ADSC)));
	loop_until_bit_is_clear(ADCSRA, ADSC);
	value = ADCW;
	sprintf(ADC_pomiar, "%d  ", value);		//zamiana na system dziesiêtny
}


int main(void) {

	initialization();
	structure_init();


	while (1) {
		pomiar();
		read_key();
		if (menu_event) {
			change_menu();
		}
	}
}


