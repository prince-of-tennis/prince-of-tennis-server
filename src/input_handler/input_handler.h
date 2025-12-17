#pragma once

#include "common/player.h"
#include "common/player_input.h"
#include "common/ball.h"
#include "game/game_state.h"

void apply_player_input(GameState *state, int player_id, const PlayerInput &input, float deltaTime);
