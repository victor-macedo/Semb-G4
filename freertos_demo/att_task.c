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
int temp,uValue_Temp_New;
uint32_t flag_config;
int vel;
uint8_t show;
static const char sTemp[] = "Temperatura:";
static const char ssTemp[] = "C";
static const char sVel[] = "Velocidade:";
static const char ssVel[] = "%";
static const char barra[] = "-";
char temp_st [3];
char vel_st [3];
xQueueHandle g_pKEYQueue;
xQueueHandle g_pTempQueue;
const char sClear;
int tempo_inicio,temp_final,tempo;
char day[2];
char mes[2];
char hor[2];
char min[2];
char sec[2];
char ano_t[4];
int dias,meses,hora,minu,secu,ano;
//*****************************************************************************
//
// Essa tarefa atualiza o LCD com os valores do temperatura ou
// da velocidade se a tecla B foi pressionada
//
//*****************************************************************************



static void
AttTask()
{
    int meses_bisseistoss[13] = {366,31,29,31,30,31,30,31,31,30,31,30,31};
    int meses_normaiss[13]= {365,31,28,31,30,31,30,31,31,30,31,30,31};
    while(1)
    {
        temp_final=temp_final+1;
        //temp = xQueueReceive( g_pTempQueue, &uValue_Temp_New, 100 );
    	vTaskDelay(1000 / portTICK_RATE_MS);


        if (show > 0)
        {
        	if (show == 1)
        	{
        	sprintf(temp_st, "%d", uValue_Temp_New);
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



             if(show == 3){
                 tempo = temp_final-tempo_inicio;
                 tempo_inicio = temp_final;
                 secu = secu + (tempo % 60);

                 if (secu > 59){
                     secu = secu-59;
                     ++minu;

                 }

                 minu = minu + ((tempo/60)%60);

                 if(minu > 59){
                     minu = minu-59;
                     ++hora;

                 }


                 hora = hora + ((tempo/60*60)%24);

                 if (hora >23){
                     hora = hora-23;
                     ++dias;
                 }

                 if ((ano % 4 == 0)  && (ano % 100 != 0)){
                     if(dias > meses_bisseistoss[meses] ){
                         dias= 1 + dias - meses_bisseistoss[meses];
                         ++meses;
                         if(meses>12){
                             meses = 1;
                             ++ano;
                         }
                         }
                     }
                    else{
                        if(dias > meses_normaiss[meses] ){
                          dias= 1 + dias - meses_normaiss[meses];
                          ++meses;
                          if(meses>12){
                           meses = 1;
                          ++ano;
                          }
                        }
                    }

                 sprintf(sec, "%d", secu);
                 sprintf(min, "%d", minu);
                 sprintf(hor, "%d", hora);

                 sprintf(day, "%d", dias);
                 sprintf(mes, "%d", meses);
                 sprintf(ano_t, "%d", ano);




                 xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                 xQueueSendToBack(g_pKEYQueue, &hor[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &hor[1], 0 );
                 xQueueSendToBack(g_pKEYQueue, &barra[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &min[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &min[1], 0 );
                 xQueueSendToBack(g_pKEYQueue, &barra[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &sec[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &sec[1], 0 );
                 Key_Shift_Right(1);
                 xQueueSendToBack(g_pKEYQueue, &day[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &day[1], 0 );
                 xQueueSendToBack(g_pKEYQueue, &barra[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &mes[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, &mes[1], 0 );
                 xQueueSendToBack(g_pKEYQueue, &barra[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, & ano_t[0], 0 );
                 xQueueSendToBack(g_pKEYQueue, & ano_t[1], 0 );
                 xQueueSendToBack(g_pKEYQueue, & ano_t[2], 0 );
                 xQueueSendToBack(g_pKEYQueue, & ano_t[3], 0 );
                 Key_Shift_Left(20);

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
