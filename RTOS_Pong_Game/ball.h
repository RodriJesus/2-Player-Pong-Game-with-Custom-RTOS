#ifndef BALL_H
#define BALL_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_BALLS 4  // max number of balls

// Spawn a new ball at center (called on PF0 edge)
void Ball_SpawnNew(void);

// Clear all balls from screen (without affecting score)
void Ball_ClearAll(void);

// Reset score to 0
void Ball_ResetScore(void);

// Get how many balls have been deleted so far
uint32_t Ball_GetDeletedCount(void);

// Start ball position and velocity
void Ball_Init(void);

// Call this function every frame, update ball position
void Ball_Update(void);

#endif