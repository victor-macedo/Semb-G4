#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/softi2c.h"
//#define NUM_I2C_DATA 3
#define SLAVE_ADDRESS 0x3C



void trash_func(){
    I2CMasterSlaveAddrSet(I2C2_BASE, 0x48, false);
    I2CMasterDataPut(I2C2_BASE, 0x01);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C2_BASE));

    I2CMasterDataPut(I2C2_BASE, 0xAA);
    I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
     while(I2CMasterBusy(I2C2_BASE));

     I2CMasterDataPut(I2C2_BASE, 0xAA);
     I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
     while(I2CMasterBusy(I2C2_BASE));

}









int main(void)
{



    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);



    GPIOPinConfigure(GPIO_PE4_I2C2SCL);
    GPIOPinConfigure(GPIO_PE5_I2C2SDA);





    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_5);

    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), false);



    while(1){
        trash_func();
    }
	return 0;
}




