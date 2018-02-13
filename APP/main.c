#include "stdio.h" 
#include "string.h" 
#include "bsp.h" 
#include "CC1101.h"
#include "Delay.h"
#include "NBiotNB73.h"
#include "PowerAD.h"
#include "LaserInfrared.h"
#include "HCSR04.h"
#include "Flash_Var.h"





// USART_CSB
#define Dis_Len 2
volatile u8 Distance[Dis_Len] = {0, 0};
volatile u8 Dis_Index = 0;

u32 LongTimeCnt = 0;
u8 CanFullState, CanEPTState;
u8 SR04DistanceOld = 40;
                           // ֡ͷ  Դ��ַ  Ŀ���ַ  distance*10  ��ص��� ֡β
u8 SendBuffer[SEND_LENGTH] = {0x55,   0,    0xff,     15,         50,      0xaa}; // �ӻ�����������
                           // ֡ͷ  Դ��ַ  Ŀ���ַ  ֡β
u8 AckBuffer[ACK_LENGTH]   = {0x55,  0xff,     0,     0xaa};        // ����Ӧ������
             
void System_Initial(void);                     // ϵͳ��ʼ��
void System_GetData(void);                     // ADC�ɼ���ص�ѹ����������ࡢCC1101����

u8   RF_SendPacket(u8 *Sendbuffer, u8 length);  // �ӻ��������ݰ�
void Get_TheTime(void);                        // RTC��ȡʱ��
void RTC_AWU_Initial(uint16_t time);            // RTC�Զ�����ʱ�� time * 26.95 ms 
void DelayMs(u16 x);                            // TIM3��ʱ����
u8   Measured_Range(void);                      // ���������
void STM8_PerPwd(void);                         // STM8����͹�������
void IWDG_Init(uint8_t time_1ms);               // ��ʼ���������Ź�

void BubbleSort(u16 arr[], u16 num);             // ð������
void swap(u16 *left, u16 *right);               // ����

// printf֧��
int putchar(int c)   
{  
    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//�ȴ��������
    USART_SendData8(USART1, (uint8_t)c);
    return (c);  
}

void main(void)
{
    u8 Timer_30s = 0;                                 // 30s������
       
    System_Initial();                                 // ��ʼ��ϵͳ   ����ϵͳʱ��Ϊ4M��������ȫ���ж�  
    
    System_GetData();                                 // ADC�ɼ���ص�ѹ����������ࡢCC1101���ͺ����Sleep��STM8����͹�������
    
    while(1)
    { 
        RTC_AWU_Initial(1116);                  // RTC �����ж�    30s
        halt();                                 // ������͹���
        if(++Timer_30s == 4)                    // 10min �������  20
        {
            IWDG_Init(20);                      // ��ʼ���������Ź�   
            while(1);                           // ��ι����20ms��ֱ��IWDG��λ  
        }
    }
}





/*===========================================================================
* ����: System_Initial() => ��ʼ��ϵͳ��������                              *
============================================================================*/
void System_Initial(void)
{
    SClK_Initial();                     // ��ʼ��ϵͳʱ�ӣ�16M / 4 = 4M   
    
    GPIO_Initial();                    // ��ʼ��GPIO   LED_ON��SWITCH_ON��CC1101������(CSN��GDO0��GDO2)   
    //CSB_Initial();                     // ��ʼ��������ģ��
    TIM3_Initial();
	TIM1_Initial();
    USART1_Initial();                  // ��ʼ������1  ������ģ��ʹ�� 
   // printf("MCU Reseted.\r\n");       
                 
   // ADC_Initial();                     // ��ʼ��ADC
   // CC1101Init();                      // ��ʼ��CC1101Ϊ����ģʽ  ʹ��TIM3��1ms��׼����SPI
            
    enableInterrupts();                // ʹ��ϵͳ���ж�
}

