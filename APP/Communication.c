#include "Communication.h"
#include "bsp_usart.h"

uint16_t DHT1_data[2];//温湿度1
uint16_t DHT2_data[2];//温湿度2
uint16_t H2_1,H2_2,H2_3;//氢气值
uint16_t Speed;//风速
//传感器数据解析
void Sensors_data_read(uint8_t data_len,uint8_t *buff)
{
	if(RX485_data_flag==1)
	{
		uint8_t address = buff[0];
		switch(address)
		{
			case 1:
			{
				modbus_parse_read_hold(buff,data_len,DHT1_data);
			}break;
			case 2:
			{
				modbus_parse_read_hold(buff,data_len,DHT2_data);
			}break;
			case 3:
			{
				if(data_len>7)return;
				modbus_parse_read_hold(buff,data_len,&H2_1);
			}break;
			case 4:
			{
				if(data_len>7)return;
				modbus_parse_read_hold(buff,data_len,&H2_2);
			}break;
			case 5:
			{
				if(data_len>7)return;
				modbus_parse_read_hold(buff,data_len,&H2_3);
			}break;
			case 6:
			{
				if(data_len>7)return;
				modbus_parse_read_hold(buff,data_len,&Speed);
			}break;
		}
		RX485_data_flag=0;
	}
}