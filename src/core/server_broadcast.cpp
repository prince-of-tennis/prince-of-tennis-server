#include "server_broadcast.h"
#include <cstring>
#include "common/ability.h"
#include "log.h"

void broadcast_ball_state(ServerContext *ctx)
{
    Packet ball_packet = create_packet_ball_state(&ctx->state.ball);
    network_broadcast(ctx->players, ctx->connections, &ball_packet);
}

void broadcast_phase_update(ServerContext *ctx)
{
    if (ctx->state.phase != ctx->last_sent_phase)
    {
        LOG_DEBUG("フェーズ変更: " << ctx->last_sent_phase << " -> " << ctx->state.phase);

        Packet phase_packet = create_packet_phase(ctx->state.phase);
        network_broadcast(ctx->players, ctx->connections, &phase_packet);

        ctx->last_sent_phase = ctx->state.phase;
    }
}

void broadcast_score_update(ServerContext *ctx)
{
    // スコアが変更されたかチェック
    if (memcmp(&ctx->state.score, &ctx->last_sent_score, sizeof(GameScore)) != 0)
    {
        LOG_INFO("スコア更新を送信");

        Packet score_packet = create_packet_score(&ctx->state.score);
        network_broadcast(ctx->players, ctx->connections, &score_packet);

        ctx->last_sent_score = ctx->state.score;
    }
}

void broadcast_initial_player_states(ServerContext *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->players[i].connected && ctx->connections[i].socket)
        {
            Packet player_packet = create_packet_player_state(&ctx->state.players[i]);
            network_broadcast(ctx->players, ctx->connections, &player_packet);
        }
    }

    LOG_DEBUG("初期プレイヤー状態を送信");
}

void broadcast_ability_state(ServerContext *ctx, int player_id)
{
    if (player_id < 0 || player_id >= MAX_CLIENTS)
    {
        return;
    }

    Packet ability_packet = create_packet_ability_state(&ctx->state.ability_states[player_id]);
    network_broadcast(ctx->players, ctx->connections, &ability_packet);
}
