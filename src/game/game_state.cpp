#include "game/game_state.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "physics/court_check.h"
#include "../log.h"
#include <cstdio>

void init_game(GameState *state)
{
    // ボール初期化（コート中央、少し浮いた位置）
    state->ball.point = (Point3d){0.0f, 1.0f, 0.0f};
    state->ball.velocity = (Point3d){0.0f, 0.0f, 0.0f};
    state->ball.angle = 0;

    // 2人プレイヤー専用初期化
    // Blender座標系(Y-up, 回転90度)からゲーム座標系(Z-up)への変換
    // Blender Y → ゲーム Z（符号反転）

    // Player1: Blender(0, -24.9674, 0) → ゲーム(0, 0, 24.9674) = 手前側
    player_init(state->players[0], "Player1", 0.0f, 0.0f, PLAYER_BASELINE_DISTANCE);
    LOG_INFO("プレイヤー初期化: " << state->players[0].name
             << " 位置(0.0, 0.0, " << PLAYER_BASELINE_DISTANCE << ") [手前側]");

    // Player2: Blender(0, 24.9674, 0) → ゲーム(0, 0, -24.9674) = 奥側
    player_init(state->players[1], "Player2", 0.0f, 0.0f, -PLAYER_BASELINE_DISTANCE);
    LOG_INFO("プレイヤー初期化: " << state->players[1].name
             << " 位置(0.0, 0.0, " << -PLAYER_BASELINE_DISTANCE << ") [奥側]");
}