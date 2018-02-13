#include "PowerAD.h"
#include "Delay.h"



u8 FinalBattaryPower;


void PowerAD_Init(void)
{
	ADC_DeInit(ADC1);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);  // ʹ��ADC1ʱ��
	GPIO_Init(GPIOD, GPIO_Pin_0, GPIO_Mode_In_FL_No_IT);     // ����PA->4 Ϊ�������룬���жϽ�ֹ
	ADC_Init(ADC1,
		ADC_ConversionMode_Single,   // ����ת��ģʽ
		ADC_Resolution_10Bit,        // 12λ����ת��
		ADC_Prescaler_2              // ʱ������Ϊ2��Ƶ
	);

	ADC_ChannelCmd(ADC1,
		ADC_Channel_22,         // ����Ϊͨ��2���в���
		ENABLE);

	ADC_Cmd(ADC1, ENABLE);               // ʹ��ADC  

	ADC_Data_ReadPOWER();                      // Ԥ�ȶ�ȡ�������ϴ��ֵ
	ADC_Data_ReadPOWER();
}

uint16_t ADC_Data_ReadPOWER(void)
{
	ADC_SoftwareStartConv(ADC1);      //����ADC

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);   // �ȴ�ת������
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                   // ����жϱ�־
	return ADC_GetConversionValue(ADC1);                // ��ȡADC1��ͨ��1��ת�����

}

uint16_t ADC_Chal_PowerRead(void)
{
	PowerAD_Init();
	return ADC_Data_ReadPOWER();
}


/*
���������Ϊ4V+����ص�ѹ��2V��ѹ��
����2VΪ��������AD�����ֵΪ646��
С��3.33V��1.665V--Լ520������Ϊ0
����3.8V��1.9V--589������Ϊ100
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
