#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>
#include <SDL2/SDL_net.h>

// サーバー設定
#define MAX_CLIENTS 16
#define SERVER_PORT 5000

// クライアント情報
typedef struct
{
    TCPsocket socket;
    bool connected;
} Client;

// 公開API
TCPsocket network_init_server(int port);
TCPsocket network_accept_client(TCPsocket server_socket, Client clients[]);
int network_send(TCPsocket client_socket, const void *data, int size);
int network_receive(TCPsocket client_socket, void *buffer, int size);
void network_close_client(Client *client);
void network_shutdown_server(TCPsocket server_socket);

#endif
