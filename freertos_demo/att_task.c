#include <I2C_task.h>
#include <stdbool.h>
#include <stdio.h>
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
#include "LCD.h"

//*****************************************************************************
//
// The stack size for the ATT toggle task.
//
//*****************************************************************************
#define ATTTASKSTACKSIZE        700         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the ATT task.
//
//*****************************************************************************
float temp,uValue_Temp_New;
uint32_t flag_config;
int vel;
uint8_t show;
static const char sTemp[] = "Temperatura:";
static const char ssTemp[] = "C";
static const char sVel[] = "Velocidade:";
static const char ssVel[] = "%";
char temp_st [3];
char vel_st [3];
xQueueHandle g_pKEYQueue;
xQueueHandle g_pTempQueue;
const char sClear;
//*****************************************************************************
//
// Essa tarefa atualiza o LCD com os valores do temperatura ou
// da velocidade se a tecla B foi pressionada
//
//*****************************************************************************

static void
AttTask()
{
    while(1)
    {
        //temp = xQueueReceive( g_pTempQueue, &uValue_Temp_New, 100 );
    	vTaskDelay(1000 / portTICK_RATE_MS);


        if (show > 0)
        {
        	if (show == 1)
        	{
        	sprintf(temp_st, "%f", uValue_Temp_New);
			xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
			Lcd_Write_String(sTemp);
			xQueueSendToBack(g_pKEYQueue, &temp_st[0], 0 );
			xQueueSendToBack(g_pKEYQueue, &temp_st[1], 0 );
			xQueueSendToBack(g_pKEYQueue, &temp_st[2], 0 );
			Lcd_Write_String(ssTemp);
			Key_Shift_Left(17);
		}
             if(show == 2)
             {
                sprintf(vel_st, "%d", (vel*100/30000));
                xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                Lcd_Write_String(sVel);
                xQueueSendToBack(g_pKEYQueue, &vel_st[0], 0 );
                xQueueSendToBack(g_pKEYQueue, &vel_st[1], 0 );
                xQueueSendToBack(g_pKEYQueue, &vel_st[2], 0 );
                Lcd_Write_String(ssVel);
                Key_Shift_Left(16);
             }
        }
    }
}

//*****************************************************************************
//
// Initializes the ATT task.
//
//*****************************************************************************
uint32_t
AttTaskInit(void)
{
    //
    // Create the Idle task.
    //
    if(xTaskCreate(AttTask, (const portCHAR *)"ATT", ATTTASKSTACKSIZE, NULL,
                   tskIDLE_PRIORITY + PRIORITY_ATT_TASK, NULL) != pdTRUE)
    {
        return(1);
    }

    //
    // Success.
    //
    return(0);
}
