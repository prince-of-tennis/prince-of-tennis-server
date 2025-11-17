#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "../objects/player.h"
#include "../controller/controller.h" // PlayerInput ‚ª’è‹`‚³‚ê‚Ä‚¢‚é

void apply_player_input(Player &player, const PlayerInput &input, float deltaTime);

#endif
