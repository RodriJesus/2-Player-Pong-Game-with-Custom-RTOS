#ifndef PADDLE_H
#define PADDLE_H

#include <stdint.h>

// Initializes paddle's position
void Paddle_Init(void);
// Updates paddle's position based on joystick input
void Paddle_Update(void);
// Returns current x value of paddle
int16_t Paddle_GetX(void);

#endif