#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "comm_lib.h"

/* --------- GPIO layout ----------
   PD6   output  (send pulse)
   PD6   input   (receive pulse)
   PF0   input   (button S2, active-low)
   PF1   output  (red LED)
   -------------------------------- */
// Enable GPIO pins and clock
void Comm_Init(void)
{
    // Enable PortsC,D,F clocks
    SYSCTL_RCGCGPIO_R |= (SYSCTL_RCGCGPIO_R2 |   // C 
                          SYSCTL_RCGCGPIO_R3 |   // D 
                          SYSCTL_RCGCGPIO_R5);   // F 
    while((SYSCTL_PRGPIO_R & (SYSCTL_PRGPIO_R2 |
                              SYSCTL_PRGPIO_R3 |
                              SYSCTL_PRGPIO_R5)) == 0){}

    // PD6 � output 
    GPIO_PORTC_DIR_R |=  0x20;
    GPIO_PORTC_DEN_R |= 0x20;

    // PC5 � input with pull-up from past iteration 
    GPIO_PORTD_DIR_R &= ~0x40;
    GPIO_PORTD_DEN_R |=  0x40;
    GPIO_PORTD_PUR_R |=  0x40;

    // PF0 (button) & PF1 (LED)
    GPIO_PORTF_LOCK_R = 0x4C4F434B;      // unlock PF0 
    GPIO_PORTF_CR_R  |= 0x03;            // commit PF0/PF1 
    GPIO_PORTF_DIR_R &= ~0x01;           // PF0 input  
    GPIO_PORTF_DIR_R |=  0x02;           // PF1 output 
    GPIO_PORTF_DEN_R |=  0x03;
    GPIO_PORTF_PUR_R |=  0x01;           // pull-up on PF0 
}

// 20ms pulse so Game_Updater (30�Hz) never misses it 
void Comm_SendTrigger(void)
{
    GPIO_PORTC_DATA_R |= 0x20;                 // HIGH 
    for(volatile uint32_t i = 0; i < 600000; i++); // �20�ms 
    GPIO_PORTC_DATA_R &= ~0x20;                // LOW  
}

// Checks if PD6 is High
bool Comm_CheckReceived(void)
{
    return (GPIO_PORTD_DATA_R & 0x40u) != 0;
}

// Checks if button S2 Button is pressed 
bool Button_IsPressed(void)
{
    return (GPIO_PORTF_DATA_R & 0x01u) == 0;   // active-low
}

// Check if reset button is pressed from past iteration
bool Button_Reset_IsPressed(void)
{
    return (GPIO_PORTF_DATA_R & 0x10u) == 0;   // active-low
}

// Turns on LED
void LED_Set(bool on)
{
    if(on)  GPIO_PORTF_DATA_R |=  0x02;
    else    GPIO_PORTF_DATA_R &= ~0x02;
}
