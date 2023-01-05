#include <I2C_task.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "drivers/rgb.h"
#include "drivers/buttons.h"
#include "utils/uartstdio.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LCD.h"

#define LCDTASKSTACKSIZE        128         // Stack size in words

xQueueHandle g_pKEYQueue;

char tecla;
char string_teclado[8];
uint32_t flag_config, i_start, uTmax, uTmin,

/**************************************************************
* Function: void Lcd_Port (char a)
*
* Returns: Nothing
*
* Description: LCD Setup Routines
*
* * Hardware Description:
*
* RS ---> PF3
* R/W ---> GND
* EN ---> PF2
* D4 ---> PA5
* D5 ---> PA2
* D6 ---> PA3
* D7 ---> PA4
**************************************************************/
void Lcd_Port(char a)
{
if(a & 1)
    GPIOPinWrite(GPIO_PORTA_BASE, D4, D4);
else
    GPIOPinWrite(GPIO_PORTA_BASE, D4, 0);
if(a & 2)
    GPIOPinWrite(GPIO_PORTA_BASE, D5, D5);
else
    GPIOPinWrite(GPIO_PORTA_BASE, D5, 0);
if(a & 4)
    GPIOPinWrite(GPIO_PORTA_BASE, D6, D6);
else
    GPIOPinWrite(GPIO_PORTA_BASE, D6, 0);
if(a & 8)
    GPIOPinWrite(GPIO_PORTA_BASE, D7, D7);
else
    GPIOPinWrite(GPIO_PORTA_BASE, D7, 0);
}
/**************************************************************
* Function: void Lcd_Cmd (char a)
*
* Returns: Nothing
*
* Description: Sets LCD command
**************************************************************/
void Lcd_Cmd(char a)
{
GPIOPinWrite(GPIO_PORTF_BASE, RS, 0); // => RS = 0
Lcd_Port(a);
GPIOPinWrite(GPIO_PORTF_BASE, EN, EN); // => E = 1
SysCtlDelay(26667);
GPIOPinWrite(GPIO_PORTF_BASE, EN, 0); // => E = 0
SysCtlDelay(26667);// adding delay
}
/**************************************************************
* Function: void Lcd_Clear()
*
* Returns: Nothing
*
* Description: Clears the LCD
**************************************************************/
void Lcd_Clear(void)
{
Lcd_Cmd(0);
Lcd_Cmd(1);
}
/**************************************************************
* Function: void Lcd_Set_Cursor(char a, char b)
*
* Returns: Nothing
*
* Description: Sets the LCD cursor position
**************************************************************/
void Lcd_Set_Cursor(char a, char b)
{
char temp,z,y;
if(a == 1)
{
temp = 0x80 + b - 1;
z = temp>>4;
y = temp & 0x0F;
Lcd_Cmd(z);
Lcd_Cmd(y);
}

else if(a == 2)
{
temp = 0xC0 + b - 1;
z = temp>>4;
y = temp & 0x0F;
Lcd_Cmd(z);
Lcd_Cmd(y);
}
}
/**************************************************************
* Function: void Lcd_Init()
*
* Returns: Nothing
*
* Description: Initializes the LCD
**************************************************************/
void Lcd_Init(void)
{
Lcd_Port(0x00);
SysCtlDelay(10000);
Lcd_Cmd(0x03);
SysCtlDelay(15000);
Lcd_Cmd(0x03);
SysCtlDelay(10000);
Lcd_Cmd(0x03);
/////////////////////////////////////////////////////
Lcd_Cmd(0x02);
Lcd_Cmd(0x02);//Function set 1, 0-4bits
Lcd_Cmd(0x00);// n� linhas  font 5x8 N� de linhas 1

Lcd_Cmd(0x00);// display on/off
Lcd_Cmd(0x0F);// 1, Display-on, Cursor - 1, Blink -0


Lcd_Cmd(0x00);// entry mode set
Lcd_Cmd(0x06);// increment the address by 1, shift the cursor to right

}
/**************************************************************
* Function: void Lcd_Write_Char (char a)
*
* Returns: Nothing
*
* Description: Writes a character to the LCD
**************************************************************/
void Lcd_Write_Char(char a)
{
char temp,y;
temp = a&0x0F;
y = a&0xF0;
GPIOPinWrite(GPIO_PORTF_BASE, RS, RS ); // => RS = 1
Lcd_Port(y>>4); //Data transfer
GPIOPinWrite(GPIO_PORTF_BASE, EN, EN); //EN =1
SysCtlDelay(10000);
GPIOPinWrite(GPIO_PORTF_BASE, EN, 0);//EN = 0;
Lcd_Port(temp);
GPIOPinWrite(GPIO_PORTF_BASE, EN, EN);//EN = 1;
SysCtlDelay(400);
GPIOPinWrite(GPIO_PORTF_BASE, EN, 0);//EN = 0;
}

