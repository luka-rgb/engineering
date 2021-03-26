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
extern volatile uint8_t loop;
extern volatile int loop_date;
volatile uint8_t licznik_dni;
volatile uint8_t poziom_wody_flaga;
volatile uint8_t water_level_flag;
char  ADC_pomiar[17];
volatile uint16_t value;
uint8_t program_saved;

//czy muszê dodac tutaj wszystkie zmienne, z których korzystam w menu.c??

void change_menu();
void menu_idle(uint8_t a,uint8_t b,uint8_t c);	//czy to jest potrzebne? jak zrobic ¿eby po stracie zasilania nie trzeba by³o za ka¿dym razem definiowac programu, jakas flaga podczas zapisu?
void display_change_date(void);
void change_day(void);
void change_month(void);
void check_water_level(void);
void change_year(void);	//dodac w menu.c change_date
void display_change_time(void);
void change_hour(void);
void change_minute(void);
void display_change_humidity(void);
void change_humidity(void);
void display_temperature(void);
void change_temperature(void);
void display_change_lighting(void);
void change_lighting(void);
void display_watering_amount(void);
void change_watering_amount(void);
void display_watering_freq(void);
void change_watering_freq(void);
void check_if_water(void);
void save_date_time(void);
void save(void);
void save_parameters(void);
void read_parameters(void);
void if_bissextile(void);
void show_date_time(void);
void show_temp_hum(void);
void read_key();
void watering(void);
void check_hour(void);

void printTime(uint8_t y, uint8_t x, uint8_t time);
void printInt(uint8_t y, uint8_t x, uint8_t hours);

uint8_t getSaved(void);
uint8_t getTemperature(void);
uint8_t getHumidity(void);
uint8_t getLighting(void);
uint8_t getWateringAmount(void);
uint8_t getWateringFreq(void);

void setSaved(uint8_t saved);
void setTemperature(uint8_t temperature);
void setHumidity(uint8_t humidity);
void setLighting(uint8_t Lighting);
void setWateringAmount(uint8_t wateringAmount);
void setWateringFreq(uint8_t wateringFreq);

#endif /* MENU_MENU_H_ */
