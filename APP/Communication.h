#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include <stdint.h>    

extern uint16_t DHT1_data[2];
extern uint16_t DHT2_data[2];
extern uint16_t H2_1,H2_2,H2_3;//氢气值
extern uint16_t Speed;//风速

void Sensors_data_read(uint8_t data_len,uint8_t *buff);

#endif 
