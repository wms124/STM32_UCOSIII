例12-1 UCOSIII事件标志组

实验内容：
   本实验目的是熟悉和学会使用UCOSIII的事件标志组，在很实验中我们创建了3个任务start_task、main_task和flagsprocess_task。start_task任务主要用于创建事件标志组EventFlags、任务main_task和任务flagsprocess_task。
   main_task任务为主任务，在本任务中我们通过按键来模拟不同的事件，根据按键的不同来向事件标志组EventFlags发送不同的标志，在这个任务中我们还用来控制LED0的闪烁。
   flagsprocess_task任务等待事件标志组EventFlags，当等待的事件发生的话就刷新LCD上相应区域的背景，并控制LED1闪烁。
注意：
1、 UCOSIII中以下优先级用户程序不能使用，ALIENTEK将这些优先级分配给了UCOSIII的5个系统内部任务。
    优先级0：中断服务服务管理任务 OS_IntQTask()
    优先级1：时钟节拍任务 OS_TickTask()
    优先级2：定时任务 OS_TmrTask()
    优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
    优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()


               	正点原子@ALIENTEK
               	2014-12-8
		广州市星翼电子科技有限公司
                联系电话（传真）：020-38271790
	       	购买：http://shop62103354.taobao.com 
                      http://shop62057469.taobao.com
               	技术支持论坛：www.openedv.com