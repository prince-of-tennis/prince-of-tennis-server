#include "network.h"
#include "../log.h"

#include <SDL2/SDL_net.h>
#include <cstring>


// -----------------------------------------------------
// サーバー初期化
// -----------------------------------------------------
TCPsocket network_init_server(int port)
{
    if (SDLNet_Init() < 0)
    {
        LOG_ERROR("SDLNet初期化失敗: " << SDLNet_GetError());
        return nullptr;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, port) < 0)
    {
        LOG_ERROR("ホスト解決失敗: " << SDLNet_GetError());
        return nullptr;
    }

    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server)
    {
        LOG_ERROR("サーバーソケット作成失敗: " << SDLNet_GetError());
        return nullptr;
    }

    LOG_SUCCESS("サーバー起動: ポート " << port << " で待機中...");

    return server;
}

// -----------------------------------------------------
// クライアント受付
// -----------------------------------------------------
TCPsocket network_accept_client(TCPsocket server_socket, Player players[])
{
    TCPsocket client = SDLNet_TCP_Accept(server_socket);
    if (!client)
        return nullptr;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!players[i].connected)
        {
            players[i].socket = client;
            players[i].connected = true;
            players[i].player_id = i;

            LOG_INFO("クライアント接続 (スロット " << i << ")");

            return client;
        }
    }

    LOG_WARN("サーバー満員のため接続を拒否");
    SDLNet_TCP_Close(client);
    return nullptr;
}

// グローバル変数の宣言（main.cppで定義されている）
extern volatile sig_atomic_t g_running;

// -----------------------------------------------------
// 必要人数が揃うまで待機（ノンブロッキング）
// -----------------------------------------------------
void wait_for_clients(TCPsocket server_socket, Player players[])
{
    LOG_INFO("クライアント接続を待機中...");

    // サーバーソケットを監視セットに追加
    SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!socketSet)
    {
        LOG_ERROR("ソケットセット作成失敗: " << SDLNet_GetError());
        return;
    }

    SDLNet_TCP_AddSocket(socketSet, server_socket);

    while (g_running)
    {
        // 100msタイムアウトでノンブロッキング待機
        int numReady = SDLNet_CheckSockets(socketSet, 100);

        if (numReady > 0)
        {
            if (SDLNet_SocketReady(server_socket))
            {
                network_accept_client(server_socket, players);

                // 現在の接続数をカウント
                int connected_count = 0;
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (players[i].connected)
                        connected_count++;
                }

                LOG_INFO("接続済みクライアント: " << connected_count << " / " << REQUIRED_CLIENTS);

                // 必要人数が揃ったらループを抜ける
                if (connected_count >= REQUIRED_CLIENTS)
                {
                    LOG_SUCCESS("全員接続完了！ゲーム開始！");
                    break;
                }
            }
        }
    }

    SDLNet_FreeSocketSet(socketSet);
}

// -----------------------------------------------------
// データ送信
// -----------------------------------------------------
int network_send(TCPsocket client_socket, const void *data, int size)
{
    int result = SDLNet_TCP_Send(client_socket, data, size);
    if (result < size)
    {
        LOG_ERROR("送信失敗: " << SDLNet_GetError());
    }
    return result;
}

// -----------------------------------------------------
// データ受信
// -----------------------------------------------------
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    int result = SDLNet_TCP_Recv(client_socket, buffer, size);
    if (result < 0)
    {
        LOG_ERROR("受信失敗: " << SDLNet_GetError());
    }
    return result;
}

// -----------------------------------------------------
// クライアント切断
// -----------------------------------------------------
void network_close_client(Player *player)
{
    if (player->connected)
    {
        SDLNet_TCP_Close(player->socket);
        player->socket = nullptr;
        player->connected = false;

        LOG_WARN("クライアント切断 (プレイヤーID: " << player->player_id << ")");
    }
}

// -----------------------------------------------------
// サーバー終了
// -----------------------------------------------------
void network_shutdown_server(TCPsocket server_socket)
{
    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
    LOG_INFO("サーバー終了");
}

// =====================================================
// 汎用ブロードキャスト関数の実装
// =====================================================

void network_broadcast(Player players[], PacketType type, const void *data, size_t data_size)
{
    Packet packet;
    memset(&packet, 0, sizeof(Packet));

    packet.type = type;
    packet.size = data_size;

    if (data != nullptr && data_size > 0)
    {
        if (data_size > sizeof(packet.data))
        {
            LOG_ERROR("パケットデータが大きすぎます");
            return;
        }
        memcpy(packet.data, data, data_size);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected)
        {
            if (network_send(players[i].socket, &packet, sizeof(Packet)) < (int)sizeof(Packet))
            {
                LOG_ERROR("クライアント " << i << " への送信失敗");
            }
        }
    }
}