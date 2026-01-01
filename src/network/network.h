#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL_net.h>
#include "common/packet.h"
#include "common/player.h"
#include "common/util/point_3d.h"
#include "common/ball.h"
#include "common/GameScore.h"
#include "common/GamePhase.h"

#define MAX_CLIENTS 2
#define REQUIRED_CLIENTS 2
#define SERVER_PORT 5000

// サーバー専用のソケット管理構造体
struct ClientConnection
{
    TCPsocket socket;
    int player_id;  // playersインデックスと対応
};


// サーバー初期化関連
TCPsocket network_init_server(int port);
TCPsocket network_accept_client(TCPsocket server_socket, Player players[], ClientConnection connections[]);
void wait_for_clients(TCPsocket server_socket, Player players[], ClientConnection connections[]);
void network_shutdown_server(TCPsocket server_socket);

// 通信（送受信)
int network_send_packet(TCPsocket client_socket, const Packet *packet);
int network_receive(TCPsocket client_socket, void *buffer, int size);
int network_receive_packet(TCPsocket client_socket, Packet *packet);

// クライアント管理
void network_close_client(Player *player, ClientConnection *connection);

// ブロードキャスト
void network_broadcast(Player players[], ClientConnection connections[], const Packet *packet);

// パケット生成ヘルパー関数
Packet create_packet_player_id(int player_id);
Packet create_packet_player_state(const Player *player);
Packet create_packet_ball_state(const Ball *ball);
Packet create_packet_score(const GameScore *score);
Packet create_packet_phase(GamePhase phase);

// ユーティリティ関数
int count_connected_clients(const Player players[]);

#endif
