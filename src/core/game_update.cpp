#include "game_update.h"
#include <cstring>
#include "log.h"
#include "input_handler/input_handler.h"
#include "physics/ball_physics.h"
#include "game/score_logic.h"
#include "game/game_phase_manager.h"
#include "game/point_judge.h"
#include "common/ability.h"
#include "common/ability_config.h"
#include "common/player_input.h"
#include "common/player_swing.h"
#include "common/game_constants.h"
#include "server_broadcast.h"

static void handle_point_scored(ServerContext *ctx, int winner_id)
{
    bool game_continues = add_point(&ctx->state.score, winner_id);
    broadcast_score_update(ctx);
    print_score(&ctx->state.score);

    if (!game_continues)
    {
        ctx->state.match_winner = winner_id;
        set_game_phase(&ctx->state, GAME_PHASE_GAME_FINISHED);
        return;
    }

    set_game_phase(&ctx->state, GAME_PHASE_START_GAME);
    int next_server = GameConstants::get_opponent_player_id(winner_id);
    reset_ball(&ctx->state.ball, next_server);
    ctx->state.server_player_id = next_server;
}

static void handle_ability_toggle(ServerContext *ctx, int player_id, const AbilityActivateRequest *request)
{
    AbilityState *state = &ctx->state.ability_states[player_id];
    state->player_id = player_id;

    if (request->trigger == TRIGGER_INSTANT)
    {
        state->active_ability = request->ability_type;
        state->remaining_frames = 1;
    }
    else
    {
        state->active_ability = ABILITY_NONE;
        state->remaining_frames = 0;
    }
    broadcast_ability_state(ctx, player_id);
}

static void handle_ability_standard(ServerContext *ctx, int player_id, const AbilityActivateRequest *request)
{
    const AbilityConfig *config = ability_get_config(request->ability_type);
    if (config == nullptr || !config->requires_server)
        return;

    AbilityState *state = &ctx->state.ability_states[player_id];
    state->player_id = player_id;
    state->active_ability = request->ability_type;
    state->remaining_frames = config->duration_frames;
    broadcast_ability_state(ctx, player_id);
}

void game_handle_client_input(ServerContext *ctx, float dt)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!ctx->players[i].connected || !ctx->connections[i].socket)
            continue;

        while (SDLNet_SocketReady(ctx->connections[i].socket))
        {
            Packet packet;
            int size = network_receive_packet(ctx->connections[i].socket, &packet);

            if (size <= 0)
            {
                LOG_WARN("クライアント " << i << " から切断されました");
                SDLNet_TCP_DelSocket(ctx->socket_set, ctx->connections[i].socket);
                network_close_client(&ctx->players[i], &ctx->connections[i]);
                break;
            }

            PacketType pkt_type = (PacketType)packet.type;

            if (pkt_type == PACKET_TYPE_PLAYER_INPUT && packet.size == sizeof(PlayerInput))
            {
                PlayerInput input;
                memcpy(&input, packet.data, sizeof(PlayerInput));
                apply_player_input(&ctx->state, i, &input, dt);

                bool has_input = input.right || input.left || input.front || input.back;
                if (has_input)
                {
                    Packet player_packet = create_packet_player_state(&ctx->state.players[i]);
                    network_broadcast(ctx->players, ctx->connections, &player_packet);
                }
            }
            else if (pkt_type == PACKET_TYPE_PLAYER_SWING && packet.size == sizeof(PlayerSwing))
            {
                PlayerSwing swing;
                memcpy(&swing, packet.data, sizeof(PlayerSwing));
                apply_player_swing(&ctx->state, i, &swing);
            }
            else if (pkt_type == PACKET_TYPE_ABILITY_REQUEST && packet.size == sizeof(AbilityActivateRequest))
            {
                AbilityActivateRequest request;
                memcpy(&request, packet.data, sizeof(AbilityActivateRequest));

                if (request.ability_type == ABILITY_GIANT || request.ability_type == ABILITY_CLONE)
                    handle_ability_toggle(ctx, i, &request);
                else
                    handle_ability_standard(ctx, i, &request);
            }
        }
    }
}

void game_update_physics_and_scoring(ServerContext *ctx, float dt)
{
    if (is_physics_active_phase(ctx->state.phase))
        update_ball(&ctx->state.ball, dt);

    if (ctx->state.phase == GAME_PHASE_IN_RALLY)
    {
        Ball *ball = &ctx->state.ball;

        bool crossed_net = (ball->previous_z * ball->point.z < 0.0f) ||
                           (ball->previous_z == GameConstants::NET_POSITION_Z) ||
                           (ball->point.z == GameConstants::NET_POSITION_Z);

        if (crossed_net && ball->point.y <= GameConstants::NET_HEIGHT)
        {
            int winner_id = GameConstants::get_opponent_player_id(ball->last_hit_player_id);
            handle_point_scored(ctx, winner_id);
            return;
        }
    }

    if (is_physics_active_phase(ctx->state.phase) &&
        handle_bounce(&ctx->state.ball, GameConstants::GROUND_Y, GameConstants::BOUNCE_RESTITUTION))
    {
        ctx->state.ball.bounce_count++;

        int winner_id = judge_point(&ctx->state);
        if (winner_id != GameConstants::PLAYER_ID_INVALID)
            handle_point_scored(ctx, winner_id);
    }
}
