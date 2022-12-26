//*****************************************************************************
//
// interrupts.c - Interrupt preemption and tail-chaining example.
//
// Copyright (c) 2011-2017 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of revision 2.1.4.178 of the DK-TM4C123G Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "grlib/grlib.h"
//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Interrupts (interrupts)</h1>
//!
//! This example application demonstrates the interrupt preemption and
//! tail-chaining capabilities of Cortex-M4 microprocessor and NVIC.  Nested
//! interrupts are synthesized when the interrupts have the same priority,
//! increasing priorities, and decreasing priorities.  With increasing
//! priorities, preemption will occur; in the other two cases tail-chaining
//! will occur.  The currently pending interrupts and the currently executing
//! interrupt will be displayed on the display; GPIO pins D0, D1 and D2 will
//! be asserted upon interrupt handler entry and de-asserted before interrupt
//! handler exit so that the off-to-on time can be observed with a scope or
//! logic analyzer to see the speed of tail-chaining (for the two cases where
//! tail-chaining is occurring).
//
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
// The count of interrupts received.  This is incremented as each interrupt
// handler runs, and its value saved into interrupt handler specific values to
// determine the order in which the interrupt handlers were executed.
//
//*****************************************************************************
volatile uint32_t g_ui32Index;

//*****************************************************************************
//
// The value of g_ui32Index when the INT_GPIOC interrupt was processed.
//
//*****************************************************************************
volatile uint32_t g_ui32GPIOc;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Delay for the specified number of seconds.  Depending upon the current
// SysTick value, the delay will be between N-1 and N seconds (i.e. N-1 full
// seconds are guaranteed, along with the remainder of the current second).
//
//*****************************************************************************
void
Delay(uint32_t ui32Seconds)
{
    //
    // Loop while there are more seconds to wait.
    //
    while(ui32Seconds--)
    {
        //
        // Wait until the SysTick value is less than 1000.
        //
        while(ROM_SysTickValueGet() > 1000)
        {
        }

        //
        // Wait until the SysTick value is greater than 1000.
        //
        while(ROM_SysTickValueGet() < 1000)
        {
        }
    }
}
//*****************************************************************************
//
// This is the handler for INT_GPIOC.
//*****************************************************************************
void
IntGPIOc(void)
{
    //
    // Disable the interrupts.
    //
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_5);
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_INT_PIN_7);

    // Varredura das teclas

    if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_3)!=0)
       {
          row =0;
       }
    else if(GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_3)!=0)
       {
          row =1;
       }
    else if(GPIOPinRead(GPIO_PORTD_BASE,GPIO_PIN_6)!=0)
       {
          row =2;
       }
    else if(GPIOPinRead(GPIO_PORTD_BASE,GPIO_PIN_7)!=0)
       {
          row =3;
       }
    if(GPIOPinRead(GPIO_PORTC_BASE,GPIO_INT_PIN_4)==0)
       {
        col = 0;
       }
    else if(GPIOPinRead(GPIO_PORTC_BASE,GPIO_INT_PIN_5)==0)
       {
        col = 1;
       }
    else if(GPIOPinRead(GPIO_PORTC_BASE,GPIO_INT_PIN_6)==0)
       {
        col = 2;
       }
    else if(GPIOPinRead(GPIO_PORTC_BASE,GPIO_INT_PIN_7)==0)
       {
        col = 3;
        if (row == 0) { flag_config = 1;}
        else if (row == 1) { flag_config = 2;}
        else if (row == 2) { flag_config = 3;}
        else if (row == 3) { flag_config = 4;}
        }

        tecla = symbol[col][row]; //Adquire o valor da tecla

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

    //
    // Enable the interrupt.
    //
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_5);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_7);
}
int
main(void)
{

    //
    // Set the clocking to run directly from the crystal.
    //
   SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |       SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_5);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_7);

    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_6);
    GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_7);

    GPIOIntTypeSet(INT_GPIOC,GPIO_PIN_4,GPIO_FALLING_EDGE);
    GPIOIntTypeSet(INT_GPIOC,GPIO_PIN_5,GPIO_FALLING_EDGE);
    GPIOIntTypeSet(INT_GPIOC,GPIO_PIN_6,GPIO_FALLING_EDGE);
    GPIOIntTypeSet(INT_GPIOC,GPIO_PIN_7,GPIO_FALLING_EDGE);

    GPIOIntRegister(GPIO_PORTC_BASE,IntGPIOc);

    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_5);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_6);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_7);

    //
    // Loop forever.
    //
    while(1)
    {
    //Varredura do teclado
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);

       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3);
       GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0);

       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_PIN_6);
       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_6, 0);

       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_7, GPIO_PIN_7);
       GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_7, 0);

   // Fim da Varredura
    }
}
