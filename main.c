#include <reg52.h>
#include <eeprom.h>
#define NEXIE_PORT P1

sbit SER_L = P0^0;
sbit OE_L = P0^1;
sbit RCLK_L = P0^2;
sbit SRCLK_L = P0^3;
sbit SRCLR_L = P0^4;

sbit SER_R = P2^0;
sbit OE_R = P2^1;
sbit RCLK_R = P2^2;
sbit SRCLK_R = P2^3;
sbit SRCLR_R = P2^4;

sbit SER_B = P2^5;
sbit RCLK_B = P2^6;
sbit SRCLK_B = P2^7;

sbit BUTTON_UP = P3^6;
sbit BUTTON_DOWN = P3^4;
sbit BUTTON_LEFT = P3^7;
sbit BUTTON_RIGHT = P3^3;
sbit BUTTON_MIDDLE = P3^5;

#define RED 0
#define GREEN 1

#define LEFT 0
#define RIGHT 1

#define ROW_SIZE 8
#define COLUMN_SIZE 16
#define AREA_SIZE 128

#define GETX(position) (position)&0x0F
#define GETY(position) ((position)>>4)&0x07
#define RESET_VRAM(position) vram[GETY(position)][GETX(position)] = 0;
#define VRAM(x,y) vram[y][x]

#define SCAN_PERIOD_H 0xF4
#define SCAN_PERIOD_L 0x60

#define TOWARD_NORTH 0x00
#define TOWARD_EAST 0x01
#define TOWARD_SOUTH 0x02
#define TOWARD_WEST 0x03
char orientation;

#define TURN_LEFT 0xFF
#define TURN_RIGHT 0x01
#define GO_STRAIGHT 0x00
char forwardDirection;

#define ALIVE 0x00
#define TURN_NORTH 0x01
#define TURN_EAST 0x02
#define TURN_SOUTH 0x03
#define TURN_WEST 0x04
#define DEAD 0x05
char state;

/************************************************
 *                     VRAMS                    *
 ************************************************/
char xdata vram[8][16];
//{
//	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//	{0,2,2,0,0,2,2,0,0,1,1,0,0,1,1,0},
//	{2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1},
//	{2,2,3,2,2,3,2,2,1,1,1,1,1,1,1,1},
//	{0,2,2,3,3,2,2,0,0,1,1,1,1,1,1,0},
//	{0,0,2,2,2,2,0,0,0,0,1,1,1,1,0,0},
//	{0,0,0,2,2,0,0,0,0,0,0,1,1,0,0,0},
//	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//};
char digits[4];//digits on nixie

/************************************************
 *                  code tables                 *
 ************************************************/
char code colorfulLightCode[] = {
	0x40,//extinguish
	0x00//light up
};
char code buzzerCode[] = {
	0x80,//extinguish
	0x00//light up
};
char code digitSelectCode[] = {0x30,0x00,0x20,0x10};
char code numberCode[] = {0x00,0x01,0x08,0x09,0x04,0x05,0x0C,0x0D,0x02,0x03,0xFF};
/* ROW MAP
 * 0x0001 0x0002 0x0004 0x0008 0x0010 0x0020 0x0040 0x0080 0x0100 0x0200 0x0400 0x0800 0x1000 0x2000 0x4000 0x8000 
 * R4			R3     R2     R5     R6     R7     R8     R1     L1     L2     L8     L7     L6     L3     L4     L5
 *
 * COLUMN MAP(RED left)
 * 0xXXFE 0xXXFD 0xXXFB 0xXXF7 0xXXEF 0xXXDF 0xXXBF 0xXX7F
 * 8      7      6      5      4      3      2      1
 *
 * COLUMN MAP(RED right)
 * 0xXXFE 0xXXFD 0xXXFB 0xXXF7 0xXXEF 0xXXDF 0xXXBF 0xXX7F
 * 8      7      6      5      4      3      2      1
 *
 * COLUMN MAP(GREEN left)
 * 0xFEXX 0xFDXX 0xFBXX 0xF7XX 0xEFXX 0xDFXX 0xBFXX 0x7FXX
 * 8      7      6      5      4      3      2      1
 *
 * COLUMN MAP(GREEN right)
 * 0xFEXX 0xFDXX 0xFBXX 0xF7XX 0xEFXX 0xDFXX 0xBFXX 0x7FXX
 * 6      7      8      5      4      3      2      1
 */
