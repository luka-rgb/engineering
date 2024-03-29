/*
 * menu.c
 *
 *  Created on: 17 gru 2020
 *      Author: �ukasz �ukowski
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/eeprom.h>

#include "../I2C/I2C.h"
#include "menu.h"
#include "../LCD/lcd44780.h"
#include "../DHT/dht.h"
#include "../INTERRUPTS/interrupts.h"

volatile uint8_t current_menu = 0;
volatile uint8_t menu_event = E_IDDLE;
volatile uint8_t loop = 0;
char ADC_pomiar_poziom[17];
volatile uint16_t value;
volatile uint8_t water_level;
volatile uint8_t licznik_dni = 0;

unsigned int program_array[6];

uint8_t bufor[7];
uint8_t program_saved = 0;

uint8_t seconds = 0;
uint8_t minutes = 51;
uint8_t hours = 17;
uint8_t days = 8;
uint8_t months = 4;
uint8_t years = 21;
uint8_t bissextile;

uint8_t humidity_temp = 40;
uint8_t temperature_temp = 23;
uint8_t lighting_on = 12;
uint8_t lighting_off = 12;
uint16_t watering_amount = 200;
uint8_t watering_freq = 1;
uint8_t	seconds_temp, minutes_temp, hours_temp, days_temp, months_temp, years_temp;
uint8_t is_light_on;


const char START1[] PROGMEM = { "Wci" "\x82" "nij ok" };
const char START2[] PROGMEM = { "\x83" "eby" " zacz" "\x80" "\x81" };
const char M_1_1[] PROGMEM = { "Ustaw" };
const char M_1_2[] PROGMEM = { "czas i dat" "\x84" };
const char M_1_3[] PROGMEM = { "Czas i data" };
const char M_1_4[] PROGMEM = { "zapisane" };
const char M_2_1[] PROGMEM = { "Zdefiniuj" };
const char M_2_2[] PROGMEM = { "program" };
const char M_3_1[] PROGMEM = { "Wci" "\x82" "nij ok " "\x83" "eby"};
const char M_3_2[] PROGMEM = { "zapisa" "\x81" " program"};
const char M_4_1[] PROGMEM = { "Wci" "\x82" "nij ok " "\x83" "eby"};
const char M_4_2[] PROGMEM = { "zmieni" "\x81" " program"};

//struktura menu-----------------------------------------------------------------------------------------------------------------
typedef struct {
	uint8_t next_state[5];						//przej�cia do nast�pnych stan�w
	void (*callback)(void);						//funkcja zwrotna
	const char *first_line;						//tekst dla 1. linii LCD
	const char *second_line;					//tekst dla 2. linii LCD
} menu_item;

//menu glowne-----------------------------------------------------------------------------------------------------------------
const menu_item menu[] = {
				//  LP UP DN OK PREV
				{ { 0, 0, 0, 1, 0 }, 			NULL					,	START1	,	START2	},
				{ { 1, 1, 1, 2, 1 }, 			NULL					,	M_1_1	,	M_1_2	},
				{ { 2, 2, 2, 3, 1 }, 			change_year				,	NULL	,	NULL	},
				{ { 3, 3, 3, 4, 2 }, 			change_month			,	NULL	,	NULL	},
				{ { 4, 4, 4, 5, 3 }, 			change_day				,	NULL	,	NULL	},
				{ { 5, 5, 5, 6, 4 },			change_hour				,	NULL	,	NULL	},
				{ { 6, 6, 6, 7, 5 },			change_minute			,	NULL	,	NULL	},
				{ { 7, 7, 7, 8, 6 }, 			save_date_time			,	M_1_3	,	M_1_4	},
				{ { 8, 8, 8, 9, 7 }, 			show_date_time			,	NULL	,	NULL	},
				{ { 9, 9, 9, 10, 8 }, 			NULL					,	M_2_1	,	M_2_2	},
				{ { 10, 10, 10, 11, 9 }, 		show_temp_hum			,	NULL	,	NULL	},
				{ { 11, 11, 11, 12, 10 }, 		change_temperature		,	NULL	,	NULL	},
				{ { 12, 12, 12, 13, 11 },		change_humidity			,	NULL	,	NULL	},
				{ { 13, 13, 13, 14, 12 },		change_lighting			,	NULL	,	NULL	},
				{ { 14, 14, 14, 15, 13 },		change_watering_freq	,	NULL	,	NULL	},
				{ { 15, 15, 15, 16, 14 },		change_watering_amount	,	NULL	,	NULL	},
				{ { 16, 16, 16, 17, 15 },		NULL					,	M_3_1	,	M_3_2	},
				{ { 17, 17, 17, 18 , 16 },		save					,	NULL	,	NULL	},
				{ { 18, 18, 18, 2, 18 },		NULL					,	M_4_1	,	M_4_2	},

						};


void change_menu() {
	//przejdz do nastepnego stanu
	if (menu_event < 5) {
		current_menu = menu[current_menu].next_state[menu_event];

		//wyswietl komunikaty
		lcd_cls();

		//wy�wietl 1-sz� lini�
		if (menu[current_menu].first_line) {
			lcd_locate(0, 0);
			lcd_str_P((char*) menu[current_menu].first_line);
		}

		//wy�wietl 2-g� lini�
		if (menu[current_menu].second_line) {
			lcd_locate(1, 0);
			lcd_str_P((char*) menu[current_menu].second_line);
		}

		//wywolaj funkcje zwrotna
		if (menu[current_menu].callback)
			menu[current_menu].callback();

		//skasuj zdarzenie

		menu_event = 0;
	}
}

void read_key(void) {
	if (((value > 310) && (value < 350)) && (!key_lock)) {	//wci�ni�ty S1
		menu_event = E_UP;
		key_lock = 1;
	} else if ((value > 550) && (value < 590) && (!key_lock)) {	//wci�ni�ty S2
		menu_event = E_DW;
		key_lock = 1;
	} else if ((value > 730) && (value < 770) && (!key_lock)) {	//wci�ni�ty S3
		menu_event = E_OK;
		key_lock = 1;
	} else if ((value > 870) && (value < 920) && (!key_lock)) { //naci�ni�ty S4
		menu_event = E_PREV;
		key_lock = 1;
	} else if (((value > 1000) && key_lock)) { //�aden przycisk nie jest naci�ni�ty
		key_lock = 0;
		menu_event = E_IDDLE;
	}
}

void display_change_date(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Data");
	if (days < 10) {
		lcd_locate(1, 0);
		lcd_int(0);
		lcd_locate(1, 1);
		lcd_int(days);
	} else {
		lcd_locate(1, 0);
		lcd_int(days);
	}
	lcd_locate(1, 2);
	lcd_char('.');
	if (months < 10) {
		lcd_locate(1, 3);
		lcd_int(0);
		lcd_locate(1, 4);
		lcd_int(months);
	} else {
		lcd_locate(1, 3);
		lcd_int(months);
	}
	if(years < 10) {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("200");
		lcd_locate(1,9);
		lcd_int(years);
	} else if(years < 100) {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("20");
		lcd_locate(1,8);
		lcd_int(years);
	} else {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("2");
		lcd_locate(1,7);
		lcd_int(years);
	}
}

void display_change_time(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Godzina - GG/MM");
	printTime(1,0,hours);
	lcd_locate(1, 2);
	lcd_char(':');
	printTime(1,3,minutes);
}

void display_temperature(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Temperatura");
	lcd_locate(1, 2);
	lcd_int(temperature_temp);
	lcd_locate(1, 4);
	lcd_str("\x87" "C");
}

void display_change_humidity(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Wilgotno" "\x82" "\x81");
	lcd_locate(1, 0);
	lcd_int(humidity_temp);
	lcd_locate(1, 2);
	lcd_str(" %RH");
}

void display_change_lighting(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("\x82" "wiat" "\x86" "o" " w" "\x86" ":" "wy" "\x86");

	if (lighting_on < 10) {
		lcd_locate(1, 9);
		lcd_int(lighting_on);
		lcd_locate(1, 10);
		lcd_char(':');
		lcd_locate(1, 11);
		lcd_int(lighting_off);
	} else {
		lcd_locate(1, 8);
		lcd_int(lighting_on);
		lcd_locate(1, 10);
		lcd_char(':');
		lcd_locate(1, 11);
		lcd_int(lighting_off);
	}
}

void display_watering_amount(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Ilo" "\x82" "\x81" " wody");
	lcd_locate(1, 2);
	lcd_int(watering_amount);
	lcd_locate(1, 6);
	lcd_str("ml");
}

void display_watering_freq(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Podlewanie co");
	if (watering_freq < 2) {
		lcd_locate(1, 1);
		lcd_int(watering_freq);
		lcd_locate(1, 3);
		lcd_str("dzie" "\x85");
	} else if (watering_freq > 1) {
		lcd_locate(1, 1);
		lcd_int(watering_freq);
		lcd_locate(1, 3);
		lcd_str("dni");
	}
}

void change_year(void) {
	read_date_time();
	display_change_date();
	lcd_locate(1, 9);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (years < 100) {	//maksymalny rok dla DS1307 to 2100
			years += 1;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	case E_DW:
		if (years > 0) {
			years--;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	}
}

void change_month(void) {
	display_change_date();
	lcd_locate(1, 4);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (months < 12) {
			months++;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	case E_DW:
		if (months > 1) {
			months--;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	}
}

void change_day(void) {
	if_bissextile();

	display_change_date();
	lcd_locate(1, 1);
	lcd_cursor_on();

	read_key();
	switch (menu_event) {
	case E_UP:
		if ( (bissextile == 1) && (months == 2) ) {
			if (days < 29) {
				days++;
			}
		} else if ((bissextile == 0) && (months == 2)) {
			if (days < 28) {
				days++;
			}
		} else if ( (months == 4) || (months == 6) || (months == 9) || (months == 11) ) {
			if (days < 30) {
				days++;
			}
		} else if ( (months == 1) || (months == 3) || (months == 5) || (months == 7) || (months == 8) || (months == 10) || (months == 12) ) {
			if (days < 31) {
				days++;
			}
		}
		lcd_cursor_off();
		display_change_date();
		break;
	case E_DW:
		if (days > 1) {
			days--;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	}
}

void if_bissextile(void) {
	if ((years % 4 == 0 && years % 100 != 0) || years % 400 == 0) {
		bissextile = 1;
	} else {
		bissextile = 0;
	}
}

void change_hour(void) {

	display_change_time();
	lcd_locate(1, 1);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		hours++;
		if (hours > 23) {
			hours = 0;
		}
		lcd_cursor_off();
		display_change_time();
		break;
	case E_DW:
		hours--;
		if (hours < 0 || hours > 24) {
			hours = 23;
		}
		lcd_cursor_off();
		display_change_time();
		break;

	}
}

void change_minute(void) {
	display_change_time();
	lcd_locate(1, 4);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		minutes++;

		if (minutes > 59) {
			minutes = 0;
		}
		lcd_cursor_off();
		display_change_time();
		break;
	case E_DW:
		minutes--;
		if (minutes < 0 || minutes > 60) {
			minutes = 59;
		}
		lcd_cursor_off();
		display_change_time();
		break;
	}

}

void change_temperature(void) {
	display_temperature();
	lcd_locate(1, 3);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (temperature_temp < 40) {
			temperature_temp++;
		}
		lcd_cursor_off();
		display_temperature();
		break;
	case E_DW:
		if (temperature_temp > 23) {
			temperature_temp--;
		}
		lcd_cursor_off();
		display_temperature();
		break;
	}
}

void change_humidity(void) {
	display_change_humidity();
	lcd_locate(1, 1);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (humidity_temp < 90) {
			humidity_temp++;
		}
		lcd_cursor_off();
		display_change_humidity();
		break;
	case E_DW:
		if (humidity_temp > 20) {
			humidity_temp--;
		}
		lcd_cursor_off();
		display_change_humidity();
		break;
	}
}

void change_lighting(void) {
	display_change_lighting();
	lcd_locate(1, 9);
	lcd_cursor_on();
	switch (menu_event) {
	case E_UP:
		if (lighting_on < 24) {
			lighting_on++;
			lighting_off--;
		}
		lcd_cursor_off();
		display_change_lighting();
		break;
	case E_DW:
		if (lighting_on > 0) {
			lighting_on--;
			lighting_off++;
		}
		lcd_cursor_off();
		display_change_lighting();
		break;
	}
}

void change_watering_amount(void) {
	display_watering_amount();
	lcd_locate(1, 4);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (watering_amount <= 200) {
			watering_amount += 100;
		}
		lcd_cursor_off();
		display_watering_amount();
		break;
	case E_DW:
		if (watering_amount >= 200) {
			watering_amount -= 100;
		}
		lcd_cursor_off();
		display_watering_amount();
		break;
	}
}

void change_watering_freq(void) {
	display_watering_freq();
	lcd_locate(1, 1);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (watering_freq < 7) {
			watering_freq++;
			licznik_dni = watering_freq;
		}
		lcd_cursor_off();
		display_watering_freq();
		break;
	case E_DW:
		if (watering_freq > 1) {
			watering_freq--;
			licznik_dni = watering_freq;
		}
		lcd_cursor_off();
		display_watering_freq();
		break;
	case E_OK:
		lcd_cursor_off();
		break;
	}
}

void setSaved(uint8_t saved) {
	program_array[0] = saved;
}

void setTemperature(uint8_t temperature) {
	program_array[1] = temperature;
}

void setHumidity(uint8_t humidity) {
	program_array[2] = humidity;
}

void setLighting(uint8_t lighting) {
	program_array[3] = lighting;
}

void setWateringAmount(uint8_t wateringAmount) {
	program_array[4] = wateringAmount;
}

void setWateringFreq(uint8_t wateringFreq) {
	program_array[5] = wateringFreq;
}

uint8_t getSaved(void) {
	return program_array[0];
}

uint8_t getTemperature(void) {
	return program_array[1];
}

uint8_t getHumidity(void) {
	return program_array[2];
}

uint8_t getLighting(void) {
	return program_array[3];
}

uint8_t getWateringAmount(void) {
	return program_array[4];
}

uint8_t getWateringFreq(void) {
	return program_array[5];
}

void printTime(uint8_t y, uint8_t x, uint8_t time) {
	if (time < 10) {
		printInt(y, x, 0);
		printInt(y, x + 1, time);

	} else {
		printInt(y, x, time);
	}
}

void printInt(int8_t y, int8_t x, int16_t z) {
	lcd_locate(y, x);
	lcd_int(z);
}

void check_water_level(void) {

	ADMUX = (ADMUX & 0xF8) | 1;				//wyb�r u�ywanego pinu ADC, domy�lanie u�ywany jest PA0 razem z zerowaniem pozosta�ych miejsc rejestru
	_delay_us(250);
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ustawienie przeskalera na 8
	ADMUX |= (1 << REFS0);					//wyb�r napi�cia odniesienia z wcze�niej zdefiniowanych makr, W TYM PRZYPADKU vcc


	ADCSRA |= (1 << ADSC);	//start konwersji
	loop_until_bit_is_clear(ADCSRA, ADSC);
	water_level = ADC;
	sprintf(ADC_pomiar_poziom, "%d  ", water_level);

	if (water_level < 150) {
		humidity_water_level_flag = 0;
	} else {
		humidity_water_level_flag = 1;
	}

}

void check_if_water(void) {
	uint8_t is_water = PINA & (1 << PA3);

	for (int i = 0; i < 2; i++) {
		is_water &= PINA & (1 << PA3);	//operacja iloczynu bitowego
		_delay_ms(1000);
	}
	watering_water_level_flag = is_water;
}

void save_date_time(void) {
	lcd_cursor_off();

	bufor[0] = dec2bcd((seconds) & 0x7F);			//operacja bitowa and gwarantuje, �e na pierwszym miejscu b�dzie 0 i osc zostanie w��czony i zacznie zliczac czas
	bufor[1] = dec2bcd(minutes);
	bufor[2] = dec2bcd(hours);
	bufor[4] = dec2bcd(days);
	bufor[5] = dec2bcd(months);
	bufor[6] = dec2bcd(years);

	I2C_WRITE_BUFFER(DS1307_ADDR, 0x00, 7, bufor);
}

void show_date_time(void) {

	I2C_READ_BUFFER( DS1307_ADDR, 0x00, 7, bufor);

	seconds_temp = bcd2dec(bufor[0]);
	minutes_temp = bcd2dec(bufor[1]);
	hours_temp = bcd2dec(bufor[2]);
	//rejestr bufor[3] jest na dzie� tygodnia
	days_temp = bcd2dec(bufor[4]);
	months_temp = bcd2dec(bufor[5]);
	years_temp = bcd2dec(bufor[6]);

/*wy�wietlenie czasu na LCD*/
	lcd_cls();
	printTime(0,0,hours_temp);
	lcd_str(":");
	printTime(0,3,minutes_temp);
	lcd_str(":");
	printTime(0,6,seconds_temp);

