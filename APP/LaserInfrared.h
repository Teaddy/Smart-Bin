#ifndef __LASERINFRARED_H_
#define __LASERINFRARED_H_

#include "stm8l15x_it.h"


extern u8 FinalLaserDstc;

#define InfLaser_R_10CM		257
#define InfLaser_L_10CM		85

#define InfLaser_R_20CM		517
#define InfLaser_L_20CM		286

#define InfLaser_R_30CM		592
#define InfLaser_L_30CM		370

#define InfLaser_R_40CM		621
#define InfLaser_L_40CM		408

void Right_Infrared_Init(void);
uint16_t ReadADC_Data_RightLaser(void);
uint16_t ADC_Chanl_RightLaser(void);

void Left_Infrared_Init(void);
uint16_t ReadADC_Data_LeftLaser(void);
uint16_t ADC_Chanl_LeftLaser(void);

void InfraredLaser_Distance(void);

#endif
