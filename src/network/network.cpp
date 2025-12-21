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
TCPsocket network_accept_client(TCPsocket server_socket, Player players[], ClientConnection connections[])
{
    TCPsocket client = SDLNet_TCP_Accept(server_socket);
    if (!client)
        return nullptr;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!players[i].connected)
        {
            connections[i].socket = client;
            connections[i].player_id = i;
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
void wait_for_clients(TCPsocket server_socket, Player players[], ClientConnection connections[])
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
                network_accept_client(server_socket, players, connections);

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
// パケット送信（Packet構造体専用）
// -----------------------------------------------------
int network_send_packet(TCPsocket client_socket, const Packet *packet)
{
    if (!packet)
    {
        LOG_ERROR("パケットがnullptrです");
        return -1;
    }

    if (!client_socket)
    {
        LOG_ERROR("ソケットがnullptrです");
        return -1;
    }

    // Packet構造体全体を送信
    int total_sent = 0;
    int packet_size = sizeof(Packet);
    const uint8_t* data = (const uint8_t*)packet;

    while (total_sent < packet_size)
    {
        int sent = SDLNet_TCP_Send(client_socket, data + total_sent, packet_size - total_sent);

        if (sent <= 0)
        {
            LOG_ERROR("送信失敗: " << SDLNet_GetError()
                     << " (" << total_sent << " / " << packet_size << " バイト送信済み)");
            return sent;
        }

        total_sent += sent;
        LOG_DEBUG("送信: " << sent << " バイト (合計: " << total_sent << " / " << packet_size << ")");
    }

    LOG_DEBUG("パケット送信完了: type=" << packet->type << ", size=" << packet->size
             << " (" << total_sent << " バイト)");

    return total_sent;
}

// -----------------------------------------------------
// データ受信（確実に全データを受信）
// -----------------------------------------------------
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    int total_received = 0;
    uint8_t* buf = (uint8_t*)buffer;

    while (total_received < size)
    {
        int received = SDLNet_TCP_Recv(client_socket, buf + total_received, size - total_received);

        if (received <= 0)
        {
            if (total_received > 0)
            {
                LOG_WARN("部分的な受信: " << total_received << " / " << size << " バイト");
            }
            return received;  // エラーまたは切断
        }

        total_received += received;

        LOG_DEBUG("受信: " << received << " バイト (合計: " << total_received << " / " << size << ")");
    }

    return total_received;
}

// -----------------------------------------------------
// パケット受信（Packet構造体専用）
// -----------------------------------------------------
int network_receive_packet(TCPsocket client_socket, Packet *packet)
{
    if (!packet)
    {
        LOG_ERROR("パケットがnullptrです");
        return -1;
    }

    if (!client_socket)
    {
        LOG_ERROR("ソケットがnullptrです");
        return -1;
    }

    // Packet構造体をゼロクリア
    memset(packet, 0, sizeof(Packet));

    // Packet構造体全体を受信
    int received_size = network_receive(client_socket, packet, sizeof(Packet));

    if (received_size <= 0)
    {
        LOG_DEBUG("パケット受信失敗または切断 (received=" << received_size << ")");
        return received_size;
    }

    // パケットサイズが不完全
    if (received_size < (int)sizeof(Packet))
    {
        LOG_WARN("不完全なパケット受信: " << received_size << " / " << sizeof(Packet) << " バイト");
        return -1;
    }

    LOG_DEBUG("パケット受信完了: type=" << packet->type << ", size=" << packet->size);

    // パケット内容の検証
    if (packet->type > 100)  // PacketTypeの最大値を超えている
    {
        LOG_WARN("不正なパケットタイプ: " << packet->type);

        #ifdef DEBUG
        std::cerr << "パケットヘッダーの16進数: ";
        uint8_t* raw = (uint8_t*)packet;
        for (int j = 0; j < 16; j++) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02x ", raw[j]);
            std::cerr << buf;
        }
        std::cerr << std::endl;
        #endif

        return -1;
    }

    // データサイズ検証
    if (packet->size > PACKET_MAX_SIZE)
    {
        LOG_WARN("不正なパケットサイズ: " << packet->size
                << " (最大: " << PACKET_MAX_SIZE << " バイト, type=" << packet->type << ")");

        #ifdef DEBUG
        std::cerr << "パケット内容(最初の16バイト): ";
        uint8_t* raw = (uint8_t*)packet;
        for (int j = 0; j < 16; j++) {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02x ", raw[j]);
            std::cerr << buf;
        }
        std::cerr << std::endl;
        #endif

        return -1;
    }

    return received_size;
}

// -----------------------------------------------------
// クライアント切断
// -----------------------------------------------------
void network_close_client(Player *player, ClientConnection *connection)
{
    if (player->connected)
    {
        if (connection->socket)
        {
            SDLNet_TCP_Close(connection->socket);
            connection->socket = nullptr;
        }
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

void network_broadcast(Player players[], ClientConnection connections[], const Packet *packet)
{
    if (!packet)
    {
        LOG_ERROR("パケットがnullptrです");
        return;
    }

    // パケットサイズの検証
    if (packet->size > PACKET_MAX_SIZE)
    {
        LOG_ERROR("パケットデータが大きすぎます: " << packet->size << " バイト (最大: " << PACKET_MAX_SIZE << ")");
        return;
    }

    LOG_DEBUG("ブロードキャスト: type=" << packet->type << ", size=" << packet->size);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected && connections[i].socket)
        {
            if (network_send_packet(connections[i].socket, packet) < (int)sizeof(Packet))
            {
                LOG_ERROR("クライアント " << i << " への送信失敗");
            }
        }
    }
}