/*
 * main.c
 *
 *  Created on: 14 gru 2020
 *      Author: lukas
 */


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "LCD/lcd44780.h"
#include "MENU/menu.h"
#include <stdio.h>

#define REF_256 (1<<REFS1)|(1<<REFS0)	//makro na napiêcie odniesienia wewnêtrzne 2,56
#define REF_VCC (1<<REFS0)		//makro na napiêcie odniesienia VCC
char  ADC_pomiar[17];
volatile uint16_t value;
volatile uint8_t key_lock;	//volatile oznacza, ¿e zmienna mo¿e byc zmieniona z zewn¹trz


int main(void)
{


    lcd_init();

    	//inicjalizacja ADC
    	ADCSRA |= (1<<ADEN); 	//w³¹czenie przetwornika ADC
    	ADCSRA |= (1<<ADPS2);
    	ADCSRA |= (1<<ADPS1);	//preskaler = 16 teraz 64
    	ADMUX  |= REF_256; 		//wybór napiêcia odniesiena z wczeœniejszych makr
    	//ADMUX |= 0;				//wybranie pinu ADC, którego u¿ywam, zakomentowane bo domyœlnie u¿ywany jest PA0



    while(1){


				ADCSRA |= (1<<ADSC);	//start konwersji
        		loop_until_bit_is_clear(ADCSRA,ADSC);
        		value = ADC;
        		sprintf(ADC_pomiar,"%d  ", value);


        		read_key();
        		if (menu_event)
        								{change_menu();}
        								if (current_menu == E_IDDLE){//menu_idle(sekundy,minuty,godziny);
        								}

    }
    return 0;
}
