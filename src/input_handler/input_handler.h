#pragma once

#include "common/player.h"
#include "common/player_input.h"
#include "common/player_swing.h"
#include "common/ball.h"
#include "game/game_state.h"

void apply_player_input(GameState *state, int player_id, const PlayerInput *input, float deltaTime);
void apply_player_swing(GameState *state, int player_id, const PlayerSwing *swing);
