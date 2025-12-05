// I2C_Talk2Each.h
#ifndef I2C_TALK2EACH_H
#define I2C_TALK2EACH_H

#include <stdint.h>
#include <stdbool.h>

// call this once in main() before launching your OS
void I2C_InitSlave(void);

// call this whenever you want to send the local ball's X over I²C
void I2C_SendX(uint8_t x);

// after you call I2C_InitSlave(), these will be driven by the ISR:
extern volatile bool    I2C_NewBallFlag;  // set when a byte arrives
extern volatile uint8_t I2C_SharedX;      // last byte received


#endif // I2C_TALK2EACH_H
