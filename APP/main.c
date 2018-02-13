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
                           // 帧头  源地址  目标地址  distance*10  电池电量 帧尾
u8 SendBuffer[SEND_LENGTH] = {0x55,   0,    0xff,     15,         50,      0xaa}; // 从机待发送数据
                           // 帧头  源地址  目标地址  帧尾
u8 AckBuffer[ACK_LENGTH]   = {0x55,  0xff,     0,     0xaa};        // 主机应答数据
             
void System_Initial(void);                     // 系统初始化
void System_GetData(void);                     // ADC采集电池电压、超声波测距、CC1101发送

u8   RF_SendPacket(u8 *Sendbuffer, u8 length);  // 从机发送数据包
void Get_TheTime(void);                        // RTC获取时间
void RTC_AWU_Initial(uint16_t time);            // RTC自动唤醒时间 time * 26.95 ms 
void DelayMs(u16 x);                            // TIM3延时函数
u8   Measured_Range(void);                      // 超声波测距
void STM8_PerPwd(void);                         // STM8外设低功耗配置
void IWDG_Init(uint8_t time_1ms);               // 初始化独立看门狗

void BubbleSort(u16 arr[], u16 num);             // 冒泡排序
void swap(u16 *left, u16 *right);               // 交换

// printf支持
int putchar(int c)   
{  
    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
    USART_SendData8(USART1, (uint8_t)c);
    return (c);  
}

void main(void)
{
    u8 Timer_30s = 0;                                 // 30s计数器
       
    System_Initial();                                 // 初始化系统   设置系统时钟为4M，并开启全局中断  
    
    System_GetData();                                 // ADC采集电池电压、超声波测距、CC1101发送后进入Sleep、STM8外设低功耗配置
    
    while(1)
    { 
        RTC_AWU_Initial(1116);                  // RTC 唤醒中断    30s
        halt();                                 // 挂起，最低功耗
        if(++Timer_30s == 4)                    // 10min 重启检测  20
        {
            IWDG_Init(20);                      // 初始化独立看门狗   
            while(1);                           // 不喂狗，20ms后直接IWDG复位  
        }
    }
}





/*===========================================================================
* 函数: System_Initial() => 初始化系统所有外设                              *
============================================================================*/
void System_Initial(void)
{
    SClK_Initial();                     // 初始化系统时钟，16M / 4 = 4M   
    
    GPIO_Initial();                    // 初始化GPIO   LED_ON、SWITCH_ON、CC1101控制线(CSN、GDO0、GDO2)   
    //CSB_Initial();                     // 初始化超声波模块
    TIM3_Initial();
	TIM1_Initial();
    USART1_Initial();                  // 初始化串口1  超声波模块使用 
   // printf("MCU Reseted.\r\n");       
                 
   // ADC_Initial();                     // 初始化ADC
   // CC1101Init();                      // 初始化CC1101为发送模式  使能TIM3（1ms基准）、SPI
            
    enableInterrupts();                // 使能系统总中断
}

