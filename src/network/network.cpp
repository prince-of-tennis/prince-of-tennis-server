#include "network.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define REQUIRED_CLIENTS 2   // ← 必要なクライアント人数を設定

// サーバー初期化
TCPsocket network_init_server(int port)
{
    if (SDLNet_Init() < 0)
    {
        printf("SDLNet_Init error: %s\n", SDLNet_GetError());
        return NULL;
    }

    IPaddress ip;

    if (SDLNet_ResolveHost(&ip, NULL, port) < 0)
    {
        printf("SDLNet_ResolveHost error: %s\n", SDLNet_GetError());
        return NULL;
    }

    TCPsocket server_socket = SDLNet_TCP_Open(&ip);
    if (!server_socket)
    {
        printf("SDLNet_TCP_Open error: %s\n", SDLNet_GetError());
        return NULL;
    }

    printf("[SERVER] Listening on port %d...\n", port);
    return server_socket;
}

// クライアント接続受付
TCPsocket network_accept_client(TCPsocket server_socket, Client clients[])
{
    TCPsocket client_socket = SDLNet_TCP_Accept(server_socket);

    if (!client_socket)
    {
        return NULL;
    }

    // 空いてるスロットに登録
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].connected)
        {
            clients[i].socket = client_socket;
            clients[i].connected = true;

            printf("[SERVER] Client connected (Slot %d)\n", i);

            return client_socket;
        }
    }

    printf("Server full, rejecting client.\n");
    SDLNet_TCP_Close(client_socket);
    return NULL;
}

// ここから追加 ------------------------------

// 必要人数揃うまで待つ
void wait_for_clients(TCPsocket server_socket, Client clients[])
{
    printf("[SERVER] 他のクラインアントの接続を待っています。\n", REQUIRED_CLIENTS);

    while (1)
    {
        int connected_count = 0;

        // 接続受付
        network_accept_client(server_socket, clients);

        // 現在の接続数を数える
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i].connected)
                connected_count++;
        }

        printf("[SERVER] Connected clients: %d / %d\n",
               connected_count, REQUIRED_CLIENTS);

        // 必要人数に達したら抜ける
        if (connected_count >= REQUIRED_CLIENTS)
        {
            printf("[SERVER] All clients connected! Starting game...\n");
            break;
        }
    }
}


// データ送信
int network_send(TCPsocket client_socket, const void *data, int size)
{
    return SDLNet_TCP_Send(client_socket, data, size);
}

// データ受信
int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    return SDLNet_TCP_Recv(client_socket, buffer, size);
}

// クライアント切断
void network_close_client(Client *client)
{
    if (client->connected)
    {
        SDLNet_TCP_Close(client->socket);
        client->connected = false;
        client->socket = NULL;

        printf("[SERVER] Client disconnected.\n");
    }
}

// サーバー終了
void network_shutdown_server(TCPsocket server_socket)
{
    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
}
