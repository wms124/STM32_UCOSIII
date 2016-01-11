#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "sram.h"
#include "beep.h"
#include "key.h"
#include "includes.h"
//ALIENTEK 探索者STM32F407开发板 UCOSIII实验
//例14-1 UCOSIII 内存管理

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
//任务函数
void main_task(void *p_arg);

//任务优先级
#define MEMMANAGE_TASK_PRIO	5
//任务堆栈大小
#define MEMMANAGE_STK_SIZE	128
//任务控制块
OS_TCB	MemManage_TaskTCB;
//任务堆栈
CPU_STK MEMMANAGE_TASK_STK[MEMMANAGE_STK_SIZE];
//任务函数
void memmanage_task(void *p_arg);

//定义一个存储区
OS_MEM INTERNAL_MEM;	
//存储区中存储块数量
#define INTERNAL_MEM_NUM		5
//每个存储块大小
//由于一个指针变量占用4字节所以块的大小一定要为4的倍数
//而且必须大于一个指针变量(4字节)占用的空间,否则的话存储块创建不成功
#define INTERNAL_MEMBLOCK_SIZE	100  
//存储区的内存池，使用内部RAM
CPU_INT08U Internal_RamMemp[INTERNAL_MEM_NUM][INTERNAL_MEMBLOCK_SIZE];

//定义一个存储区
OS_MEM EXTERNAL_MEM;	
//存储区中存储块数量
#define EXTRENNAL_MEM_NUM		5
//每个存储块大小
//由于一个指针变量占用4字节所以块的大小一定要为4的倍数
//而且必须大于一个指针变量(4字节)占用的空间,否则的话存储块创建不成功
#define EXTERNAL_MEMBLOCK_SIZE	100
//存储区的内存池，使用外部SRAM
__align(32) volatile CPU_INT08U External_RamMemp[EXTRENNAL_MEM_NUM][EXTERNAL_MEMBLOCK_SIZE]  __attribute__((at(0X68000000)));	

					
//加载主界面
void ucos_load_main_ui(void)
{
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"Explorer STM32F4");	
	LCD_ShowString(30,30,200,16,16,"UCOSIII Examp 14-1");
	LCD_ShowString(30,50,200,16,16,"Memory Manage");
	POINT_COLOR = BLUE;
	LCD_ShowString(30,70,200,16,16,"INTERNAL_MEM:");
	POINT_COLOR = RED;
	LCD_ShowString(30,90,200,16,16,"KEY_UP:malloc KEY_1:free");
	POINT_COLOR = BLUE;
	LCD_ShowString(30,110,200,16,16,"EXTERNAL_MEM:");
	POINT_COLOR = RED;
	LCD_ShowString(30,130,200,16,16,"KEY2:malloc KEY0:free");
	POINT_COLOR = BLACK;
	LCD_DrawLine(0,147,239,147);
	POINT_COLOR = BLUE;
	LCD_ShowString(5,148,200,16,16,"INTERNAL_MEM:");
	LCD_ShowString(5,228,200,16,16,"EXTERNAL_MEM:");
	POINT_COLOR = RED; 
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
	ucos_load_main_ui();//加载主UI1
	
	OSInit(&err);		    //初始化UCOSIII
	OS_CRITICAL_ENTER();	//进入临界区	
	//创建一个存储分区
	OSMemCreate((OS_MEM*	)&INTERNAL_MEM,
				(CPU_CHAR*	)"Internal Mem",
				(void*		)&Internal_RamMemp[0][0],
				(OS_MEM_QTY	)INTERNAL_MEM_NUM,
				(OS_MEM_SIZE)INTERNAL_MEMBLOCK_SIZE,
				(OS_ERR*	)&err);
	//创建一个存储分区
	OSMemCreate((OS_MEM*	)&EXTERNAL_MEM,
				(CPU_CHAR*	)"External Mem",
				(void*		)&External_RamMemp[0][0],
				(OS_MEM_QTY	)EXTRENNAL_MEM_NUM,
				(OS_MEM_SIZE)EXTERNAL_MEMBLOCK_SIZE,
				(OS_ERR*	)&err);
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
	//创建一个内存块检查任务
	OSTaskCreate((OS_TCB*     )&MemManage_TaskTCB,		
				 (CPU_CHAR*   )"MemManage task", 		
                 (OS_TASK_PTR )memmanage_task, 			
                 (void*       )0,					
                 (OS_PRIO	  )MEMMANAGE_TASK_PRIO,     
                 (CPU_STK*    )&MEMMANAGE_TASK_STK[0],	
                 (CPU_STK_SIZE)MEMMANAGE_STK_SIZE/10,	
                 (CPU_STK_SIZE)MEMMANAGE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);
	OS_CRITICAL_EXIT();	//退出临界区
	OSTaskDel((OS_TCB*)0,&err);	//删除start_task任务自身
}

