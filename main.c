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

#include "LCD/lcd44780.h"
#include "MENU/menu.h"
#include "I2C/I2C.h"
#include "DHT/dht.h"

#define REF_256 (1<<REFS1)|(1<<REFS0)	//makro na napiêcie odniesienia wewnêtrzne 2,56
#define REF_VCC (1<<REFS0)		//makro na napiêcie odniesienia VCC



//uint8_t sekundy = 0, minuty, godziny, dni, miesiace;
//uint16_t lata;
uint8_t bufor[7];


char  ADC_pomiar[17];
volatile uint16_t value;
volatile uint8_t key_lock;	//volatile oznacza, ¿e zmienna mo¿e byc zmieniona z zewn¹trz


extern uint8_t watering_freq_temp;
extern uint8_t godziny;
extern uint8_t lighting_on_temp;

// global variable to count the number of overflows
// global variable to count the number of overflows
volatile uint8_t tot_overflow;


int i = 0;

// initialize timer, interrupt and variable
void licznik1_init()
{
    // set up timer with prescaler = 256
    TCCR1B |= (1 << CS12);

    // initialize counter
    TCNT1 = 3036;		//wartoœc startowa, ¿eby opóŸnienie wynosi³o równo 2s

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
	// 1 overflows = 2 seconds delay, wiêc docelowo ma byc 1800 bo co godzinê ma sprawdzac
	if (tot_overflow >= 1800) { // NOTE: '>=' used instead of '=='	wywo³ywanie przerwania co godzinê
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
		} else if (godziny == 10) {

			PORTD |= (1 << PD3);
		} else if (godziny == (7 + lighting_on_temp)) {

			PORTD &= ~(1 << PD3);
		}
		tot_overflow = 0;   // reset overflow counter

	}
}

int main(void) {
	lcd_init();

	// initialize timer
	licznik1_init();

	//inicjalizacja ADC
	ADCSRA = (1 << ADEN); 	//w³¹czenie przetwornika ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ustawienie przeskalera na 64
	ADMUX |= REF_256;//wybór napiêcia odniesienia z wczeœniej zdefiniowanych makr

	i2cSetBitrate(100); // USTAWIAMY prêdkoœæ 100 kHz na magistrali I2C

	//przenieœc to do inicjalizacji
	DDRA |= (1 << PA2);
	DDRA &= ~(1 << PA3);

	PORTA |= (1 << PA2);


	while (1) {

	 ADCSRA |= (1 << ADSC);	//start konwersji
	 loop_until_bit_is_clear(ADCSRA, ADSC);
	 value = ADC;
	 sprintf(ADC_pomiar, "%d  ", value);

//czy da siê przeniesc start konwersji i resztê do funkcji read_key?
	 //		lcd_locate(0,0);
	 //		lcd_int(value);



	 read_key();
	 if (menu_event) {
	 change_menu();
	 }

		 }
}
