#include "player_movement.h"
#include "player_manager.h" // 既存の player_move 関数を利用

void player_update_position(Player &player, const PlayerInput &input, float deltaTime)
{
    float dx = 0.0f;
    float dy = 0.0f; // テニスなので、コート上（X-Z平面）の移動が主
    float dz = 0.0f;

    // 入力に基づき移動方向を決定
    if (input.right) {
        dx += 1.0f;
    }
    if (input.left) {
        dx -= 1.0f;
    }
    if (input.front) {
        dz += 1.0f;
    }
    if (input.back) {
        dz -= 1.0f;
    }
    
    // swingの入力がTrueの場合、移動ロジックに影響を与える処理をここに追加できます。
    // if (input.swing) { ... }

    // 既存のplayer_move関数を使用して位置を更新します。
    // player_moveは方向ベクトルを受け取り、プレイヤーの速度と経過時間に基づいて移動を処理します。
    player_move(player, dx, dy, dz, deltaTime);
}