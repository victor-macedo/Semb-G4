

/* 
 * File:   LCD.h
 * Author: JP
 * Comments:
 * Revision history: 
 */

/* Program Description: This program header provides routines
for controlling
* a STD HITACHI HD44780 and compatible LCDs
*
* Hardware Description:
*
* RS ---> RB0
* R/W ---> GND
* EN ---> PF3
* D4 ---> RB5
* D5 ---> RB2
* D6 ---> RB4
* D7 ---> RB3
*
*/

/**************************************************************
*Includes and defines
**************************************************************/
// STD XC8 include
#include <inc/tm4c123gh6pm.h>
//#include "mcc_generated_files/mcc.h"
#include <stdlib.h>
#include <math.h>
#define RS GPIO_PIN_3 //Register Select (Character or Instruction)
#define EN GPIO_PIN_2 //LCD Clock Enable PIN, Falling Edge Triggered
// 4 bit operation
#define D4 GPIO_PIN_5 //Bit 4
#define D5 GPIO_PIN_2 //Bit 5
#define D6 GPIO_PIN_3 //Bit 6
#define D7 GPIO_PIN_4 //Bit 7
// function prototypes
void Lcd_Port(char a);
void Lcd_Cmd(char a);
void Lcd_Clear(void);
void Lcd_Set_Cursor(char b);
void Lcd_Init(void);
void Lcd_Write_Char(char a);
void Lcd_Write_String(const char *a);
void Lcd_Shift_Right(void);
void Lcd_Shift_Left(void);
void Lcd_Write_Integer(int v);
void Lcd_Write_Float(float f);
