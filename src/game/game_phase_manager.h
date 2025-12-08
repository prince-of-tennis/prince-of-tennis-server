#ifndef GAME_PHASE_MANAGER_H
#define GAME_PHASE_MANAGER_H

#include "game_state.h"

void init_phase_manager(GameState *state);
void update_game_phase(GameState *state, GamePhase new_phase);
void update_phase(GameState *state, float dt);

#endif