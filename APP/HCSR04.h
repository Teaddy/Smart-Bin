#ifndef __HCSR04_H_
#define __HCSR04_H_

#include "stm8l15x_it.h"
#include "bsp.h"
#include "STM8l15x_conf.h"


extern u8 FinalSRDstc;
void TIM1_Initial(void);
void GetDistance_SR04(void);

/* ∫Í∂®“Â --------------------------------------------------------------------*/
#define ULTRA_IN_PORT   GPIOD
#define ULTRA_OUT_PORT  GPIOD

#define ULTRA_IN_PIN    GPIO_Pin_1    //PC7
#define ULTRA_OUT_PIN   GPIO_Pin_2    //PC6

#define TRIG_ON()   GPIO_SetBits(ULTRA_OUT_PORT, ULTRA_OUT_PIN)
#define TRIG_OFF()  GPIO_ResetBits(ULTRA_OUT_PORT, ULTRA_OUT_PIN)

#define GET_ECHO()  GPIO_ReadInputDataBit(ULTRA_IN_PORT, ULTRA_IN_PIN)


#define SR04_10CM	13
#define SR04_20CM	27
#define	SR04_30CM	41
#define SR04_40CM	55
#define SR04_50CM	67




#endif