int code rowSelectCode[] = {0x0180,0x0204,0x2002,0x4001,0x8008,0x1010,0x0820,0x0440};
int code columnSelectCode[2][2][ROW_SIZE] = {
	{
		{0xFF7F, 0xFFBF, 0xFFDF, 0xFFEF, 0xFFF7, 0xFFFB, 0xFFFD, 0xFFFE},
		{0xFF7F, 0xFFBF, 0xFFDF, 0xFFEF, 0xFFF7, 0xFFFB, 0xFFFD, 0xFFFE}
	},
	{
		{0x7FFF, 0xBFFF, 0xDFFF, 0xEFFF, 0xF7FF, 0xFBFF, 0xFDFF, 0xFEFF},
		{0x7FFF, 0xBFFF, 0xDFFF, 0xEFFF, 0xF7FF, 0xFEFF, 0xFDFF, 0xFBFF}
	}
};
char code forwardCode[4] = {0xF0, 0x01, 0x10, 0xFF};

/************************************************
 *    Variables in timer interrupt function     *
 ************************************************/
unsigned char interruptCounter;
char refreshCounter;
char refreshCounter_buffer;
char buzzCounter;
char digitSelect;
int leftColSelectCode[2][ROW_SIZE];
int rightColSelectCode[2][ROW_SIZE];
char scanner;
char buzz;
bit isBuzzing;
bit colorfulLight;

/************************************************
 *               data structure                 *
 ************************************************/
char xdata snake[AREA_SIZE];
unsigned char ptrHead = 0;
unsigned char ptrTail = 0;
unsigned char length = 0;
char food = 0xFF;

/************************************************
 *               other variables                *
 ************************************************/
char nextOrientation;
char currentPosition;
char nextPosition;
char currentX;
char currentY;
bit biteSelf;
bit hitWall;
bit isRefreshing;
unsigned char rand;
int point;
int highestPoint;

/************************************************
 *             Functions of 74HC595             *
 ************************************************/
void Load_74HC595(int dat_B, int dat_L, int dat_R){
	char i;
	for(i=0;i<16;i++){
		SER_B=dat_B&0x8000;
		dat_B=dat_B<<1;
		SER_L=dat_L&0x8000;
		dat_L=dat_L<<1;
		SER_R=dat_R&0x8000;
		dat_R=dat_R<<1;
	 
		SRCLK_B=1;
		SRCLK_L=1;
		SRCLK_R=1;
		SRCLK_B=0;
		SRCLK_L=0;
		SRCLK_R=0;
	}
}
void Output_74HC595(){
	RCLK_B=1;
	RCLK_L=1;
	RCLK_R=1;
	RCLK_B=0;
	RCLK_L=0;
	RCLK_R=0;
}

void refresh(){
	char i,j;
	refreshCounter_buffer=refreshCounter+1;
	refreshCounter_buffer%=2;
	isRefreshing = 1;
	for(i=0;i<8;i++){
		leftColSelectCode[refreshCounter_buffer][i] = 0xFFFF;
		rightColSelectCode[refreshCounter_buffer][i] = 0xFFFF;
		for(j=0;j<8;j++){
			leftColSelectCode[refreshCounter_buffer][i] &= (vram[i][j]&0x01)!=0    ? columnSelectCode[refreshCounter][LEFT][j]   : 0xFFFF;
			leftColSelectCode[refreshCounter_buffer][i] &= (vram[i][j]&0x02)!=0  ? columnSelectCode[refreshCounter_buffer][LEFT][j] : 0xFFFF;
			rightColSelectCode[refreshCounter_buffer][i]&= (vram[i][j+8]&0x01)!=0  ? columnSelectCode[refreshCounter][RIGHT][j]  : 0xFFFF;
			rightColSelectCode[refreshCounter_buffer][i]&= (vram[i][j+8]&0x02)!=0? columnSelectCode[refreshCounter_buffer][RIGHT][j]: 0xFFFF;
		}
	}
	refreshCounter=refreshCounter_buffer;
	isRefreshing = 0;
}

