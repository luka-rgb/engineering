/*
 * interrupts.h
 *
 *  Created on: 8 kwi 2021
 *      Author: £ukasz ¯ukowski
 */

#ifndef INTERRUPTS_INTERRUPTS_H_
#define INTERRUPTS_INTERRUPTS_H_

void licznik0_init(void);
void licznik1_init(void);


typedef enum {WATER, HUM, TEMP, AIR, LIGHT, BUZZ } function_name;

typedef struct  {
	int16_t time;
	function_name name;
} akcja;

akcja actions[6];

void structure_init(void);
void check_actions(void);
void check_parameters(void);
void end_action(function_name name);


#endif /* INTERRUPTS_INTERRUPTS_H_ */
