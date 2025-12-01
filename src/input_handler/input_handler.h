#pragma once

#include "common/player.h"
#include "common/player_input.h"

void apply_player_input(Player &player, const PlayerInput &input, float deltaTime);
