#include "game_phase_manager.h"
#include "score_logic.h"
#include "../log.h"
#include "common/game_constants.h"
#include <cstdlib>

using namespace GameConstants;

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

    switch (next_phase)
    {
    case GAME_PHASE_START_GAME:
        LOG_INFO("フェーズ: ゲーム開始 / サービス");
        break;

    case GAME_PHASE_IN_RALLY:
        LOG_INFO("フェーズ: ラリー中");
        break;

    case GAME_PHASE_POINT_SCORED:
        LOG_INFO("フェーズ: ポイント獲得");
        break;

    case GAME_PHASE_GAME_FINISHED:
        LOG_SUCCESS("フェーズ: ゲームセット！");
        break;

    default:
        break;
    }
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

// フェーズタイマーを更新（自動遷移処理）
void update_phase_timer(GameState *state, float dt)
{
    state->state_timer += dt;

    switch (state->phase)
    {
    case GAME_PHASE_MATCH_COMPLETE:
        if (state->state_timer > TIME_MATCH_COMPLETE)
        {
            set_game_phase(state, GAME_PHASE_START_GAME);
        }
        break;

    case GAME_PHASE_POINT_SCORED:
        if (state->state_timer > TIME_AFTER_POINT)
        {
            state->ball.hit_count = 0;

            if (match_finished(&state->score))
            {
                set_game_phase(state, GAME_PHASE_GAME_FINISHED);
            }
            else
            {
                set_game_phase(state, GAME_PHASE_START_GAME);
            }
        }
        break;

    case GAME_PHASE_GAME_FINISHED:
        if (state->state_timer > TIME_GAME_FINISHED)
        {
            LOG_INFO("サーバーをシャットダウンします...");
            exit(0);
        }
        break;

    default:
        break;
    }
}