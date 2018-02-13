#include "PowerAD.h"
#include "Delay.h"



u8 FinalBattaryPower;


void PowerAD_Init(void)
{
	ADC_DeInit(ADC1);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);  // 使能ADC1时钟
	GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);     // 设置PA->4 为悬空输入，并中断禁止
	ADC_Init(ADC1,
		ADC_ConversionMode_Single,   // 单次转换模式
		ADC_Resolution_10Bit,        // 12位精度转换
		ADC_Prescaler_2              // 时钟设置为2分频
	);

	ADC_ChannelCmd(ADC1,
		ADC_Channel_22,         // 设置为通道2进行采样
		ENABLE);

	ADC_Cmd(ADC1, ENABLE);               // 使能ADC  

	ADC_Data_ReadPOWER();                      // 预先读取两次误差较大的值
	ADC_Data_ReadPOWER();
}

uint16_t ADC_Data_ReadPOWER(void)
{
	ADC_SoftwareStartConv(ADC1);      //启动ADC

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);   // 等待转换结束
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                   // 清除中断标志
	return ADC_GetConversionValue(ADC1);                // 读取ADC1，通道1的转换结果

}

uint16_t ADC_Chal_PowerRead(void)
{
	PowerAD_Init();
	return ADC_Data_ReadPOWER();
}


/*
电池满电量为4V+，电池电压用2V分压，
所以2V为满电量，AD测出的值为646，
小于3.33V（1.665V--约520）电量为0
大于3.8V（1.9V--589）电量为100
*/
void Collect_BatPower(void)
{
	u32 BatPowerSum = 0;
	u8 i;

	for (i = 0; i < 5; i++)
	{
		BatPowerSum += ADC_Chal_PowerRead();
		DelayMs(1);
	}
	BatPowerSum /= 5;

	if (BatPowerSum > 589)
	{
		BatPowerSum = 589;
	}
	if (BatPowerSum < 520)
	{
		BatPowerSum = 520;
	}

	FinalBattaryPower = (u8)((BatPowerSum - 520) * 1.45f);
	if (FinalBattaryPower >= 100)
		FinalBattaryPower = 99;

	//printf("Bat:%d \r\n", FinalBattaryPower);
}
