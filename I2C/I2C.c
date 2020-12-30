/*
 * I2C.c
 *
 *  Created on: 22 gru 2020
 *      Author: lukas
 */
#include <avr/io.h>
#include "I2C.h"
#include "../LCD/lcd44780.h"

void I2C_START(void){
	TWCR = ( 1<<TWINT ) | ( 1<<TWEN ) | ( 1<<TWSTA );		// | (TWSR = 0x00) NIC NIE ZMIENI£O
	while ( !( TWCR & ( 1<<TWINT ) ) );
}

void I2C_STOP(void){
	TWCR = ( 1<<TWINT ) | ( 1<<TWEN ) | ( 1<<TWSTO );
	while ( ( TWCR & ( 1<<TWSTO ) ) );
}

void I2C_WRITE(uint8_t bajt){
	TWDR = bajt;
	TWCR = ( 1<<TWINT ) | ( 1<<TWEN );
	while( !( TWCR & ( 1<<TWINT ) ) );
}
uint8_t I2C_READ(uint8_t ack){
	TWCR = ( 1<<TWINT ) | ( ack<<TWEA ) | ( 1<<TWEN);
	while( !( TWCR & ( 1<<TWINT ) ) );
	return TWDR;
}

void I2C_WRITE_BUFFER(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf) {

	I2C_START();
	I2C_WRITE(SLA);
	I2C_WRITE(adr);
	while (len--) {
		I2C_WRITE(*buf++);
	}
	I2C_STOP();
}

void I2C_READ_BUFFER(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf){
	I2C_START();
	I2C_WRITE(SLA);
	I2C_WRITE(adr);
	I2C_START();
	I2C_WRITE(SLA + 1);
	while( len-- ) *buf++ = I2C_READ( len ? ACK : NACK);
	I2C_STOP();
}

void i2cSetBitrate(uint16_t bitrateKHz) {
	uint8_t bitrate_div;

	bitrate_div = ((F_CPU/1000l)/bitrateKHz);
	if(bitrate_div >= 16)
		bitrate_div = (bitrate_div-16)/2;

	TWBR = bitrate_div;
}

uint8_t dec2bcd(uint8_t dec){
	return ((dec/10)<<4)|(dec % 10);
}

uint8_t bcd2dec(uint8_t bcd){
	return ((((bcd)>>4)&0x0F)*10)+((bcd)&0x0F);
}

void pokaz_godzine( uint8_t sekundy, uint8_t minuty,uint8_t godziny)//czy to jest tutaj konieczne w ogóle?
{

	/*if(godziny<10)
	{
		lcd_locate(1,0);
		lcd_char('0');
		lcd_locate(1,1);
		lcd_int(godziny);
	}
		else
		{
			lcd_locate(1,0);
			lcd_int(godziny);
	}

		lcd_locate(1,2);
		lcd_char(':');

	if(minuty<10){
		lcd_locate(1,3);
		lcd_char('0');
		lcd_locate(1,4);
		lcd_int(minuty);

		}
		else{
			lcd_locate(1,3);
			lcd_int(minuty);
		}
			lcd_locate(1,5);
		lcd_char(':');

		if(sekundy<10){
			lcd_locate(1,6);
			lcd_char('0');
			lcd_locate(1,7);
			lcd_int(sekundy);
		}
		else{
		lcd_locate(1,6);
		lcd_int(sekundy);
}*/

}

