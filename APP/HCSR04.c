#include "HCSR04.h"


u8 FinalSRDstc = 0;			//³¬Éù²¨¼ì²â¾àÀë

void TIM1_Initial(void)
{
	TIM1_DeInit();

	CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, ENABLE);

	TIM1_TimeBaseInit(0x02, TIM1_CounterMode_Up, 0xffff, 0);
	TIM1_ARRPreloadConfig(ENABLE);
	TIM1_Cmd(ENABLE);

	TRIG_OFF();    //³¬Éù²¨·¢ËÍ½Å

}


void GetDistance_SR04(void)
{
	uint16_t tmp_dist = 0, tmp_cnt[2];
	uint8_t count = 0;
	uint16_t tim_count1 = 0;//, tim_count2 = 0; 
	u8 i;

	TIM1_Cmd(ENABLE);
	for (i = 0; i < 3; i++)
	{
		for (count = 0; count < 2; count++)
		{
			TRIG_ON();
			Delay_us(30); //delay > 10us
			TIM1_SetCounter(0);
			TRIG_OFF();

			while (RESET == GET_ECHO());

			//tim_count1 = TIM1_GetCounter();
			TIM1_SetCounter(0);
			while (GET_ECHO());
			tim_count1 = TIM1_GetCounter();

			tmp_cnt[count] = tim_count1;//tim_count2 - tim_count1;
			DelayMs(100);
		}
	}
	TIM1_Cmd(DISABLE);

	// us / 58 = cm
	tmp_dist = ((tmp_cnt[0] + tmp_cnt[1]) >> 1) / 58;
	//printf("Dis:%d \r\n",tmp_dist);
	if (tmp_dist <= 13)
	{
		FinalSRDstc = 0;
	}
	if ((tmp_dist > 13) && (tmp_dist <= 27))
	{
		FinalSRDstc = 0x0a;
	}
	if ((tmp_dist > 27) && (tmp_dist <= 41))
	{
		FinalSRDstc = 20;
	}
	if ((tmp_dist > 41) && (tmp_dist <= 55))
	{
		FinalSRDstc = 30;
	}
	if ((tmp_dist > 55) && (tmp_dist <= 67))
	{
		FinalSRDstc = 40;
	}
	if (tmp_dist > 67)
	{
		FinalSRDstc = 50;
	}


}




