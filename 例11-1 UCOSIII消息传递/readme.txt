例11-1 UCOSIII消息传递

实验内容：
   本实验目的是熟悉和学会使用UCOSIII的消息队列，本实验有4个任务，Start_Task任务用于创建其他3个任务、2个消息队列和1个定时器，创建完成后Start_Task任务就会自行删除掉。
    本实验中定义了2个消息队列KEY_Msg和DATA_Msg,KEY_Msg用来模拟消息邮箱，DATA_Msg作为消息队列。KEY_Msg只用来传递按键值，DATA_Msg作为消息队列用来传递消息。 
    定时器1用来定时向消息队列DATA_Msg中发送数据。
    main_task为主任务，主要用于检测按键，然后将按键值作为消息通过函数OSQPost()发送到消息队列中。main_task任务还用来检测消息队列DATA_Msg的总大小和剩余大小，另外main_task任务还用于控制LED0的闪烁。
    Keyprocess_task任务通过读取消息队列KEY_Msg中的信息，然后根据不同的键值做出不同的处理。
    msgdis_task任务获取消息队列DATA_Msg中的信息，然后将其显示到LCD上。
注意：
1、 UCOSIII中以下优先级用户程序不能使用，ALIENTEK将这些优先级分配给了UCOSIII的5个系统内部任务。
    优先级0：中断服务服务管理任务 OS_IntQTask()
    优先级1：时钟节拍任务 OS_TickTask()
    优先级2：定时任务 OS_TmrTask()
    优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
    优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
2、当消息队列DATA_Msg满的时候，定时器0就会自动停止，以停止向消息队列DATA_Msg中发送数据，如果需要重新开启数据发送的话就需要按下KEY1键重新打开定时器1。

               	正点原子@ALIENTEK
               	2014-12-8
		广州市星翼电子科技有限公司
                联系电话（传真）：020-38271790
	       	购买：http://shop62103354.taobao.com 
                      http://shop62057469.taobao.com
               	技术支持论坛：www.openedv.com