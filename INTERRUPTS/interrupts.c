/*
 * interrupts.c
 *
 *  Created on: 8 kwi 2021
 *      Author: £ukasz ¯ukowski
 */
#include <avr/interrupt.h>
#include "interrupts.h"
#include "../MENU/menu.h"
#include "../DHT/dht.h"

uint8_t tot_overflow = 0;


void structure_init() {
	akcja water = { 0, WATER };
	akcja humidity = { 0, HUM };
	akcja temperature = { 0, TEMP };
	akcja ventilation = { 0, AIR };
	akcja lighting = { 0, LIGHT };
	akcja buzz = { 0, BUZZ };

	actions[0] = water;
	actions[1] = humidity;
	actions[2] = temperature;
	actions[3] = ventilation;
	actions[4] = lighting;
	actions[5] = buzz;
}

void check_actions() {
	for (int i = 0; i < sizeof(actions); i++) {

		if (actions[i].time > 1) {
			actions[i].time--;
		} else if (actions[i].time == 1) {
			actions[i].time--;
			end_action(i);
		} else if (actions[i].time < 1) {

		}
	}
}

void check_parameters() {

	if (tot_overflow >= 10/*60*/) {//je¿eli siê zacina to na RTC
		reg_temp_hum();
		check_lighting();//sprawdzic pin uzywane
		check_watering();

		tot_overflow = 0;
	}
}

void end_action(function_name name) {

	switch (name) {
	case WATER:
		PORTD &= ~(1 << PD2);
		break;

	case HUM:
		PORTD &= ~(1 << PD1);
		break;

	case TEMP:
		PORTD &= ~(1 << PD0);
		break;

	case AIR:
		PORTD &= ~(1 << PD3);
		break;

	case LIGHT:
		PORTD &= ~(1 << PD4);
		break;

	case BUZZ:
		PORTD &= ~(1 << PD5);
		break;
	}
}

void licznik1_init() {

	/*set up timer with prescaler = 256*/
	TCCR1B |= (1 << CS12);

	/*ustawienie licznika w tryb CTC*/
	TCCR1B |= (1 << WGM12);

	/*initialize counter with starting value = 0*/
	TCNT1 = 0;

	/*enable overflow interrupt*/
	TIMSK |= (1 << OCIE1A);

	/*enable global interrupts*/
	sei();

	/*set comparison number, 34286 ticks = 1 second delay*/
	OCR1A = 31249;
}

ISR(TIMER1_COMPA_vect) {	//sekundowy
	tot_overflow++;

	check_actions();
	check_parameters();
}

