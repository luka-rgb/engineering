/*
 * menu.c
 *
 *  Created on: 17 gru 2020
 *      Author: lukas
 */

#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "../I2C/I2C.h"
#include "menu.h"
#include "../LCD/lcd44780.h"
#include "../DHT/dht.h"

volatile uint8_t current_menu = 0;		//pocz¹tkowy stan menu
volatile uint8_t menu_event = E_IDDLE;	//???
volatile uint8_t loop = 0;
char  ADC_pomiar[17];
volatile uint16_t value;


uint8_t bufor[7];
//zamiast zmiennych poni¿ej zrobic struct??
uint8_t seconds_temp = 0;
uint8_t minutes_temp = 30;
uint8_t hours_temp = 10;
uint8_t days_temp = 16;
uint8_t months_temp = 6;
uint8_t years_temp = 18;
uint8_t bissextile;
uint8_t humidity_temp = 40;			//okreslic mo¿liw¹ wartosc humidity
uint8_t temperature_temp = 10;		//okreslic mozliw¹ wartosc temperature
uint8_t lighting_on_temp = 12;
uint8_t lighting_off_temp = 12;
uint8_t watering_amount_temp;
uint8_t watering_freq_temp = 1;
uint8_t sekundy = 0, minuty, godziny, dni, miesiace, lata;

uint8_t water_level_flag;

const char START1[] PROGMEM = { "Wci" "\x82" "nij ok" };
const char START2[] PROGMEM = { "\x83" "eby" " zacz" "\x80" "\x81" };
const char M_1_1[] PROGMEM = { "Ustaw" };
const char M_1_2[] PROGMEM = { "czas i dat" "\x84" };
const char M_1_3[] PROGMEM = { "Czas i data" };
const char M_1_4[] PROGMEM = { "zapisane" };
const char M_2_1[] PROGMEM = { "Zdefiniuj" };
const char M_2_2[] PROGMEM = { "program" };
const char M_3_1[] PROGMEM = { "To save press ok " };
const char M_4_1[] PROGMEM = { "Modify program"};

//struktura menu-----------------------------------------------------------------------------------------------------------------
typedef struct {
	uint8_t next_state[5];						//przejœcia do nastêpnych stanów
	void (*callback)(void);						//funkcja zwrotna
	const char *first_line;						//tekst dla 1. linii LCD
	const char *second_line;					//tekst dla 2. linii LCD
} menu_item;

//menu glowne-----------------------------------------------------------------------------------------------------------------
//zmienic wszyskie change_parametr NA BEZ PETLI
//KURSOR ZOSTAJE PODSWIETLONY JAK RAZ SIE GO W£¥CZY
const menu_item menu[] = {	//DOKOÑCZYC MENU Z domyslnym I zrobic zapis
				//  LP UP DN OK PREV
				{ { 0, 0, 0, 1, 0 }, 			NULL					,	START1	,	START2	},
				{ { 1, 1, 1, 2, 1 }, 			NULL					,	M_1_1	,	M_1_2	},
				{ { 2, 2, 2, 3, 1 }, 			check_water_level				,	NULL	,	NULL	},
				{ { 3, 3, 3, 4, 2 }, 			change_month			,	NULL	,	NULL	},
				{ { 4, 4, 4, 5, 3 }, 			change_day				,	NULL	,	NULL	},
				{ { 5, 5, 5, 6, 4 },			change_hour				,	NULL	,	NULL	},
				{ { 6, 6, 6, 7, 5 },			change_minute			,	NULL	,	NULL	},
				{ { 7, 7, 7, 8, 6 }, 			save_date_time			,	M_1_3	,	M_1_4	},
				{ { 8, 8, 8, 9, 7 }, 			show_date_time			,	NULL	,	NULL	},
				{ { 9, 9, 9, 10, 8 }, 			NULL					,	M_2_1	,	M_2_2	},
				{ { 10, 10, 10, 11, 9 }, 		show_temp_hum			,	NULL	,	NULL	},
				{ { 11, 11, 11, 12, 10 }, 		change_humidity			,	NULL	,	NULL	},
				{ { 12, 12, 12, 13, 11 },		change_temperature		,	NULL	,	NULL	},
				{ { 13, 13, 13, 14, 12 },		change_lighting			,	NULL	,	NULL	},
				{ { 14, 14, 14, 15, 13 },		change_watering_amount	,	NULL	,	NULL	},				//zmodyfikowac
				{ { 15, 15, 15, 15, 14 },		change_watering_freq	,	NULL	,	NULL	},


				//{ { 8, 8, 8, 9, 7 }, 			show_date_time			,	NULL	,	NULL	},

						/*

				{ { 8, 8, 8, 9 , 7 }, 			change_watering_amount	,	NULL	,	NULL	},
				{ { 10, 10, 10, 11, 9 }, 		NULL					,	M_3_1	,	NULL	},
				{ { 11, 11, 11, 12, 9 },		save					,	NULL	,	NULL	},	//zapisz, dodac pozosta³e parametry, jak zrobic ¿eby zapisa³o i od razu przechodzi³o do kolejnego menu?
				{ { 12, 12, 12, 12, 12 }, 		NULL					,	M_4_1	,	NULL	},	//domyslne menu???
	*/	};


