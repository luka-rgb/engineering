/*
 * main.c
 *
 *  Created on: 17 gru 2020
 *      Author: lukas
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

#define REF_256 (1<<REFS1)|(1<<REFS0)	//makro na napi�cie odniesienia wewn�trzne 2,56
#define REF_VCC (1<<REFS0)		//makro na napi�cie odniesienia VCC

uint8_t bufor[7];

char ADC_pomiar[17];
volatile uint16_t value;
volatile uint8_t key_lock;//volatile oznacza, �e zmienna mo�e byc zmieniona z zewn�trz

extern uint8_t watering_freq_temp;
extern uint8_t godziny;
extern uint8_t lighting_on_temp;

// global variable to count the number of overflows
// global variable to count the number of overflows
volatile uint8_t tot_overflow;

int i = 0;

// initialize timer, interrupt and variable
void licznik1_init() {
	// set up timer with prescaler = 256
	TCCR1B |= (1 << CS12);

	// initialize counter
	TCNT1 = 3036;		//warto�c startowa, �eby op�nienie wynosi�o r�wno 2s

	// enable overflow interrupt
	TIMSK |= (1 << TOIE1);

	// enable global interrupts
	sei();

	// initialize overflow counter variable
	tot_overflow = 0;
}

// TIMER1 overflow interrupt service routine
// called whenever TCNT1 overflows
ISR(TIMER1_OVF_vect) {
	// keep a track of number of overflows
	tot_overflow++;

	// check for number of overflows here itself
	// 1 overflows = 2 seconds delay, wi�c docelowo ma byc 1800 bo co godzin� ma sprawdzac
	if (tot_overflow >= 1800) { // NOTE: '>=' used instead of '=='	wywo�ywanie przerwania co godzin�
		check_hour();
		if (godziny == 19) {		//podlewanie o 19
			//PORTD |= (1<<PD2);
			if (licznik_dni == watering_freq_temp) {
				check_if_water();
				if (poziom_wody_flaga == 1) {
					watering();
					// no timer reset required here as the timer
					// is reset every time it overflows
				} else {
					//miganie czerwonej diody? dodac buzzer zamiast diody
					PORTD |= (1 << PD0);
					_delay_ms(1000);
					PORTD &= ~(1 << PD0);
				}
				licznik_dni = 0;
			} else {
				licznik_dni++;
			}
		} else if (godziny == 7) {

			PORTD |= (1 << PD3);	//start cyklu o�wietlenia
		} else if (godziny == (7 + lighting_on_temp)) {

			PORTD &= ~(1 << PD3);	//zako�czenie cyklu o�wietlenia
		}
		tot_overflow = 0;   // reset overflow counter

	}
}
uint8_t ByteOfData ;
unsigned int R_array[15],W_array[15];	//zmienic na tyle miejsc ile potrzebuje parametr�w

int main(void) {
	lcd_init();

	//eeprom_write_word(&W_array[0], 12);

	R_array[0] = eeprom_read_word(&W_array[0]);


		lcd_locate(0,0);
		lcd_int(R_array[0]);		/* Print Read_array on LCD */

	while (1) {

	}
}
/*int main(void) {
	lcd_init();

	// initialize timer
	licznik1_init();

	//inicjalizacja ADC
	ADCSRA = (1 << ADEN); 	//w��czenie przetwornika ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ustawienie przeskalera na 64
	ADMUX |= REF_256;//wyb�r napi�cia odniesienia z wcze�niej zdefiniowanych makr

	i2cSetBitrate(100); // USTAWIAMY pr�dko�� 100 kHz na magistrali I2C

	//przenie�c to do inicjalizacji
	DDRA |= (1 << PA2);
	DDRA &= ~(1 << PA3);

	PORTA |= (1 << PA2);

	while (1) {

//czy da si� przeniesc start konwersji i reszt� do funkcji read_key?
		//		lcd_locate(0,0);
		//		lcd_int(value);
		ADCSRA |= (1 << ADSC);	//start konwersji
		loop_until_bit_is_clear(ADCSRA, ADSC);
		value = ADC;		//ADC to makro
		sprintf(ADC_pomiar, "%d  ", value);		//zamiana na system dziesi�tny

		read_key();
		if (menu_event) {
			change_menu();
		}

	}
}*/
