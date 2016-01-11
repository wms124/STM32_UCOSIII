例14-1 UCOSIII事件标志组

实验内容：
   本实验目的是熟悉和学会使用UCOSIII再带的内存管理，实验中建立了两个内存块，一个是在STM32内部RAM中，另一个是在外部SRAM中。
    main_task任务用来测试内存管理，按下KEY_UP键从内部RAM的内存块中申请一块内存，按下KEY1键释放申请到的内存，按下KEY2键从外部SRAM中申请一块内存，按下KEY0键释放从外部SRAM中申请到的内存。
注意：
1、 UCOSIII中以下优先级用户程序不能使用，ALIENTEK将这些优先级分配给了UCOSIII的5个系统内部任务。
    优先级0：中断服务服务管理任务 OS_IntQTask()
    优先级1：时钟节拍任务 OS_TickTask()
    优先级2：定时任务 OS_TmrTask()
    优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
    优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()


               	正点原子@ALIENTEK
               	2015-3-26
		广州市星翼电子科技有限公司
                联系电话（传真）：020-38271790
	       	购买：http://shop62103354.taobao.com 
                      http://shop62057469.taobao.com
               	技术支持论坛：www.openedv.com