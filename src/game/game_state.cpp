#include "game/game_state.h"

void init_game(GameState *state)
{
    state->ball.position = (Vec3){0.0f, 1.0f, 0.0f};
    state->ball.velocity = (Vec3){0.0f, 0.0f, 0.0f};
}
