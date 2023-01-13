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
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "priorities.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

//*****************************************************************************
//
// The stack size for the LED toggle task.
//
//*****************************************************************************
#define I2CTASKSTACKSIZE        1000         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the LED task.
//
//*****************************************************************************
xQueueHandle g_pTempQueue;

extern xSemaphoreHandle ;
#define SLAVE_ADDRESS_WRITE 0x48
#define SLAVE_ADDRESS_READ 0x48
#define CONFIG_TMP 0x01
#define CONFIG_TMP_BITS 0x00
#define TEMP_REG 0x00

//uint32_t temp;
uint32_t uValue_Temp_New,uValue_Temp_Old, uTmax, uTmin, uVel;
static void
I2CSend(uint32_t slave_addr, uint8_t reg){
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);
    I2CMasterDataPut(I2C1_BASE, reg);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));
}

//*****************************************************************************
//
// This task toggles the user selected LED at a user selected frequency. User
// can make the selections by pressing the left and right buttons.
//
//*****************************************************************************

static void
I2CSENDCONFIG(){
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_WRITE, false);
    I2CMasterDataPut(I2C1_BASE, CONFIG_TMP);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));
    I2CMasterDataPut(I2C1_BASE, CONFIG_TMP_BITS);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH );
    while(I2CMasterBusy(I2C1_BASE));
}
static uint32_t
I2CReceive(uint32_t slave_addr)
{
    uint32_t temp, temp1, temp2;
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_WRITE, false);
    I2CMasterDataPut(I2C1_BASE, TEMP_REG);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE)); // delay de 40 ms
    vTaskDelay(100 / portTICK_RATE_MS);
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_READ, true);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE));
    temp1 = I2CMasterDataGet(I2C1_BASE);
    vTaskDelay(1 / portTICK_RATE_MS);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C1_BASE));
    temp2 = I2CMasterDataGet(I2C1_BASE);
    temp = (int)((temp1<<1)|(temp2>>8))*128/255;//nao sei se esta certp
    vTaskDelay(100 / portTICK_RATE_MS);
    return temp;
}

static void
I2CTask()
{
    uValue_Temp_Old = 0;
    while(1)
    {
        vTaskDelay(1000 / portTICK_RATE_MS);
        uValue_Temp_New = I2CReceive(SLAVE_ADDRESS_READ);
        while (uValue_Temp_New != uValue_Temp_Old)
        {
            uValue_Temp_Old = uValue_Temp_New;
            xQueueSendToBack( g_pTempQueue, &uValue_Temp_New, 10000 / portTICK_RATE_MS );
        }
    }
}

//*****************************************************************************
//
// Initializes the LED task.
//
//*****************************************************************************
uint32_t
I2CTaskInit(void)
{
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
        SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

        GPIOPinConfigure(GPIO_PA6_I2C1SCL);
        GPIOPinConfigure(GPIO_PA7_I2C1SDA);

        GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
        GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

        I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

        I2CSENDCONFIG();

        g_pTempQueue = xQueueCreate(2, sizeof(uint32_t)); //A definir o exato numero de valores na queue

    //
    // Create the LED task.
    //
    if(xTaskCreate(I2CTask, (const portCHAR *)"I2C", I2CTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_I2C_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
