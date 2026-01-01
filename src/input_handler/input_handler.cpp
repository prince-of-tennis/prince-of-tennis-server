#include "input_handler.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "../server_constants.h"
#include "../log.h"
#include <math.h>
#include <stdio.h>

// デバッグ用enum（enumハック）
enum SwingResult {
    SWING_RESULT_TOO_FAR = 0,       // ボールが遠すぎる
    SWING_RESULT_SUCCESS = 1,       // スイング成功
    SWING_RESULT_WRONG_PHASE = 2,   // フェーズが不正
    SWING_RESULT_COUNT              // 要素数
};

// enumハック: デバッグ用文字列配列
const char* swing_result_strings[SWING_RESULT_COUNT] = {
    "ボールが遠すぎる",
    "スイング成功",
    "フェーズが不正"
};

// 打球方向を決定する（プレイヤーの位置から相手コートへの方向）
static Point3d determine_shot_direction(const Player *player)
{
    Point3d dir;
    dir.x = 0.0f; // 左右は狙わず、常にセンター（ネット真ん中）へ返す
    dir.y = BALL_SHOT_ANGLE_Y;

    // プレイヤーの位置に基づいて打つ方向を決定
    // Player1 (Z > 0, 手前側) → 相手コート (Z < 0, 奥側) へ
    // Player2 (Z < 0, 奥側) → 相手コート (Z > 0, 手前側) へ
    if (player->point.z > 0)
    {
        dir.z = -1.0f;  // 奥へ打つ
        LOG_DEBUG("打球方向: 手前側から奥側へ (Z負方向)");
    }
    else
    {
        dir.z = 1.0f;   // 手前へ打つ
        LOG_DEBUG("打球方向: 奥側から手前側へ (Z正方向)");
    }

    return dir;
}

// スイング処理を実行
static void handle_player_swing(GameState *state, int player_id)
{
    Player *player = &state->players[player_id];
    Ball *ball = &state->ball;

    // フェーズチェック: スイング可能なフェーズかどうか
    if (!is_swing_allowed_phase(state->phase))
    {
        LOG_DEBUG("スイング失敗: フェーズが不正 (phase=" << state->phase << ")");
        return;
    }

    // 3D距離を計算（Y軸も含む）
    float dx = player->point.x - ball->point.x;
    float dy = player->point.y - ball->point.y;
    float dz = player->point.z - ball->point.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    // スイング範囲内かチェック
    if (dist > PLAYER_SWING_RADIUS)
    {
        LOG_DEBUG("スイング失敗: " << swing_result_strings[SWING_RESULT_TOO_FAR]
                 << " (距離=" << dist << "m, 最大=" << PLAYER_SWING_RADIUS << "m)");
        return;
    }

    // スイング成功！
    bool is_serve = (state->phase == GAME_PHASE_START_GAME);
    bool is_return = (state->phase == GAME_PHASE_IN_RALLY);

    if (is_serve)
    {
        LOG_INFO("サーブ成功！ player_id=" << player_id << " 距離=" << dist << "m");
    }
    else if (is_return)
    {
        LOG_INFO("打ち返し成功！ player_id=" << player_id << " 距離=" << dist << "m "
                << "(前回打者: player_id=" << ball->last_hit_player_id
                << ", ラリー回数=" << ball->hit_count << ")");
    }

    // 打つ方向を決める
    Point3d dir = determine_shot_direction(player);

    // ボールを打つ
    handle_racket_hit(ball, dir, BALL_SHOT_SPEED);

    // 最後に打ったプレイヤーを記録
    int previous_player_id = ball->last_hit_player_id;
    ball->last_hit_player_id = player_id;

    // バウンド回数をリセット
    ball->bounce_count = 0;

    // ヒット回数を増やす（ラリー回数）
    ball->hit_count++;

    // フェーズ遷移: サーブ(START_GAME)ならラリー(IN_RALLY)へ
    if (state->phase == GAME_PHASE_START_GAME)
    {
        set_game_phase(state, GAME_PHASE_IN_RALLY);
        ball->hit_count = 1;  // サーブが最初のヒット
        LOG_INFO("サービスヒット！フェーズ -> ラリー中");
    }

    LOG_DEBUG("スイング結果: " << swing_result_strings[SWING_RESULT_SUCCESS]
             << " 打球方向=(x:" << dir.x << ", y:" << dir.y << ", z:" << dir.z
             << ") 速度=" << BALL_SHOT_SPEED
             << " player_id=" << player_id
             << " (前回: player_id=" << previous_player_id
             << ", ヒット回数=" << ball->hit_count << ")");
}

// プレイヤー入力を適用
void apply_player_input(GameState *state, int player_id, const PlayerInput *input, float deltaTime)
{
    // プレイヤーID検証
    if (!GameConstants::is_valid_player_id(player_id))
    {
        LOG_ERROR("無効なプレイヤーID: " << player_id);
        return;
    }

    Player *player = &state->players[player_id];

    // 移動処理
    float move_x = 0.0f;
    float move_z = 0.0f;

    if (input->right)
        move_x += 1.0f;
    if (input->left)
        move_x -= 1.0f;
    if (input->front)
        move_z -= 1.0f;
    if (input->back)
        move_z += 1.0f;

    player_move(player, move_x, 0.0f, move_z, deltaTime);

    // スイング処理
    if (input->swing)
    {
        handle_player_swing(state, player_id);
    }
}
