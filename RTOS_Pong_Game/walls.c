#include "walls.h"
#include "BSP.h"

// Screen dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 160

// Draw wall borders
void Walls_Draw(void) {
  BSP_LCD_FillRect(0, 0, 2, SCREEN_HEIGHT, LCD_WHITE);                 // Left wall
  BSP_LCD_FillRect(SCREEN_WIDTH - 2, 0, 2, SCREEN_HEIGHT, LCD_WHITE); // Right wall
  BSP_LCD_FillRect(0, 0, SCREEN_WIDTH, 2, LCD_WHITE);                 // Top wall
}