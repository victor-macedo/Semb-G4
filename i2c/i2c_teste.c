#include<stdint.h>
#include<stdbool.h>
#include"inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "inc/hw_i2c.h"
#include "driverlib/pin_map.h"

/**
 * main.c
 */


void I2CSend(uint8_t slave_addr, uint8_t message,uint8_t message2){

    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);

    I2CMasterDataPut(I2C1_BASE, message);


    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterDataPut(I2C1_BASE, message2);


    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);


    while(I2CMasterBusy(I2C1_BASE));
}











int main(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), true);
    HWREG(I2C1_BASE + I2C_O_FIFOCTL) = 80008000;


    while(1){
        I2CSend(0x33, 0xaa,0x38);


    }
	return 0;
}
