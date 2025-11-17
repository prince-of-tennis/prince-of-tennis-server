#include <stdio.h>
#include <unistd.h>
#include "game/game_state.h"
#include "physics/ball_physics.h"
#include "ball.h"

int main()
{
    GameState state;
    init_game(&state);

    const float dt = 0.016f; // 60 FPS

    while (1)
    {
        update_ball(&state.ball, dt);
        handle_bounce(&state.ball, 0.0f, 0.7f);

        printf("Ball pos: (%.2f, %.2f, %.2f)\n",
               state.ball.position.x,
               state.ball.position.y,
               state.ball.position.z);

        usleep(16000);
    }

    return 0;
}
