例10-6 UCOSIII任务内嵌信号量

实验内容：
   创建3个任务，任务start_task用于创建另外两个任务。任务1检测按键，当检测到按下KEY_UP键的时候发送一次信号量，
使用任务2内嵌的信号量。任务2请求任务2内嵌的信号量，如果请求成功的话就进行下面的操作，否则就一直阻塞。在LCD上
我们也显示出了任务内嵌信号量值，当信号量值为0的时候任务2肯定就会被阻塞，任务2每请求一次信号量，信号量就会减1，
直到为0，阻塞任务。

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