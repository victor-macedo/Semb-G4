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
// The stack size for the I2C toggle task.
//
//*****************************************************************************
#define I2CTASKSTACKSIZE        1400         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the I2C task.
//
//*****************************************************************************
xQueueHandle g_pTempQueue;

#define SLAVE_ADDRESS_WRITE 0x48
#define SLAVE_ADDRESS_READ 0x48
#define CONFIG_TMP 0x01
#define CONFIG_TMP_BITS 0x00
#define TEMP_REG 0x00

float uValue_Temp_New,uValue_Temp_Old;

//*****************************************************************************
//
// Usando o protocolo de I2C essa tarefa adquire os valores da temperatura
// do sensor e repassa para PWM atraves de uma queue
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
static float
I2CReceive(uint32_t slave_addr)
{
    float temp;
    uint32_t temp1, temp2;
    float ms,ml;

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_WRITE, false);

    I2CMasterDataPut(I2C1_BASE, TEMP_REG);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE)); // delay de 40 ms
    vTaskDelay(100 / portTICK_RATE_MS);

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_READ, true);


    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE));
    vTaskDelay(1 / portTICK_RATE_MS);
    temp1 = I2CMasterDataGet(I2C1_BASE);
    ms = temp1;

    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C1_BASE));
    temp2 = I2CMasterDataGet(I2C1_BASE);
    vTaskDelay(100 / portTICK_RATE_MS);
    ml = temp2;
    temp = ((temp1<<1)|(temp2>>8))*128/255;

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
// Initializes the I2C task.
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

        g_pTempQueue = xQueueCreate(4, sizeof(uint32_t));

    //
    // Create the I2C task.
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
