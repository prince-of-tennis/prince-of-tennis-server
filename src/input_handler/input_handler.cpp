#include "input_handler.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "game/game_phase_manager.h"
#include "../log.h"
#include <cmath>
#include <cstdio>

// ============================================
// パラメータ定義（マジックナンバー回避）
// ============================================
namespace PlayerParams {
    constexpr float SWING_RADIUS = 5.0f;  // スイング可能範囲（ボールとの距離）
}

namespace BallParams {
    constexpr float SHOT_SPEED = 25.0f;   // 打球速度
    constexpr float SHOT_ANGLE_Y = 0.5f;  // 打球の上方向成分
}

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

void apply_player_input(GameState *state, int player_id, const PlayerInput &input, float deltaTime)
{
    // ショートカットを作成
    Player &player = state->players[player_id];
    Ball &ball = state->ball;

    float move_x = 0.0f;
    float move_z = 0.0f; // Z軸（前後）用の変数

    // --- 1. 移動処理 ---
    if (input.right)
        move_x += 1.0f;
    if (input.left)
        move_x -= 1.0f;
    if (input.front)
        move_z -= 1.0f;
    if (input.back)
        move_z += 1.0f;

    // プレイヤーの移動を実行 (Y軸=0.0fで地面移動)
    player_move(player, move_x, 0.0f, move_z, deltaTime);

    // --- 2. ラケットで球を打つ（改善版） ---
    if (input.swing)
    {
        // GamePhaseチェック: START_GAME または IN_RALLY の時のみスイング可能
        if (state->phase != GAME_PHASE_START_GAME && state->phase != GAME_PHASE_IN_RALLY)
        {
            LOG_WARN("スイング失敗: " << swing_result_strings[SWING_RESULT_WRONG_PHASE]
                     << " (現在フェーズ=" << state->phase << ")");
            return;
        }

        // 3D距離を計算（Y軸も含む）
        float dx = player.point.x - ball.point.x;
        float dy = player.point.y - ball.point.y;
        float dz = player.point.z - ball.point.z;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        // スイング範囲内かチェック
        if (dist > PlayerParams::SWING_RADIUS)
        {
            LOG_WARN("スイング失敗: " << swing_result_strings[SWING_RESULT_TOO_FAR]
                     << " (距離=" << dist << "m, 必要距離=" << PlayerParams::SWING_RADIUS << "m)"
                     << " ボール位置=(" << ball.point.x << ", " << ball.point.y << ", " << ball.point.z << ")"
                     << " プレイヤー位置=(" << player.point.x << ", " << player.point.y << ", " << player.point.z << ")");
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
                    << "(前回打者: player_id=" << ball.last_hit_player_id
                    << ", ラリー回数=" << ball.hit_count << ")");
        }

        // 打つ方向を決める
        Point3d dir;
        dir.x = 0.0f; // 左右は狙わず、常にセンター（ネット真ん中）へ返す
        dir.y = BallParams::SHOT_ANGLE_Y;  // パラメータから角度取得

        // プレイヤーの位置に基づいて打つ方向を決定
        // Player1 (Z > 0, 手前側) → 相手コート (Z < 0, 奥側) へ
        // Player2 (Z < 0, 奥側) → 相手コート (Z > 0, 手前側) へ
        if (player.point.z > 0)
        {
            dir.z = -1.0f;  // 奥へ打つ
            LOG_DEBUG("打球方向: 手前側から奥側へ (Z負方向)");
        }
        else
        {
            dir.z = 1.0f;   // 手前へ打つ
            LOG_DEBUG("打球方向: 奥側から手前側へ (Z正方向)");
        }

        // ボールを打つ
        handle_racket_hit(&ball, dir, BallParams::SHOT_SPEED);

        // 最後に打ったプレイヤーを記録
        int previous_player_id = ball.last_hit_player_id;
        ball.last_hit_player_id = player_id;

        // バウンド回数をリセット
        ball.bounce_count = 0;

        // ヒット回数を増やす（ラリー回数）
        ball.hit_count++;

        // フェーズ遷移: サーブ(START_GAME)ならラリー(IN_RALLY)へ
        if (state->phase == GAME_PHASE_START_GAME)
        {
            update_game_phase(state, GAME_PHASE_IN_RALLY);
            ball.hit_count = 1;  // サーブが最初のヒット
            LOG_INFO("サービスヒット！フェーズ -> ラリー中");
        }

        LOG_DEBUG("スイング結果: " << swing_result_strings[SWING_RESULT_SUCCESS]
                 << " 打球方向=(x:" << dir.x << ", y:" << dir.y << ", z:" << dir.z
                 << ") 速度=" << BallParams::SHOT_SPEED
                 << " player_id=" << player_id
                 << " (前回: player_id=" << previous_player_id
                 << ", ヒット回数=" << ball.hit_count << ")");
    }
}
