//*****************************************************************************
//
// switch_task.c - A simple switch task to process the buttons.
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
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/pwm.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
// The stack size for the display task.
//
//*****************************************************************************
#define PWMTASKSTACKSIZE       500        // Stack size in words

extern xQueueHandle g_pLEDQueue;
extern xSemaphoreHandle g_pUARTSemaphore;

uint8_t temp_max,temp_min,temp;
bool bstart;
xQueueHandle g_pTempQueue;
//*****************************************************************************
//
// This task reads the buttons' state and passes this information to LEDTask.
//
//*****************************************************************************
static void
PWMTask(void *pvParameters)
{
    portBASE_TYPE xStatus;
    //Temp por uma Queue que recebe caso o valor seja diferente
    uint32_t vel,temp;
    while (1)
    {
        xStatus = xQueueReceive( g_pTempQueue, &temp, portMAX_DELAY );
        if( xStatus == pdPASS )
        {
            if (temp>=temp_max){
                PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
            }
            else{
                PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
            }
            if(temp>temp_min){
                if(bstart == false){
                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000);
                    PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true);//inicializacvao do motor para ultrapassar a velocidade minima
                    //delay
                    bstart = true;
                }
                if(temp < temp_max){
                    vel = (temp-temp_min)/(temp_max-temp_min)*30000;
                    if(vel>(30000*0.2)){
                         PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,vel);
                    }
                    else {
                        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000*0.2);
                    }
                }
                if(temp >= temp_max){
                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000);
                }
            }
            else{
                PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true);
                bstart = false;

            }
        }
    }
}

//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
PWMTaskInit(void)
{
     SysCtlPWMClockSet(SYSCTL_PWMDIV_1); // sem pre scaler

     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

     SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

     GPIOPinConfigure(GPIO_PB4_M0PWM2);
     GPIOPinConfigure(GPIO_PB5_M0PWM3);

     GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_5);
     GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_4);

     PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

     //Set the Period (expressed in clock ticks)
     PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 30000);

     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2,15000);//pwm do buzzer
     PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,0);

     PWMGenEnable(PWM0_BASE, PWM_GEN_1);




     PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
     PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, false);

    //
    // Create the switch task.
    //
    if(xTaskCreate(PWMTask, (const portCHAR *)"PWM",
                   PWMTASKSTACKSIZE, NULL, tskIDLE_PRIORITY +
                   PRIORITY_PWM_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
