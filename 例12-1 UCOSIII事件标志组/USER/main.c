#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "sram.h"
#include "malloc.h"
#include "beep.h"
#include "key.h"
#include "includes.h"
//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例12-1 UCOSIII事件标志组

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
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define MAIN_TASK_PRIO		4
//任务堆栈大小	
#define MAIN_STK_SIZE 		128
//任务控制块
OS_TCB Main_TaskTCB;
//任务堆栈	
CPU_STK MAIN_TASK_STK[MAIN_STK_SIZE];
void main_task(void *p_arg);

//任务优先级
#define FLAGSPROCESS_TASK_PRIO	5
//任务堆栈大小	
#define FLAGSPROCESS_STK_SIZE 	128
//任务控制块
OS_TCB Flagsprocess_TaskTCB;
//任务堆栈	
CPU_STK FLAGSPROCESS_TASK_STK[FLAGSPROCESS_STK_SIZE];
//任务函数
void flagsprocess_task(void *p_arg);


//LCD刷屏时使用的颜色
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

////////////////////////事件标志组//////////////////////////////
#define KEY0_FLAG		0x01
#define KEY1_FLAG		0x02
#define KEYFLAGS_VALUE	0X00						
OS_FLAG_GRP	EventFlags;		//定义一个事件标志组
						
						
//加载主界面
void ucos_load_main_ui(void)
{
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,30,200,16,16,"UCOSIII Examp 12-1");
	LCD_ShowString(30,50,200,16,16,"Event Flags");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2014/12/9");
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,130,234,314);	//画矩形
	POINT_COLOR = BLUE;
	LCD_ShowString(30,110,220,16,16,"Event Flags Value:0");
}


//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init(168);  //时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);   //串口初始化
	LED_Init();         //LED初始化	
	LCD_Init();			//LCD初始化	
	KEY_Init();			//按键初始化
	BEEP_Init();		//初始化蜂鸣器
	FSMC_SRAM_Init();	//初始化SRAM
	my_mem_init(SRAMIN);//初始化内部RAM
	ucos_load_main_ui();//加载主UI
	
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
	//创建一个事件标志组
	OSFlagCreate((OS_FLAG_GRP*)&EventFlags,		//指向事件标志组
                 (CPU_CHAR*	  )"Event Flags",	//名字
                 (OS_FLAGS	  )KEYFLAGS_VALUE,	//事件标志组初始值
                 (OS_ERR*  	  )&err);			//错误码
	//创建主任务
	OSTaskCreate((OS_TCB*     )&Main_TaskTCB,		
				 (CPU_CHAR*   )"Main task", 		
                 (OS_TASK_PTR )main_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )MAIN_TASK_PRIO,     
                 (CPU_STK*    )&MAIN_TASK_STK[0],	
                 (CPU_STK_SIZE)MAIN_STK_SIZE/10,	
                 (CPU_STK_SIZE)MAIN_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);						
	//创建MSGDIS任务
	OSTaskCreate((OS_TCB*     )&Flagsprocess_TaskTCB,		
				 (CPU_CHAR*   )"Flagsprocess task", 		
                 (OS_TASK_PTR )flagsprocess_task, 			
                 (void* 	  )0,					
                 (OS_PRIO	  )FLAGSPROCESS_TASK_PRIO,     
                 (CPU_STK* 	  )&FLAGSPROCESS_TASK_STK[0],	
                 (CPU_STK_SIZE)FLAGSPROCESS_STK_SIZE/10,	
                 (CPU_STK_SIZE)FLAGSPROCESS_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void* 	  )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR* 	  )&err);	
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}

//主任务的任务函数
void main_task(void *p_arg)
{
	u8 key,num;
	OS_FLAGS flags_num;
	OS_ERR err;
	while(1)
	{
		key = KEY_Scan(0);  //扫描按键
		if(key == KEY0_PRES)
		{
			//向事件标志组EventFlags发送标志
			flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
								 (OS_FLAGS	  )KEY0_FLAG,
								 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
					             (OS_ERR*	  )&err);
			LCD_ShowxNum(174,110,flags_num,1,16,0);
			printf("事件标志组EventFlags的值:%d\r\n",flags_num);
		}
		else if(key == KEY1_PRES)
		{
			//向事件标志组EventFlags发送标志
			flags_num=OSFlagPost((OS_FLAG_GRP*)&EventFlags,
								 (OS_FLAGS	  )KEY1_FLAG,
								 (OS_OPT	  )OS_OPT_POST_FLAG_SET,
								 (OS_ERR*     )&err);
			LCD_ShowxNum(174,110,flags_num,1,16,0);
			printf("事件标志组EventFlags的值:%d\r\n",flags_num);
		}
		num++;
		if(num==50)
		{
			num=0;
			LED0 = ~LED0;
		}
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);   //延时10ms
	}
}

//事件标志组处理任务
void flagsprocess_task(void *p_arg)
{
	u8 num;
	OS_ERR err; 
	while(1)
	{
		//等待事件标志组
		OSFlagPend((OS_FLAG_GRP*)&EventFlags,
				   (OS_FLAGS	)KEY0_FLAG+KEY1_FLAG,
		     	   (OS_TICK     )0,
				   (OS_OPT	    )OS_OPT_PEND_FLAG_SET_ALL+OS_OPT_PEND_FLAG_CONSUME,
				   (CPU_TS*     )0,
				   (OS_ERR*	    )&err);
		num++;
		LED1 = ~LED1;
		LCD_Fill(6,131,233,313,lcd_discolor[num%14]);
		printf("事件标志组EventFlags的值:%d\r\n",EventFlags.Flags);
		LCD_ShowxNum(174,110,EventFlags.Flags,1,16,0);
	}
}





	


