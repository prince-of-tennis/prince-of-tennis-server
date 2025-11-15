#include "network.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>

// サーバー初期化
int network_init_server(int port)
{
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("socket error");
        return -1;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("listen error");
        close(server_socket);
        return -1;
    }

    printf("[SERVER] Listening on port %d...\n", port);

    return server_socket;
}

// クライアント接続受付
int network_accept_client(int server_socket, Client clients[])
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
    if (client_socket < 0)
    {
        return -1; // No new client
    }

    // 空いてるスロットに登録
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!clients[i].connected)
        {
            clients[i].socket = client_socket;
            clients[i].connected = true;

            printf("[SERVER] Client connected: socket=%d\n", client_socket);

            return i; // 登録したクライアントID
        }
    }

    // 空きがなければ拒否
    printf("Server full, rejecting client.\n");
    close(client_socket);

    return -1;
}

// データ送信
int network_send(int client_socket, const void *data, int size)
{
    return send(client_socket, data, size, 0);
}

// データ受信
int network_receive(int client_socket, void *buffer, int size)
{
    return recv(client_socket, buffer, size, 0);
}

// クライアント切断
void network_close_client(Client *client)
{
    if (client->connected)
    {
        close(client->socket);
        client->connected = false;

        printf("[SERVER] Client disconnected.\n");
    }
}

// サーバー終了
void network_shutdown_server(int server_socket)
{
    close(server_socket);
}
