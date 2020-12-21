/*
 * menu.h
 *
 *  Created on: 17 gru 2020
 *      Author: lukas
 */

#ifndef MENU_MENU_H_
#define MENU_MENU_H_

#define E_IDDLE	0
#define E_UP	1
#define E_DW	2
#define E_OK	3
#define E_PREV	4


extern volatile uint8_t	current_menu;
extern volatile uint8_t	menu_event;
extern volatile uint16_t value;
extern volatile uint8_t key_lock;		// extern oznacza, ¿e zmienna jest zadeklarowana w innym pliku

void change_menu();
void menu_idle(uint8_t a,uint8_t b,uint8_t c);
void wyswietl1(void);
void wyswietl2(void);
void zmiana_godzin(void);
void zmiana_minut(void);
void read_key();

#endif /* MENU_MENU_H_ */
