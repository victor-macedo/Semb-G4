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
#include "LCD.h"

//*****************************************************************************
//
// The stack size for the ATT toggle task.
//
//*****************************************************************************
#define ATTTASKSTACKSIZE        1000         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the ATT task.
//
//*****************************************************************************

uint32_t temp,flag_config,show,vel;
static const char sTemp[] = "Temperatura:    C";
static const char sVel[] = "Velocidade:    %";

xQueueHandle g_pKEYQueue;
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
    	vTaskDelay(1000 / portTICK_RATE_MS);
        if (flag_config == 0)
        {
        	if (show == 0)
        	{
			xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
			xQueueSendToBack(g_pKEYQueue, &sTemp, 0 );
			Key_Shift_Left(4);
			xQueueSendToBack(g_pKEYQueue, &temp, 0 ); //Não sei se vai funcionar por ser um int maa vale a pena tentar
		}
		else
		{
			xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
			xQueueSendToBack(g_pKEYQueue, &sVel, 0 );
			Key_Shift_Left(3);
			xQueueSendToBack(g_pKEYQueue, &vel, 0 ); //Não sei se vai funcionar por ser um int maa vale a pena tentar
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
