#include "eeprom.h"
   
void ISP_IAP_Enable(void)
{
    EA = 0;
    ISP_CONTR = ISP_CONTR & 0x18;  
    ISP_CONTR = ISP_CONTR | WaitTime;
    ISP_CONTR = ISP_CONTR | 0x80;
}
   
void ISP_IAP_Disable(void)
{
	 ISP_CONTR = ISP_CONTR & 0x7f;
     ISP_CMD = 0x00;
	 ISP_TRIG = 0x00;
	 EA   =   1;
}

void ISPTrig(void)
{
	 ISP_TRIG = 0x46;
	 ISP_TRIG = 0xb9;
	 _nop_();
}
 

unsigned char byte_read(unsigned int byte_addr)
{
   unsigned char  dat = 0;
 
	 EA = 0;
	 ISP_ADDRH = (unsigned char)(byte_addr >> 8);
	 ISP_ADDRL = (unsigned char)(byte_addr & 0x00ff);
     ISP_IAP_Enable();
	 ISP_CMD   = ISP_CMD & 0xf8;
	 ISP_CMD   = ISP_CMD | RdCommand;
	 ISPTrig();
	 dat = ISP_DATA;
     ISP_IAP_Disable();
	 EA  = 1;
	 return dat;
}

void byte_write(unsigned int byte_addr,unsigned char Orig_data)
{
	 EA  = 0;
	 ISP_ADDRH = (unsigned char)(byte_addr >> 8);
	 ISP_ADDRL = (unsigned char)(byte_addr & 0x00ff);
	 ISP_IAP_Enable();
     ISP_CMD  = ISP_CMD & 0xf8;
	 ISP_CMD  = ISP_CMD | PrgCommand;
	 ISP_DATA = Orig_data;
	 ISPTrig();
	 ISP_IAP_Disable();
	 EA =1;
}

void SectorErase(unsigned int sector_addr)
{
	 EA = 0;   
	 ISP_ADDRH = (unsigned char)(sector_addr >> 8);
	 ISP_ADDRL = (unsigned char)(sector_addr & 0x00ff);
	 ISP_IAP_Enable(); 
     ISP_CMD = ISP_CMD & 0xf8;
	 ISP_CMD = ISP_CMD | EraseCommand;
	 ISPTrig();
	 ISP_IAP_Disable();
}