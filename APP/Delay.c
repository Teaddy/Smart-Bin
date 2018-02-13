#include "Delay.h"


volatile u32  Cnt1ms = 0;     // 1ms计数变量，每1ms加一 
int32_t  RecvWaitTime = 0;        // 接收等待时间   


/*===========================================================================
* 函数 : DelayMs() => 延时函数(ms级)                                        *
* 输入 ：x, 需要延时多少(0-65535)                                             *
============================================================================*/
void DelayMs(u16 x)
{
	u32 timer_ms;

	timer_ms = x;
	Cnt1ms = 0;
	TIM3_Set(1);
	while (Cnt1ms < timer_ms);
	TIM3_Set(0);
}


/*===========================================================================
* 函数 ：TIM3_1MS_ISR() => 定时器3服务函数, 定时时间基准为1ms               *
============================================================================*/
void TIM3_1MS_ISR(void)
{
	Cnt1ms++;
	//if (RecvWaitTime > 0) RecvWaitTime--;    // 数据接收计时
}

void Delay_us(unsigned int nCount)
{
	for (; nCount != 0; nCount--);
}

