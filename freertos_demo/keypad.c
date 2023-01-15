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
// The stack size for the KEY toggle task.
//
//*****************************************************************************
#define KEYTASKSTACKSIZE        3000         // Stack size in words

//*****************************************************************************
//
// The queue that holds messages sent to the KEY task.
//
//*****************************************************************************

xQueueHandle g_pKEYQueue;
extern xSemaphoreHandle g_pSTARTSemaphore;
//*****************************************************************************
/* Array of 4x4 to define characters which will be printe on specific key pressed */
char symbol[4][4] =            {{ '1', '2',  '3', 'F'},
                               { '4', '5',  '6', 'E'},
                               { '7', '8',  '9', 'D'},
                               { 'A', '0',  'B', 'C'}};
char tecla;
static const char sTMax[] = "Temp max:000 C";
static const char sinv[] = "invalido";
static const char sTMin[] = "Temp min:000 C";
static const char sData[] = "Data: dd-mm-yyyy";
static const char sHora[] = "Hora: hh:mm:ss";
static const char sVel[] = "Velocidade: ";
static const char start[] = "start";
static const char stop[] = "stop";
static const char sClear = 'W';
static const char sLeft = 'Z';
static const char sRight = 'B';
uint8_t col, row, flag_config, i_count,dias,meses,a,hora,minu,secu,show;
uint16_t  temp_max, temp_min;
uint32_t utempo_inicio,vel,ano;
char t_min[3];
char t_max[3];
char day[2];
char mes[2];
char hor[2];
char min[2];
char sec[2];
char ano_t[4];
uint8_t meses_bisseistos[13] = {1,31,29,31,30,31,30,31,31,30,31,30,31};
uint8_t meses_normais[13]= {1,31,28,31,30,31,30,31,31,30,31,30,31};

bool bvarre, bstart,test;
#define TEMP_MAX_MAX 150