/*===========================================================================
* 函数: System_GetData() => ADC采集电池电压、超声波测距、CC1101发送           *
============================================================================*/
void System_GetData(void)
{
	u8 i;
	//printf("Reset\r\n");
//**************************完整的程序******************************************	
		CanFullState = FLASH_ReadByte(CanFullAdres);
		DelayMs(10);
		CanEPTState = FLASH_ReadByte(CanEptAdres);
		DelayMs(10);
		SR04DistanceOld = FLASH_ReadByte(CanDisAdres);

		//USART_SendData8(USART1, (uint8_t)SR04DistanceOld);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕

		GPIO_SetBits(GPIOB, GPIO_Pin_3);
		GetDistance_SR04();
		//DelayMs(1000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_3);  //关掉超声波
		//USART_SendData8(USART1, (uint8_t)FinalSRDstc);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
		if (SR04DistanceOld == FinalSRDstc)
		{
			//printf("Old\r\n");
		}
		else
		{
			//USART_SendData8(USART1, (uint8_t)0x32);
			//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
			if ((CanEPTState == 0) && (CanFullState == 1) && (FinalSRDstc >= 30))
			{
				Collect_BatPower();			//测电量
				NBiot_UpdateJude();			//NB上传数据	
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
				Collect_BatPower();			//测电量
				NBiot_UpdateJude();			//NB上传数据	
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
		

		STM8_PerPwd();                     // 低功耗IO配置  包括LED_OFF、SWITCH_OFF                          
//**********************************************************************************
	while (1)
	{
		
		RTC_AWU_Initial(1000);      //27S左右
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



// 初始化独立看门狗
void IWDG_Init(uint8_t time_1ms)
{
  IWDG_SetReload(time_1ms);                         // 复位时间： time_1ms * 4
  IWDG_Enable();                                    // 先写0XCC 
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);     // 后写0X55
  
  IWDG_SetPrescaler(IWDG_Prescaler_64);             // 64KHZ / 64 = 1KHz  即1ms
}

void Get_TheTime(void)
{
  RTC_TimeTypeDef GETRTC_Time;
  RTC_DateTypeDef GETRTC_Data;
  //unsigned char sec_st,sec_su , min_mt,min_mu ,hour_ht , hour_hu , midd ,status;
  if(RTC_GetFlagStatus(RTC_FLAG_RSF) == SET)  //有时间更新 
  {
    RTC_GetDate(RTC_Format_BIN , &GETRTC_Data);
    RTC_GetTime(RTC_Format_BIN , &GETRTC_Time);  
      
     RTC_ClearFlag(RTC_FLAG_RSF);   //清除标志
     printf("20%d/%d/%d Day%d %d:%d:%d\r\n", GETRTC_Data.RTC_Year, GETRTC_Data.RTC_Month, GETRTC_Data.RTC_Date, GETRTC_Data.RTC_WeekDay, GETRTC_Time.RTC_Hours, GETRTC_Time.RTC_Minutes, GETRTC_Time.RTC_Seconds);
  }
}

// 外部时钟32K
void RTC_AWU_Initial(uint16_t time)    // time * 32 ms 
{ 
    RTC_DeInit();   // 初始化默认状态 
 
#if RTC_CLK == RTC_CLK_LSE   // 外部32K时钟
    CLK_LSEConfig(CLK_LSE_ON);  
    while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET);  
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_64);  // 选择RTC时钟源LSE/64=500Hz 
    
#else                        // 内部38K时钟
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_64);  // 选择RTC时钟源LSI/64=593.75Hz 
 
#endif 
    
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);      // 允许RTC时钟 
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);        // 500Hz/16=31.25Hz t = 32ms 
    RTC_ITConfig(RTC_IT_WUT, ENABLE);  // 开启中断 
    RTC_SetWakeUpCounter(time);        // 设置RTC Weakup计算器初值 
    RTC_WakeUpCmd(ENABLE);             // 使能自动唤醒 
} 


// 返回距离   0~255  cm
// 0:      距离大于量程最大值
// 1:      距离[0,11]
// else :  正确距离
//u8 Measured_Range(void)
//{
//    u8 distance_cm = 0, error_timer = 0, threshold_timer = 0;
//    
//Detectde:
//    CSB_Sleep();
//    distance_cm = 0;
//    Distance[0] = 0;    // 清零，重新测距
//    Distance[1] = 0;    
//    Dis_Index = 0;
//    CSB_Wakeup();
//    //DelayMs(1);       // 至少50us 唤醒
//    
//    DelayMs(5);         // 系统唤醒3ms后，发送测距触发信号0x55  
//    
//    //U1_Set(1);        // 开启U1接收中断，准备接收测量结果
//    while(!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
//    USART_SendData8(USART1, 0x55); 
//    
//    DelayMs(20);      // 等待串口返回测量结果   25
//    CSB_Sleep(); 
//    //U1_Set(0);        // 关闭串口1
//    
//    if(Dis_Index == Dis_Len) // 串口收到距离信息
//    {
//        distance_cm = ( (( (u16)Distance[0] << 8 ) + Distance[1]) / 10 ) & 0xff;    // 限定distance_cm在[0, 255]范围内
//        if(distance_cm <= 12)      // 盲区距离，判定为已满
//        {
//            if(++threshold_timer == 20) 
//            {
//                printf("Threshold ERROR\r\n");
//                return 1;        // 已满，返回1     连续100次测距 <= 11cm
//            }
//            DelayMs(15);
//            goto Detectde;
//        }
//        else return distance_cm;  // 测距正确   [12, 255]
//    }
//    else
//    {
//        if(++error_timer == 10) 
//        {
//            printf("Timer_10 ERROR\r\n");
//            return 0;           // 测距出错，返回0   连续10次测距 串口未收到返回数据
//        }
//        DelayMs(15);
//        goto Detectde;
//    }
//}

