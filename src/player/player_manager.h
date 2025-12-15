#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "common/util/point_3d.h" // Point3dを使用
#include "common/player.h" // Player structを使用
#include "common/player_input.h" // PlayerInput structを使用

// 既存の関数宣言
void player_init(Player &player, const std::string &name, float x, float y, float z);
void player_move(Player &player, float dx, float dy, float dz, float deltaTime);
void player_set_position(Player &player, float x, float y, float z);

// 新規追加：プレイヤーの入力を基に位置を更新する関数
void player_update_position(Player &player, const PlayerInput &input, float deltaTime);

#endif