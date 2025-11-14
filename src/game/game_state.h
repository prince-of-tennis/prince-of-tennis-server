#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "physics/ball_physics.h"

typedef struct
{
    Ball ball;
} GameState;

void init_game(GameState *state);

#endif
