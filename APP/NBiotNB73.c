#include "NBiotNB73.h"
#include "stdio.h"
#include "Delay.h"
#include "LaserInfrared.h"
#include "PowerAD.h"
#include "HCSR04.h"


//地址取值范围在0-9
#define AddressH2	0x0
#define AddressH1	0x0
#define AddressL1	0x0
#define AddressL2	0x2

//扫描周期,最小单位小时
u8 ScanPeriod = 1;
//上传周期，最小单位小时
u8 UpdtaPeriod = 1;


//数据临时存储
u8 NBiotRcvTemp[50] = { 0 };
//临时数据计数
u8 NBiotRcvCnt = 0;
//NBiot数据转存
u8 NBiotRcvAt[50] = { 0 };
//NB接收字符长度	
u8 NBiotRcvLength_Cnt = 0;
//接收完成标志
u8 NBiotEndFlag = 0;
//信号强度变量
u8 Signal_dBM;
//信号强度获取数组
u8 NBiot_GetSigCSQ[35] = { 0 };
u8 NB_GetSigCSQ_Flag = 0,NB_GetSig_Cnt = 0;
//上传数组
u8 NBiot_UpData_Cap[14] = { 0 };



//**************************CHECK CGATT******************************************
//
u8 CgattRCVTemp[50] = { 0 };
u8 CgattRcvCnt = 0;
u8 CgattRcvAt[50] = { 0 };
u8 CgattRcvLength_Cnt = 0;
u8 CgattEndFlag = 0;

void Recieve_CGATT_Mesag(u8 Cgatt_Usart_Mes)
{
	u8 i = 0;

	CgattRCVTemp[CgattRcvCnt++] = Cgatt_Usart_Mes;

	if (CgattRcvCnt > 2)
	{
		if ((CgattRCVTemp[CgattRcvCnt - 2] == 0x4f) && (CgattRCVTemp[CgattRcvCnt - 1] == 0x4b))
		{
			for (i = 0; i < 50; i++)
			{
				CgattRcvAt[i] = 0;
			}
			for (i = 0; i < CgattRcvCnt; i++)
			{
				CgattRcvAt[i] = CgattRCVTemp[i];
				CgattRCVTemp[i] = 0;
			}
			CgattRcvLength_Cnt = CgattRcvCnt;
			CgattRcvCnt = 0;
			CgattEndFlag = 1;
		}
	}
}

uint8_t* CGATT_check_cmd(uint8_t *str)
{
	char *strx = 0;
	if (CgattEndFlag == 1)
	{
		CgattRcvAt[CgattRcvLength_Cnt] = 0;
		strx = strstr((const char*)CgattRcvAt, (const char*)str);
		//printf("%s", NBiotRcvAt);
	}
	return (uint8_t *)strx;
}

uint8_t CGATT_send_Cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
	uint8_t res = 0;
	if ((uint32_t)cmd <= 0xFF)
	{
		USART_SendData8(USART1, (uint8_t)cmd);
		while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	}
	else
	{
		printf("%s\r\n", cmd);
	}
	if (ack && waittime)
	{

		while (--waittime)
		{

			DelayMs(10);

			if (CgattEndFlag == 1)
			{
				if (CGATT_check_cmd(ack))
				{
					//USART_SendData8(USART1, (uint8_t)0X55);
					//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完
					break;
				}
				CgattRcvLength_Cnt = 0;
				CgattEndFlag = 0;
			}
		}
		if (waittime == 0)res = 1;

	}
	return res;
}

//*******************************************************************************


void Recieve_NB_Mesag(u8 Rec_Usart_Mes)
{
	u8 i = 0;

	//NBiotRcvTemp[NBiotRcvCnt++] = Rec_Usart_Mes;
	
	if (NBiotRcvCnt > 2)  //确保数据大于4
	{
		//USART_SendData8(USART1, (uint8_t)NBiotRcvTemp[0]);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
		//USART_SendData8(USART1, (uint8_t)NBiotRcvTemp[1]);
		//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
		if ((NBiotRcvTemp[NBiotRcvCnt - 2] == 0x0d) && (NBiotRcvTemp[NBiotRcvCnt - 1] == 0x0a))
		{
			for (i = 0; i < 50; i++)
			{
				NBiotRcvAt[i] = 0;
			}
			for (i = 0; i < NBiotRcvCnt; i++)
			{
				NBiotRcvAt[i] = NBiotRcvTemp[i];
				NBiotRcvTemp[i] = 0;
			}
			NBiotRcvLength_Cnt = NBiotRcvCnt;
			NBiotRcvCnt = 0;
			NBiotEndFlag = 1;
		}
	}
	if (NBiotRcvCnt > 49)
	{
		NBiotRcvCnt = 0;
	}

}


uint8_t* NBiot_check_cmd(uint8_t *str)
{
	char *strx = 0;
	if (NBiotEndFlag == 1)
	{
		NBiotRcvAt[NBiotRcvLength_Cnt] = 0;
		strx = strstr((const char*)NBiotRcvAt, (const char*)str);
		//printf("%s",NBiotRcvAt);
	}
	return (uint8_t *)strx;
}


uint8_t NBiot_send_Cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
	uint8_t res = 0;
	if ((uint32_t)cmd <= 0xFF)
	{
		USART_SendData8(USART1, (uint8_t)cmd);
		while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	}
	else
	{
		printf("%s\r\n", cmd);
	}
	if (ack && waittime)
	{
		
		while (--waittime)
		{
			
			DelayMs(10);
			
			if (NBiotEndFlag == 1)
			{
				if (NBiot_check_cmd(ack))
				{
					//USART_SendData8(USART1, (uint8_t)0X55);
					//while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完
					break;
				}
				NBiotRcvLength_Cnt = 0;
				NBiotEndFlag = 0;
			}
		}
		if (waittime == 0)res = 1;

	}
	return res;
}


