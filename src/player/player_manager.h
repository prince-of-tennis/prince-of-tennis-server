#ifndef PLAYER_H
#define PLAYER_H

#include "common/util/point_3d.h"
#include "common/player.h"

void player_init(Player *player, const char *name, float x, float y, float z);
void player_move(Player *player, float dx, float dy, float dz, float deltaTime);
void player_set_position(Player *player, float x, float y, float z);

#endif