/************************************************
 *           data structure function            *
 ************************************************/
void AddHead(char newNode){
	char ytemp = GETY(newNode);
	char xtemp = GETX(newNode);
	snake[ptrHead] = newNode;
	ptrHead = (ptrHead+1)%AREA_SIZE;
	length++;
	vram[ytemp][xtemp] = (xtemp+ytemp)%2==0?1:2;
}
void DeleteTail(){
	RESET_VRAM(snake[ptrTail]);
	ptrTail = (ptrTail+1)%AREA_SIZE;
	length--;
}


/************************************************
 *              logic and actions               *
 ************************************************/
void SetFood(){
	char i,j,k;
	rand = TL1&0x7F;//get random number
	rand = rand%(AREA_SIZE-length)+1;
	k=0;
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			if(vram[i][j]==0){
				k++;
				if(k>=rand){
					vram[i][j]=3;
					food = j|(i<<4);
					return;
				}
			}
		}
	}
	
}
void Score(){
	char i;
	AddHead(food);
	SetFood();
	point++;
	isBuzzing=1;
	buzzCounter=0;
	for(i=0;i<4;i++){
		digits[i]++;
		if(digits[i]>=10){
			digits[i]-=10;
		}
		else{
			break;
		}
	}
}
char Creep(){
	nextOrientation = (orientation+forwardDirection)&0x03;
	currentPosition = snake[(ptrTail+length-1)%AREA_SIZE];
	nextPosition = currentPosition+forwardCode[nextOrientation];
	currentX = GETX(currentPosition);
	currentY = GETY(currentPosition);
	biteSelf = VRAM(GETX(nextPosition),GETY(nextPosition));
	hitWall = 
		(currentX==0				&&	nextOrientation==TOWARD_WEST) ||
		(currentX==COLUMN_SIZE-1	&&	nextOrientation==TOWARD_EAST) ||
		(currentY==0				&&	nextOrientation==TOWARD_NORTH) ||
		(currentY==ROW_SIZE-1		&&	nextOrientation==TOWARD_SOUTH);
	if(nextPosition==food){
		Score();
	}
	else if(biteSelf||hitWall){
		return DEAD;
	}
	else{
		AddHead(nextPosition);
		DeleteTail();
	}
	orientation = nextOrientation;
	return ALIVE;
}
char FSM(){
	switch(state){
		case ALIVE:
			forwardDirection=GO_STRAIGHT;
			break;
		case TURN_NORTH:
			if(orientation==TOWARD_WEST)		{forwardDirection=TURN_RIGHT;}
			else if(orientation==TOWARD_EAST)	{forwardDirection=TURN_LEFT;}
			else							{forwardDirection=GO_STRAIGHT;}
			break;
		case TURN_EAST:
			if(orientation==TOWARD_NORTH)		{forwardDirection=TURN_RIGHT;}
			else if(orientation==TOWARD_SOUTH){forwardDirection=TURN_LEFT;}
			else							{forwardDirection=GO_STRAIGHT;}
			break;
		case TURN_SOUTH:
			if(orientation==TOWARD_EAST)		{forwardDirection=TURN_RIGHT;}
			else if(orientation==TOWARD_WEST) {forwardDirection=TURN_LEFT;}
			else							{forwardDirection=GO_STRAIGHT;}
			break;
		case TURN_WEST:
			if(orientation==TOWARD_SOUTH)		{forwardDirection=TURN_RIGHT;}
			else if(orientation==TOWARD_NORTH){forwardDirection=TURN_LEFT;}
			else							{forwardDirection=GO_STRAIGHT;}
			break;
		default:
			forwardDirection=GO_STRAIGHT;
			break;
	}
	return Creep();
}
void restart(){
	char i,j;
	for(i=0;i<8;i++){
		for(j=0;j<16;j++){
			vram[i][j]=0;
		}
	}
	
	ptrHead = 0;
	ptrTail = 0;
	length = 0;
	interruptCounter=0;
	refreshCounter=0;
	colorfulLight=0;
	point=0;
	isBuzzing=0;
	
	AddHead(0x00);
	AddHead(0x01);
	AddHead(0x02);
	orientation = TOWARD_EAST;
	forwardDirection = GO_STRAIGHT;
	state = ALIVE;
	
	SetFood();
	refresh();
}
/************************************************
 *                 Initializer                  *
 ************************************************/