// STM8外设低功耗配置
void STM8_PerPwd(void)
{   
    // 模拟开关                OK
    GPIO_Init(GPIOD, GPIO_Pin_1 | GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);    // 有模拟开关时，关闭模拟开关   相当于SWITCH_OFF

    // CSB  UART LED           OK
    GPIO_Init(GPIOC, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow); // CSB_Sleep LED_OFF
    GPIO_Init(GPIOC, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Slow);           // 已测试，最低功耗
    
    // 除能外设
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM3, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM1, DISABLE);
    // 未使用IO  设置为输出低  功耗最低
    GPIO_Init(GPIOA, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOC, GPIO_Pin_1 | GPIO_Pin_5 | GPIO_Pin_6,  GPIO_Mode_Out_PP_Low_Slow);
    GPIO_Init(GPIOD, GPIO_Pin_0 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
    
    // SWIM   RST    ADC          OK
    GPIO_Init(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);  // 已测试，最低功耗
    
    // CC1101 SPI                 OK
    GPIO_Init(GPIOB, GPIO_Pin_3, GPIO_Mode_Out_PP_Low_Slow);           // 已测试，最低功耗
    GPIO_Init(GPIOB, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Slow);           // 已测试，最低功耗
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

//// RTC-AWU测试
//    while(1)
//    {
//        LED_TOG();                // LED闪烁，用于指示发送成功
//        printf("OK!\r\n");            
//        RTC_AWU_Initial(186);     // RTC 唤醒中断    186 * 26.95 ms = 5s
//        halt();//挂起，最低功耗
//    }

//    // CSB测试
//    while(1)
//    {
//        SWITCH_ON();                       // 接通CC1101、CSB电源
//        //CSB_Initial();                     // 初始化超声波模块
//        CC1101Init();                      // 初始化CC1101为发送模式 
//        distance = Measured_Range();       // 测距 
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
//        SWITCH_OFF();                      // 关闭CC1101、CSB电源
//        RTC_AWU_Initial(2232);             // RTC 唤醒中断    60s
//        halt();                            // 挂起，最低功耗
////        DelayMs(1500); 
////        DelayMs(1500); 
//    }
    
// // ADC测试 
//    while(1)
//    {
//        ADC_Value = 0;
//        for(i = 0; i < 4; i++) ADC_Value += ADC_Data_Read();                  // PA4
//        ADC_Value = ADC_Value / 0x0FFF * Voltage_Refer / 4.0;
//        printf("ADC_Value = %.2f V\r\n", ADC_Value); 
//        DelayMs(1000);DelayMs(1000);
//    }

////  RTC测试 
//    RTC_Initial();            // 初始化RTC   LSI
//    while(1)
//    {
//        Get_TheTime();
//        DelayMs(1000);DelayMs(1000);
//    }
    
///// 通信测试
//    CC1101Init();                          // 初始化CC1101模块
//    while(1)
//    {
//        LED_ON();                          // LED闪烁，用于指示发送成功
// send:        
//        res = RF_SendPacket(SendBuffer, SEND_LENGTH);
//        if(res != 0) 
//        {
//          printf("Send ERROR:%d\r\n", (int)res);  // 发送失败
//          DelayMs(25);
//          goto send;
//        }
//        else  printf("Send OK!\r\n");              // 发送成功
//        LED_OFF();
//        
//        DelayMs(1000);DelayMs(1000);DelayMs(1000);DelayMs(1000);DelayMs(1000);
//    }

//    while(1)
//    {
//        printf("Timer_30s=%d\r\n", (int)Timer_30s);  
//        if(Timer_30s++ == 6)                   // 约 3 Min     30s * 6
//        {
//            // ADC采集
//            ADC_Value = ADC_Data_Read();                  // PA4
//            ADC_Value = ADC_Value / 0x0FFF * Voltage_Refer;
//            printf("ADC_Value = %.2f V\r\n", ADC_Value); 
//          
//            SWITCH_ON();                       // 接通CC1101、CSB电源
//            LED_ON();                          // LED闪烁，用于指示发送成功
//            //CSB_Initial();                     // 初始化超声波模块
//            CC1101Init();                      // 初始化CC1101模块
//            SendError_Time = 0;                // 出错次数清零
//              
//            distance = Measured_Range();       // 超声波测距 
//            if(distance)  
//            {
//                SendBuffer[3] = distance;
//                printf("distance = %d cm\r\n", distance);
//            }
//            else 
//            {
//                SendBuffer[3] = 0;             // 测量出错  发送0
//                printf("Measured_Error\r\n");
//            } 
//send:            
//            res = RF_SendPacket(SendBuffer, SEND_LENGTH);
//            if(res != 0) 
//            {
//                printf("Send ERROR:%d\r\n", (int)res);  // 发送失败
//                DelayMs(25);
//                if(++SendError_Time < 20) goto send;   //  出错次数达到20次，则放弃此次传输
//                printf("Send Canceled!\r\n");  // 发送失败
//            }
//            else printf("Send OK!\r\n");              // 发送成功
//            
//            SWITCH_OFF();
//            LED_OFF();
//            Timer_30s = 5;    // 1
//        }
//        RTC_AWU_Initial(1116);     // RTC 唤醒中断    1116 * 26.95 ms = 30s
//        halt();//挂起，最低功耗
//    }