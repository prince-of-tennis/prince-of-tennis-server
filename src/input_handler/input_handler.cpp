#include "input_handler.h"
#include "player/player_manager.h"
#include "physics/ball_physics.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "common/ability.h"
#include "../server_constants.h"
#include "../log.h"
#include <math.h>

static float clamp(float value, float min_val, float max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

static Point3d normalize_direction(Point3d dir)
{
    float mag = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
    if (mag > 0.0001f)
    {
        dir.x /= mag;
        dir.y /= mag;
        dir.z /= mag;
    }
    return dir;
}

static Point3d calculate_shot_direction(Player *player, float acc_x, float acc_y, float acc_z)
{
    float opponent_direction = (player->point.z > 0) ? -1.0f : 1.0f;

    float ratio_x = clamp(acc_x / SWING_ACC_MAX_X, -1.0f, 1.0f);
    float ratio_y = clamp(acc_y / SWING_ACC_MAX_Y, -1.0f, 1.0f);

    Point3d dir;
    dir.x = SWING_ANGLE_X_MIN + (ratio_x + 1.0f) * 0.5f * (SWING_ANGLE_X_MAX - SWING_ANGLE_X_MIN);
    dir.y = SWING_ANGLE_Y_MIN + fmaxf(ratio_y, 0.0f) * (SWING_ANGLE_Y_MAX - SWING_ANGLE_Y_MIN);
    dir.z = opponent_direction * SWING_ANGLE_Z_BASE;

    Point3d normalized = normalize_direction(dir);
    if (normalized.x == dir.x && normalized.y == dir.y && normalized.z == dir.z)
    {
        normalized.x = 0.0f;
        normalized.y = BALL_SHOT_ANGLE_Y;
        normalized.z = opponent_direction;
    }
    return normalize_direction(dir);
}

static float calculate_shot_speed(float acc_x, float acc_y, float acc_z)
{
    float acc_magnitude = sqrtf(acc_x * acc_x + acc_y * acc_y + acc_z * acc_z);
    float speed = BALL_SHOT_SPEED_BASE + acc_magnitude * SWING_SPEED_MULTIPLIER;
    return clamp(speed, SWING_SPEED_MIN, SWING_SPEED_MAX);
}

static void handle_player_swing(GameState *state, int player_id, float acc_x, float acc_y, float acc_z, int shot_type)
{
    Player *player = &state->players[player_id];
    Ball *ball = &state->ball;

    if (!is_swing_allowed_phase(state->phase))
        return;

    float dx = player->point.x - ball->point.x;
    float dy = player->point.y - ball->point.y;
    float dz = player->point.z - ball->point.z;
    float dist = sqrtf(dx * dx + dy * dy + dz * dz);

    if (dist > PLAYER_SWING_RADIUS)
        return;

    Point3d dir = calculate_shot_direction(player, acc_x, acc_y, acc_z);
    float speed = calculate_shot_speed(acc_x, acc_y, acc_z);

    bool is_lob = (shot_type == SHOT_TYPE_LOB);
    if (is_lob)
        speed *= LOB_SHOT_SPEED_MULTIPLIER;

    handle_racket_hit(ball, dir, speed);
    ball->velocity.z *= Z_VELOCITY_DAMPING;

    if (is_lob)
    {
        ball->velocity.y *= LOB_SHOT_Y_BOOST;
        ball->velocity.z *= LOB_SHOT_Z_BOOST;
        ball->gravity_multiplier = LOB_SHOT_GRAVITY_MULTIPLIER;
    }
    else
    {
        ball->gravity_multiplier = 1.0f;
    }

    AbilityState *ability_state = &state->ability_states[player_id];
    if (ability_state->active_ability == ABILITY_SPEED_UP && ability_state->remaining_frames > 0)
    {
        ball->velocity.z *= 3.0f;
        ball->gravity_multiplier = 2.0f;
        ability_state->active_ability = ABILITY_NONE;
        ability_state->remaining_frames = 0;
    }

    ball->last_hit_player_id = player_id;
    ball->bounce_count = 0;
    ball->hit_count++;

    if (state->phase == GAME_PHASE_START_GAME)
    {
        set_game_phase(state, GAME_PHASE_IN_RALLY);
        ball->hit_count = 1;
    }
}

static void clamp_player_to_court(Player *player, int player_id)
{
    if (player_id == 0 && player->point.z < GameConstants::NET_POSITION_Z)
        player->point.z = GameConstants::NET_POSITION_Z;
    else if (player_id == 1 && player->point.z > GameConstants::NET_POSITION_Z)
        player->point.z = GameConstants::NET_POSITION_Z;
}

void apply_player_input(GameState *state, int player_id, const PlayerInput *input, float deltaTime)
{
    if (!GameConstants::is_valid_player_id(player_id))
        return;

    Player *player = &state->players[player_id];

    float move_x = 0.0f;
    float move_z = 0.0f;

    if (input->right) move_x += 1.0f;
    if (input->left) move_x -= 1.0f;
    if (input->front) move_z -= 1.0f;
    if (input->back) move_z += 1.0f;

    player_move(player, move_x, 0.0f, move_z, deltaTime);
    clamp_player_to_court(player, player_id);
}

void apply_player_swing(GameState *state, int player_id, const PlayerSwing *swing)
{
    if (!GameConstants::is_valid_player_id(player_id))
        return;

    handle_player_swing(state, player_id, swing->acc_x, swing->acc_y, swing->acc_z, swing->shot_type);
}