//主任务的任务函数
void main_task(void *p_arg)
{
	u8 key,num;
	static u8 internal_memget_num;
	static u8 external_memget_num;
	CPU_INT08U *internal_buf;
	CPU_INT08U *external_buf;
	OS_ERR err;
	while(1)
	{
		key = KEY_Scan(0);  //扫描按键
		switch(key)
		{
			case WKUP_PRES:		//按下KEY_UP键
				internal_buf=OSMemGet((OS_MEM*)&INTERNAL_MEM,
								      (OS_ERR*)&err);
				if(err == OS_ERR_NONE) //内存申请成功
				{
					printf("internal_buf内存申请之后的地址为:%#x\r\n",(u32)(internal_buf));
					LCD_ShowString(30,180,200,16,16,"Memory Get success!  ");
					internal_memget_num++;
					POINT_COLOR = BLUE;
					sprintf((char*)internal_buf,"INTERNAL_MEM Use %d times",internal_memget_num);
					LCD_ShowString(30,196,200,16,16,internal_buf); 
					POINT_COLOR = RED;
				}
				if(err == OS_ERR_MEM_NO_FREE_BLKS) //内存块不足
				{
					LCD_ShowString(30,180,200,16,16,"INTERNAL_MEM Empty!  ");
				}
				break;
			case KEY1_PRES:
				if(internal_buf != NULL) //internal_buf不为空就释放内存
				{
					OSMemPut((OS_MEM*	)&INTERNAL_MEM,		//释放内存
							 (void*		)internal_buf,
							 (OS_ERR* 	)&err);
					printf("internal_buf内存释放之后的地址为:%#x\r\n",(u32)(internal_buf));
					LCD_ShowString(30,180,200,16,16,"Memory Put success!   ");
				}
				break;
			case KEY2_PRES:
				external_buf=OSMemGet((OS_MEM*)&EXTERNAL_MEM,
									  (OS_ERR*)&err);
				if(err == OS_ERR_NONE) //内存申请成功
				{
					printf("external_buf内存申请之后的地址为:%#x\r\n",(u32)(external_buf));
					LCD_ShowString(30,260,200,16,16,"Memory Get success!  ");
					external_memget_num++;
					POINT_COLOR = BLUE;
					sprintf((char*)external_buf,"EXTERNAL_MEM Use %d times",external_memget_num);
					LCD_ShowString(30,276,200,16,16,external_buf); 
					POINT_COLOR = RED;
				}
				if(err == OS_ERR_MEM_NO_FREE_BLKS) //内存块不足
				{
					LCD_ShowString(30,260,200,16,16,"EXTERNAL_MEM Empty!  ");
				}
				break;
			case KEY0_PRES:
				if(external_buf != NULL) //external_buf不为空就释放内存
				{
					OSMemPut((OS_MEM*	)&EXTERNAL_MEM,		//释放内存
							 (void*		)external_buf,
							 (OS_ERR* 	)&err);
					printf("external_buf内存释放之后的地址为:%#x\r\n",(u32)(external_buf));
					LCD_ShowString(30,260,200,16,16,"Memory Put success!   ");
				}
				break;
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

//内存管理任务
void memmanage_task(void *p_arg)
{
	OS_ERR err;
	LCD_ShowString(5,164,200,16,16,"Total:  Remain:");
	LCD_ShowString(5,244,200,16,16,"Total:  Remain:");
	while(1)
	{
		POINT_COLOR = BLUE;
		LCD_ShowxNum(53,164,INTERNAL_MEM.NbrMax,1,16,0);	
		LCD_ShowxNum(125,164,INTERNAL_MEM.NbrFree,1,16,0);	
		LCD_ShowxNum(53,244,EXTERNAL_MEM.NbrMax,1,16,0);	
		LCD_ShowxNum(125,244,EXTERNAL_MEM.NbrFree,1,16,0);	
		POINT_COLOR = RED;
		OSTimeDlyHMSM(0,0,0,100,OS_OPT_TIME_PERIODIC,&err);//延时100ms
	}
}






	


