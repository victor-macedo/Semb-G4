//*****************************************************************************
//
// led_task.c - A simple flashing LED task.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <I2C_task.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"
#include "driverlib/interrupt.h"
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

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define KEYTASKSTACKSIZE        3000         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the LED task.
//
//*****************************************************************************

xQueueHandle g_pKEYQueue;

//*****************************************************************************
/* Array of 4x4 to define characters which will be printe on specific key pressed */
char symbol[4][4] =            {{ '1', '2',  '3', 'F'},//Talvez um array 3 3 seja suficiente
                               { '4', '5',  '6', 'E'},
                               { '7', '8',  '9', 'D'},
                               { 'A', '0',  'B', 'C'}};
char tecla;
static const char sTMax[] = "Temp max:000 C";
static const char sTMin[] = "Temp min:000 C";
static const char sData[] = "Data: dd-mm-yyyy";
static const char sHora[] = "Hora: hh:mm";
static const char sClear = 'W';
static const char sLeft = 'Z';
static const char sRight = 'B';
uint8_t col, row, flag_config, i_count, temp_max, temp_min;
bool bvarre, bstart,test;
uint32_t utempo_inicio;

//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************
void
Key_Shift_Left(int a)
{
    int i;
    for(i=0; i<a; i++)
    {
        xQueueSendToBack(g_pKEYQueue, &sLeft, 0 );
    }
}
void
Key_Shift_Right(int a)
{
    int i;
    for(i=0; i<a; i++)
    {
        xQueueSendToBack(g_pKEYQueue, &sRight, 0 );
    }
}
static void
vInterrupt_Key()
{
    test = true;
    uint8_t status = 0;
    char sTMax[] = "Temp max:000 C";
    char sTMin[] = "Temp min:000 C";
    char sData[] = "Data: dd-mm-yyyy";
    char sHora[] = "Hora: hh:mm:ss";

    status = GPIOIntStatus(GPIO_PORTC_BASE,true);
    GPIOIntClear(GPIO_PORTC_BASE, status);
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
    vTaskDelay(500/ portTICK_RATE_MS);

    // Varredura das teclas
    if((status & GPIO_INT_PIN_4) == GPIO_INT_PIN_4)
       {
        col = 0;
       }
    else if((status & GPIO_INT_PIN_5) == GPIO_INT_PIN_5)
       {
        col = 1;
       }
    else if((status & GPIO_INT_PIN_6) == GPIO_INT_PIN_6)
       {
        col = 2;
       }
    else if((status & GPIO_INT_PIN_7) == GPIO_INT_PIN_7)
       {
        col = 3;
        if (row == 0) { flag_config = 1;}
        else if (row == 1) { flag_config = 2;}
        else if (row == 2) { flag_config = 3;}
        else if (row == 3) { flag_config = 4;}
        }
        tecla = symbol[row][col]; //Adquire o valor da tecla
        if ((row == 3) && (col == 0)){ flag_config = 5;}
        if ((row == 3) && (col == 2)){ flag_config = 6;}

        xQueueSendToBack(g_pKEYQueue, &tecla, 0 );

        switch (flag_config){
             case(0):
             {
                 break;
             }

             case (1)://Data
             {
                 if (i_count == 0)
                 {
                     xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                     Lcd_Write_String(sData);
                     Key_Shift_Left(10);
                     i_count++;
                 }
                 else if (i_count < 8)
                      {
                         //Salva o valor ou mostra no display
                         i_count++;
                      }
                 else
                     {
                         i_count = 0;
                         flag_config = 0;
                         xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                     }
                 break;
             }
             case (2): // Hora
              {
                  if (i_count == 0)
                  {
                       xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       Lcd_Write_String(sHora);
                       Key_Shift_Left(5);
                       i_count++;
                  }
                 else if (i_count < 6)
                       {
                          //Salva o valor ou mostra no display
                         i_count++;
                       }
                  else
                      {
                          //Necessidade de adaptar o valor recebido
                      utempo_inicio = (SysTickValueGet()/ (portTICK_RATE_MS*1000));
                      i_count = 0;
                      flag_config = 0;
                      xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                      }
                  break;
              }
             case(3): // Min Temp
               {
                   if (i_count == 0)
                    {
                       xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       Lcd_Write_String(sTMin);
                       Key_Shift_Left(5);
                       i_count++;
                    }
                   else if (i_count < 3)
                        {
                           //Salva o valor ou mostra no display
                           i_count++;
                        }
                   else
                       {
                           //temp_min = Converter string em int;
                           i_count = 0;
                           flag_config = 0;
                           xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       }
                   break;
               }
             case(4): // Max temp
               {
                   if (i_count == 0)
                   {
                          xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                          Lcd_Write_String(sTMax);
                          Key_Shift_Left(5);
                          i_count++;
                   }
                   else if (i_count < 3)
                        {
                           //strncat(string_teclado,&tecla,1);
                           i_count++;
                        }
                   else
                       {
                           //temp_max = Converter string em int;
                           i_count = 0;
                           flag_config = 0;
                           xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       }
                   break;
               }

             case(5): //Start
                {
                 //bstart = 1;
                  xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                 //Lcd_Write_String(str);

                 break;
                }
             case(6): //Velocidade
                {
                  xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                 break;
                }
             }
        vTaskDelay(500/ portTICK_RATE_MS);
        GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
        test = false;
}

static void
IntGPIOc(void)
{

    if(test==false){
        vInterrupt_Key();
    }

    else{
        uint8_t status = 0;
        status = GPIOIntStatus(GPIO_PORTC_BASE,true);
        GPIOIntClear(GPIO_PORTC_BASE, status);
    }
}
static void
KEYTask()
{
    i_count = 0;
    test = false;
    flag_config = 0;
    SysTickEnable();
    while(test ==false)
    {
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
       row = 3;
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);

       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);
       row = 0;
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0);

       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_PIN_6);
       row = 1;
       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0);

       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7);
       row = 2;
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);
    }
}

//*****************************************************************************
//
// Initializes the LED task.
//
//*****************************************************************************
uint32_t
KEYTaskInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_7);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6);

    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_FALLING_EDGE);


    GPIOIntRegister(GPIO_PORTC_BASE,IntGPIOc);

    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
    //
    // Create the LCD task.
    //
    if(xTaskCreate(KEYTask, (const portCHAR *)"KEY", KEYTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_KEY_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
