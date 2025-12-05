#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "physics/ball_physics.h"
#include "../../../prince-of-tennis-common/GamePhase.h"

typedef struct
{
    Ball ball;
    GamePhase current_phase;
} GameState;

void init_game(GameState *state);
void update_game(GameState *state, float dt);
#endif
