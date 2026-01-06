#include "input_handler.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "../server_constants.h"
#include "../log.h"
#include <math.h>

// スイング処理を実行（加速度パラメータを使用）
static void handle_player_swing(GameState *state, int player_id, float acc_x, float acc_y, float acc_z)
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
        LOG_DEBUG("スイング失敗: ボールが遠すぎる"
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

    // 加速度ベースの角度調整システム
    Point3d dir;

    // 相手コート方向を計算（プレイヤーの位置に基づく）
    float opponent_direction = (player->point.z > 0) ? -1.0f : 1.0f;

    // 加速度の大きさから打球速度を計算
    float acc_magnitude = sqrtf(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);

    // デバッグ：受信した加速度値をログ出力
    LOG_INFO("【サーバー】player_id=" << player_id
             << " 受信加速度: x=" << acc_x << " y=" << acc_y << " z=" << acc_z
             << " magnitude=" << acc_magnitude);

    // 各軸の加速度を-1.0～1.0の割合に正規化
    float ratio_x = acc_x / SWING_ACC_MAX_X;
    float ratio_y = acc_y / SWING_ACC_MAX_Y;
    float ratio_z = acc_z / SWING_ACC_MAX_Z;

    // 割合を-1.0～1.0の範囲にクランプ
    if (ratio_x < -1.0f) ratio_x = -1.0f;
    if (ratio_x > 1.0f) ratio_x = 1.0f;
    if (ratio_y < -1.0f) ratio_y = -1.0f;
    if (ratio_y > 1.0f) ratio_y = 1.0f;
    if (ratio_z < -1.0f) ratio_z = -1.0f;
    if (ratio_z > 1.0f) ratio_z = 1.0f;

    // 方向を計算（割合に基づいて角度範囲内で調整）
    // X軸: 左右の打ち分け（-1.0=左、0.0=中央、1.0=右）
    dir.x = SWING_ANGLE_X_MIN + (ratio_x + 1.0f) * 0.5f * (SWING_ANGLE_X_MAX - SWING_ANGLE_X_MIN);

    // Y軸: 高さ調整（-1.0=低い弾道、1.0=高い弾道）
    // ratio_yを0.0～1.0の範囲に変換（負の値は無視、上方向のみ使用）
    float ratio_y_positive = (ratio_y > 0.0f) ? ratio_y : 0.0f;
    dir.y = SWING_ANGLE_Y_MIN + ratio_y_positive * (SWING_ANGLE_Y_MAX - SWING_ANGLE_Y_MIN);

    // Z軸: 固定で相手コート方向
    dir.z = opponent_direction * SWING_ANGLE_Z_BASE;

    // 方向ベクトルを正規化
    float dir_magnitude = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (dir_magnitude > 0.0001f)
    {
        dir.x /= dir_magnitude;
        dir.y /= dir_magnitude;
        dir.z /= dir_magnitude;
    }
    else
    {
        // フォールバック
        dir.x = 0.0f;
        dir.y = BALL_SHOT_ANGLE_Y;
        dir.z = opponent_direction;
    }

    // 速度を計算（加速度に基づくが、下限と上限を設定）
    float speed = BALL_SHOT_SPEED_BASE + acc_magnitude * SWING_SPEED_MULTIPLIER;
    if (speed < SWING_SPEED_MIN) speed = SWING_SPEED_MIN;  // 最低速度
    if (speed > SWING_SPEED_MAX) speed = SWING_SPEED_MAX;  // 最高速度

    // デバッグ：最終的な打球方向をログ出力
    LOG_INFO("  最終方向: x=" << dir.x << " y=" << dir.y << " z=" << dir.z
             << " 速度=" << speed << " m/s");
    LOG_INFO("  意味: " << (dir.x < -0.01f ? "左" : dir.x > 0.01f ? "右" : "中央")
             << "/" << (dir.y < 0.3f ? "低" : dir.y > 0.4f ? "高" : "中")
             << "/" << (dir.z * opponent_direction > 0 ? "前方" : "後方"));

    // ボールを打つ
    handle_racket_hit(ball, dir, speed);

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

    LOG_DEBUG("スイング成功"
             << " 打球方向=(x:" << dir.x << ", y:" << dir.y << ", z:" << dir.z
             << ") 速度=" << speed
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

    // 移動処理（従来通り）
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
}

// プレイヤースイングを適用
void apply_player_swing(GameState *state, int player_id, const PlayerSwing *swing)
{
    // プレイヤーID検証
    if (!GameConstants::is_valid_player_id(player_id))
    {
        LOG_ERROR("無効なプレイヤーID: " << player_id);
        return;
    }

    // スイング処理を実行
    handle_player_swing(state, player_id, swing->acc_x, swing->acc_y, swing->acc_z);
}
