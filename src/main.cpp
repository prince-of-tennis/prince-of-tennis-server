#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "network/network.h"
#include "game/game_state.h"
#include "physics/ball_physics.h"
#include "common/packet.h"
#include "common/player_input.h"
#include "input_handler/input_handler.h"
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

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].connected)
        {
            SDLNet_TCP_AddSocket(socket_set, clients[i].socket);
        }
    }

    printf("[SERVER] Game started!\n");

    // 前回のフェーズを記憶する変数（変更検知用）
    GamePhase last_sent_phase = (GamePhase)-1;
    // 前回のスコアを記憶する変数（変更検知用）
    GameScore last_sent_score = state.score;

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
                    Packet packet;
                    int size = network_receive(clients[i].socket, &packet, sizeof(Packet));

                    if (size <= 0)
                    {
                        SDLNet_TCP_DelSocket(socket_set, clients[i].socket);
                        network_close_client(&clients[i]);
                        continue;
                    }

                    // パケットの種類をチェック
                    if (packet.type == PACKET_TYPE_PLAYER_INPUT)
                    {
                        PlayerInput input;
                        if (packet.size == sizeof(PlayerInput))
                        {
                            memcpy(&input, packet.data, sizeof(PlayerInput));

                            apply_player_input(state.players[i], state.ball, input, dt);
                        }
                    }

                    printf("[SERVER] Client %d says: %d\n", i, packet.type);
                }
            }
        }

        // --- 物理更新 ---
        update_ball(&state.ball, dt);
        handle_bounce(&state.ball, 0.0f, 0.7f);

        // --- ボール座標の送信処理---
        network_broadcast(clients, PACKET_TYPE_BALL_STATE, &state.ball.point, sizeof(Point3d));

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            PlayerStatePacket p_packet;
            p_packet.player_id = i;                     // ID (0~3)
            p_packet.position = state.players[i].point; // 現在の座標

            // 全員にブロードキャスト
            network_broadcast(clients,
                              PACKET_TYPE_PLAYER_STATE,
                              &p_packet,
                              sizeof(PlayerStatePacket));
        }

        // --- スコア送信 ---
        if (memcmp(&state.score, &last_sent_score, sizeof(GameScore)) != 0)
        {
            printf("[SERVER] Score Changed! Broadcasting...\n");

            ScorePacket s_packet;
            // 現在のポイント (0, 15, 30, 40...)
            s_packet.current_game_p1 = state.score.current_game_p1;
            s_packet.current_game_p2 = state.score.current_game_p2;

            // 現在のセットのゲーム取得数
            int current_set = state.score.current_set;
            if (current_set >= MAX_SETS)
                current_set = MAX_SETS - 1;

            s_packet.games_p1 = state.score.games_in_set[current_set][0];
            s_packet.games_p2 = state.score.games_in_set[current_set][1];

            // セット取得数はまだ実装していない場合は 0 でOK
            s_packet.sets_p1 = 0;
            s_packet.sets_p2 = 0;

            // 全員に送信
            network_broadcast(clients,
                              PACKET_TYPE_SCORE_UPDATE,
                              &s_packet,
                              sizeof(ScorePacket));

            // 送信したので「前回のスコア」を更新
            last_sent_score = state.score;
        }

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
