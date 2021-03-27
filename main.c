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

#define REF_256 (1<<REFS1)|(1<<REFS0)	//makro na napiêcie odniesienia wewnêtrzne 2,56
#define REF_VCC (1<<REFS0)		//makro na napiêcie odniesienia VCC

/*extern uint8_t bufor[7];*/

char ADC_pomiar[17];
volatile uint16_t value;
volatile uint8_t key_lock;//volatile oznacza, ¿e zmienna mo¿e byc zmieniona z zewn¹trz

extern uint8_t watering_freq_temp;
extern uint8_t godziny;
extern uint8_t lighting_on_temp;

// global variable to count the number of overflows
// global variable to count the number of overflows
volatile uint8_t tot_overflow;

int i = 0;	//czy to jest u¿ywane??

// initialize timer, interrupt and variable
void licznik1_init() {
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
				if (watering_water_level_flag == 0) {
					watering();
					// no timer reset required here as the timer
					// is reset every time it overflows
				} else if (watering_water_level_flag == 8) {
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

			PORTD |= (1 << PD3);	//start cyklu oœwietlenia
		} else if (godziny == (7 + lighting_on_temp)) {

			PORTD &= ~(1 << PD3);	//zakoñczenie cyklu oœwietlenia
		}
		tot_overflow = 0;   // reset overflow counter
	}
}


unsigned int program_array[6];

void initialization(void) {
	lcd_init();

	ADCSRA = (1 << ADEN); 	//w³¹czenie przetwornika ADC

	/*USTAWIAMY prêdkoœæ 100 kHz na magistrali I2C*/
	i2cSetBitrate(100);

	/*ustawianie portów na wejœcie/wyjœcie dla czujnika poziomu wody*/
	DDRA |= (1 << PA2);
	DDRA &= ~(1 << PA3);

	PORTA |= (1 << PA2);

	/*inicjalizacja timera*/
	licznik1_init();

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
	//loop_until_bit_is_clear(ADCSRA, ADSC);
	value = ADCW;		//ADC to makro
	sprintf(ADC_pomiar, "%d  ", value);		//zamiana na system dziesiêtny
}

int main(void) {

	initialization();

lcd_cls();
	while (1){
	check_if_water();
	}
	}
		/*
		pomiar();
		read_key();
		 if (menu_event) {
		 change_menu();
		 }
	}
}*/