void NBiot_UpdateJude(void)        //NB上传函数
{
	//先检查是否已经联网
	u8 i;
	DelayMs(1000);
	/*for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
	}*/
	while (CGATT_send_Cmd((u8 *)"AT+CGATT?\r\n", (u8 *)"1", 300))
	{
		/*DelayMs(1000);
		for (i = 0; i < 50; i++)
		{
			NBiotRcvAt[i] = 0;
		}*/
	}
	DelayMs(1000);
	for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
	}
	NBiot_GetSigdBM();

	DelayMs(1000);
	for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
	}

	while(NBiot_send_Cmd((u8 *)"AT+NSOCR=DGRAM,17,4587,1\r\n", (u8 *)"0", 300));
	DelayMs(1000);
	for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
		
	}
	//while (NBiot_send_Cmd((u8 *)"AT+NSOST=0,112.26.45.226,8283,5,8081B1E485\r\n",(u8 *)"5",300));	
	Update_CheckData();
	DelayMs(1000);
	for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
	}

	printf("AT+NSOCL=0 ");
	USART_SendData8(USART1, (uint8_t)0x0d);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	USART_SendData8(USART1, (uint8_t)0x0a);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	//while (1);
}

void NBiot_GetSigdBM(void)					//获得信号强度
{
	u8 i;

ReCheckSigdBM:	NB_GetSigCSQ_Flag = 1;
	DelayMs(1000);
	for (i = 0; i < 50; i++)
	{
		NBiotRcvAt[i] = 0;
	}
	while (NBiot_send_Cmd((u8 *)"AT+CSQ\r\n", (u8 *)"OK", 200)) //指令发送不成功
	{
		
		for (i = 0; i < 35; i++)
		{
			NBiot_GetSigCSQ[i] = 0;
		}
		NB_GetSig_Cnt = 0;

	}
	
	//NB_GetSig_Cnt = 0;
	Signal_dBM = (NBiot_GetSigCSQ[NB_GetSig_Cnt - 3] - 48) * 10 + (NBiot_GetSigCSQ[NB_GetSig_Cnt - 2] - 48);
	if (Signal_dBM > 31)
		goto ReCheckSigdBM;
	NB_GetSig_Cnt = 0;
	for (i = 0; i < 35; i++)
	{
		NBiot_GetSigCSQ[i] = 0;
	}
	//printf("dBM:%d \r\n",Signal_dBM);
	/*USART_SendData8(USART1, (uint8_t)Signal_dBM);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕*/
	
		
	

}

void Update_CheckData(void)
{
	u8 i = 0;
	//NBiot_send_Cmd((u8 *)"AT+NSOST=0,112.26.45.226,8283,5,8081B1E485\r\n", (u8 *)"5", 300);

	NBiot_UpData_Cap[0] = AddressH2 + 0x30;
	NBiot_UpData_Cap[1] = AddressH1 + 0x30;//AddressL;
	NBiot_UpData_Cap[2] = AddressL1 + 0x30;//FinalLaserDstc;
	NBiot_UpData_Cap[3] = AddressL2 + 0x30;//FinalBattaryPower;

	NBiot_UpData_Cap[4] = Deal_Number_H(FinalSRDstc) + 0x30;//Signal_dBM;
	NBiot_UpData_Cap[5] = Deal_Number_L(FinalSRDstc) + 0x30;//ScanPeriod + 0x80;
	NBiot_UpData_Cap[6] = Deal_Number_H(FinalBattaryPower) + 0x30;//UpdtaPeriod + 0x80;
	NBiot_UpData_Cap[7] = Deal_Number_L(FinalBattaryPower) + 0x30;//0x80;
	NBiot_UpData_Cap[8] = Deal_Number_H(Signal_dBM) + 0x30;
	NBiot_UpData_Cap[9] = Deal_Number_L(Signal_dBM) + 0x30;
	NBiot_UpData_Cap[10] = Deal_Number_H(ScanPeriod) + 0x30;
	NBiot_UpData_Cap[11] = Deal_Number_L(ScanPeriod) + 0x30;
	NBiot_UpData_Cap[12] = Deal_Number_H(UpdtaPeriod) + 0x30;
	NBiot_UpData_Cap[13] = Deal_Number_L(UpdtaPeriod) + 0x30;
		
	USART_SendData8(USART1, (uint8_t)0x0d);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	USART_SendData8(USART1, (uint8_t)0x0a);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	SendString("AT+NSOST=0,112.26.45.226,8283,7,");

	for (i = 0; i < 14; i++)
	{
		USART_SendData8(USART1, (uint8_t)NBiot_UpData_Cap[i]);
		while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	}
	//NBiot_send_Cmd((u8 *)"\r\n ",(u8 *)"OK",300);
	USART_SendData8(USART1, (uint8_t)0x0d);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
	USART_SendData8(USART1, (uint8_t)0x0a);
	while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
}

void SendString(u8 *pString)
{
	while (*pString)
	{
		USART_SendData8(USART1, *pString);
		while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE));//等待发送完毕
		pString++;
	}
}

u8 Deal_Number_H(u8 DNum)
{
	u8 DnumH;

	DnumH = DNum / 10;
	return DnumH;
}

u8 Deal_Number_L(u8 DNum)
{
	u8 DnumL;
	DnumL = DNum % 10;
	return DnumL;
}


