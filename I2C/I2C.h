/*
 * I2C.h
 *
 *  Created on: 22 gru 2020
 *      Author: lukas
 */

#ifndef I2C_I2C_H_
#define I2C_I2C_H_

#define ACK 1
#define NACK 0
#define DS1307_ADDR 0xD0	//adres RTC

void i2cSetBitrate(uint16_t bitrateKHz);
void I2C_START(void);
void I2C_STOP(void);
void I2C_WRITE(uint8_t bajt);
uint8_t I2C_READ(uint8_t ack);

void I2C_WRITE_BUFFER(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf);
void I2C_READ_BUFFER(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf);

//dodane przeze mnie
uint8_t dec2bcd(uint8_t dec);
uint8_t bcd2dec(uint8_t bcd);

#endif /* I2C_I2C_H_ */