void change_menu() {
	//przejdz do nastepnego
	if (menu_event < 5) {//dlaczego tutaj jest 5? bo taka jest liczba przycisków?
		current_menu = menu[current_menu].next_state[menu_event];	//stanu

		//wyswietl komunikaty
		lcd_cls();

		//wyœwietl 1-sz¹ liniê
		if (menu[current_menu].first_line) {
			lcd_locate(0, 0);
			lcd_str_P((char*) menu[current_menu].first_line);
		}

		//wyœwietl 2-g¹ liniê
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

void display_change_time(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Godzina - GG/MM");
	if (hours_temp > 9) {
		lcd_locate(1, 0);
		lcd_int(hours_temp);
	} else {
		lcd_locate(1, 1);
		lcd_int(hours_temp);
	}
	lcd_locate(1, 2);
	lcd_char(':');
	if (minutes_temp > 9) {
		lcd_locate(1, 3);
		lcd_int(minutes_temp);
	} else {
		lcd_locate(1, 4);
		lcd_int(minutes_temp);
	}
}

void if_bissextile(void) {
	if ((years_temp % 4 == 0 && years_temp % 100 != 0) || years_temp % 400 == 0)
		bissextile = 1;
	else
		bissextile = 0;
}

void display_change_date(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Data");
	if (days_temp < 10) {
		lcd_locate(1, 0);
		lcd_int(0);
		lcd_locate(1, 1);
		lcd_int(days_temp);
	} else {
		lcd_locate(1, 0);
		lcd_int(days_temp);
	}
	lcd_locate(1, 2);
	lcd_char('.');
	if (months_temp < 10) {
		lcd_locate(1, 3);
		lcd_int(0);
		lcd_locate(1, 4);
		lcd_int(months_temp);
	} else {
		lcd_locate(1, 3);
		lcd_int(months_temp);
	}
	if(years_temp < 10) {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("200");
		lcd_locate(1,9);
		lcd_int(years_temp);
	} else if(years_temp < 100) {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("20");
		lcd_locate(1,8);
		lcd_int(years_temp);
	} else {
		lcd_locate(1, 5);
			lcd_char('.');
		lcd_locate(1,6);
		lcd_str("2");
		lcd_locate(1,7);
		lcd_int(years_temp);
	}
}

void change_day(void) {
	display_change_date();
	lcd_locate(1, 1);
	lcd_cursor_on();		//czy po wyjœciu z pêtli gaœnie kursor?
	if_bissextile();
	read_key();
	switch (menu_event) {
	case E_UP:
		if ( (bissextile == 1) && (months_temp == 2) ) {
			if (days_temp < 29) {
				days_temp++;
			}
		} else if ((bissextile == 0) && (months_temp == 2)) {
			if (days_temp < 28) {
				days_temp++;
			}
		} else if ( (months_temp == 4) || (months_temp == 6) || (months_temp == 9) || (months_temp == 11) ) {
			if (days_temp < 30) {
				days_temp++;
			}
		} else if ( (months_temp == 1) || (months_temp == 3) || (months_temp == 5) || (months_temp == 7) || (months_temp == 8) || (months_temp == 10) || (months_temp == 12) ) {
			if (days_temp < 31) {
				days_temp++;
			}
		}
		display_change_date();
		lcd_cursor_off();
		break;
	case E_DW:
		if (days_temp > 1) {
			days_temp--;
		}

		display_change_date();
		lcd_cursor_off();
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
		if (months_temp < 12) {
			months_temp++;
		}
		display_change_date();
		lcd_cursor_off();
		break;
	case E_DW:
		if (months_temp > 1) {
			months_temp--;
		}

		display_change_date();
		lcd_cursor_off();
		break;
	}
}

void change_year(void) {
	display_change_date();
	lcd_cursor_on();
	lcd_locate(1, 9);
	read_key();
	switch (menu_event) {
	case E_UP:
		if (years_temp < 100) {	//maksymalny rok dla DS1307 to 2100
			years_temp += 1;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	case E_DW:
		if (years_temp > 0) {
			years_temp--;
		}
		lcd_cursor_off();
		display_change_date();
		break;
	}
}

void check_water_level(void) {//dodac zmienn¹ zamiast wyœwietlania wartoœci na wyœwietlaczu, napisac if je¿eli nie ma wody - czerwona dioda ma zaswiecic
	ADCSRA = (1 << ADEN); 					//w³¹czenie przetwornika ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);	//ustawienie przeskalera na 8
	ADMUX |= (1 << REFS0);//wybór napiêcia odniesienia z wczeœniej zdefiniowanych makr, W TYM PRZYPADKU vcc
	ADMUX |= 1; 		//wybór u¿ywanego pinu ADC, domyœlanie u¿ywany jest PA0

	ADCSRA |= (1 << ADSC);	//start konwersji
	loop_until_bit_is_clear(ADCSRA, ADSC);
	value = ADC;
	sprintf(ADC_pomiar, "%d  ", value);

	if (value < 900) {	//nie ma wody

		water_level_flag = 0;
	} else {
		water_level_flag = 1;
	}
}

void check_if_water(void) {	//sprawdzic dzia³anie

	if (PINA & (1 << PA3)) {

		poziom_wody_flaga = 1;

	} else if (!(PINA & (1 << PA3))) {

		poziom_wody_flaga = 0;
	}

}

void display_watering_amount(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Ilo" "\x82" "\x83" " wody");
	lcd_locate(1, 2);
	lcd_int(watering_amount_temp);
	lcd_locate(1, 6);
	lcd_str("ml");

}

void change_watering_amount(void) {//jak okreslic ilosc wody
	display_watering_amount();
	lcd_locate(1, 3);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (watering_amount_temp <= 100) {
			watering_amount_temp += 100;
		}
		lcd_cursor_off();
		display_watering_amount();
		break;
	case E_DW:
		if (watering_amount_temp >= 200) {
			watering_amount_temp -= 100;
		}
		lcd_cursor_off();
		display_watering_amount();
		break;
	}
}


void display_watering_freq(void) {//dodac warunki na ró¿ne rzêdy wielkoœci dla kursora? dokoñczyc przerabianie funkcji
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Podlewanie co");
	if (watering_freq_temp < 2) {
		lcd_locate(1, 1);
		//lcd_cursor_on();
		lcd_int(watering_freq_temp);
		lcd_locate(1, 4);
		lcd_str("dzie" "\x85");
	} else if (watering_freq_temp > 1) {
		lcd_locate(1, 1);
		lcd_int(watering_freq_temp);
		lcd_locate(1, 4);
		lcd_str("dni");
	}
}

void change_watering_freq(void) {
		lcd_locate(1, 3);
		lcd_cursor_on();
		read_key();
		switch (menu_event) {
		case E_UP:
			if (watering_freq_temp < 7) {
				watering_freq_temp++;
				licznik_dni = watering_freq_temp;
			}
			display_watering_freq();
			break;
		case E_DW:
			if (watering_freq_temp > 2) {
				watering_freq_temp--;
				licznik_dni = watering_freq_temp;
			}
			display_watering_freq();
			break;
		}
	}


//sprawdzic uklad wszystkiich case w switchach i poprawic
void display_change_humidity(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Wilgotno" "\x82" "\x81");
	lcd_locate(1, 0);
	lcd_int(humidity_temp);
	lcd_locate(1, 2);
	lcd_str("%RH");
}

void change_humidity(void) {
	display_change_humidity();
	lcd_locate(1, 3);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (humidity_temp < 100) {
			humidity_temp++;
		}
		lcd_cursor_off();
		display_change_humidity();
		break;
	case E_DW:
		if (humidity_temp > 30) {
			humidity_temp--;
		}
		lcd_cursor_off();
		display_change_humidity();
		break;
	}

}

void display_change_lighting(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("\x82" "wiat" "\x86" "o" " w" "\x86" ":" "wy" "\x86");

	if (lighting_on_temp < 10) {
		lcd_locate(1, 9);
		lcd_int(lighting_on_temp);
		lcd_locate(1, 10);
		lcd_char(':');
		lcd_locate(1, 11);
		lcd_int(lighting_off_temp);
	} else {
		lcd_locate(1, 8);
		lcd_int(lighting_on_temp);
		lcd_locate(1, 10);
		lcd_char(':');
		lcd_locate(1, 11);
		lcd_int(lighting_off_temp);
	}

}

void change_lighting(void) {
	display_change_lighting();
	lcd_locate(1, 9);
	lcd_cursor_on();
	switch (menu_event) {
	case E_UP:
		if (lighting_on_temp < 24) {
			lighting_on_temp++;
			lighting_off_temp--;
		}
		lcd_cursor_off();
		display_change_lighting();
		break;
	case E_DW:
		if (lighting_on_temp > 0) {
			lighting_on_temp--;
			lighting_off_temp++;
		}
		lcd_cursor_off();
		display_change_lighting();
		break;
	}
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

void change_temperature(void) {
	display_temperature();
	lcd_locate(1, 4);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		if (temperature_temp < 40) {
			temperature_temp++;
		}
		display_temperature();
		break;
	case E_DW:
		if (temperature_temp > 20) {//jaki ustawic zakres mo¿liwy do ustawienia?
			temperature_temp--;
		}
		display_temperature();
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
		minutes_temp++;

		if (minutes_temp > 59) {
			minutes_temp = 0;
		}
		display_change_time();

		break;
	case E_DW:
		minutes_temp--;
		if (minutes_temp < 0 || hours_temp > 59) {
			minutes_temp = 59;
		}
		display_change_time();
		break;
	}
}

void change_hour(void) {
	display_change_time();
	lcd_locate(1, 1);
	lcd_cursor_on();
	read_key();
	switch (menu_event) {
	case E_UP:
		hours_temp++;
		if (hours_temp > 23) {
			hours_temp = 0;
		}
		display_change_time();
		break;
	case E_DW:
		hours_temp--;
		if (hours_temp < 0 || hours_temp > 24) {
			hours_temp = 23;
		}
		display_change_time();
		break;

	}
}

void read_key(void) {
	if (((value >= 1021) && (value <= 1023)) && (!key_lock)) {
		menu_event = E_UP;
		key_lock = 1;
	} else if ((value > 900) && (value <= 1020) && (!key_lock)) {
		menu_event = E_DW;
		key_lock = 1;
	} else if ((value > 590) && (value <= 690) && (!key_lock)) {
		menu_event = E_OK;
		key_lock = 1;
	} else if ((value > 420) && (value <= 520) && (!key_lock)) { //naciœniêty S4
		menu_event = E_PREV;
		key_lock = 1;
	} else if (((value < 100) && key_lock)) { //¿aden przycisk nie jest naciœniêty
		key_lock = 0;
		menu_event = E_IDDLE;
	}
}

void save_date_time(void) {
	bufor[0] = dec2bcd((seconds_temp) & 0x7F);		//operacja bitowa and gwarantuje, ¿e na pierwszym miejscu bêdzie 0 i osc zostanie w³¹czony i zacznie isc czas
	bufor[1] = dec2bcd(minutes_temp);
	bufor[2] = dec2bcd(hours_temp);
	bufor[4] = dec2bcd(days_temp);
	bufor[5] = dec2bcd(months_temp);
	bufor[6] = dec2bcd(years_temp);

	I2C_WRITE_BUFFER(DS1307_ADDR, 0x00, 7, bufor);
}

void save_parameters(void) {

}

void show_date_time(void) {
	I2C_READ_BUFFER( DS1307_ADDR, 0x00, 7, bufor);
	lcd_locate(1, 0);

	sekundy = bcd2dec(bufor[0]);
	minuty = bcd2dec(bufor[1]);
	godziny = bcd2dec(bufor[2]);
	dni = bcd2dec(bufor[4]);
	miesiace = bcd2dec(bufor[5]);
	lata = bcd2dec(bufor[6]);

	/*wyœwietlenie czasu na LCD*/
	lcd_cls();
	lcd_locate(0, 0);
	if (godziny < 10)
		lcd_str("0");
	lcd_int(godziny);
	lcd_str(":");
	if (minuty < 10)
		lcd_str("0");
	lcd_int(minuty);
	lcd_str(":");
	if (sekundy < 10)
		lcd_str("0");
	lcd_int(sekundy);

	/*wyswietlanie daty na lcd*/
	if (dni < 10) {
		lcd_locate(1, 0);
		lcd_int(0);
		lcd_locate(1, 1);
		lcd_int(dni);
	} else {
		lcd_locate(1, 0);
		lcd_int(dni);
	}
	lcd_locate(1, 2);
	lcd_char('.');
	if (miesiace < 10) {
		lcd_locate(1, 3);
		lcd_int(0);
		lcd_locate(1, 4);
		lcd_int(miesiace);
	} else {
		lcd_locate(1, 3);
		lcd_int(miesiace);
	}
	if (lata < 10) {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("200");
		lcd_locate(1, 9);
		lcd_int(lata);
	} else if (lata < 100) {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("20");
		lcd_locate(1, 8);
		lcd_int(lata);
	} else {
		lcd_locate(1, 5);
		lcd_char('.');
		lcd_locate(1, 6);
		lcd_str("2");
		lcd_locate(1, 7);
		lcd_int(lata);
	}

}

void watering(void) {		//zmienic czasy, które dobrac eksperymentalnie
	if (watering_amount_temp == 100) {
		PORTD |= (1 << PD2);
		_delay_ms(1000);
		PORTD &= ~(1 << PD2);
	} else if (watering_amount_temp == 200) {
		PORTD |= (1 << PD2);
		_delay_ms(2000);
		PORTD &= ~(1 << PD2);
	}

}

void check_hour(void) {//czy to dzia³a, sprawdzic po up³ywie czasu
	I2C_READ_BUFFER( DS1307_ADDR, 0x00, 7, bufor);
	godziny = bcd2dec(bufor[2]);
}

void save(void) {
	save_parameters();
	lcd_locate( 0 , 0 );
	lcd_str("Data saved");
}
