#include "network.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

// サーバー初期化
TCPsocket network_init_server(int port)
{
    // SDL2_net初期化
    if (SDLNet_Init() < 0)
    {
        printf("SDLNet_Init error: %s\n", SDLNet_GetError());
        return NULL;
    }

    IPaddress ip;

    // サーバーのアドレスとポートを設定
    if (SDLNet_ResolveHost(&ip, NULL, port) < 0)
    {
        printf("SDLNet_ResolveHost error: %s\n", SDLNet_GetError());
        return NULL;
    }

    // サーバーソケットを開く
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
    // クライアントの接続を受け付ける
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

    // 空きがなければ拒否
    printf("Server full, rejecting client.\n");
    SDLNet_TCP_Close(client_socket);
    return NULL;
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
