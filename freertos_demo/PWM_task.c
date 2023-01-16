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
#define PWMTASKSTACKSIZE       900        // Stack size in words


uint32_t temp_max,temp_min,temp;
int vel;
bool bstart,btemp;
static const char sAMin[] = "Temp min atingida";
static const char sAMax[] = "Temp max atingida";
static const char sClear = 'W';

xQueueHandle g_pTempQueue;
xQueueHandle g_pKEYQueue;

xSemaphoreHandle g_pSTARTSemaphore;
//*****************************************************************************
//
// A tarefa recebe os valores de temperatura, calcula a velocidade e manda
// um sinal PWM para a ventoinha
//
//*****************************************************************************
static void
PWMTask(void *pvParameters)
{

      portBASE_TYPE xStatus,xStart;
      vSemaphoreCreateBinary(g_pSTARTSemaphore);
      xSemaphoreTake( g_pSTARTSemaphore, 0 );
      xStart = xSemaphoreTake(g_pSTARTSemaphore,portMAX_DELAY);
      if(xStart == pdTRUE){
        while(1)
        {
            xStatus = xQueueReceive( g_pTempQueue, &temp, portMAX_DELAY );
            if( xStatus == pdPASS )
            {
                if (bstart == 1)
                {
                    if(temp<temp_min){
                        btemp = false;
                        PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, false);
                        vel = 0;

                    }

                    if (temp>=temp_max)
                    {
                        PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
                    }
                    if(temp<temp_max){
                        PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
                        }
                    if(temp>temp_min)
                    {
                        if(btemp == false)
                        {
                            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,30000);
                            PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, true); //inicializacvao do motor para ultrapassar a velocidade minima
                            vTaskDelay(2000/ portTICK_RATE_MS);
                            btemp = true;
                        }
                        if(temp < temp_max)
                            {
                                vel = (temp-temp_min)/(temp_max-temp_min)*30000;
                                if(vel>(30000*0.2))
                                {
                                 PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,vel);
                                }
                                else
                                {
                                    vel =30000*0.2;
                                    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,vel);
                                }
                            }
                        if(temp >= temp_max)
                        {
                            btemp = true;
                            vel = 30000;
                            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_3,vel);
                            //xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                            //xQueueSendToBack(g_pKEYQueue, &sAMax, 0 );

                        }
                    }
                    else
                    {
                        PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
                        PWMOutputState(PWM0_BASE, PWM_OUT_3_BIT, false);
                        btemp = false;
                        vel = 0;
                     }
                }
            }
        }
      }
}
//*****************************************************************************
//
// Initializes the PWM task.
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
    // Create the PWM task.
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
