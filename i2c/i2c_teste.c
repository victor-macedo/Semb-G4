#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/softi2c.h"
//#define NUM_I2C_DATA 3
#define SLAVE_ADDRESS 0x3C



void trash_func(){
    I2CMasterSlaveAddrSet(I2C1_BASE, 0x48, false);
    I2CMasterDataPut(I2C1_BASE, 0x01);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));
    
    I2CMasterDataPut(I2C1_BASE, 0xAA);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
     while(I2CMasterBusy(I2C1_BASE));
        
     I2CMasterDataPut(I2C1_BASE, 0xAA);
     I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
     while(I2CMasterBusy(I2C1_BASE));

}









int main(void)
{

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);


    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);



    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);





    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);



    while(1){
        trash_func();
    }
	return 0;
}



