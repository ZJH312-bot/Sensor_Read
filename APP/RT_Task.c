/*************************************************************************
*                       RT-Thread任务管理
*
*此处使用原版RTOS，移植时注意核对参数
**************************************************************************/
#include "RT_Task.h"
#define STACK_SIZE      4096            //线程堆栈空间
#define THREAD_PRIORITY1   3            //任务1优先级
#define THREAD_PRIORITY2   5            //任务2优先级
#define THREAD_PRIORITY3   6            //任务3优先级
#define TASK1_CYCLE      10            	//任务1周期10ms
#define TASK2_CYCLE      50             //任务2周期
#define TASK3_CYCLE      100 						//任务3周期
#define TICK_COUNT_MAX_VALUE 327680000  //滴答定时器计数最大值


ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[STACK_SIZE];
static struct rt_thread Task1;
static char thread2_stack[STACK_SIZE];
static struct rt_thread Task2;
static char thread3_stack[STACK_SIZE];
static struct rt_thread Task3;

uint8_t count;
float Hum1,Hum2,Temper1,Temper2;
float Wind_Speed;//风速

/*任务1入口函数*/
static void Thread_Entry1(void* param)
{
	for(;;)
  {
				

		rt_thread_suspend(&Task1);  //挂起线程
		rt_schedule();              //启动调度器
	}
}

/*任务2入口函数*/
static void Thread_Entry2(void* param)
{
	for(;;)
	{

		Sensors_data_read(RX485_data_len,RX485_buff);
		rt_thread_suspend(&Task2);  //挂起线程
		rt_schedule();              //启动调度器
	}
}
uint8_t Print_Buffe[8];
/*任务3入口函数*/
static void Thread_Entry3(void* parameter)
{
	for(;;)
	{
		UART1_Printf("%u,%u,%u,%u,%u,%u,%u,%u\r\n",DHT1_data[0],DHT1_data[1],DHT2_data[0],DHT2_data[1],H2_1,H2_2,H2_3,Speed);
		if(RX485_data_flag==0)
		{
			if(count==0)modbus_build_read_hold(&huart2, 0x01, 0x03, 0x0000, 0x0002, Print_Buffe);//温湿度
			if(count==1)modbus_build_read_hold(&huart2, 0x02, 0x03, 0x0000, 0x0002, Print_Buffe);//温湿度
			if(count==2)modbus_build_read_hold(&huart2, 0x03, 0x03, 0x000E, 0x0001, Print_Buffe);//氢气
			if(count==3)modbus_build_read_hold(&huart2, 0x04, 0x03, 0x000E, 0x0001, Print_Buffe);//氢气
			if(count==4)modbus_build_read_hold(&huart2, 0x05, 0x03, 0x000E, 0x0001, Print_Buffe);//氢气
			if(count==5)modbus_build_read_hold(&huart2, 0x06, 0x03, 0x0004, 0x0001, Print_Buffe);//风速
		}
		count++;
		if(count>=6)count=0;
		Hum1=DHT1_data[0]*0.1;		Hum2=DHT2_data[0]*0.1;
		Temper1=DHT1_data[1]*0.1;	Temper2=DHT2_data[1]*0.1;
		Wind_Speed=Speed*0.01;
//		if(RX485_data_flag==0)modbus_build_read_hold(&huart2, 0x05, 0x03, 0x000E, 0x0001, Print_Buffe);//氢气
//		if(RX485_data_flag==0)modbus_build_read_hold(&huart2, 0x06, 0x03, 0x0004, 0x0001, Print_Buffe);//风速	
//		modbus_build_read_hold(&huart2, 0x01, 0x06, 0x002F, 0x0005, Print_Buffe);//氢气地址更改
//			modbus_build_read_hold(&huart2, 0x01, 0x06, 0x0000, 0x0006, Print_Buffe);//风速地址修改
		rt_thread_suspend(&Task3);  //挂起线程
		rt_schedule();              //启动调度器
	}
}
/*创建并启动任务*/
void Task_Init_Start(void)
{    
    rt_thread_init(&Task1, "Thread1", Thread_Entry1, RT_NULL, &thread1_stack[0],
                   sizeof(thread1_stack), THREAD_PRIORITY1, 5);
    rt_thread_startup(&Task1);
                    
    rt_thread_init(&Task2, "Thread2", Thread_Entry2, RT_NULL, &thread2_stack[0],
                   sizeof(thread2_stack), THREAD_PRIORITY2, 5);
    rt_thread_startup(&Task2);
                    
    rt_thread_init(&Task3, "Thread3", Thread_Entry3, RT_NULL, &thread3_stack[0],
                   sizeof(thread3_stack), THREAD_PRIORITY3, 5);
    rt_thread_startup(&Task3);
	
}

/*任务调度*/
void Task_Schedule(void)
{
	static uint32_t task_count = 0;
	//任务1周期
	if(task_count%TASK1_CYCLE == 0)   rt_thread_resume(&Task1);
	//任务2周期
	if(task_count%TASK2_CYCLE == 0)   rt_thread_resume(&Task2);
  //任务3周期	
	if(task_count%TASK3_CYCLE == 0)   rt_thread_resume(&Task3);
	// 任务4：每1个FAST_TASK_CYCLE执行一次
	rt_schedule();
	task_count++;
    //计数值达到一定值后清零，防止任务调度错乱
	if(task_count == TICK_COUNT_MAX_VALUE)	task_count = 0; 
}
void SysTick_Handler(void)
{
    rt_interrupt_enter();
    
    rt_tick_increase();
		Task_Schedule();
    rt_interrupt_leave();
}
