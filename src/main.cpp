#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "network/network.h"
#include "game/game_state.h"
#include "physics/ball_physics.h"
#include "common/packet.h"
#include "game/game_phase_manager.h"

Client clients[MAX_CLIENTS] = {0};

int main(int argc, char *argv[])
{
    // --- サーバー起動 ---
    TCPsocket server_socket = network_init_server(SERVER_PORT);
    if (!server_socket)
    {
        printf("Server start failed.\n");
        return 1;
    }

    GameState state;
    init_game(&state);
    init_phase_manager(&state);

    const float dt = 0.016f; // 60FPS

    // --- ここで必要人数が集まるまで待つ ---
    wait_for_clients(server_socket, clients);

    update_game_phase(&state, GAME_PHASE_MATCH_COMPLETE);

    // --- ソケットセット作成 ---
    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!socket_set)
    {
        printf("SDLNet_AllocSocketSet error: %s\n", SDLNet_GetError());
        return 1;
    }

    // サーバーソケットをセットに追加
    SDLNet_TCP_AddSocket(socket_set, server_socket);

    printf("[SERVER] Game started!\n");

    // 前回のフェーズを記憶する変数（変更検知用）
    GamePhase last_sent_phase = (GamePhase)-1;

    while (1)
    {
        int ready_count = SDLNet_CheckSockets(socket_set, 0);

        if (ready_count < 0)
        {
            printf("SDLNet_CheckSockets error: %s\n", SDLNet_GetError());
            break;
        }

        if (ready_count > 0)
        {
            if (SDLNet_SocketReady(server_socket))
            {
                TCPsocket new_client_socket = network_accept_client(server_socket, clients);
                if (new_client_socket)
                {
                    SDLNet_TCP_AddSocket(socket_set, new_client_socket);
                }
            }

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].connected && SDLNet_SocketReady(clients[i].socket))
                {
                    char buffer[256] = {0};
                    int size = network_receive(clients[i].socket, buffer, sizeof(buffer));

                    if (size <= 0)
                    {
                        SDLNet_TCP_DelSocket(socket_set, clients[i].socket);
                        network_close_client(&clients[i]);
                        continue;
                    }

                    printf("[SERVER] Client %d says: %s\n", i, buffer);
                }
            }
        }

        // --- 物理更新 ---
        update_ball(&state.ball, dt);
        handle_bounce(&state.ball, 0.0f, 0.7f);

        // --- ボール座標の送信処理---
        network_broadcast(clients, PACKET_TYPE_BALL_STATE, &state.ball.point, sizeof(Point3d));

        // --- ゲームフェーズ更新 ---
        update_phase(&state, dt);
        if (state.phase != last_sent_phase)
        {
            printf("[SERVER] Phase Changed: %d -> %d\n", last_sent_phase, state.phase);

            // フェーズ変更パケットを全員に送信
            network_broadcast(clients,
                              PACKET_TYPE_GAME_PHASE,
                              &state.phase,
                              sizeof(GamePhase));

            last_sent_phase = state.phase;
        }

        // --- デバッグログ ---
        printf("Ball: (%.2f, %.2f, %.2f)\n",
               state.ball.point.x,
               state.ball.point.y,
               state.ball.point.z);

        SDL_Delay(16); // 60fps
    }

    SDLNet_FreeSocketSet(socket_set);
    network_shutdown_server(server_socket);
    return 0;
}
