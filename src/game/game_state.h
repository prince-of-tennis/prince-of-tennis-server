#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "physics/ball_physics.h"
#include "common/GameScore.h"
#include "common/GamePhase.h"

typedef struct
{
    Ball ball;
    GameScore score;
    GamePhase phase;
    float state_timer;
} GameState;

void init_game(GameState *state);

#endif