/*wyswietlanie daty na lcd*/
	printTime(1,0,days_temp);
	lcd_char('.');
	printTime(1,3,months_temp);
	if (years_temp < 10) {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("200");
		lcd_locate(1, 9);
		lcd_int(years_temp);
	} else if (years_temp < 100) {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("20");
		lcd_locate(1, 8);
		lcd_int(years_temp);
	} else {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("2");
		lcd_locate(1, 7);
		lcd_int(years_temp);
	}
}

void read_date_time(void) {
	I2C_READ_BUFFER( DS1307_ADDR, 0x00, 7, bufor);


	minutes = bcd2dec(bufor[1]);
	hours = bcd2dec(bufor[2]);
	//rejestr bufor[3] jest na dzie� tygodnia
	days = bcd2dec(bufor[4]);
	months = bcd2dec(bufor[5]);
	years = bcd2dec(bufor[6]);
}

void watering(void) {

	if (watering_amount == 100) {
		PORTD |= (1 << PD2);
		actions[0].time = 5;

	} else if (watering_amount == 200) {
		PORTD |= (1 << PD2);
		actions[0].time = 10;
	}
	licznik_dni = 0;
}

void check_watering(void) {

	check_hour();
	if ((hours_temp == 7) && (minutes_temp == 0)) {
		licznik_dni++;
		if (licznik_dni == watering_freq) {
			check_if_water();
			if (watering_water_level_flag == 0) {
				watering();

			} else if (watering_water_level_flag == 8) {
				PORTD |= (1 << PD5);
				actions[5].time = 3;
				licznik_dni = 0;
			}
		}

	}
}


