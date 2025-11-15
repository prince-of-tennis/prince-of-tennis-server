#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>

// サーバー設定
#define MAX_CLIENTS 16
#define SERVER_PORT 5000

// クライアント情報
typedef struct
{
    int socket;
    bool connected;
} Client;

// 公開API
int network_init_server(int port);
int network_accept_client(int server_socket, Client clients[]);
int network_send(int client_socket, const void *data, int size);
int network_receive(int client_socket, void *buffer, int size);
void network_close_client(Client *client);
void network_shutdown_server(int server_socket);

#endif
