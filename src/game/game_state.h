#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "physics/ball_physics.h"
#include "common/GameScore.h"
#include "common/GamePhase.h"

typedef struct
{
    Ball ball;
    GameScore score;
    float state_timer;

    GamePhase phase;

    int server_player_id;
} GameState;

void init_game(GameState *state);
void update_game(GameState *state, float dt);
#endif
