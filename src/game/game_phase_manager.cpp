#include "game_phase_manager.h"
#include "score_logic.h"
#include "../log.h"
#include "common/game_constants.h"
#include "../server_constants.h"

// フェーズ管理の初期化
void init_phase_manager(GameState *state)
{
    state->phase = GAME_PHASE_WAIT_FOR_MATCH;
    state->state_timer = 0.0f;
    state->server_player_id = 0;
}

// ゲームフェーズを設定
void set_game_phase(GameState *state, GamePhase next_phase)
{
    state->phase = next_phase;
    state->state_timer = 0.0f;

}

// 物理更新が有効なフェーズかチェック
bool is_physics_active_phase(GamePhase phase)
{
    return phase != GAME_PHASE_START_GAME &&
           phase != GAME_PHASE_POINT_SCORED;
}

// スイング可能なフェーズかチェック
bool is_swing_allowed_phase(GamePhase phase)
{
    return phase == GAME_PHASE_START_GAME ||
           phase == GAME_PHASE_IN_RALLY;
}

// マッチング完了フェーズの処理
static void handle_match_complete_phase(GameState *state)
{
    if (state->state_timer > TIME_MATCH_COMPLETE)
    {
        set_game_phase(state, GAME_PHASE_START_GAME);
    }
}

// ポイント獲得後フェーズの処理
static void handle_point_scored_phase(GameState *state)
{
    if (state->state_timer > TIME_AFTER_POINT)
    {
        state->ball.hit_count = 0;

        if (match_finished(&state->score))
        {
            // 勝者を記録
            state->match_winner = get_match_winner(&state->score);
            LOG_SUCCESS("試合終了！勝者: Player " << (state->match_winner + 1));
            set_game_phase(state, GAME_PHASE_GAME_FINISHED);
        }
        else
        {
            set_game_phase(state, GAME_PHASE_START_GAME);
        }
    }
}

// ゲーム終了フェーズの処理（待機なしで即座に終了）
static void handle_game_finished_phase(GameState *state, volatile int *running)
{
    if (running)
    {
        *running = 0;
    }
}

// フェーズタイマーを更新（自動遷移処理）
void update_phase_timer(GameState *state, float dt, volatile int *running)
{
    state->state_timer += dt;

    switch (state->phase)
    {
    case GAME_PHASE_MATCH_COMPLETE:
        handle_match_complete_phase(state);
        break;

    case GAME_PHASE_POINT_SCORED:
        handle_point_scored_phase(state);
        break;

    case GAME_PHASE_GAME_FINISHED:
        handle_game_finished_phase(state, running);
        break;

    default:
        break;
    }
}