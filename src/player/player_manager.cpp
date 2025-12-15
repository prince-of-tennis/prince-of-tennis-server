#include "common/player.h"
#include "player_manager.h"
#include "common/player_input.h" // PlayerInputの定義のためにインクルード
#include <cmath>

// ?拷v?拷?拷?拷C?拷?拷?拷[?拷?拷?拷?拷?拷?拷?拷?拷?拷?拷?拷?拷
void player_init(Player &player, const std::string &name, float x, float y, float z)
{
    player.name = name;
    player.point = {x, y, z};
    player.speed = 5.0f;
}

// ?拷v?拷?拷?拷C?拷?拷?拷[?拷?拷?拷趽锟拷?拷?拷?拷?拷閺堬拷?拷
void player_move(Player &player, float dx, float dy, float dz, float deltaTime)
{
    float len = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (len > 0.0001f)
    {
        dx /= len;
        dy /= len;
        dz /= len;
    }

    player.point.x += dx * player.speed * deltaTime;
    player.point.y += dy * player.speed * deltaTime;
    player.point.z += dz * player.speed * deltaTime;
}

// ?拷v?拷?拷?拷C?拷?拷?拷[?拷?拷?拷w?拷?拷?拷?拷W?拷?拷?拷[?拷v?拷?拷?拷?拷?拷?拷i?拷鈴曪拷p?拷j
void player_set_position(Player &player, float x, float y, float z)
{
    player.point.x = x;
    player.point.y = y;
    player.point.z = z;
}

// 新規追加：プレイヤーの入力を基に位置を更新する関数
void player_update_position(Player &player, const PlayerInput &input, float deltaTime)
{
    float dx = 0.0f;
    float dy = 0.0f; // Y軸方向（高さ）は基本動かないと仮定
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
    
    // 既存のplayer_move関数（移動量と時間を考慮して位置を更新するロジック）を呼び出す
    player_move(player, dx, dy, dz, deltaTime);
}
} // この波括弧は元のファイルの末尾にあったものです。