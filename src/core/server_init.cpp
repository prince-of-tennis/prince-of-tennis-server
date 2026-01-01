#include "server_init.h"
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "common/game_constants.h"
#include "game/game_phase_manager.h"
#include "../server_constants.h"

bool server_initialize(ServerContext *ctx)
{
    // コンテキストをゼロクリア
    memset(ctx, 0, sizeof(ServerContext));

    // フェーズ・スコアの初期値設定
    ctx->last_sent_phase = (GamePhase)GAME_SCORE_INVALID;
    ctx->last_sent_score.current_set = GAME_SCORE_INVALID;
    ctx->last_sent_score.current_game_p1 = GAME_SCORE_INVALID;
    ctx->last_sent_score.current_game_p2 = GAME_SCORE_INVALID;
    for (int i = 0; i < MAX_SETS; i++)
    {
        ctx->last_sent_score.games_in_set[i][0] = GAME_SCORE_INVALID;
        ctx->last_sent_score.games_in_set[i][1] = GAME_SCORE_INVALID;
    }

    // サーバーソケット初期化
    ctx->server_socket = network_init_server(SERVER_PORT);
    if (!ctx->server_socket)
    {
        LOG_ERROR("サーバーソケット初期化失敗");
        return false;
    }

    // ゲーム状態初期化
    init_game(&ctx->state);
    init_phase_manager(&ctx->state);

    // ソケットセット作成
    ctx->socket_set = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!ctx->socket_set)
    {
        LOG_ERROR("ソケットセット作成失敗: " << SDLNet_GetError());
        return false;
    }

    // サーバーソケットをセットに追加
    SDLNet_TCP_AddSocket(ctx->socket_set, ctx->server_socket);

    LOG_SUCCESS("サーバー初期化完了");
    return true;
}

bool server_wait_for_clients(ServerContext *ctx)
{
    SDLNet_SocketSet wait_set = SDLNet_AllocSocketSet(1);
    if (!wait_set)
    {
        LOG_ERROR("待機用ソケットセット作成失敗: " << SDLNet_GetError());
        return false;
    }
    SDLNet_TCP_AddSocket(wait_set, ctx->server_socket);

    int connected_count = count_connected_clients(ctx->players);
    LOG_INFO("クライアント待機開始: 必要人数=" << MAX_CLIENTS);

    int player_id = connected_count;
    while (connected_count < MAX_CLIENTS && *(ctx->running))
    {
        int ready = SDLNet_CheckSockets(wait_set, SOCKET_TIMEOUT_INIT_WAIT_MS);
        if (ready < 0)
        {
            LOG_ERROR("ソケットチェック失敗 (待機中): " << SDLNet_GetError());
            SDLNet_FreeSocketSet(wait_set);
            return false;
        }

        if (ready > 0 && SDLNet_SocketReady(ctx->server_socket))
        {
            TCPsocket new_client_socket = network_accept_client(ctx->server_socket, ctx->players, ctx->connections);
            if (new_client_socket)
            {
                LOG_DEBUG("新しいクライアントを受け付けました (スロット " << player_id << ")");

                // PlayerにIDを割り当てて送信
                Packet packet = create_packet_player_id(player_id);
                network_send_packet(new_client_socket, &packet);

                // ソケットセットに追加
                SDLNet_TCP_AddSocket(ctx->socket_set, new_client_socket);

                player_id++;
                connected_count = count_connected_clients(ctx->players);
                LOG_DEBUG("接続済みクライアント: " << connected_count << " / " << MAX_CLIENTS);
            }
        }
    }

    SDLNet_FreeSocketSet(wait_set);

    if (!*(ctx->running))
    {
        LOG_INFO("クライアント待機中に中断されました");
        return false;
    }

    LOG_SUCCESS("全クライアント接続完了");
    return true;
}

void server_cleanup(ServerContext *ctx)
{
    LOG_INFO("サーバーをクリーンアップしています...");

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