void check_lighting(void) {
	check_hour();

	if (hours_temp == 7) {
		if (is_light_on == 0) {
			PORTD |= (1 << PD4);	//start cyklu o�wietlenia
			is_light_on = 1;
		}
	} else if (hours_temp == (7 + lighting_on)) {
		if (is_light_on == 1) {
			PORTD &= ~(1 << PD4);	//zako�czenie cyklu o�wietlenia
			is_light_on = 0;
		}
	}
}

void check_hour(void) {
	I2C_READ_BUFFER( DS1307_ADDR, 0x00, 7, bufor);
	hours_temp = bcd2dec(bufor[2]);
	minutes_temp = bcd2dec(bufor[1]);
}

void save_parameters(void) {
	setSaved(program_saved);
	setTemperature(temperature_temp);
	setHumidity(humidity_temp);
	setLighting(lighting_on);
	setWateringAmount(watering_amount);
	setWateringFreq(watering_freq);

	eeprom_update_block(program_array, 0, sizeof(program_array));
}

void read_parameters(void) {
	eeprom_read_block(program_array, 0, sizeof(program_array));

	program_saved = getSaved();
	temperature_temp = getTemperature();
	humidity_temp = getHumidity();
	lighting_on = getLighting();
	watering_amount = getWateringAmount();
	watering_freq = getWateringFreq();
}

void save(void) {
	lcd_cursor_off();
	program_saved = 1;
	save_parameters();
	lcd_locate( 0 , 0 );
	lcd_str("Program zapisany");

	/*inicjalizacja timera*/
		licznik1_init();	//sekundowy

}
