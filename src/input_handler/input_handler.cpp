#include "input_handler.h"
#include "../player/player_manager.h"

void apply_player_input(Player &player, const PlayerInput &input, float deltaTime)
{
    float dx = 0.0f;
    float dy = 0.0f;

    if(input.right) dx += 1.0f;
    if(input.left)  dx -= 1.0f;
    if(input.up)    dy += 1.0f;
    if(input.down)  dy -= 1.0f;

    // 移動処理
    player_move(player, dx, dy, 0.0f, deltaTime);

    // ラケット振り処理
    if(input.swing)
    {
        // TODO: スイング判定やボールとの衝突処理
    }
}
