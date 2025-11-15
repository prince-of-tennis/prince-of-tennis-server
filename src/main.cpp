#include <stdio.h>
#include <unistd.h>

#include "network/network.h"
#include "game/game_state.h"
#include "physics/ball_physics.h"

Client clients[MAX_CLIENTS] = {0};

int main()
{
    // --- サーバー起動 ---
    int server_socket = network_init_server(SERVER_PORT);
    if (server_socket < 0)
    {
        printf("Server start failed.\n");
        return 1;
    }

    GameState state;
    init_game(&state);

    const float dt = 0.016f; // 60FPS

    while (1)
    {
        // --- 新規クライアント受付 ---
        network_accept_client(server_socket, clients);

        // --- 全クライアントからの受信処理 ---
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i].connected)
            {
                continue;
            }

            char buffer[256] = {0};
            int size = network_receive(clients[i].socket, buffer, sizeof(buffer));

            if (size <= 0)
            {
                network_close_client(&clients[i]);
                continue;
            }

            printf("[SERVER] Client %d says: %s\n", i, buffer);

            // 例: 受信した内容をそのまま送り返す
            network_send(clients[i].socket, buffer, size);
        }

        // --- 物理更新 ---
        update_ball(&state.ball, dt);
        handle_bounce(&state.ball, 0.0f, 0.7f);

        // --- デバッグログ ---
        printf("Ball: (%.2f, %.2f, %.2f)\n",
            state.ball.position.x,
            state.ball.position.y,
            state.ball.position.z);

        usleep(16000);
    }

    network_shutdown_server(server_socket);
    return 0;
}
