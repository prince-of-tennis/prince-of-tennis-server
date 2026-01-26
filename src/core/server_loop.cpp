#include "server_loop.h"
#include <SDL2/SDL.h>
#include "log.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "common/ability.h"
#include "server_broadcast.h"
#include "game_update.h"
#include "../server_constants.h"

static void update_ability_states(ServerContext *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        AbilityState *state = &ctx->state.ability_states[i];

        if (state->active_ability == ABILITY_GIANT || state->active_ability == ABILITY_CLONE)
            continue;

        if (state->remaining_frames > 0)
        {
            state->remaining_frames--;
            if (state->remaining_frames == 0)
            {
                state->active_ability = ABILITY_NONE;
                broadcast_ability_state(ctx, i);
            }
        }
    }
}

void server_run_main_loop(ServerContext *ctx)
{
    const float dt = GameConstants::FRAME_TIME;

    broadcast_initial_player_states(ctx);
    set_game_phase(&ctx->state, GAME_PHASE_START_GAME);
    broadcast_score_update(ctx);

    LOG_SUCCESS("ゲーム開始");

    while (*(ctx->running) != 0)
    {
        int ready_count = SDLNet_CheckSockets(ctx->socket_set, SOCKET_TIMEOUT_MAIN_LOOP_MS);

        if (ready_count < 0)
        {
            LOG_ERROR("ソケットチェック失敗");
            break;
        }

        if (ready_count > 0)
        {
            if (SDLNet_SocketReady(ctx->server_socket))
            {
                TCPsocket new_socket = network_accept_client(ctx->server_socket, ctx->players, ctx->connections);
                if (new_socket)
                    SDLNet_TCP_AddSocket(ctx->socket_set, new_socket);
            }
            game_handle_client_input(ctx, dt);
        }

        update_phase_timer(&ctx->state, dt, ctx->running);
        game_update_physics_and_scoring(ctx, dt);
        update_ability_states(ctx);

        broadcast_ball_state(ctx);
        broadcast_phase_update(ctx);

        if (ctx->state.phase == GAME_PHASE_GAME_FINISHED && !ctx->state.match_result_sent)
        {
            broadcast_match_result(ctx, ctx->state.match_winner);
            ctx->state.match_result_sent = true;
        }

        SDL_Delay(GameConstants::FRAME_DELAY_MS);
    }

    LOG_INFO("メインループ終了");
}
