#ifndef COMM_LIB_H
#define COMM_LIB_H

#include <stdint.h>
#include <stdbool.h>

// Init GPIO pins used for communication
void Comm_Init(void);

// Sends a 20 ms high pulse on PC5
void Comm_SendTrigger(void);

// Cheks if PD6 is HIGH return true if incoming signal is high
bool Comm_CheckReceived(void);

// Checks if Button S2 is pressed
bool Button_IsPressed(void);

// Turns the onboard LED red on or off
void LED_Set(bool on);

#endif
