#include "game_phase_manager.h"
#include "score_logic.h"
#include "physics/court_check.h"
#include "score_logic.h"
#include <stdio.h>
#include <stdlib.h>

#define TIME_AFTER_POINT 3.0f    // 得点表示から次のサーブまで
#define TIME_MATCH_COMPLETE 2.0f // 人が揃ってからゲーム開始まで
#define TIME_GAME_FINISHED 5.0f

void init_phase_manager(GameState *state)
{
    state->phase = GAME_PHASE_WAIT_FOR_MATCH;
    state->state_timer = 0.0f;
    state->server_player_id = 0;
}

void update_game_phase(GameState *state, GamePhase next_phase)
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
            update_game_phase(state, GAME_PHASE_START_GAME);
        }
        break;

    case GAME_PHASE_POINT_SCORED:
        if (state->state_timer > TIME_AFTER_POINT)
        {

            if (match_finished(&state->score))
            {
                update_game_phase(state, GAME_PHASE_GAME_FINISHED);
            }
            else
            {
                update_game_phase(state, GAME_PHASE_START_GAME);
            }
        }
        break;

    case GAME_PHASE_GAME_FINISHED:
        if (state->state_timer > TIME_GAME_FINISHED)
        {
            printf("[SERVER] Shutting down...\n");
            exit(0);
        }
        break;

    default:
        break;
    }
}