/*===========================================================================
* ����: System_GetData() => ADC�ɼ���ص�ѹ����������ࡢCC1101����           *
============================================================================*/
void System_GetData(void)
{
	u8 i;
	//printf("Reset\r\n");
//**************************�����ĳ���******************************************	
		CanFullState = FLASH_ReadByte(CanFullAdres);
		DelayMs(10);
		CanEPTState = FLASH_ReadByte(CanEptAdres);
		DelayMs(10);
		SR04DistanceOld = FLASH_ReadByte(CanDisAdres);

		//USART_SendData8(USART1, (uint8_t)SR04DistanceOld);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//�ȴ��������

		GPIO_SetBits(GPIOB, GPIO_Pin_3);
		GetDistance_SR04();
		//DelayMs(1000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_3);  //�ص�������
		//USART_SendData8(USART1, (uint8_t)FinalSRDstc);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//�ȴ��������
		if (SR04DistanceOld == FinalSRDstc)
		{
			//printf("Old\r\n");
		}
		else
		{
			//USART_SendData8(USART1, (uint8_t)0x32);
			//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//�ȴ��������
			if ((CanEPTState == 0) && (CanFullState == 1) && (FinalSRDstc >= 30))
			{
				Collect_BatPower();			//�����
				NBiot_UpdateJude();			//NB�ϴ�����	
				CanFullState = 0;							//CanFullState = 0;
				CanEPTState = 1;
				FlashWirteOneByte(CanFullAdres, CanFullState);
				DelayMs(10);
				FlashWirteOneByte(CanEptAdres, CanEPTState);
				DelayMs(10);
				FlashWirteOneByte(CanDisAdres, FinalSRDstc);
			}

			if (FinalSRDstc < 30)
			{
				Collect_BatPower();			//�����
				NBiot_UpdateJude();			//NB�ϴ�����	
				CanFullState = 1;
				CanEPTState = 0;
				FlashWirteOneByte(CanFullAdres, CanFullState);
				DelayMs(10);
				FlashWirteOneByte(CanEptAdres, CanEPTState);
				DelayMs(10);
				FlashWirteOneByte(CanDisAdres, FinalSRDstc);
			}
		}
		//DelayMs(1000);
		//printf("DSTC:%d  \r\n", FinalLaserDstc);
		

		STM8_PerPwd();                     // �͹���IO����  ����LED_OFF��SWITCH_OFF                          
//**********************************************************************************
	while (1)
	{
		
		RTC_AWU_Initial(1000);      //27S����
		halt();
		if (++LongTimeCnt >= 2)     //10MIN  30	60min 133
		{
			LongTimeCnt = 0;
			IWDG_Init(20);
			while (1);
		}
		
		//Get_TheTime()
      
    }
  
   
}



// ��ʼ���������Ź�
void IWDG_Init(uint8_t time_1ms)
{
  IWDG_SetReload(time_1ms);                         // ��λʱ�䣺 time_1ms * 4
  IWDG_Enable();                                    // ��д0XCC 
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);     // ��д0X55
  
  IWDG_SetPrescaler(IWDG_Prescaler_64);             // 64KHZ / 64 = 1KHz  ��1ms
}

void Get_TheTime(void)
{
  RTC_TimeTypeDef GETRTC_Time;
  RTC_DateTypeDef GETRTC_Data;
  //unsigned char sec_st,sec_su , min_mt,min_mu ,hour_ht , hour_hu , midd ,status;
  if(RTC_GetFlagStatus(RTC_FLAG_RSF) == SET)  //��ʱ����� 
  {
    RTC_GetDate(RTC_Format_BIN , &GETRTC_Data);
    RTC_GetTime(RTC_Format_BIN , &GETRTC_Time);  
      
     RTC_ClearFlag(RTC_FLAG_RSF);   //�����־
     printf("20%d/%d/%d Day%d %d:%d:%d\r\n", GETRTC_Data.RTC_Year, GETRTC_Data.RTC_Month, GETRTC_Data.RTC_Date, GETRTC_Data.RTC_WeekDay, GETRTC_Time.RTC_Hours, GETRTC_Time.RTC_Minutes, GETRTC_Time.RTC_Seconds);
  }
}

// �ⲿʱ��32K
void RTC_AWU_Initial(uint16_t time)    // time * 32 ms 
{ 
    RTC_DeInit();   // ��ʼ��Ĭ��״̬ 
 
#if RTC_CLK == RTC_CLK_LSE   // �ⲿ32Kʱ��
    CLK_LSEConfig(CLK_LSE_ON);  
    while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET);  
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_64);  // ѡ��RTCʱ��ԴLSE/64=500Hz 
    