//*****************************************************************************
//
// Essa tarefa realiza a varredura das teclas e deteta a tecla,
// além disso há as configuracoes do sistema
//
//*****************************************************************************
void
Key_Shift_Left(int a)
{
    int i;
    for(i=0; i<a; i++)
    {
        xQueueSendToBack(g_pKEYQueue, &sLeft, 0 );
    }
}
void
Key_Shift_Right(int a)
{
    int i;
    for(i=0; i<a; i++)
    {
        xQueueSendToBack(g_pKEYQueue, &sRight, 0 );
    }
}
static void
vInterrupt_Key()
{
    test = true;
    uint8_t status = 0;

    status = GPIOIntStatus(GPIO_PORTC_BASE,true);
    GPIOIntClear(GPIO_PORTC_BASE, status);
    GPIOIntDisable(GPIO_PORTC_BASE, GPIO_INT_PIN_4 |GPIO_INT_PIN_5 | GPIO_INT_PIN_6 | GPIO_INT_PIN_7);
    vTaskDelay(500/ portTICK_RATE_MS);

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
        if (row == 0) { flag_config = 1; a= 0;}
        else if (row == 1) { flag_config = 2;a=0;}
        else if (row == 2) { flag_config = 3;a=0;}
        else if (row == 3) { flag_config = 4;a=0;}
        }
        if ((row == 3) && (col == 0)){ flag_config = 5;a= 0;}
        if ((row == 3) && (col == 2)){ flag_config = 6;a= 0;}

        tecla = symbol[row][col];
        xQueueSendToBack(g_pKEYQueue, &tecla, 0 );

        switch (flag_config){
             case(0):
             {
                 break;
             }

             case (1)://Data
             {
                 if(a==3){
                    ano_t[i_count-5]= symbol[row][col];
                    i_count++;
                    if(i_count == 9){
                        ++a;
                     }
                  }
                  if(a==2){
                     mes[i_count-3]= symbol[row][col];
                     i_count++;
                     if(i_count == 5){
                        Key_Shift_Right(1);
                         ++a;
                       }
                    }

                 if(a==1){
                    day[i_count-1]= symbol[row][col];
                    i_count++;
                    if(i_count == 3){
                      Key_Shift_Right(1);
                       ++a;
                       }
                     }
                 if (i_count == 0 || a ==0)
                 {
                     i_count = 0;
                     show = 0;
                     xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                     Lcd_Write_String(sData);
                     Key_Shift_Left(10);
                     if(ano != 0){
                         xQueueSendToBack(g_pKEYQueue, &day[0], 0 );
                         xQueueSendToBack(g_pKEYQueue, &day[1], 0 );
                         Key_Shift_Right(1);
                         xQueueSendToBack(g_pKEYQueue, &mes[0], 0 );
                         xQueueSendToBack(g_pKEYQueue, &mes[1], 0 );
                         Key_Shift_Right(1);
                         xQueueSendToBack(g_pKEYQueue, &ano_t[0], 0 );
                         xQueueSendToBack(g_pKEYQueue, &ano_t[1], 0 );
                         xQueueSendToBack(g_pKEYQueue, &ano_t[2], 0 );
                         xQueueSendToBack(g_pKEYQueue, &ano_t[3], 0 );
                         Key_Shift_Left(10);
                     }
                     i_count++;
                     ++a;
                 }

                 if(a==4)
                     {
                         ano = atoi(ano_t);
                         meses   = atoi(mes);
                         dias  = atoi(day);
                         a = 0;
                         i_count = 0;
                         flag_config = 0;
                         xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                         if(meses<13 && meses>0 && dias>0 && ano > 0 ){
                           if ((ano % 4 == 0)  && (ano % 100 != 0)){
                               if(dias>meses_bisseistos[meses]){
                                   dias = 0;
                                   meses = 0;
                                   ano = 0;
                                   Lcd_Write_String(sinv);

                               }
                               }
                            else{
                                if(dias>meses_normais[meses]){
                                    dias = 0;
                                    meses = 0;
                                    ano = 0;
                                    Lcd_Write_String(sinv);

                                }
                             }
                         }
                         else{
                             dias = 0;
                             meses = 0;
                             ano = 0;
                             Lcd_Write_String(sinv);
                         }
                     }
                 break;
             }

             case (2): // Hora
              {


                 if(a==3){
                   sec[i_count-5]= symbol[row][col];
                   i_count++;
                   if(i_count == 7){
                       ++a;
                   }
                 }

                 if(a==2){
                  min[i_count-3]= symbol[row][col];
                  i_count++;
                  if(i_count == 5){
                    Key_Shift_Right(1);
                    ++a;
                     }
                  }

                 if(a==1){
                     hor[i_count-1]= symbol[row][col];
                     i_count++;
                     if(i_count == 3){
                       Key_Shift_Right(1);
                       ++a;
                     }
                 }


                  if (i_count == 0|| a ==0)
                  {
                       i_count = 0;
                       show = 0;
                       xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       Lcd_Write_String(sHora);
                       Key_Shift_Left(8);
                       if(secu != 0){
                           xQueueSendToBack(g_pKEYQueue, &hor[0], 0 );
                           xQueueSendToBack(g_pKEYQueue, &hor[1], 0 );
                           Key_Shift_Right(1);
                           xQueueSendToBack(g_pKEYQueue, &min[0], 0 );
                           xQueueSendToBack(g_pKEYQueue, &min[1], 0 );
                           Key_Shift_Right(1);
                           xQueueSendToBack(g_pKEYQueue, &sec[0], 0 );
                           xQueueSendToBack(g_pKEYQueue, &sec[1], 0 );
                           Key_Shift_Left(8);
                       }

                       i_count++;
                       a = 1;
                  }
                  if(a==4)
                      {
                      secu =atoi(sec);
                      minu =atoi(min);
                      hora =atoi(hor);
                      if( hora<24 && hora>0 && minu < 60 && minu>0 && secu<60 && secu>0 ){
                          utempo_inicio = (SysTickValueGet()/ (portTICK_RATE_MS*1000));
                          xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                      }
                      else {
                          xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                          Lcd_Write_String(sinv);
                          hora = 0;
                          minu = 0;
                          secu = 0;
                      }
                      //Necessidade de adaptar o valor recebid
                      i_count = 0;
                      flag_config = 0;
                      a= 0;
                      }
                  break;
              }
             case(3): // Min Temp
               {
                   if (i_count == 0|| a == 0)
                    {
                       i_count = 0;
                       show = 0;
                       xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                       Lcd_Write_String(sTMin);
                       Key_Shift_Left(5);
                       if(temp_min != 0){
                           xQueueSendToBack(g_pKEYQueue, &t_min[0], 0 );
                           xQueueSendToBack(g_pKEYQueue, &t_min[1], 0 );
                           xQueueSendToBack(g_pKEYQueue, &t_min[2], 0 );
                           Key_Shift_Left(3);

                       }

                       i_count++;
                       a++;
                    }
                   else if (i_count < 3)
                        {
                           //Precisa adicionar aqui a função pra juntar as teclas em uma string
                           //Salva o valor ou mostra no display
                           t_min[i_count-1]= symbol[row][col];
                           i_count++;
                        }
                   else
                       {
                           //temp_min = Converter string em int;
                           t_min[i_count-1]= symbol[row][col];
                           temp_min = atoi(t_min);
                           i_count=0;
                           if(temp_min > temp_max){
                               xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                               bstart = false;
                               temp_min = 0;
                               Lcd_Write_String(sinv);
                           }
                           else {
                                xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                                }
                           a = 0;
                           flag_config = 0;
                       }
                   break;
               }
             case(4): // Max temp
               {
                   if (i_count == 0|| a == 0)
                   {
                          show = 0;
                          i_count = 0;
                          xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                          Lcd_Write_String(sTMax);
                          Key_Shift_Left(5);
                          if(temp_max != 0){
                           xQueueSendToBack(g_pKEYQueue, &t_max[0], 0 );
                           xQueueSendToBack(g_pKEYQueue, &t_max[1], 0 );
                           xQueueSendToBack(g_pKEYQueue, &t_max[2], 0 );
                           Key_Shift_Left(3);

                           }
                          i_count++;
                          a++;
                   }
                   else if (i_count < 3)
                        {
                           //Precisa adicionar aqui a função pra juntar as teclas em uma string
                           //strncat(string_teclado,&tecla,1);
                           t_max[i_count-1]= symbol[row][col];
                           i_count++;

                        }
                   else
                       {
                           //temp_max = Converter string em int;
                           t_max[i_count-1]= symbol[row][col];
                           temp_max = atoi(t_max);
                           i_count = 0;
                           if(temp_max>150 ||temp_max<temp_min ){
                               xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                               bstart = false;
                               temp_max = 0;
                               Lcd_Write_String(sinv);
                           }
                           else {
                               xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                           }
                           a = 0;
                           flag_config = 0;
                       }
                   break;
               }

             case(5): //Start
                {
                     //xSemaphoreGive( g_pSTARTSemaphore );
                     //xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                 if(temp_max>temp_min){
                     show = 0;
                     if( bstart == true){
                        bstart = false;
                        xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                        Lcd_Write_String(stop);
                    }

                     else{
                         bstart = true;
                         xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                         Lcd_Write_String(start);
                     }
                 }
                     else{
                         xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                         Lcd_Write_String(sinv);
                     }



                     break;
                }
             case(6): //Velocidade
                {
                     xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                     ++show;
                     if(show > 2){
                        show = 0;
                     }
                     //xQueueSendToBack(g_pKEYQueue, &sClear, 0 );
                     //xQueueSendToBack(g_pKEYQueue, &sVel, 0 );
                     //xQueueSendToBack(g_pKEYQueue, &vel, 0 ); //Não sei se essa função vai funcionar, pois talvez seja necessario converter o int de velocidade para uma string
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
    flag_config = 0;
    bstart = 0;
    SysTickEnable();
    g_pSTARTSemaphore= xSemaphoreCreateBinary();
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
// Initializes the Key task.
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
    // Create the KEY task.
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
