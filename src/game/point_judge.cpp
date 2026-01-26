#include "point_judge.h"
#include "physics/court_check.h"
#include "common/game_constants.h"

int judge_point(GameState *state)
{
    if (!state || state->phase != GAME_PHASE_IN_RALLY)
        return GameConstants::PLAYER_ID_INVALID;

    Ball *ball = &state->ball;
    bool is_in = is_in_court(ball->point);

    // アウト: 1回目のバウンドでコート外
    if (ball->bounce_count == 1 && !is_in)
        return GameConstants::get_opponent_player_id(ball->last_hit_player_id);

    // ツーバウンド: 打った人の得点
    if (ball->bounce_count == 2)
        return ball->last_hit_player_id;

    return GameConstants::PLAYER_ID_INVALID;
}
