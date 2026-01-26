#include "game/game_state.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "common/game_constants.h"
#include "game/score_logic.h"
#include <cstring>

void init_game(GameState *state)
{
    // Player1: 手前側 (Z > 0)
    player_init(&state->players[0], "Player1", 0.0f, GameConstants::GROUND_Y, GameConstants::PLAYER_BASELINE_DISTANCE);
    state->players[0].player_id = 0;
    state->players[0].connected = false;

    // Player2: 奥側 (Z < 0)
    player_init(&state->players[1], "Player2", 0.0f, GameConstants::GROUND_Y, -GameConstants::PLAYER_BASELINE_DISTANCE);
    state->players[1].player_id = 1;
    state->players[1].connected = false;

    // ボール初期化
    constexpr float INITIAL_SERVE_Z = GameConstants::PLAYER_BASELINE_DISTANCE - GameConstants::BALL_SERVE_OFFSET_FROM_BASELINE;
    state->ball.point = (Point3d){0.0f, GameConstants::BALL_SERVE_HEIGHT, INITIAL_SERVE_Z};
    state->ball.velocity = (Point3d){0.0f, 0.0f, 0.0f};
    state->ball.angle = 0;
    state->ball.last_hit_player_id = 0;
    state->ball.bounce_count = 0;
    state->ball.hit_count = 0;

    init_score(&state->score);
    std::memset(state->ability_states, 0, sizeof(state->ability_states));

    state->match_winner = -1;
    state->match_result_sent = false;
}
