#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL_net.h>
#include "common/packet.h"
#include "common/player.h"
#include "common/util/point_3d.h"

#define MAX_CLIENTS 4
#define REQUIRED_CLIENTS 2
#define SERVER_PORT 5000

// プレイヤー状態パケット（ClientとPlayerの責務を統合）
struct PlayerStatePacket
{
    int player_id;
    Point3d position;
    char name[32];
};

// スコアパケット（ClientとPlayerの責務を統合）
struct ScorePacket
{
    int current_game_p1;
    int current_game_p2;
    int games_p1;
    int games_p2;
    int sets_p1;
    int sets_p2;
};

// サーバー初期化関連
TCPsocket network_init_server(int port);
TCPsocket network_accept_client(TCPsocket server_socket, Player players[]);
void wait_for_clients(TCPsocket server_socket, Player players[]);
void network_shutdown_server(TCPsocket server_socket);

// 通信（送受信)
int network_send(TCPsocket client_socket, const void *data, int size);
int network_receive(TCPsocket client_socket, void *buffer, int size);

// クライアント管理
void network_close_client(Player *player);

// ブロードキャスト
void network_broadcast(Player players[], PacketType type, const void *data, size_t data_size);

#endif
