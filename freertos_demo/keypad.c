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
#define KEYTASKSTACKSIZE        128         // Stack size in words

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
char tecla;
int col, row, flag_config, i_start, i_count = 0;

//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************
static void
teclado()
{
    uint32_t status = 0;
    status = GPIOIntStatus(GPIO_PORTC_BASE,true);
    GPIOIntClear(GPIO_PORTC_BASE, status);

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

     // Rotina de configuração

     if(flag_config != 0)
     {
         if (flag_config == 1) //Data
         {
             if (i_count < 8)
                  {
                     i_count = i_count + 1;
                     //Salva o valor ou mostra no display
                  }
             else
                 {
                     i_count = 0;
                     flag_config = 0;
                 }
         }
         else if (flag_config == 2) // Hora
          {
              if (i_count < 6)
                   {
                      i_count = i_count + 1;
                      //Salva o valor ou mostra no display
                   }
              else
                  {
                      i_count = 0;
                      flag_config = 0;
                  }
          }
         else if (flag_config == 3) // Min Temp
           {
               if (i_count < 2)
                    {
                       i_count = i_count + 1;
                       //Salva o valor ou mostra no display
                    }
               else
                   {
                       i_count = 0;
                       flag_config = 0;
                   }
           }
         else if (flag_config == 4) // Max temp
           {
               if (i_count < 2)
                    {
                       i_count = i_count + 1;
                       //Salva o valor ou mostra no display
                    }
               else
                   {
                       i_count = 0;
                       flag_config = 0;
                   }
           }
     }
     if(tecla == 'A')
     {
         i_start = ~i_start; //Switch do motor
     }
     if(tecla =='B')
     {
         //Mostra velocidade
     }
     Lcd_Write_Char(tecla);
}

static void
IntGPIOc(void)
{
    teclado();
}
static void
KEYTask()
{
    while(1)
    {
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
       row = 3;
       SysCtlDelay(30000);
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);

       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);
       row = 0;
       SysCtlDelay(30000);
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0);

       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_PIN_6);
       row = 1;
       SysCtlDelay(30000);
       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0);

       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7);
       row = 2;
       SysCtlDelay(30000);
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

    GPIOIntTypeSet(INT_GPIOC,GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7,GPIO_FALLING_EDGE);


    GPIOIntRegister(GPIO_PORTC_BASE,IntGPIOc);

    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
//*********************************************************
//
//  Configuração Keypad e LCD
//**********************************************************
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, D4 | D5 | D6 | D7);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, EN | RS);

    Lcd_Init();
    Lcd_Clear();

    //
    // Create the LED task.
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