/**************************************************************
* Function: void Lcd_Write_String (const char *a)
*
* Returns: Nothing
*
* Description: Writes a string to the LCD
**************************************************************/
void Lcd_Write_String(const char *a)
{
int i;
for(i=0;a[i]!='\0';i++)
Lcd_Write_Char(a[i]);
}
/**************************************************************
* Function: void Lcd_Shift_Right()
*
* Returns: Nothing
*
* Description: Shifts text on the LCD right
**************************************************************/
void Lcd_Shift_Right(void)
{
Lcd_Cmd(0x01);
Lcd_Cmd(0x0C);
}

/**************************************************************
* Function: void Lcd_Shift_Left()
*
* Returns: Nothing
*
* Description: Shifts text on the LCD left
**************************************************************/
void Lcd_Shift_Left(void)
{
Lcd_Cmd(0x01);
Lcd_Cmd(0x08);
}
static void
LCDTask()
{
        portBASE_TYPE xStatus;
           while(1)
           {
               xStatus = xQueueReceive( g_pKEYQueue, &tecla, portMAX_DELAY );
               if( xStatus == pdPASS )
               {
                    Lcd_Write_Char(tecla);

                         // Rotina de configuração

                         if(flag_config != 0 || tecla == 'F' || tecla == 'E' || tecla == 'D'|| tecla == 'C'|| tecla == 'A'|| tecla == 'B')
                         {
                             if (flag_config == 1 || tecla == 'F') //Data
                             {
                                 flag_config =1;
                                 if (i_count < 8)
                                      {
                                         i_count = i_count + 1;
                                         //Salva o valor ou mostra no display
                                      }
                                 else
                                     {
                                         i_count = 0;
                                         flag_config = 0;
                                         Lcd_Clear();
                                     }
                             }
                             else if (flag_config == 2 || tecla == 'E') // Hora
                              {
                                  flag_config = 2;
                                  if (i_count < 6)
                                       {
                                          i_count = i_count + 1;
                                          //Salva o valor ou mostra no display
                                       }
                                  else
                                      {
                                          i_count = 0;
                                          flag_config = 0;
                                          Lcd_Clear();
                                      }
                              }
                             else if (flag_config == 3 || tecla == 'D') // Min Temp
                               {
                                   flag_config = 3;
                                   if (i_count < 2)
                                        {
                                           i_count = i_count + 1;
                                           //Salva o valor ou mostra no display
                                        }
                                   else
                                       {

                                           //uTmin = 0;
                                           i_count = 0;
                                           flag_config = 0;
                                           Lcd_Clear();
                                       }
                               }
                             else if (flag_config == 4 || tecla == 'C') // Max temp
                               {
                                   flag_config = 4;
                                   if (i_count < 2)
                                        {
                                           i_count = i_count + 1;
                                           strncat(string_teclado,&tecla,1);
                                           //Salva o valor ou mostra no display
                                        }
                                   else
                                       {
                                           //uTmax = 0
                                           i_count = 0;
                                           flag_config = 0;
                                           Lcd_Clear();
                                       }
                               }
                         }
                         if(tecla == 'A')
                         {
                             i_start = ~i_start; //Switch do motor
                             Lcd_Clear();
                         }
                         if(tecla =='B')
                         {
                             //Mostra velocidade
                         }
                }
           }
}
uint32_t
LCDTaskInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, D4 | D5 | D6 | D7);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, EN | RS);
    i_count = 0;
    Lcd_Init();
    Lcd_Clear();

    //
    // Create the LCD task.
    //
    if(xTaskCreate(LCDTask, (const portCHAR *)"LCD", LCDTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_LCD_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}

