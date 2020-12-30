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

volatile uint8_t int0_flag=1;



int main(void)
{

		// Przerwanie INT0
			MCUCR |= (1<<ISC01);	// wyzwalanie zboczem opadaj¹cym
			GICR |= (1<<INT0);		// odblokowanie przerwania
			PORTD |= (1<<PD2);		// podci¹gniêcie pinu INT0 do VCC

		lcd_init();

			i2cSetBitrate( 100 ); // USTAWIAMY prêdkoœæ 100 kHz na magistrali I2C

			sei();


			//char stopien = 0xF3;
			//lcd_char(stopien);


    /*inicjalizacja ADC*/
    ADCSRA |= (1 << ADEN);	 	//w³¹czenie przetwornika ADC
	ADCSRA |= (1 << ADPS2);
	ADCSRA |= (1 << ADPS1);		//preskaler = 16 teraz 64
	ADMUX |= REF_256; 			//wybór napiêcia odniesiena z wczeœniejszych makr
	//ADMUX |= 0;				//wybranie pinu ADC, którego u¿ywam, zakomentowane bo domyœlnie u¿ywany jest PA0




    while(1)
    	{

    	ADCSRA |= (1 << ADSC);	//start konwersji
		loop_until_bit_is_clear(ADCSRA, ADSC);
		value = ADC;
		/*sprintf(ADC_pomiar, "%d  ", value);
*/
	/*	read_key();
		if (menu_event)
			{
				change_menu();
			}
*/


		//lcd_locate(0,14);
		//lcd_char(stopien);
		get_temp_hum();
						_delay_ms(1000);

    	}
    return 0;
}
