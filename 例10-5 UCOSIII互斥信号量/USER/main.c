#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "includes.h"
//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例10-5 UCOSIII互斥信号量

//UCOSIII中以下优先级用户程序不能使用，ALIENTEK
//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

//任务优先级
#define START_TASK_PRIO			10
//任务堆栈大小	
#define START_STK_SIZE 			128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define HIGH_TASK_PRIO			7
//任务堆栈大小	
#define HIGH_STK_SIZE 			128
//任务控制块
OS_TCB High_TaskTCB;
//任务堆栈	
CPU_STK HIGH_TASK_STK[HIGH_STK_SIZE];
void high_task(void *p_arg);


//任务优先级
#define MIDDLE_TASK_PRIO		8
//任务堆栈大小	
#define MIDDLE_STK_SIZE 		128
//任务控制块
OS_TCB Middle_TaskTCB;
//任务堆栈	
CPU_STK MIDDLE_TASK_STK[MIDDLE_STK_SIZE];
void middle_task(void *p_arg);

//任务优先级
#define LOW_TASK_PRIO			9
//任务堆栈大小	
#define LOW_STK_SIZE 			128
//任务控制块
OS_TCB Low_TaskTCB;
//任务堆栈	
CPU_STK LOW_TASK_STK[LOW_STK_SIZE];
void low_task(void *p_arg);

//LCD刷屏时使用的颜色
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

OS_MUTEX	TEST_MUTEX;		//定义一个互斥信号量

//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init(168);  //时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);   //串口初始化
	LED_Init();         //LED初始
	LCD_Init();			//LCD初始化	
	
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,30,200,16,16,"UCOSIII Examp 10-5");
	LCD_ShowString(30,50,200,16,16,"Mutex test");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2014/12/5");
	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区			 
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);      //开启UCOSIII
}


//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;
	
	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif	
		
	OS_CRITICAL_ENTER();	//进入临界区
	//创建一个互斥信号量
	OSMutexCreate((OS_MUTEX*	)&TEST_MUTEX,
				  (CPU_CHAR*	)"TEST_MUTEX",
                  (OS_ERR*		)&err);
	//创建HIGH任务
	OSTaskCreate((OS_TCB 	* )&High_TaskTCB,		
				 (CPU_CHAR	* )"High task", 		
                 (OS_TASK_PTR )high_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )HIGH_TASK_PRIO,     
                 (CPU_STK   * )&HIGH_TASK_STK[0],	
                 (CPU_STK_SIZE)HIGH_STK_SIZE/10,	
                 (CPU_STK_SIZE)HIGH_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);			
	//创建MIDDLE任务
	OSTaskCreate((OS_TCB 	* )&Middle_TaskTCB,		
				 (CPU_CHAR	* )"Middle task", 		
                 (OS_TASK_PTR )middle_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )MIDDLE_TASK_PRIO,     
                 (CPU_STK   * )&MIDDLE_TASK_STK[0],	
                 (CPU_STK_SIZE)MIDDLE_STK_SIZE/10,	
                 (CPU_STK_SIZE)MIDDLE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);		
	//创建LOW任务
	OSTaskCreate((OS_TCB 	* )&Low_TaskTCB,		
				 (CPU_CHAR	* )"Low task", 		
                 (OS_TASK_PTR )low_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )LOW_TASK_PRIO,     
                 (CPU_STK   * )&LOW_TASK_STK[0],	
                 (CPU_STK_SIZE)LOW_STK_SIZE/10,	
                 (CPU_STK_SIZE)LOW_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);					 
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}

//高优先级任务的任务函数
void high_task(void *p_arg)
{
	u8 num;
	OS_ERR err;
	
	CPU_SR_ALLOC();
	POINT_COLOR = BLACK;
	OS_CRITICAL_ENTER();
	LCD_DrawRectangle(5,110,115,314); 	//画一个矩形	
	LCD_DrawLine(5,130,115,130);		//画线
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,"High Task");
	OS_CRITICAL_EXIT();
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);   		//延时500ms
		num++;
		printf("high task Pend Mutex\r\n");
		OSMutexPend (&TEST_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);	//请求互斥信号量
		printf("high task Running!\r\n");
		LCD_Fill(6,131,114,313,lcd_discolor[num%14]); 				//填充区域
		LED1 = ~LED1;
		OSMutexPost(&TEST_MUTEX,OS_OPT_POST_NONE,&err);				//释放互斥信号量
		OSTimeDlyHMSM(0,0,0,500,OS_OPT_TIME_PERIODIC,&err);   		//延时500ms
	}
}

//中等优先级任务的任务函数
void middle_task(void *p_arg)
{	
	u8 num;
	OS_ERR err;
	CPU_SR_ALLOC();
	
	POINT_COLOR = BLACK;
	OS_CRITICAL_ENTER();
	LCD_DrawRectangle(125,110,234,314); //画一个矩形	
	LCD_DrawLine(125,130,234,130);		//画线
	POINT_COLOR = BLUE;
	LCD_ShowString(126,111,110,16,16,"Middle Task");
	OS_CRITICAL_EXIT();
	while(1)
	{
		num++;
		printf("middle task Running!\r\n");
		LCD_Fill(126,131,233,313,lcd_discolor[13-num%14]); //填充区域
		LED0 = ~LED0;
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);   //延时1s
	}
}

//低优先级任务的任务函数
void low_task(void *p_arg)
{	
	static u32 times;
	OS_ERR err;
	while(1)
	{
		OSMutexPend (&TEST_MUTEX,0,OS_OPT_PEND_BLOCKING,0,&err);//请求互斥信号量
		printf("low task Running!\r\n");
		for(times=0;times<20000000;times++)
		{
			OSSched();											//发起任务调度
		}
		OSMutexPost(&TEST_MUTEX,OS_OPT_POST_NONE,&err);			//释放互斥信号量
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);   	//延时1s
	}
}




	


