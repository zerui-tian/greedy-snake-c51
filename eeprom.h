#ifndef _EEPROM_H_
#define _EEPROM_H_
 
#include <intrins.h>
#include <reg52.h>
 
typedef  unsigned int uint;
typedef  unsigned char uchar;

/********STC89C52 section*******
1st section:2000H--21FF
2nd section:2200H--23FF
3rd section:2400H--25FF
4th section:2600H--27FF
5th section:2800H--29FF
6th section:2A00H--2BFF
7th section:2C00H--2DFF
8th section:2E00H--2FFF
*******************************/ 
 
#define RdCommand 0x01  
#define PrgCommand 0x02 
#define EraseCommand 0x03
 
#define Error 1
#define Ok 0
#define WaitTime 0x01
  
sfr ISP_DATA = 0xE2;
sfr ISP_ADDRH = 0xE3;
sfr ISP_ADDRL = 0xE4;
sfr ISP_CMD = 0xE5;
sfr ISP_TRIG = 0xE6;
sfr ISP_CONTR = 0xE7;
 
 
unsigned char byte_read(unsigned int byte_addr);
void byte_write(unsigned int byte_addr,unsigned char Orig_data);
void SectorErase(unsigned int sector_addr);
 
#endif