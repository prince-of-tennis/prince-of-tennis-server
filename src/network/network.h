#ifndef NETWORK_H
#define NETWORK_H

#include <SDL2/SDL_net.h>
#include "common/packet.h"
#include "common/player.h"
#include "common/util/point_3d.h"

#define MAX_CLIENTS 2
#define REQUIRED_CLIENTS 2
#define SERVER_PORT 5000

#define DEBUG

// サーバー専用のソケット管理構造体
struct ClientConnection
{
    TCPsocket socket;
    int player_id;  // playersインデックスと対応
};

// サーバー専用パケット定義
// スコアパケット（ネットワーク送信用）
struct ScorePacket
{
    int current_game_p1;    // 0, 15, 30, 40
    int current_game_p2;
    int games_p1;           // 現在のセットの取得ゲーム数
    int games_p2;
    int sets_p1;            // 取得セット数
    int sets_p2;
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

#endif
