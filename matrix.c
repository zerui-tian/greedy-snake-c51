#include <reg52.h>
#include <intrins.h>
#include "matrix.h"



//void Refresh_Matrix(char row){
//	int leftCode = 0xFFFF;
//	int rightCode = 0xFFFF;
//	int column;
//	
//	for(column=0;column<8;column++){
//		if(GET_VRAM(column+8,row,VRAM_red)){
//			leftCode &= columnSelectCode[RED][LEFT][column];
//		}
//		if(GET_VRAM(column+8,row,VRAM_green)){
//			leftCode &= columnSelectCode[GREEN][LEFT][column];
//		}
//		if(GET_VRAM(column,row,VRAM_red)){
//			rightCode &= columnSelectCode[RED][RIGHT][column];
//		}
//		if(GET_VRAM(column,row,VRAM_green)){
//			rightCode &= columnSelectCode[GREEN][RIGHT][column];
//		}
//	}
//	
//	OE_R = 1;
//	OE_L = 1;
//	
//	Load_74HC595_B(rowSelectCode[row]);
//	Output_74HC595_B();
//	Load_74HC595_L(0x00);
//	Output_74HC595_L();
//	Load_74HC595_R(0x00);
//	Output_74HC595_R();
//	
//	OE_R = 0;
//	OE_L = 0;
//}