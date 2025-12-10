#include "common/player.h"
#include "player_manager.h"
#include <cmath>

// ?øΩv?øΩ?øΩ?øΩC?øΩ?øΩ?øΩ[?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ?øΩ
void player_init(Player &player, const std::string &name, float x, float y, float z)
{
    player.name = name;
    player.point = {x, y, z};
    player.speed = 5.0f;
}

// ?øΩv?øΩ?øΩ?øΩC?øΩ?øΩ?øΩ[?øΩ?øΩ?øΩ⁄ìÔøΩ?øΩ?øΩ?øΩ?øΩ?øΩÈèàÔøΩ?øΩ
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

// ?øΩv?øΩ?øΩ?øΩC?øΩ?øΩ?øΩ[?øΩ?øΩ?øΩw?øΩ?øΩ?øΩ?øΩW?øΩ÷??øΩ?øΩ[?øΩv?øΩ?øΩ?øΩ?øΩ?øΩ?øΩi?øΩ‚èïÔøΩp?øΩj
void player_set_position(Player &player, float x, float y, float z)
{
    player.point.x = x;
    player.point.y = y;
    player.point.z = z;
}
