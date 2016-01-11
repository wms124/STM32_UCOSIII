例13-1 UCOSIII同时等待多个内核对象

实验内容：
   创建4个任务，任务start_task用于创建另外三个任务、2个信号量和1个消息队列。任务1检测按键，当按下KEY1的时候发送信号量Test_Sem1，按下KEY1的时候发送信号量Test_Sem2。任务2也检测按键，当按下KEY_UP键就会发送消息邮箱Test_Q。
任务multi_task会同时等待这3个内核对象。

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