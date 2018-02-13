#ifndef __FLASH_VAR_
#define __FLASH_VAR_



#include "STM8l15x_conf.h"

#define CanFullAdres	0x1000
#define CanEptAdres		0x1040
#define CanDisAdres		0x1080

void FlashWirteOneByte(u16 Faddress, u8 Datanum);


#endif
