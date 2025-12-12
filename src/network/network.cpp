#include "network.h"

#include <SDL2/SDL_net.h>
#include <iostream>
#include <cstring>

using namespace std;

// -----------------------------------------------------
// サーバー初期化
// -----------------------------------------------------
TCPsocket network_init_server(int port)
{
    if (SDLNet_Init() < 0)
    {
        cout << "SDLNet_Init error: " << SDLNet_GetError() << endl;
        return nullptr;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, port) < 0)
    {
        cout << "SDLNet_ResolveHost error: " << SDLNet_GetError() << endl;
        return nullptr;
    }

    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server)
    {
        cout << "SDLNet_TCP_Open error: " << SDLNet_GetError() << endl;
        return nullptr;
    }

    cout << "[SERVER] Listening on port " << port << " ..." << endl;

    return server;
}

// -----------------------------------------------------
// クライアント受け付け
// -----------------------------------------------------
TCPsocket network_accept_client(TCPsocket server_socket, Client clients[])
{
    TCPsocket client = SDLNet_TCP_Accept(server_socket);
    if (!client)
        return nullptr;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].connected)
        {
            clients[i].socket = client;
            clients[i].connected = true;

            cout << "[SERVER] Client connected (slot " << i << ")" << endl;

            return client;
        }
    }

    cout << "[SERVER] Server full, rejecting client." << endl;
    SDLNet_TCP_Close(client);
    return nullptr;
}

// -----------------------------------------------------
// 必要人数揃うまで完全にブロッキングして待つ
// -----------------------------------------------------
void wait_for_clients(TCPsocket server_socket, Client clients[])
{
    cout << "[SERVER] Waiting for other clients..." << endl;

    // サーバーソケットを監視するセット
    SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    SDLNet_TCP_AddSocket(socketSet, server_socket);

    while (true)
    {
        // クライアント接続があるまで無限に待つ
        int numReady = SDLNet_CheckSockets(socketSet, -1); // -1 = 無限待ち

        if (numReady > 0)
        {
            network_accept_client(server_socket, clients);
        }

        // 現在の接続数をカウント
        int connected_count = 0;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].connected)
                connected_count++;
        }

        cout << "[SERVER] Connected clients: "
             << connected_count << " / " << REQUIRED_CLIENTS << endl;

        // 必要人数が揃ったら抜ける
        if (connected_count >= REQUIRED_CLIENTS)
        {
            cout << "[SERVER] 全員接続完了！ゲーム開始！" << endl;
            break;
        }
    }

    SDLNet_FreeSocketSet(socketSet);
}

// -----------------------------------------------------
// データ送信
// -----------------------------------------------------
int network_send(TCPsocket client_socket, const void *data, int size)
{
    return SDLNet_TCP_Send(client_socket, data, size);
}

// -----------------------------------------------------
// データ受信
// -----------------------------------------------------
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    return SDLNet_TCP_Recv(client_socket, buffer, size);
}

// -----------------------------------------------------
// クライアント切断
// -----------------------------------------------------
void network_close_client(Client *client)
{
    if (client->connected)
    {
        SDLNet_TCP_Close(client->socket);

        client->socket = nullptr;
        client->connected = false;

        cout << "[SERVER] Client disconnected." << endl;
    }
}

// -----------------------------------------------------
// サーバー終了
// -----------------------------------------------------
void network_shutdown_server(TCPsocket server_socket)
{
    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
}

// =====================================================
// 汎用ブロードキャスト関数の実装
// =====================================================

void network_broadcast(Client clients[], PacketType type, const void *data, size_t data_size)
{
    Packet packet;
    memset(&packet, 0, sizeof(Packet));

    packet.type = type;
    packet.size = data_size;

    if (data != nullptr && data_size > 0)
    {
        if (data_size > sizeof(packet.data))
        {
            cout << "[SERVER] Error: Data too large for packet!" << endl;
            return;
        }
        memcpy(packet.data, data, data_size);
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connected)
        {
            if (network_send(clients[i].socket, &packet, sizeof(Packet)) < (int)sizeof(Packet))
            {
                cout << "[SERVER] Send error to client " << i << ": " << SDLNet_GetError() << endl;
            }
        }
    }
}