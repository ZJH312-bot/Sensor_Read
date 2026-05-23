#ifndef __RT_TASK_H__
#define __RT_TASK_H__
#include <stdint.h>     //变量类型重定义
#include <stdio.h>      //printf使用
#include "rtthread.h"
#include <string.h>     //字符串操作支持
#include "usart.h"
#include "bsp_usart.h"
#include "Communication.h"

/*创建并启动任务*/
void Task_Init_Start(void);
/*任务调度*/
void Task_Schedule(void);


#endif /*__RT_Task_H__*/
