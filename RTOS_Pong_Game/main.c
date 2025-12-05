// Libraries included 
#include <stdint.h>
#include "BSP.h"
#include "CortexM.h"
#include "os.h"
#include "paddle.h"
#include "ball.h"
#include "walls.h"
#include "comm_lib.h"

static bool ledPrev = false;   // Remember previous LED level
int32_t CommSema;

// Runs every 33 ms to update game
void Game_Updater(void)
{
    // Move local game objects & handle button input
    Paddle_Update();
    Ball_Update();
}

void CommThread(void) {
    while (1) {
        OS_Wait(&CommSema);  // Block here until signaled

        bool level = Comm_CheckReceived();
        LED_Set(level);

        static bool prevLevel = false;
        static uint32_t riseCount = 0;

        if (!prevLevel && level) {
            riseCount++;
            if (riseCount > 1) {
                Ball_SpawnNew();
            }
        }
        prevLevel = level;
    }
}

void CommSignalThread(void) {
    OS_Signal(&CommSema);
}

// Idle Task
void Idle(void){ while(1){} }

// Main loop
int main(void)
{
    DisableInterrupts();
    BSP_Clock_InitFastest();  // Max CPU Speed
    BSP_LCD_Init();  // LCD Set up
    BSP_Joystick_Init();  // Joystick Init

    Comm_Init();  // Communication Init

    BSP_LCD_FillScreen(LCD_BLACK);  // LCD reset
    Paddle_Init();  // Init game functions
    Ball_Init();
    Walls_Draw();
		Ball_ClearAll();  // Clear all balls
		OS_InitSemaphore(&CommSema, 0);  // Start at 0 = waiting

    OS_Init();  // Set up RTOS
    OS_AddThreads(&CommThread,&Idle,&Idle,&Idle,&Idle,&Idle);  // Add 6 threads as placeholders
    OS_AddPeriodicEventThread(&Game_Updater, 33);  // 30 Hz game update
    OS_AddPeriodicEventThread(&CommSignalThread, 33);
		OS_Launch(10000);  // Launch OS at counter of 10,000 clk cycles

    while(1){}
}
