#ifndef __NBIOTNB73_H_
#define __NBIOTNB73_H_

#include "STM8l15x_conf.h"


extern u8 NBiot_GetSigCSQ[35];
extern u8 NB_GetSigCSQ_Flag, NB_GetSig_Cnt;
extern u8 NBiotRcvTemp[50];
extern u8 NBiotRcvCnt;


void Update_CheckData(void);
void Recieve_NB_Mesag(u8 Rec_Usart_Mes);
uint8_t NBiot_send_Cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);
void NBiot_UpdateJude(void);
void NBiot_GetSigdBM(void);	//获得信号强度

void Recieve_CGATT_Mesag(u8 Cgatt_Usart_Mes);
uint8_t* CGATT_check_cmd(uint8_t *str);
uint8_t CGATT_send_Cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime);
void SendString(u8 *pString);


u8 Deal_Number_H(u8 DNum);
u8 Deal_Number_L(u8 DNum);


#endif
