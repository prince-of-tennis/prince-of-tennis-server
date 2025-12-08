#pragma once

#include "common/player.h"
#include "common/player_input.h"
#include "common/ball.h"

void apply_player_input(Player &player, Ball &ball, const PlayerInput &input, float deltaTime);
