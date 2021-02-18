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


int main(void) {

lcd_init();

//inicjalizacja ADC
	ADCSRA = (1<<ADEN); 	//w³¹czenie przetwornika ADC
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1);	//ustawienie przeskalera na 64
	ADMUX |= REF_256;	//wybór napiêcia odniesienia z wczeœniej zdefiniowanych makr
//	ADMUX |= 1; 		//wybór u¿ywanego pinu ADC, domyœlanie u¿ywany jest PA0

i2cSetBitrate(100); // USTAWIAMY prêdkoœæ 100 kHz na magistrali I2C




//przenieœc to do inicjalizacji
DDRA |= (1<<PA2);
DDRA &= ~(1<<PA3);

PORTA |= (1<<PA2);





while (1) {

	ADCSRA |= (1 << ADSC);	//start konwersji
				loop_until_bit_is_clear(ADCSRA, ADSC);
				value = ADC;
				sprintf(ADC_pomiar, "%d  ", value);

		//		lcd_locate(0,0);
		//		lcd_int(value);

	//if (PINA & (1<<PA3)) {
		//lcd_cls();
		//lcd_locate(1, 0);
		//lcd_str("connected");
		//_delay_ms(200);

	//} else if (!(PINA & (1<<PA3)))	{
		//lcd_cls();
		//lcd_locate(0,0);
		//lcd_str("not");
	//	lcd_locate(1,0);
//		lcd_str("connected");
		//_delay_ms(200);
	//}
	read_key();
	if (menu_event) {
		change_menu();
	}

	//lcd_char(stopien);
}
return 0;
}
