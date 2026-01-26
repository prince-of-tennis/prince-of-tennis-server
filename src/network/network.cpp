#include "network.h"
#include "../log.h"
#include "../server_constants.h"

#include <SDL2/SDL_net.h>
#include <string.h>

#include "common/player_id.h"
#include "common/ball.h"
#include "common/GameScore.h"
#include "common/GamePhase.h"

extern volatile int g_running;

TCPsocket network_init_server(int port)
{
    if (SDLNet_Init() < 0)
    {
        LOG_ERROR("SDLNet初期化失敗: " << SDLNet_GetError());
        return nullptr;
    }

    IPaddress ip;
    if (SDLNet_ResolveHost(&ip, nullptr, port) < 0)
    {
        LOG_ERROR("ホスト解決失敗: " << SDLNet_GetError());
        return nullptr;
    }

    TCPsocket server = SDLNet_TCP_Open(&ip);
    if (!server)
    {
        LOG_ERROR("サーバーソケット作成失敗: " << SDLNet_GetError());
        return nullptr;
    }

    LOG_SUCCESS("サーバー起動: ポート " << port);
    return server;
}

TCPsocket network_accept_client(TCPsocket server_socket, Player players[], ClientConnection connections[])
{
    TCPsocket client = SDLNet_TCP_Accept(server_socket);
    if (!client)
        return nullptr;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (!players[i].connected)
        {
            connections[i].socket = client;
            connections[i].player_id = i;
            players[i].connected = true;
            players[i].player_id = i;
            LOG_SUCCESS("クライアント接続 (スロット " << i << ")");
            return client;
        }
    }

    LOG_WARN("サーバー満員");
    SDLNet_TCP_Close(client);
    return nullptr;
}

void wait_for_clients(TCPsocket server_socket, Player players[], ClientConnection connections[])
{
    SDLNet_SocketSet socketSet = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!socketSet)
    {
        LOG_ERROR("ソケットセット作成失敗: " << SDLNet_GetError());
        return;
    }

    SDLNet_TCP_AddSocket(socketSet, server_socket);

    while (g_running)
    {
        int numReady = SDLNet_CheckSockets(socketSet, 100);
        if (numReady > 0 && SDLNet_SocketReady(server_socket))
        {
            network_accept_client(server_socket, players, connections);

            int connected_count = count_connected_clients(players);
            if (connected_count >= REQUIRED_CLIENTS)
            {
                LOG_SUCCESS("全員接続完了");
                break;
            }
        }
    }

    SDLNet_FreeSocketSet(socketSet);
}

static bool validate_network_params(const void *packet, TCPsocket client_socket)
{
    if (!packet || !client_socket)
    {
        LOG_ERROR("無効なネットワークパラメータ");
        return false;
    }
    return true;
}

int network_send_packet(TCPsocket client_socket, const Packet *packet)
{
    if (!validate_network_params(packet, client_socket))
        return -1;

    if (SDLNet_TCP_Send(client_socket, packet, sizeof(Packet)) < sizeof(Packet))
    {
        LOG_ERROR("送信失敗: " << SDLNet_GetError());
        return 0;
    }
    return sizeof(Packet);
}

int network_receive(TCPsocket client_socket, void *buffer, int size)
{
    int total_received = 0;
    uint8_t* buf = (uint8_t*)buffer;
    int attempts = 0;

    while (total_received < size && attempts < NETWORK_RECEIVE_MAX_ATTEMPTS)
    {
        int received = SDLNet_TCP_Recv(client_socket, buf + total_received, size - total_received);
        if (received <= 0)
            return received;

        total_received += received;
        attempts++;

        if (total_received < size)
            SDL_Delay(1);
    }

    if (attempts >= NETWORK_RECEIVE_MAX_ATTEMPTS)
    {
        LOG_ERROR("受信タイムアウト");
        return -1;
    }

    return total_received;
}

int network_receive_packet(TCPsocket client_socket, Packet *packet)
{
    if (!validate_network_params(packet, client_socket))
        return -1;

    memset(packet, 0, sizeof(Packet));
    int received_size = network_receive(client_socket, packet, sizeof(Packet));

    if (received_size <= 0)
        return received_size;

    if (received_size < (int)sizeof(Packet))
    {
        LOG_WARN("不完全なパケット: " << received_size << "/" << sizeof(Packet));
        return -1;
    }

    if (packet->type >= PACKET_TYPE_MAX)
    {
        LOG_WARN("不正なパケットタイプ: " << (int)packet->type);
        return -1;
    }

    if (packet->size > PACKET_MAX_SIZE)
    {
        LOG_WARN("不正なパケットサイズ: " << packet->size);
        return -1;
    }

    return received_size;
}

void network_close_client(Player *player, ClientConnection *connection)
{
    if (player->connected)
    {
        if (connection->socket)
        {
            SDLNet_TCP_Close(connection->socket);
            connection->socket = nullptr;
        }
        player->connected = false;
        LOG_WARN("クライアント切断: ID=" << player->player_id);
    }
}

void network_shutdown_server(TCPsocket server_socket)
{
    SDLNet_TCP_Close(server_socket);
    SDLNet_Quit();
    LOG_INFO("サーバー終了");
}

void network_broadcast(Player players[], ClientConnection connections[], const Packet *packet)
{
    if (!packet || packet->size > PACKET_MAX_SIZE)
    {
        LOG_ERROR("ブロードキャスト: 無効なパケット");
        return;
    }

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected && connections[i].socket)
            network_send_packet(connections[i].socket, packet);
    }
}

static Packet create_packet_with_data(PacketType type, const void *data, size_t data_size)
{
    Packet packet;
    memset(&packet, 0, sizeof(Packet));
    packet.type = type;
    packet.size = data_size;
    if (data && data_size > 0)
        memcpy(packet.data, data, data_size);
    return packet;
}

Packet create_packet_player_id(int player_id)
{
    PlayerId id = { player_id };
    return create_packet_with_data(PACKET_TYPE_SET_PLAYER_ID, &id, sizeof(PlayerId));
}

Packet create_packet_player_state(const Player *player)
{
    return create_packet_with_data(PACKET_TYPE_PLAYER_STATE, player, sizeof(Player));
}

Packet create_packet_ball_state(const Ball *ball)
{
    return create_packet_with_data(PACKET_TYPE_BALL_STATE, ball, sizeof(Ball));
}

Packet create_packet_score(const GameScore *score)
{
    return create_packet_with_data(PACKET_TYPE_SCORE_UPDATE, score, sizeof(GameScore));
}

Packet create_packet_phase(GamePhase phase)
{
    return create_packet_with_data(PACKET_TYPE_GAME_PHASE, &phase, sizeof(GamePhase));
}

Packet create_packet_ability_state(const AbilityState *state)
{
    return create_packet_with_data(PACKET_TYPE_ABILITY_STATE, state, sizeof(AbilityState));
}

Packet create_packet_match_result(int winner_id)
{
    return create_packet_with_data(PACKET_TYPE_MATCH_RESULT, &winner_id, sizeof(int));
}

int count_connected_clients(const Player players[])
{
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected)
            count++;
    }
    return count;
}
