#include "game_phase_manager.h"
#include <stdio.h>

#define TIME_AFTER_POINT 3.0f    // 得点表示から次のサーブまで
#define TIME_MATCH_COMPLETE 2.0f // 人が揃ってからゲーム開始まで

void init_phase_manager(GameState *state)
{
    state->phase = GAME_PHASE_WAIT_FOR_MATCH;
    state->state_timer = 0.0f;
    state->server_player_id = 0;
}

void change_phase(GameState *state, GamePhase next_phase)
{
    state->phase = next_phase;
    state->state_timer = 0.0f;

    switch (next_phase)
    {
    case GAME_PHASE_START_GAME:
        printf("[PHASE] Start Game / Service\n");
        break;

    case GAME_PHASE_IN_RALLY:
        printf("[PHASE] In Rally\n");
        break;

    case GAME_PHASE_POINT_SCORED:
        printf("[PHASE] Point Scored\n");
        break;

    case GAME_PHASE_GAME_FINISHED:
        printf("[PHASE] Game Set!\n");
        break;

    default:
        break;
    }
}

void update_phase(GameState *state, float dt)
{
    state->state_timer += dt;

    switch (state->phase)
    {
    case GAME_PHASE_MATCH_COMPLETE:

        if (state->state_timer > TIME_MATCH_COMPLETE)
        {
            change_phase(state, GAME_PHASE_START_GAME);
        }
        break;

    case GAME_PHASE_POINT_SCORED:
        if (state->state_timer > TIME_AFTER_POINT)
        {
            change_phase(state, GAME_PHASE_START_GAME);
        }
        break;

    default:
        break;
    }
}