#include "ball.h"
#include "BSP.h"
#include "paddle.h"
#include "comm_lib.h"
#include <stdint.h>
#include <stdbool.h>

// Constants
#define SCREEN_WIDTH     128
#define SCREEN_HEIGHT    160
#define BALL_SIZE          4

#define PADDLE_Y         120
#define PADDLE_WIDTH      20

#ifndef MAX_BALLS
  #define MAX_BALLS 4
#endif

// Width of a text character in pixels
#define LCD_CHAR_W        6

// Ball type
typedef struct {
    int16_t  x, y;            // Current position 
    int16_t  prev_x, prev_y;  // Previous position
    int8_t   dx, dy;          // Velocity 
    bool     active;          // Is ball active
} Ball_t;

//  Globals 
static Ball_t   balls[MAX_BALLS];
static bool     lastButton  = false;        // S2 edge detect            
static uint32_t deletedCnt  = 0;            // Score                     
static uint32_t lastShown   = 0xFFFFFFFF;   // Last score drawn          

// This is added to make the trajectories of the balls more random
static const int8_t nudgeTable[9] = { -9,5,-3, 3, 7, 0, -7 ,9 };
static uint8_t      nudgeIndex    = 0;      // Index into nudge table                

// Converts uint32_t to ASCII string
static void u32ToStr(uint32_t n, char *buf)
{
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[10]; int i = 0;
    while (n && i < 10) { tmp[i++] = '0' + (n % 10); n /= 10; }
    int j = 0; while (i) { buf[j++] = tmp[--i]; }
    buf[j] = '\0';
}

// Draws the score in top-left corner
static void drawScore(uint32_t val)
{
    char buf[12]; u32ToStr(val, buf);

    uint16_t row = 0, col = 0;
    BSP_LCD_DrawString(col, row, "                     ", LCD_BLACK); // Clear line
    BSP_LCD_DrawString(col,     row, "Score:", LCD_WHITE);
    BSP_LCD_DrawString(col + 7, row, buf,     LCD_WHITE);
}

// Spawns a new ball in the center of the screen
static void spawnAtCenter(void)
{
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].active) {
            balls[i].x  = SCREEN_WIDTH  / 2;
            balls[i].y  = SCREEN_HEIGHT / 2;
            balls[i].prev_x = balls[i].x;
            balls[i].prev_y = balls[i].y;
            balls[i].dx = 1;
            balls[i].dy = -1;
            balls[i].active = true;
            return;
        }
    }
}

// Helper functions
void Ball_SpawnNew(void)             { spawnAtCenter(); }
uint32_t Ball_GetDeletedCount(void)  { return deletedCnt; }

// Erases previous ball location
static void erasePrev(const Ball_t *b)
{
    BSP_LCD_FillRect(b->prev_x, b->prev_y, BALL_SIZE+2, BALL_SIZE+2, LCD_BLACK);
    if (b->prev_x >= SCREEN_WIDTH - 3)
        BSP_LCD_DrawFastVLine(SCREEN_WIDTH - 1, 0, SCREEN_HEIGHT, LCD_WHITE);
}

// Draws current ball location
static void drawBall(const Ball_t *b)
{
    BSP_LCD_FillRect(b->x, b->y, BALL_SIZE, BALL_SIZE, LCD_WHITE);
}

// Adds variation to ball trajectory (Horizontal shift)
static void applyNudge(Ball_t *b)
{
    int8_t  delta  = nudgeTable[nudgeIndex];
    nudgeIndex     = (nudgeIndex + 1) % 5;

    int16_t newX   = b->x + delta;
    if (newX < 3) newX = 3;
    if (newX > SCREEN_WIDTH - BALL_SIZE - 3)
        newX = SCREEN_WIDTH - BALL_SIZE - 3;

    b->x = newX;
}

// Updates a single ball: movement, collisions, erasing and redrawing
static void updateOne(Ball_t *b)
{
    erasePrev(b);

    b->x += b->dx;
    b->y += b->dy;

    // Remove ball if it goes past bottom
    if (b->y >= SCREEN_HEIGHT - 5) {
        b->active = false;
        deletedCnt++;
        return;
    }

    // Side wall collisions
    if (b->x <= 2) {
        b->x = 3; b->dx = -b->dx; applyNudge(b);
    } else if (b->x >= SCREEN_WIDTH - BALL_SIZE - 2) {
        b->x = SCREEN_WIDTH - BALL_SIZE - 3; b->dx = -b->dx; applyNudge(b);
    }

    // Top wall bounce
    if (b->y <= 2) {
        b->dy = -b->dy; applyNudge(b);
    }

    // Paddle top surface bounce
    int16_t px = Paddle_GetX();
    bool topHit = (b->dy > 0) &&                              /* moving down    */
                  (b->prev_y + BALL_SIZE < PADDLE_Y) &&       /* was above      */
                  (b->y       + BALL_SIZE >= PADDLE_Y);       /* now at/under   */

    if (topHit &&
        (b->x + BALL_SIZE >= px) &&
        (b->x <= px + PADDLE_WIDTH))
    {
        b->dy = -b->dy;
        b->y  = PADDLE_Y - BALL_SIZE - 1;
        applyNudge(b);
    }

    drawBall(b);

    b->prev_x = b->x;
    b->prev_y = b->y;
}

// Init ball
void Ball_Init(void)
{
    for (int i = 0; i < MAX_BALLS; i++) balls[i].active = false;
    deletedCnt = 0; lastShown = 0xFFFFFFFF; nudgeIndex = 0;

    drawScore(0);
    spawnAtCenter();
}

// Update ball
void Ball_Update(void)
{
    static bool lastSelectState = true; // Select button is active-low
    
    // Button press: spawn and notify peer
    bool pressed = Button_IsPressed();
    if (pressed && !lastButton) { 
        spawnAtCenter(); 
        Comm_SendTrigger(); 
    }
    lastButton = pressed;
    
    // Joystick select press: reset game
    uint8_t selectState;
    uint16_t x, y;
    BSP_Joystick_Input(&x, &y, &selectState);
    
    if (!selectState && lastSelectState) { // Active-low, detect falling edge
        Ball_ClearAll();    // First clear all balls
        Ball_ResetScore();  // Then reset the score
    }
    lastSelectState = selectState;

    // Update all active balls 
    for (int i = 0; i < MAX_BALLS; i++)
        if (balls[i].active) updateOne(&balls[i]);

    // Refresh score display only if changed 
    if (deletedCnt != lastShown) {
        drawScore(deletedCnt);
        lastShown = deletedCnt;
    }
}

// Reset score
void Ball_ResetScore(void)
{
    deletedCnt = 0;
    lastShown = 0xFFFFFFFF; // Force score redraw
    drawScore(0); // Immediately update display
}

// Erase all balls
void Ball_ClearAll(void)
{
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            // Erase the ball from screen
            erasePrev(&balls[i]);
            // Deactivate the ball without counting it in score
            balls[i].active = false;
        }
    }
}