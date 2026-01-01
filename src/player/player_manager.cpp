#include "common/player.h"
#include "common/game_constants.h"
#include "player_manager.h"
#include <math.h>
#include <string.h>

// プレイヤー初期化
void player_init(Player *player, const char *name, float x, float y, float z)
{

    strncpy(player->name, name, sizeof(player->name) - 1);
    player->name[sizeof(player->name) - 1] = '\0';

    player->point = {x, y, z};
    player->speed = GameConstants::PLAYER_MOVE_SPEED;
}

// プレイヤー移動処理
void player_move(Player *player, float dx, float dy, float dz, float deltaTime)
{
    float len = sqrtf(dx * dx + dy * dy + dz * dz);

    if (len > GameConstants::PLAYER_MOVEMENT_EPSILON)
    {
        dx /= len;
        dy /= len;
        dz /= len;
    }

    player->point.x += dx * player->speed * deltaTime;
    player->point.y += dy * player->speed * deltaTime;
    player->point.z += dz * player->speed * deltaTime;
}

// プレイヤー位置指定
void player_set_position(Player *player, float x, float y, float z)
{
    player->point.x = x;
    player->point.y = y;
    player->point.z = z;
}
