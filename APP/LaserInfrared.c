#include "LaserInfrared.h"
#include "Delay.h"

u8 LaserDistance_Right = 0, LaserDistance_Left = 0;
u8 FinalLaserDstc;	//���յĲ�����


//******************�ۺϴ���**************************************
void InfraredLaser_Distance(void)
{
	u32 LaserRight_Sum = 0, LaserLeft_Sum = 0;
	u8 i;

	for (i = 0; i < 5; i++)
	{
		LaserRight_Sum += ADC_Chanl_RightLaser();
		DelayMs(1);
	}
	LaserRight_Sum /= 5;

	for (i = 0; i < 5; i++)
	{
		LaserLeft_Sum += ADC_Chanl_LeftLaser();
		DelayMs(1);
	}
	LaserLeft_Sum /= 5;

//	if (LaserRight_Sum >= InfLaser_R_40CM)
//	{
//		LaserDistance_Right = 40;
//	}
//	if ((LaserRight_Sum < InfLaser_R_40CM) && (LaserRight_Sum >= InfLaser_R_30CM))
//	{
//		LaserDistance_Right = 30;
//	}
//	if ((LaserRight_Sum < InfLaser_R_30CM) && (LaserRight_Sum >= InfLaser_R_20CM))
//	{
//		LaserDistance_Right = 20;
//	}
//	if ((LaserRight_Sum < InfLaser_R_20CM) && (LaserRight_Sum >= InfLaser_R_10CM))
//	{
//		LaserDistance_Right = 10;
//	}
//	if (LaserRight_Sum < InfLaser_R_10CM)
//	{
//		LaserDistance_Right = 0;
//	}
////*************************************************************
//	if (LaserLeft_Sum >= InfLaser_L_40CM)
//	{
//		LaserDistance_Left = 40;
//	}
//	if ((LaserLeft_Sum < InfLaser_L_40CM) && (LaserLeft_Sum >= InfLaser_L_30CM))
//	{
//		LaserDistance_Left = 30;
//	}
//	if ((LaserLeft_Sum < InfLaser_L_30CM) && (LaserLeft_Sum >= InfLaser_L_20CM))
//	{
//		LaserDistance_Left = 20;
//	}
//	if ((LaserLeft_Sum < InfLaser_L_20CM) && (LaserLeft_Sum >= InfLaser_L_10CM))
//	{
//		LaserDistance_Left = 10;
//	}
//	if (LaserLeft_Sum < InfLaser_L_10CM)
//	{
//		LaserDistance_Left = 0;
//	}
//
//	if (LaserDistance_Left > LaserDistance_Right)
//	{
//		FinalLaserDstc = LaserDistance_Left;
//	}
//	else
//	{
//		FinalLaserDstc = LaserDistance_Right;
//	}
	printf("DR:%ld ,DL:%ld\r\n", LaserRight_Sum, LaserLeft_Sum);
}


//******************************************************************

//**********************�Ҳ���⴫�в��****************************
void Right_Infrared_Init(void)
{
	ADC_DeInit(ADC1);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);  // ʹ��ADC1ʱ��
	GPIO_Init(GPIOD, GPIO_Pin_1, GPIO_Mode_In_FL_No_IT);     // ����PA->4 Ϊ�������룬���жϽ�ֹ
	ADC_Init(ADC1,
		ADC_ConversionMode_Single,   // ����ת��ģʽ
		ADC_Resolution_10Bit,        // 12λ����ת��
		ADC_Prescaler_2              // ʱ������Ϊ2��Ƶ
	);

	ADC_ChannelCmd(ADC1,
		ADC_Channel_21,         // ����Ϊͨ��2���в���
		ENABLE);

	ADC_Cmd(ADC1, ENABLE);               // ʹ��ADC  

	ReadADC_Data_RightLaser();                      // Ԥ�ȶ�ȡ�������ϴ��ֵ
	ReadADC_Data_RightLaser();
}

uint16_t ReadADC_Data_RightLaser(void)
{
	ADC_SoftwareStartConv(ADC1);      //����ADC

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);   // �ȴ�ת������
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                   // ����жϱ�־
	return ADC_GetConversionValue(ADC1);
}

uint16_t ADC_Chanl_RightLaser(void)
{
	Right_Infrared_Init();
	return ReadADC_Data_RightLaser();
}

//*********************************************************

//****************�Ҳ���⴫�в��*********************************
void Left_Infrared_Init(void)
{
	ADC_DeInit(ADC1);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);  // ʹ��ADC1ʱ��
	GPIO_Init(GPIOD, GPIO_Pin_2, GPIO_Mode_In_FL_No_IT);     // ����PA->4 Ϊ�������룬���жϽ�ֹ
	ADC_Init(ADC1,
		ADC_ConversionMode_Single,   // ����ת��ģʽ
		ADC_Resolution_10Bit,        // 12λ����ת��
		ADC_Prescaler_2              // ʱ������Ϊ2��Ƶ
	);

	ADC_ChannelCmd(ADC1,
		ADC_Channel_20,         // ����Ϊͨ��2���в���
		ENABLE);

	ADC_Cmd(ADC1, ENABLE);               // ʹ��ADC  

	ReadADC_Data_LeftLaser();                      // Ԥ�ȶ�ȡ�������ϴ��ֵ
	ReadADC_Data_LeftLaser();
}

uint16_t ReadADC_Data_LeftLaser(void)
{
	ADC_SoftwareStartConv(ADC1);      //����ADC

	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == 0);   // �ȴ�ת������
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                   // ����жϱ�־
	return ADC_GetConversionValue(ADC1);
}


uint16_t ADC_Chanl_LeftLaser(void)
{
	Left_Infrared_Init();
	return ReadADC_Data_LeftLaser();
}


//*****************************************************************