#else                        // �ڲ�38Kʱ��
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_64);  // ѡ��RTCʱ��ԴLSI/64=593.75Hz 
 
#endif 
    
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);      // ����RTCʱ�� 
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);        // 500Hz/16=31.25Hz t = 32ms 
    RTC_ITConfig(RTC_IT_WUT, ENABLE);  // �����ж� 
    RTC_SetWakeUpCounter(time);        // ����RTC Weakup��������ֵ 
    RTC_WakeUpCmd(ENABLE);             // ʹ���Զ����� 
} 


// ���ؾ���   0~255  cm
// 0:      ��������������ֵ
// 1:      ����[0,11]
// else :  ��ȷ����
//u8 Measured_Range(void)
//{
//    u8 distance_cm = 0, error_timer = 0, threshold_timer = 0;
//    
//Detectde:
//    CSB_Sleep();
//    distance_cm = 0;
//    Distance[0] = 0;    // ���㣬���²��
//    Distance[1] = 0;    
//    Dis_Index = 0;
//    CSB_Wakeup();
//    //DelayMs(1);       // ����50us ����
//    
//    DelayMs(5);         // ϵͳ����3ms�󣬷��Ͳ�ഥ���ź�0x55  
//    
//    //U1_Set(1);        // ����U1�����жϣ�׼�����ղ������
//    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//�ȴ��������
//    USART_SendData8(USART1, 0x55); 
//    
//    DelayMs(20);      // �ȴ����ڷ��ز������   25
//    CSB_Sleep(); 
//    //U1_Set(0);        // �رմ���1
//    
//    if(Dis_Index == Dis_Len) // �����յ�������Ϣ
//    {
//        distance_cm = ( (( (u16)Distance[0] << 8 ) + Distance[1]) / 10 ) & 0xff;    // �޶�distance_cm��[0, 255]��Χ��
//        if(distance_cm <= 12)      // ä�����룬�ж�Ϊ����
//        {
//            if(++threshold_timer == 20) 
//            {
//                printf("Threshold ERROR\r\n");
//                return 1;        // ����������1     ����100�β�� <= 11cm
//            }
//            DelayMs(15);
//            goto Detectde;
//        }
//        else return distance_cm;  // �����ȷ   [12, 255]
//    }
//    else
//    {
//        if(++error_timer == 10) 
//        {
//            printf("Timer_10 ERROR\r\n");
//            return 0;           // ������������0   ����10�β�� ����δ�յ���������
//        }
//        DelayMs(15);
//        goto Detectde;
//    }
//}

// STM8����͹�������
void STM8_PerPwd(void)
{   
    // ģ�⿪��                OK
    GPIO_Init(GPIOD, GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);    // ��ģ�⿪��ʱ���ر�ģ�⿪��   �൱��SWITCH_OFF

    // CSB  UART LED           OK
    GPIO_Init(GPIOC, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow); // CSB_Sleep LED_OFF
    GPIO_Init(GPIOC, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);           // �Ѳ��ԣ���͹���
    
    // ��������
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, DISABLE);
    // δʹ��IO  ����Ϊ�����  �������
    GPIO_Init(GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOC, GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6,  GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOD, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
    
    // SWIM   RST    ADC          OK
    GPIO_Init(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);  // �Ѳ��ԣ���͹���
    
    // CC1101 SPI                 OK
    GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow);           // �Ѳ��ԣ���͹���
    GPIO_Init(GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);           // �Ѳ��ԣ���͹���
}

void BubbleSort(u16 arr[], u16 num)
{
    int k, j;
    int flag = num;
    while (flag > 0)
    {
        k = flag;
        flag = 0;
        for (j = 1; j < k; j++)
        {
            if (arr[j - 1] > arr[j])
            {
                swap(&arr[j - 1], &arr[j]);
                flag = j;
            }
        }
    }
}

void swap(u16 *left, u16 *right)
{
    int temp = *left;
    *left = *right;
    *right = temp;
}

//// RTC-AWU����
//    while(1)
//    {
//        LED_TOG();                // LED��˸������ָʾ���ͳɹ�
//        printf("OK!\r\n");            
//        RTC_AWU_Initial(186);     // RTC �����ж�    186 * 26.95 ms = 5s
//        halt();//������͹���
//    }

