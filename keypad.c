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
unsigned  char symbol[4][4] = {{ '1', '2',  '3', 'F'},//Talvez um array 3 3 seja suficiente
                               { '4', '5',  '6', 'E'},
                               { '7', '8',  '9', 'D'},
                               { 'A', '0',  'B', 'C'}};
char tecla,sData,sHora,sTMax;
uint8_t col, row, flag_config, i_count, tempo, temp_max, temp_min;
bool bvarre, bstart,test;
uint32_t utempo_inicio;

//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************

static void
vInterrupt_Key()
{
    test = true;
    uint8_t status = 0;
    char sTMax[] = "Temp max:000 C";
    char sTMin[] = "Temp min:000 C";
    char sData[] = "Data: dd-mm-yyyy";
    char sHora[] = "Hora: hh:mm";

    status = GPIOIntStatus(GPIO_PORTC_BASE,true);
    GPIOIntClear(GPIO_PORTC_BASE, status);
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
    vTaskDelay(500/ portTICK_RATE_MS);

    flag_config = 0;
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

        xQueueSendToBack(g_pKEYQueue, &tecla, 1 );

        switch (flag_config){
             case(0):
             {
                 break;
             }

             case (1)://Data
             {
                 if (i_count == 0)
                     Lcd_Clear();
                     Lcd_Write_String(sData);
                     Lcd_Shift_Left(10);
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
                 break;
             }
             case (2): // Hora
              {
                  if (i_count == 0)
                          Lcd_Clear();
                          Lcd_Write_String(sHora);
                          Lcd_Shift_Left(5);
                  if (i_count < 6)
                       {
                          i_count = i_count + 1;
                          //Salva o valor ou mostra no display
                       }
                  else
                      {
                          //Necessidade de adaptar o valor recebido
                          utempo_inicio = SysTickValueGet();
                          i_count = 0;
                          flag_config = 0;
                          Lcd_Clear();
                      }
                  break;
              }
             case(3): // Min Temp
               {
                   if (i_count == 0)
                    {
                       Lcd_Clear();
                       Lcd_Write_String(sTMin);
                       Lcd_Shift_Left(6);
                    }
                   if (i_count < 2)
                        {
                           i_count = i_count + 1;
                           //Salva o valor ou mostra no display
                        }
                   else
                       {
                       //tempo = TimerValueGet(TIMER0_BASE,TIMER_BOTH);
                       Lcd_Write_Char(tempo);
                           //uTmin = 0;
                           i_count = 0;
                           flag_config = 0;
                           Lcd_Clear();
                       }
                   break;
               }
             case(4): // Max temp
               {
                   if (i_count == 0)
                   {
                          Lcd_Clear();
                          Lcd_Write_String(sTMax);
                          Lcd_Shift_Left(6);
                   }
                   if (i_count < 2)
                        {
                           i_count = i_count + 1;
                           //strncat(string_teclado,&tecla,1);
                           //Salva o valor ou mostra no display
                        }
                   else
                       {
                           //uTmax = 0
                           i_count = 0;
                           flag_config = 0;
                           Lcd_Clear();
                       }
                   break;
               }

             case(5): //Start
                {
                 //bstart = 1;
                 Lcd_Clear();
                 //Lcd_Write_String(str);

                 break;
                }
             case(6): //Velocidade
                {
                 Lcd_Clear();
                 //Lcd_Write_Char(9);
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