void initializer(){
	char i;
	TMOD=0x21;//Timer1 8bit_auto_load Timer0 16bit_normal
	
	TH0=SCAN_PERIOD_H;
	TL0=SCAN_PERIOD_L;

	TH1=0x00;
	TL1=0x00;
	
	//Eable timers
	TR0=1;
	TR1=1;

	
	//External interruption 0 is triggered by negative edge
	IT0=1;
	
	IP=0x02;
	
	highestPoint=0;
	//read highest point
	digits[0] = byte_read(0x2001);
	digits[1] = byte_read(0x2002);
	digits[2] = byte_read(0x2003);
	digits[3] = byte_read(0x2004);
	for(i=0;i<3;i++){
		highestPoint += digits[3-i];
		highestPoint *= 10;
	}
	highestPoint += digits[0];
	
	restart();
	
	//Only eable timer0 IRQ, timer1 functions as a random number generator
	ET0=1;
	ET1=0;

	//Interrupt switch
	EA=1;
}

void main(void){
	char i;
	initializer();
	//wait for button
	while(BUTTON_MIDDLE){
	}
	//reset nixie
	for(i=0;i<4;i++){
		digits[i]=0;
	}
	EX0=1;
	while(1){
		if(state==DEAD){
			EX0=0;
			if(point>=highestPoint){
				SectorErase(0x2000);
				byte_write(0x2001,digits[0]);
				byte_write(0x2002,digits[1]);
				byte_write(0x2003,digits[2]);
				byte_write(0x2004,digits[3]);
				highestPoint=point;
				colorfulLight=1;
			}
			else{
				colorfulLight=0;
			}
			while(BUTTON_MIDDLE){
			}
			restart();
			for(i=0;i<4;i++){
				digits[i]=0;
			}
			EX0=1;
		}
		else if(state==ALIVE){
			if(BUTTON_UP==0){
				state = TURN_NORTH;
			}
			else if(BUTTON_DOWN==0){
				state = TURN_SOUTH;
			}
			else if(BUTTON_LEFT==0){
				state = TURN_WEST;
			}
			else if(BUTTON_RIGHT==0){
				state = TURN_EAST;
			}
			else{
				state = ALIVE;
			}
		}
		else{
		}
	}
}
void EX0_IRQ(void) interrupt 0{
	state=FSM();
	if(state!=DEAD){
		refresh();
	}
}
void Timer0_IRQ(void) interrupt 1{
	interruptCounter++;
	if(isBuzzing){
		isBuzzing=((buzzCounter++)<=0x1F);
	}
	digitSelect = interruptCounter&0x03;
	scanner = interruptCounter&0x07;
	buzz = interruptCounter&(isBuzzing?0x01:0x00);
	
	
	NEXIE_PORT = 
		digitSelectCode[digitSelect] |
		numberCode[digits[digitSelect]] |
		colorfulLightCode[colorfulLight] |
		buzzerCode[buzz];
	
	if(!isRefreshing){
		OE_R = 1;
		OE_L = 1;
		Load_74HC595(rowSelectCode[scanner],leftColSelectCode[refreshCounter][scanner],rightColSelectCode[refreshCounter][scanner]);
		Output_74HC595();
		OE_R = 0;
		OE_L = 0;
	}
	
	TH0=SCAN_PERIOD_H;
	TL0=SCAN_PERIOD_L;
}