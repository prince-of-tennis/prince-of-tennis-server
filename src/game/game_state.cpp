#include "game/game_state.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "physics/court_check.h"
#include "common/game_constants.h"
#include "game/score_logic.h"
#include "../log.h"
#include <stdio.h>
#include <cstring>

void init_game(GameState *state)
{
    // 2人プレイヤー専用初期化
    // Blender座標系(Y-up, 回転90度)からゲーム座標系(Z-up)への変換
    // Blender Y → ゲーム Z（符号反転）

    // Player1: Blender(0, -24.9674, 0) → ゲーム(0, 0, 24.9674) = 手前側
    player_init(&state->players[0], "Player1", 0.0f, GameConstants::GROUND_Y, GameConstants::PLAYER_BASELINE_DISTANCE);
    state->players[0].player_id = 0;
    state->players[0].connected = false;
    LOG_INFO("プレイヤー初期化: " << state->players[0].name
             << " ID=" << state->players[0].player_id
             << " 位置(0.0, " << GameConstants::GROUND_Y << ", " << GameConstants::PLAYER_BASELINE_DISTANCE << ") [手前側]");

    // Player2: Blender(0, 24.9674, 0) → ゲーム(0, 0, -24.9674) = 奥側
    player_init(&state->players[1], "Player2", 0.0f, GameConstants::GROUND_Y, -GameConstants::PLAYER_BASELINE_DISTANCE);
    state->players[1].player_id = 1;
    state->players[1].connected = false;
    LOG_INFO("プレイヤー初期化: " << state->players[1].name
             << " ID=" << state->players[1].player_id
             << " 位置(0.0, " << GameConstants::GROUND_Y << ", " << -GameConstants::PLAYER_BASELINE_DISTANCE << ") [奥側]");

    // ボール初期化: Player1の近くに配置（サーブ位置）
    constexpr float INITIAL_SERVE_POSITION_Z = GameConstants::PLAYER_BASELINE_DISTANCE - GameConstants::BALL_SERVE_OFFSET_FROM_BASELINE;
    state->ball.point = (Point3d){0.0f, GameConstants::BALL_SERVE_HEIGHT, INITIAL_SERVE_POSITION_Z};
    state->ball.velocity = (Point3d){0.0f, 0.0f, 0.0f};
    state->ball.angle = 0;
    state->ball.last_hit_player_id = 0;  // Player1からサーブ
    state->ball.bounce_count = 0;
    state->ball.hit_count = 0;  // ラリー回数初期化

    LOG_INFO("ボール初期化: 位置(0.0, " << GameConstants::BALL_SERVE_HEIGHT << ", " << INITIAL_SERVE_POSITION_Z << ") [Player1のサーブ位置]");

    // スコア初期化
    init_score(&state->score);
    LOG_INFO("スコア初期化: 0-0");

    // 能力状態初期化
    std::memset(state->ability_states, 0, sizeof(state->ability_states));
    LOG_INFO("能力状態初期化完了");

    // 試合結果初期化
    state->match_winner = -1;
    state->match_result_sent = false;
}