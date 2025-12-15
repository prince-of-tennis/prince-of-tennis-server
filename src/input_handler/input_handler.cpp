#include "input_handler.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include <math.h>

#define RACKET_REACH 2.0f // ラケットが届く距離
#define SMASH_HEIGHT 2.0f // スマッシュができる高さ判定（今回は簡易的に使用）
#define BASE_POWER 25.0f  // 基本の打球パワー

void apply_player_input(Player &player, Ball &ball, const PlayerInput &input, float deltaTime)
{
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

    // --- 2. ラケット打球処理 ---
    if (input.swing)
    {

        float dx = player.point.x - ball.point.x;
        float dz = player.point.z - ball.point.z;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist <= RACKET_REACH)
        {
            // 打つ方向を決める
            Point3d dir;
            dir.x = 0.0f; // 左右は狙わず、常にセンター（真ん中）へ返す
            dir.y = 0.5f; // 少し打ち上げる

            // 自分が「手前(Z>0)」にいるなら「奥(Z=-1)」へ、逆なら手前へ打つ
            if (player.point.z > 0)
            {
                dir.z = -1.0f;
            }
            else
            {
                dir.z = 1.0f;
            }

            handle_racket_hit(&ball, dir, BASE_POWER);
            printf("[ACTION] Swing & Hit! Dir Z: %.1f\n", dir.z);
        }
    }
}
