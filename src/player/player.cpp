#include "player.h"
#include <cmath>

// プレイヤーを初期化する
void player_init(Player &player, const std::string &name, float x, float y, float z)
{
    player.name = name;
    player.point = { x, y, z };
    player.speed = 5.0f;
}


// プレイヤーを移動させる処理
void player_move(Player &player, float dx, float dy, float dz, float deltaTime)
{
    float len = std::sqrt(dx*dx + dy*dy + dz*dz);

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


// プレイヤーを指定座標へワープさせる（補助用）
void player_set_position(Player &player, float x, float y, float z)
{
    player.point.x = x;
    player.point.y = y;
    player.point.z = z;
}
