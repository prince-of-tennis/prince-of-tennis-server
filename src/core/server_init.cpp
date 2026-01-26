#include "server_init.h"
#include <string.h>
#include "log.h"
#include "common/game_constants.h"
#include "game/game_phase_manager.h"
#include "../server_constants.h"

bool server_initialize(ServerContext *ctx, int port)
{
    memset(ctx, 0, sizeof(ServerContext));

    ctx->last_sent_phase = (GamePhase)GAME_SCORE_INVALID;
    ctx->last_sent_score.point_p1 = GAME_SCORE_INVALID;
    ctx->last_sent_score.point_p2 = GAME_SCORE_INVALID;
    ctx->last_sent_score.sets_p1 = GAME_SCORE_INVALID;
    ctx->last_sent_score.sets_p2 = GAME_SCORE_INVALID;

    ctx->server_socket = network_init_server(port);
    if (!ctx->server_socket)
    {
        LOG_ERROR("サーバーソケット初期化失敗");
        return false;
    }

    init_game(&ctx->state);
    init_phase_manager(&ctx->state);

    ctx->socket_set = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!ctx->socket_set)
    {
        LOG_ERROR("ソケットセット作成失敗");
        return false;
    }

    SDLNet_TCP_AddSocket(ctx->socket_set, ctx->server_socket);
    LOG_SUCCESS("サーバー初期化完了");
    return true;
}

bool server_wait_for_clients(ServerContext *ctx)
{
    SDLNet_SocketSet wait_set = SDLNet_AllocSocketSet(1);
    if (!wait_set)
    {
        LOG_ERROR("待機用ソケットセット作成失敗");
        return false;
    }
    SDLNet_TCP_AddSocket(wait_set, ctx->server_socket);

    int connected_count = count_connected_clients(ctx->players);
    int player_id = connected_count;

    while (connected_count < MAX_CLIENTS && *(ctx->running))
    {
        int ready = SDLNet_CheckSockets(wait_set, SOCKET_TIMEOUT_INIT_WAIT_MS);
        if (ready < 0)
        {
            LOG_ERROR("ソケットチェック失敗");
            SDLNet_FreeSocketSet(wait_set);
            return false;
        }

        if (ready > 0 && SDLNet_SocketReady(ctx->server_socket))
        {
            TCPsocket new_socket = network_accept_client(ctx->server_socket, ctx->players, ctx->connections);
            if (new_socket)
            {
                SDLNet_TCP_AddSocket(ctx->socket_set, new_socket);
                player_id++;
                connected_count = count_connected_clients(ctx->players);
            }
        }
    }

    SDLNet_FreeSocketSet(wait_set);

    if (!*(ctx->running))
    {
        LOG_INFO("待機中断");
        return false;
    }

    LOG_SUCCESS("全クライアント接続完了");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->connections[i].socket)
        {
            Packet packet = create_packet_player_id(i);
            network_send_packet(ctx->connections[i].socket, &packet);
        }
    }
    return true;
}

void server_cleanup(ServerContext *ctx)
{
    if (ctx->socket_set)
    {
        SDLNet_FreeSocketSet(ctx->socket_set);
        ctx->socket_set = NULL;
    }

    if (ctx->server_socket)
    {
        network_shutdown_server(ctx->server_socket);
        ctx->server_socket = NULL;
    }

    LOG_SUCCESS("サーバークリーンアップ完了");
}

void server_reset_for_new_game(ServerContext *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->connections[i].socket)
        {
            SDLNet_TCP_DelSocket(ctx->socket_set, ctx->connections[i].socket);
            SDLNet_TCP_Close(ctx->connections[i].socket);
            ctx->connections[i].socket = NULL;
        }
        ctx->players[i].connected = false;
    }

    init_game(&ctx->state);
    init_phase_manager(&ctx->state);

    ctx->last_sent_phase = (GamePhase)GAME_SCORE_INVALID;
    ctx->last_sent_score.point_p1 = GAME_SCORE_INVALID;
    ctx->last_sent_score.point_p2 = GAME_SCORE_INVALID;
    ctx->last_sent_score.sets_p1 = GAME_SCORE_INVALID;
    ctx->last_sent_score.sets_p2 = GAME_SCORE_INVALID;

    LOG_SUCCESS("ゲームリセット完了");
}
