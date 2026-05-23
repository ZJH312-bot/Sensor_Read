#include "bsp_usart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
uint8_t RxProBuf2[PRO_BUF_LEN];     //串口2接收缓存区
uint8_t RX485_buff[PRO_BUF_LEN];    //传感器数据处理缓冲区
uint8_t RX485_data_len;             //接收传感器数据长度
volatile uint8_t RX485_data_flag;   //接收到传感器数据标志位

void UART1_Printf(const char *format, ...)
{
    static char buf[256];
    va_list args;

    // 格式化
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    // 串口1发送
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)buf, strlen(buf));
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart->Instance == USART2)
    {
      rt_base_t lock = rt_hw_interrupt_disable();
      RX485_data_flag = 1;
      memcpy(RX485_buff,RxProBuf2,sizeof(RxProBuf2));
      RX485_data_len = Size;
      rt_hw_interrupt_enable(lock);

    }
}
/*使用DMA传输 */
/*重启串口接收 */
void Uart_RxAgain(UART_HandleTypeDef *huart, uint8_t *RxProBuf)
{
    HAL_UARTEx_ReceiveToIdle_DMA(huart, RxProBuf, PRO_BUF_LEN);
}

//串口启动
void BSP_uart_start(void)
{
	__HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT); /* 关闭DMA传输过半中断，避免多次进入中断 */

	__HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);    //启用串口闲时中断  串口二
	Uart_RxAgain(&huart2, RxProBuf2);               //启动串口接收
	
}



// Modbus CRC16 标准算法
static uint16_t modbus_crc16(uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}
/**
  * @brief 标准Modbus-RTU协议发送函数	        
  * @param  UART_HandleTypeDef 串口句柄
  * @param  addr 从机地址
  * @param  cmd 功能码
  * @param  reg 起始寄存器起始地址
  * @param  num 寄存器数量
  * @param  buf 发送缓冲区
  * @return 无
  */

void modbus_build_read_hold(UART_HandleTypeDef *huart, uint8_t addr,uint8_t cmd, uint16_t reg, uint16_t num, uint8_t *buf)
{
    buf[0] = addr;
    buf[1] = cmd;
    buf[2] = (reg >> 8) & 0xFF;
    buf[3] = reg & 0xFF;
    buf[4] = (num >> 8) & 0xFF;
    buf[5] = num & 0xFF;

    uint16_t crc = modbus_crc16(buf, 6);
    buf[6] = crc & 0xFF;
    buf[7] = (crc >> 8) & 0xFF;

   HAL_UART_Transmit_DMA(huart, buf, 8);
}

/**
 * @brief  解析 0x03 正确响应
 * @return 0 成功
 */
uint8_t modbus_parse_read_hold(uint8_t *buf, uint16_t len, uint16_t *data)
{
    // 基础长度校验
    if (len < 5) return 1;

    // 功能码判断
    if (buf[1] != 0x03) return 2;

    uint8_t byte_count = buf[2];
    uint16_t reg_count = byte_count / 2;

    // 长度必须匹配： addr(1) + func(1) + bytes(1) + N*2 + crc(2)
    if (len != 3 + byte_count + 2) return 3;

    // CRC 校验
    uint16_t crc_calc = modbus_crc16(buf, len - 2);
    uint16_t crc_recv = buf[len-1] << 8 | buf[len-2];
    if (crc_calc != crc_recv) return 4;

    // 解析所有寄存器（任意数量）
    for (uint16_t i = 0; i < reg_count; i++)
    {
        data[i] = (buf[3 + i*2] << 8) | buf[4 + i*2];
    }

    return 0;
}