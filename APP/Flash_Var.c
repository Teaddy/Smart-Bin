#include "Flash_Var.h"



void FlashWirteOneByte(u16 Faddress, u8 Datanum)
{
	FLASH_DeInit();
	FLASH_Unlock(FLASH_MemType_Data);
	FLASH_EraseByte(Faddress);
	FLASH_ProgramByte(Faddress, Datanum);
}












