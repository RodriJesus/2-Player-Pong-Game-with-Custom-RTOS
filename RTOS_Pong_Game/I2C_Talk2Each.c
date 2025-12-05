// I2C_Talk2Each.c
#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"   // core defs + INT_I2C0
#include "hw_memmap.h"      // peripheral base addresses
#include "hw_types.h"       // HWREG macros
#include "hw_sysctl.h"      // SYSCTL_RCGC*, SYSCTL_PRGPIO
#include "hw_gpio.h"        // GPIO_O_*, GPIO_PIN_x
#include "hw_i2c.h"         // I2C_O_*, I2C_MCS_*, I2C_SRIS_*, I2C_SICR_*

//-------------------------------------------------------------------------
// Globals shared with ball logic
//-------------------------------------------------------------------------
volatile bool    I2C_NewBallFlag = false;
volatile uint8_t I2C_SharedX     = 0;
static   bool    sending         = false;

//-------------------------------------------------------------------------
// Initialize I2C0 in slave mode on PB2/PB3, address = 0x42
//-------------------------------------------------------------------------
void I2C_InitSlave(void) {
    // 1) Enable clocks for I2C0 and GPIOB
    SYSCTL_RCGCI2C_R  |= (1<<0);      // I2C0 clock
    SYSCTL_RCGCGPIO_R |= (1<<1);      // GPIOB clock
    while((SYSCTL_PRGPIO_R & (1<<1)) == 0) {}

    // 2) Configure PB2/PB3 as I2C0 pins
    GPIO_PORTB_AFSEL_R |= (1<<2) | (1<<3);  // alt func
    GPIO_PORTB_ODR_R   |= (1<<3);           // open-drain on SDA
    GPIO_PORTB_DEN_R   |= (1<<2) | (1<<3);  // digital enable

    // map PB2->I2C0SCL, PB3->I2C0SDA via PCTL (function 3)
    GPIO_PORTB_PCTL_R  = (GPIO_PORTB_PCTL_R & ~((0xF<<8)|(0xF<<12)))
                       | (3<<8) | (3<<12);

    // 3) Configure I2C0 as slave, address = 0x42, enable data interrupt
    I2C0_MCR_R   = I2C_MCR_SFE;         // slave function enable
    I2C0_SOAR_R  = 0x42;                // own address 0x42
    I2C0_SIMR_R |= I2C_SIMR_DATAIM;     // mask data interrupt

    // 4) Clear any pending slave interrupts & enable in NVIC
    I2C0_SICR_R = I2C_SICR_DATAIC;      // clear data interrupt
    NVIC_EN0_R |= (1 << INT_I2C0);      // enable I2C0 interrupt

    __enable_irq();                     // global enable
}

//-------------------------------------------------------------------------
// Send one byte as I2C master, then revert to slave
//-------------------------------------------------------------------------
void I2C_SendX(uint8_t x) {
    sending = true;

    // a) disable slave mode
    I2C0_MCR_R = 0;

    // b) master write to slave addr = 0x42
    I2C0_MSA_R = (0x42 << 1) & ~1;      // write
    I2C0_MDR_R = x;                     // data
    I2C0_MCS_R = I2C_MCS_START 
               | I2C_MCS_RUN 
               | I2C_MCS_STOP;          // start+run+stop
    while(I2C0_MCS_R & I2C_MCS_BUSY) {}

    // c) back to slave
    I2C0_MCR_R = I2C_MCR_SFE;
    sending   = false;
}

//-------------------------------------------------------------------------
// I2C0 interrupt handler: read incoming byte, set flag
//-------------------------------------------------------------------------
void I2C0_Handler(void) {
    // Read slave *raw* status, then clear those bits
    uint32_t status = I2C0_SRIS_R;      
    I2C0_SICR_R    = status;

    // If data received...
    if(status & I2C_SRIS_DATARIS) {
        uint8_t data = (uint8_t)I2C0_SDR_R;
        if(!sending) {
            I2C_SharedX     = data;
            I2C_NewBallFlag = true;
        }
    }
}
