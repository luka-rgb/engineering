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

#include "menu.h"
#include "lcd44780.h"

volatile uint8_t current_menu = 0;		//pocz¹tkowy stan menu
volatile uint8_t menu_event = E_IDDLE;	//???

uint8_t hours_temp = 9;
uint8_t minuts_temp = 30;
uint8_t dni_temp = 16;
uint8_t dnityg_temp = 3;
uint8_t miesiace_temp = 6;
uint8_t lata_temp = 17;

const char START1[] PROGMEM = { "Wcisnij ok, zeby" };
const char START2[] PROGMEM = { "zaczac." };
const char M_1_1[] PROGMEM = { "Ustawianie" };
const char M_1_2[] PROGMEM = { "czasu i daty" };
const char M_1_3[] PROGMEM = { " dzien " };
const char M_1_4[] PROGMEM = { " miesiac " };
const char M_1_5[] PROGMEM = { " rok " };
const char M_1_6[] PROGMEM = { " godzina " };
const char M_1_7[] PROGMEM = { " minuta " };
const char M_2_1[] PROGMEM = { "Zdefiniuj" };
const char M_2_2[] PROGMEM = { "program" };
const char M_2_3[] PROGMEM = { " wilgotnosc " };
const char M_2_4[] PROGMEM = { " xx RH% " };
const char M_2_5[] PROGMEM = { " temperatura " };
const char M_2_6[] PROGMEM = { " xx oC " };
const char M_2_7[] PROGMEM = { " oswietlenie " };
const char M_2_8[] PROGMEM = { " xx : yy " };
const char M_2_9[] PROGMEM = { " nawadnianie " };
const char M_2_10[] PROGMEM = { " ilosc wody [ml]" };
const char M_2_11[] PROGMEM = { " nawadniaj co " };
const char M_3_1[] PROGMEM = { " zapisz " };

//struktura menu-----------------------------------------------------------------------------------------------------------------
typedef struct {
	uint8_t next_state[5];						//przejœcia do nastêpnych stanów
	void (*callback)(void);					//funkcja zwrotna
	const char *first_line;						//tekst dla 1. linii LCD
	const char *second_line;					//tekst dla 2. linii LCD
} menu_item;

//menu glowne-----------------------------------------------------------------------------------------------------------------

const menu_item menu[] = {	//DOKOÑCZYC MENU Z WYBIERANIEM I POPRAWIC KONCOWKE
				//	 LP	 UP  DN  OK  PREV
				{ { 0, 0, 0, 1, 0 }, NULL, START1, START2 },
				{ { 1, 1, 1, 2, 0 }, NULL, M_1_1, M_1_2 },
				{ { 2, 2, 2, 3, 1 }, NULL, M_1_3, NULL },
				{ { 3, 3, 3, 4, 2 }, NULL, M_1_4, NULL },
				{ { 4, 4, 4, 5, 3 }, NULL, M_1_5, NULL },
				{ { 5, 5, 5, 6, 4 }, zmiana_godzin, M_1_6, NULL },
				{ { 6, 6, 6, 7, 5 }, zmiana_minut, M_1_7, NULL },
				{ { 7, 7, 7, 8, 6 }, NULL, M_2_1, M_2_2 },
				{ { 8, 8, 8, 9, 7 }, NULL, M_2_3, NULL },//JAK ZROBIC WYBIERANIE WARTOSCI?
				{ { 9, 9, 9, 10, 9 }, NULL, M_2_4, NULL },
				{ { 10, 10, 10, 11 , 10 }, NULL, M_2_5, NULL },
				{ { 11, 11, 11, 12, 11 }, NULL, M_2_6, NULL },
				{ { 12, 12, 12, 12, 12 }, NULL, NULL, NULL },
		};

void change_menu() {
	//przejdz do nastepnego
	if (menu_event < 5) {	//dlaczego tutaj jest 5? bo taka jest liczba przycisków?
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

void wyswietl1(void) {
	lcd_cls();
	lcd_locate(0, 0);
	lcd_str("Data - hh/mm");
	if (hours_temp > 9) {
		lcd_locate(1, 0);
		lcd_int(hours_temp);
	} else {
		lcd_locate(1, 0);
		lcd_int(0);
		lcd_locate(1, 1);
		lcd_int(hours_temp);
	}
	lcd_locate(1, 2);
	lcd_char(':');
	if (minuts_temp > 9) {
		lcd_locate(1, 3);
		lcd_int(minuts_temp);
	} else {
		lcd_locate(1, 3);
		lcd_int(0);
		lcd_locate(1, 4);
		lcd_int(minuts_temp);
	}
}

void wyswietl2(void){
	lcd_cls();
	lcd_locate(0,0);lcd_str("Data - dd/mm/yy");
	if(dni_temp<9){lcd_locate(1,0);lcd_int(0);lcd_locate(1,1);lcd_int(dni_temp);}
	else{lcd_locate(1,0);lcd_int(dni_temp);}
	lcd_locate(1,2);lcd_char('.');
	if(miesiace_temp<9){lcd_locate(1,3);lcd_int(0);lcd_locate(1,4);lcd_int(miesiace_temp);}
	else{lcd_locate(1,3);lcd_int(miesiace_temp);}
	lcd_locate(1,5);
	lcd_char('.');lcd_locate(1,6);lcd_int(lata_temp);
}

void zmiana_godzin(void) {
	wyswietl1();
uint8_t petla1=1;

while(petla1){
	lcd_locate(1,1);
	lcd_cursor_on();
read_key();
		switch (menu_event){
		case E_UP: hours_temp++; if(hours_temp>23){hours_temp=0;}; wyswietl1();  petla1=0; break;
		case E_DW: hours_temp--; if(hours_temp<0||hours_temp>24){hours_temp=23;}; wyswietl1(); petla1=0; break;
		case E_OK: petla1=0; break;

		}
}

}

void zmiana_minut(void) {
	wyswietl1();
	uint8_t petla1 = 1;
		while(petla1)	{
			lcd_locate(1, 4);
			lcd_cursor_on();
			read_key();
			switch (menu_event) {
				case E_UP:
					minuts_temp++;

						if (minuts_temp > 59) {
							minuts_temp = 0;
						}
					wyswietl1();
					petla1=0;
					break;
				case E_DW:
					minuts_temp--;
						if (minuts_temp < 0 || hours_temp > 59) {
							minuts_temp = 59;
						}
					wyswietl1();
					petla1=0;
					break;
				case E_OK:
					petla1 = 0;
					break;
								}
						}

	}
void read_key(void) {
	if (((value >= 1021) && (value <= 1023)) && (!key_lock)) {
		menu_event = E_UP;
		key_lock = 1;
		return;
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

