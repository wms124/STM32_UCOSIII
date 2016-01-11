例10-5 UCOSIII互斥信号量

实验内容：
   创建4个任务，任务start_task用于创建另外三个任务和一个互斥信号量TEST_MUTEX。high_task和low_task都会请求互斥信号量TEST_MUTEX，
由于采用了互斥信号量所以不会出现优先级反转这个问题。

注意：
1、 UCOSIII中以下优先级用户程序不能使用，ALIENTEK将这些优先级分配给了UCOSIII的5个系统内部任务。
    优先级0：中断服务服务管理任务 OS_IntQTask()
    优先级1：时钟节拍任务 OS_TickTask()
    优先级2：定时任务 OS_TmrTask()
    优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
    优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()

               	正点原子@ALIENTEK
               	2015-5-20
		广州市星翼电子科技有限公司
                联系电话（传真）：020-38271790
	       	购买：http://shop62103354.taobao.com 
                      http://shop62057469.taobao.com
               	技术支持论坛：www.openedv.com