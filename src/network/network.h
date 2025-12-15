#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL_net.h>
#include <iostream>
#include "common/packet.h"

#define MAX_CLIENTS 4
#define REQUIRED_CLIENTS 2
#define SERVER_PORT 5000
struct Client
{
    TCPsocket socket = nullptr;
    bool connected = false;
};

// サーバー初期化関連
TCPsocket network_init_server(int port);
TCPsocket network_accept_client(TCPsocket server_socket, Client clients[]);
void wait_for_clients(TCPsocket server_socket, Client clients[]);
void network_shutdown_server(TCPsocket server_socket);

//
int network_send(TCPsocket client_socket, const void *data, int size);
int network_receive(TCPsocket client_socket, void *buffer, int size);

// 通信（送受信)
void network_close_client(Client *client);

// クライアント管理
void network_broadcast(Client clients[], PacketType type, const void *data, size_t data_size);
#endif
