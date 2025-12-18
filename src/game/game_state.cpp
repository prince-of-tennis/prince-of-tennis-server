#include "game/game_state.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "../log.h"
#include <cstdio>

void init_game(GameState *state)
{
    state->ball.point = (Point3d){0.0f, 1.0f, 0.0f};
    state->ball.velocity = (Point3d){0.0f, 0.0f, 0.0f};
    state->ball.angle = 0;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        // 名前設定 (char配列対応)
        char name[32];
        snprintf(name, sizeof(name), "Player%d", i + 1);

        // 配置: X軸に少しずらして、Z=12.0f(奥)に配置
        float start_x = static_cast<float>(i - 1.5f) * 2.0f;
        float start_z = 12.0f;

        player_init(state->players[i], name, start_x, 0.0f, start_z);

        LOG_INFO("プレイヤー初期化: " << state->players[i].name << " 位置("
                 << start_x << ", 0.0, " << start_z << ")");
    }

    // スコア初期化
    // state->score.server_score = 0;
    // state->score.client_score = 0;
}