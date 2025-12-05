#include "paddle.h"
#include "BSP.h"

#define SCREEN_WIDTH 128
#define PADDLE_WIDTH 20
#define PADDLE_HEIGHT 5
#define PADDLE_Y 120

// Paddle's x position and previous x positions
static int16_t paddleX;
static int16_t prevPaddleX;

// Joystick values
static uint16_t joyX, joyY;
static uint8_t select;

// Init paddle position to center of the screen
void Paddle_Init(void) {
  paddleX = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
  prevPaddleX = paddleX;
}

// Read joystick input and update paddle position 
void Paddle_Update(void) {
  BSP_Joystick_Input(&joyX, &joyY, &select);  // Read Joystick input
  if (joyX < 400 && paddleX > 2) {  // Move paddle left
    paddleX -= 2;
  } else if (joyX > 600 && paddleX < SCREEN_WIDTH - PADDLE_WIDTH - 2) {
    paddleX += 2;  // Move paddle right
  }

  BSP_LCD_FillRect(prevPaddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, LCD_BLACK);  // Erase previous paddle position
  BSP_LCD_FillRect(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, LCD_WHITE);  // Draw paddle at new position
  prevPaddleX = paddleX;  // Update previous position
}

// Return current paddle x position
int16_t Paddle_GetX(void) {
  return paddleX;
}