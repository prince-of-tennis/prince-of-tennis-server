#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include "point_3d.h"
#include "player.h"


void player_init(Player &player, const std::string &name, float x, float y, float z);
void player_move(Player &player, float dx, float dy, float dz, float deltaTime);
void player_set_position(Player &player, float x, float y, float z);

#endif
