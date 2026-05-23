#ifndef __BSP_USART_H
#define __BSP_USART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usart.h"
#include "rtthread.h"
#include <rthw.h>
#include <string.h>

#define PRO_BUF_LEN		0xFF

extern uint8_t RxProBuf2[PRO_BUF_LEN];     //串口2接收缓存区
extern uint8_t RX485_buff[PRO_BUF_LEN];    //传感器数据处理缓冲区
extern uint8_t RX485_data_len;             //接收传感器数据长度
extern volatile uint8_t RX485_data_flag;   //接收到传感器数据标志位

void Uart_RxAgain(UART_HandleTypeDef *huart, uint8_t *RxProBuf);
void modbus_build_read_hold(UART_HandleTypeDef *huart, uint8_t addr,uint8_t cmd, uint16_t reg, uint16_t num, uint8_t *buf);
uint8_t modbus_parse_read_hold(uint8_t *buf, uint16_t len, uint16_t *data);
void BSP_uart_start(void);
void UART1_Printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif 
