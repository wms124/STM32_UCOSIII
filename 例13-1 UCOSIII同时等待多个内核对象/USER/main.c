#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "sram.h"
#include "malloc.h"
#include "includes.h"
//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例13-1 UCOSIII 同时等待多个内核对象

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
#define TASK1_TASK_PRIO		4
//任务堆栈大小	
#define TASK1_STK_SIZE 		128
//任务控制块
OS_TCB Task1_TaskTCB;
//任务堆栈	
CPU_STK TASK1_TASK_STK[TASK1_STK_SIZE];
void task1_task(void *p_arg);


//任务优先级
#define TASK2_TASK_PRIO		5
//任务堆栈大小	
#define TASK2_STK_SIZE 		128
//任务控制块
OS_TCB Task2_TaskTCB;
//任务堆栈	
CPU_STK TASK2_TASK_STK[TASK2_STK_SIZE];
void task2_task(void *p_arg);

//任务优先级
#define MULTI_TASK_PRIO		6
//任务堆栈大小	
#define MULTI_STK_SIZE 		128
//任务控制块
OS_TCB Multi_TaskTCB;
//任务堆栈	
CPU_STK MULTI_TASK_STK[MULTI_STK_SIZE];
void multi_task(void *p_arg);

//LCD刷屏时使用的颜色
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

OS_SEM	Test_Sem1;			//信号量1		
OS_SEM	Test_Sem2;			//信号量2
OS_Q	Test_Q;				//消息队列						
#define QUEUE_NUM		10	//消息队列长度
#define CORE_OBJ_NUM	3	//内核对象个数，一共3个：2个信号量和一个消息队列						
						
//主函数
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init(168);  	//时钟初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	uart_init(115200);  //串口初始化
	LED_Init();         //LED初始化	
	LCD_Init();			//LCD初始化	
	KEY_Init();			//按键初始化
	FSMC_SRAM_Init();	//初始化SRAM
	my_mem_init(SRAMIN);//初始化内部RAM
	
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,30,200,16,16,"UCOSIII Examp 13-1");
	LCD_ShowString(30,50,200,16,16,"Pend Multi");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2015/5/20");
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,110,234,314);	
	LCD_DrawLine(5,130,234,130);
	POINT_COLOR = RED;
	LCD_ShowString(50,111,200,16,16,"ObjRdy_NUM: 0");
	POINT_COLOR = BLUE;
	
	OSInit(&err);		    	//初始化UCOSIII
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
	//创建信号量Test_Sem1
	OSSemCreate ((OS_SEM*	)&Test_Sem1,
                 (CPU_CHAR*	)"Test_Sem1",
                 (OS_SEM_CTR)0,		
                 (OS_ERR*	)&err);
	//创建信号量Test_Sem2
	OSSemCreate ((OS_SEM*	)&Test_Sem2,
                 (CPU_CHAR*	)"Test_Sem2",
                 (OS_SEM_CTR)0,		
                 (OS_ERR*	)&err);
	//创建消息队列
	OSQCreate ((OS_Q*		)&Test_Q,	//消息队列
                (CPU_CHAR*	)"KEY Msg",	//消息队列名称
                (OS_MSG_QTY	)QUEUE_NUM,	//消息队列长度
                (OS_ERR*	)&err);		//错误码
	//创建TASK1任务
	OSTaskCreate((OS_TCB 	* )&Task1_TaskTCB,		
				 (CPU_CHAR	* )"Task1 task", 		
                 (OS_TASK_PTR )task1_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TASK1_TASK_PRIO,     
                 (CPU_STK   * )&TASK1_TASK_STK[0],	
                 (CPU_STK_SIZE)TASK1_STK_SIZE/10,	
                 (CPU_STK_SIZE)TASK1_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);				
	//创建MULTI测试任务
	OSTaskCreate((OS_TCB 	* )&Multi_TaskTCB,		
				 (CPU_CHAR	* )"Multi task", 		
                 (OS_TASK_PTR )multi_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )MULTI_TASK_PRIO,     
                 (CPU_STK   * )&MULTI_TASK_STK[0],	
                 (CPU_STK_SIZE)MULTI_STK_SIZE/10,	
                 (CPU_STK_SIZE)MULTI_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);	
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}


//任务1的任务函数
void task1_task(void *p_arg)
{
	u8 key;
	OS_ERR err;
	u8 num;
	u8 *pbuf;
	static u8 msg_num;
	pbuf=mymalloc(SRAMIN,10);	//申请内存
	while(1)
	{
		key = KEY_Scan(0);  //扫描按键
		switch(key)
		{
			case KEY1_PRES:
				OSSemPost(&Test_Sem1,OS_OPT_POST_1,&err);//发送信号量1
				break;
			case KEY0_PRES:
				OSSemPost(&Test_Sem2,OS_OPT_POST_1,&err);//发送信号量2
			case WKUP_PRES:
				msg_num++;
				sprintf((char*)pbuf,"ALIENTEK %d",msg_num);
		
				//发送消息
				OSQPost((OS_Q*		)&Test_Q,		
						(void*		)pbuf,
						(OS_MSG_SIZE)10,
						(OS_OPT		)OS_OPT_POST_FIFO,
						(OS_ERR*	)&err);
				break;
		}
		num++;
		if(num==50)
		{
			num=0;
			LED0=~LED0;
		}
		OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_PERIODIC,&err);   //延时10ms
	}
}

//等待多个内核对象的任务函数
void multi_task(void *p_arg)
{	
	u8 num;
	OS_ERR err;
	OS_OBJ_QTY index;
	OS_PEND_DATA pend_multi_tbl[CORE_OBJ_NUM];	
	
	pend_multi_tbl[0].PendObjPtr=(OS_PEND_OBJ*)&Test_Sem1;
	pend_multi_tbl[1].PendObjPtr=(OS_PEND_OBJ*)&Test_Sem2;
	pend_multi_tbl[2].PendObjPtr=(OS_PEND_OBJ*)&Test_Q;
	
	while(1)
	{
		index=OSPendMulti((OS_PEND_DATA*	)pend_multi_tbl,	
						  (OS_OBJ_QTY		)CORE_OBJ_NUM,	//内核数量
						  (OS_TICK		   	)0,		
						  (OS_OPT         	)OS_OPT_PEND_BLOCKING,
						  (OS_ERR*			)&err);
		LCD_ShowNum(147,111,index,1,16);  					//显示当前有几个内核对象准备好了
		num++;
		LCD_Fill(6,131,233,313,lcd_discolor[num%14]);		//刷屏
		LED1 = ~LED1;
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_PERIODIC,&err);   //延时1s
	}
}



	


