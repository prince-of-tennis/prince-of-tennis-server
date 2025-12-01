#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL_net.h>
#include <iostream>

#define MAX_CLIENTS 4
#define REQUIRED_CLIENTS 2
#define SERVER_PORT 5000  // ← これを追加！！

struct Client
{
    TCPsocket socket = nullptr;
    bool connected = false;
};

// サーバー側
TCPsocket network_init_server(int port);
TCPsocket network_accept_client(TCPsocket server_socket, Client clients[]);
void wait_for_clients(TCPsocket server_socket, Client clients[]);
void network_shutdown_server(TCPsocket server_socket);

// 通信
int network_send(TCPsocket client_socket, const void *data, int size);
int network_receive(TCPsocket client_socket, void *buffer, int size);

// クライアント管理
void network_close_client(Client *client);

#endif
