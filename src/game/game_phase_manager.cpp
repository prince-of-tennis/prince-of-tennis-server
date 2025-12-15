#include "game_phase_manager.h"
#include "score_logic.h"
#include "physics/court_check.h"
#include <stdio.h>
#include <stdlib.h>

#define TIME_AFTER_POINT 3.0f    // 得点後から次のサーブまでの時間
#define TIME_MATCH_COMPLETE 2.0f // マッチング完了（人数が揃った）後、ゲーム開始までの時間
#define TIME_GAME_FINISHED 5.0f  // ゲーム終了後、サーバーシャットダウンまでの時間

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
    case GAME_PHASE_MATCH_COMPLETE: // マッチング完了 -> ゲーム開始（サーブ）へ

        if (state->state_timer > TIME_MATCH_COMPLETE)
        {
            update_game_phase(state, GAME_PHASE_START_GAME);
        }
        break;

    case GAME_PHASE_POINT_SCORED: // 得点後 -> 試合終了判定 or 次のサーブへ
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

    case GAME_PHASE_GAME_FINISHED: // 試合終了 -> サーバー終了
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