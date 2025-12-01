#include <stdio.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "network/network.h"
#include "game/game_state.h"
#include "physics/ball_physics.h"

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

    const float dt = 0.016f; // 60FPS

    // --- ここで必要人数揃うまで待つ ---
    wait_for_clients(server_socket, clients);

    // --- ソケットセットの作成 ---
    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!socket_set)
    {
        printf("SDLNet_AllocSocketSet error: %s\n", SDLNet_GetError());
        return 1;
    }

    // サーバーソケットをセットに追加
    SDLNet_TCP_AddSocket(socket_set, server_socket);

    printf("[SERVER] Game started!\n");

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
