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
#define SLAVE_ADDRESS_WRITE 0x90
#define SLAVE_ADDRESS_READ 0x91
#define CONFIG_TMP 0x01
#define CONFIG_TMP_BITS 0x00
#define TEMP_REG 0x00



uint32_t temp;


void I2CSend(uint32_t slave_addr, uint8_t reg){
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);
    I2CMasterDataPut(I2C1_BASE, reg);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));
}


void I2CSENDCONFIG(){
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_WRITE, false);
    I2CMasterDataPut(I2C1_BASE, CONFIG_TMP);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));
    I2CMasterDataPut(I2C1_BASE, CONFIG_TMP_BITS);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH );
    while(I2CMasterBusy(I2C1_BASE));
}


void I2C_get_temp(){
    uint32_t temp1, temp2;
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_WRITE, false);
    I2CMasterDataPut(I2C1_BASE, TEMP_REG);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE)); // delay de 40 ms

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDRESS_READ, false);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_FIFO_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE));
    temp1 = I2CMasterDataGet(I2C1_BASE);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_FIFO_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C1_BASE));
    temp2 = I2CMasterDataGet(I2C1_BASE);
    temp = (temp1<<8|temp2);//nao sei se esta certp
}




uint32_t I2CReceive(uint32_t slave_addr)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2C1_BASE, 0XFF);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //return data pulled from the specified register
    return I2CMasterDataGet(I2C1_BASE);
}






int main(void)
{
    uint32_t temp;
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);


    void I2CSENDCONFIG(); //configuracao do i2c


    while(1){
        I2C_get_temp();
    }
    return 0;
}