//    // CSB����
//    while(1)
//    {
//        SWITCH_ON();                       // ��ͨCC1101��CSB��Դ
//        //CSB_Initial();                     // ��ʼ��������ģ��
//        CC1101Init();                      // ��ʼ��CC1101Ϊ����ģʽ 
//        distance = Measured_Range();       // ��� 
//        if(distance)  
//        {
//            LED_ON();
//            printf("distance = %d cm\r\n", distance);
//        }
//        else 
//        {
//            LED_OFF();
//            printf("Measured_Error\r\n");
//        } 
//        SWITCH_OFF();                      // �ر�CC1101��CSB��Դ
//        RTC_AWU_Initial(2232);             // RTC �����ж�    60s
//        halt();                            // ������͹���
////        DelayMs(1500); 
////        DelayMs(1500); 
//    }
    
// // ADC���� 
//    while(1)
//    {
//        ADC_Value = 0;
//        for(i = 0; i < 4; i++) ADC_Value += ADC_Data_Read();                  // PA4
//        ADC_Value = ADC_Value / 0x0FFF * Voltage_Refer / 4.0;
//        printf("ADC_Value = %.2f V\r\n", ADC_Value); 
//        DelayMs(1000);DelayMs(1000);
//    }

////  RTC���� 
//    RTC_Initial();            // ��ʼ��RTC   LSI
//    while(1)
//    {
//        Get_TheTime();
//        DelayMs(1000);DelayMs(1000);
//    }
    
///// ͨ�Ų���
//    CC1101Init();                          // ��ʼ��CC1101ģ��
//    while(1)
//    {
//        LED_ON();                          // LED��˸������ָʾ���ͳɹ�
// send:        
//        res = RF_SendPacket(SendBuffer, SEND_LENGTH);
//        if(res != 0) 
//        {
//          printf("Send ERROR:%d\r\n", (int)res);  // ����ʧ��
//          DelayMs(25);
//          goto send;
//        }
//        else  printf("Send OK!\r\n");              // ���ͳɹ�
//        LED_OFF();
//        
//        DelayMs(1000);DelayMs(1000);DelayMs(1000);DelayMs(1000);DelayMs(1000);
//    }

//    while(1)
//    {
//        printf("Timer_30s=%d\r\n", (int)Timer_30s);  
//        if(Timer_30s++ == 6)                   // Լ 3 Min     30s * 6
//        {
//            // ADC�ɼ�
//            ADC_Value = ADC_Data_Read();                  // PA4
//            ADC_Value = ADC_Value / 0x0FFF * Voltage_Refer;
//            printf("ADC_Value = %.2f V\r\n", ADC_Value); 
//          
//            SWITCH_ON();                       // ��ͨCC1101��CSB��Դ
//            LED_ON();                          // LED��˸������ָʾ���ͳɹ�
//            //CSB_Initial();                     // ��ʼ��������ģ��
//            CC1101Init();                      // ��ʼ��CC1101ģ��
//            SendError_Time = 0;                // ������������
//              
//            distance = Measured_Range();       // ��������� 
//            if(distance)  
//            {
//                SendBuffer[3] = distance;
//                printf("distance = %d cm\r\n", distance);
//            }
//            else 
//            {
//                SendBuffer[3] = 0;             // ��������  ����0
//                printf("Measured_Error\r\n");
//            } 
//send:            
//            res = RF_SendPacket(SendBuffer, SEND_LENGTH);
//            if(res != 0) 
//            {
//                printf("Send ERROR:%d\r\n", (int)res);  // ����ʧ��
//                DelayMs(25);
//                if(++SendError_Time < 20) goto send;   //  ���������ﵽ20�Σ�������˴δ���
//                printf("Send Canceled!\r\n");  // ����ʧ��
//            }
//            else printf("Send OK!\r\n");              // ���ͳɹ�
//            
//            SWITCH_OFF();
//            LED_OFF();
//            Timer_30s = 5;    // 1
//        }
//        RTC_AWU_Initial(1116);     // RTC �����ж�    1116 * 26.95 ms = 30s
//        halt();//������͹���
//    }