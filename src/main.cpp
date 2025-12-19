#include <cstdio>
#include <cstring>
#include <csignal>
#include <unistd.h> // 追加: write() 用
#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "log.h"
#include "network/network.h"
#include "game/game_state.h"
#include "physics/ball_physics.h"
#include "common/packet.h"
#include "common/player.h"
#include "common/player_input.h"
#include "input_handler/input_handler.h"
#include "game/game_phase_manager.h"
#include "physics/court_check.h"
#include "game/score_logic.h"

// グローバル変数: Ctrl+C対応
volatile sig_atomic_t g_running = 1;

// シグナルハンドラー
void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        // 非同期シグナル安全な処理のみ行う（LOG_* は使用しない）
        const char msg[] = "\nCtrl+C検出: サーバーを終了します...\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        g_running = 0;
    }
}

Player players[MAX_CLIENTS] = {0};

int main(int argc, char *argv[])
{
    // シグナルハンドラーを設定
    signal(SIGINT, signal_handler);

    // サーバー起動
    TCPsocket server_socket = network_init_server(SERVER_PORT);
    if (!server_socket)
    {
        LOG_ERROR("サーバー起動失敗");
        return 1;
    }

    GameState state;
    init_game(&state);
    init_phase_manager(&state);

    const float dt = 0.016f; // 60FPS

    // 必要人数が集まるまで待つ
    // ↓ ブロッキングな wait_for_clients をやめ、タイムアウト付きループで待つ（Ctrl+C に即応答）
    {
        SDLNet_SocketSet wait_set = SDLNet_AllocSocketSet(1);
        if (!wait_set)
        {
            LOG_ERROR("待機用ソケットセット作成失敗: " << SDLNet_GetError());
            network_shutdown_server(server_socket);
            return 1;
        }
        SDLNet_TCP_AddSocket(wait_set, server_socket);

        // 既に接続済みの人数をカウント
        int connected_count = 0;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (players[i].connected)
                connected_count++;
        }

        LOG_INFO("クライアント待機開始: 必要人数=" << MAX_CLIENTS);

        while (connected_count < MAX_CLIENTS && g_running)
        {
            int ready = SDLNet_CheckSockets(wait_set, 500); // 500ms タイムアウト
            if (ready < 0)
            {
                LOG_ERROR("ソケットチェック失敗 (待機中): " << SDLNet_GetError());
                break;
            }

            if (ready > 0 && SDLNet_SocketReady(server_socket))
            {
                TCPsocket new_client_socket = network_accept_client(server_socket, players);
                if (new_client_socket)
                {
                    // 新しく接続されたクライアントは network_accept_client 内で players に設定される想定
                    LOG_INFO("新しいクライアントを受け付けました");
                }
            }

            // 接続数を再カウント（network_accept_client が players を更新するため）
            connected_count = 0;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (players[i].connected)
                    connected_count++;
            }
        }

        SDLNet_FreeSocketSet(wait_set);
    }

    // Ctrl+Cが押された場合は終了
    if (!g_running)
    {
        LOG_INFO("クライアント待機中に中断されました");
        network_shutdown_server(server_socket);
        return 0;
    }

    update_game_phase(&state, GAME_PHASE_MATCH_COMPLETE);

    // ソケットセット作成
    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(MAX_CLIENTS + 1);
    if (!socket_set)
    {
        LOG_ERROR("ソケットセット作成失敗: " << SDLNet_GetError());
        return 1;
    }

    // サーバーソケットをセットに追加
    SDLNet_TCP_AddSocket(socket_set, server_socket);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected)
        {
            SDLNet_TCP_AddSocket(socket_set, players[i].socket);
        }
    }

    LOG_SUCCESS("ゲーム開始！");

    // 前回のフェーズを記憶する変数（変更検知用）
    GamePhase last_sent_phase = (GamePhase)-1;
    // 前回のスコアを記憶する変数（変更検知用）
    GameScore last_sent_score = state.score;

    while (g_running != 0)
    {
        int ready_count = SDLNet_CheckSockets(socket_set, 0);

        if (ready_count < 0)
        {
            LOG_ERROR("ソケットチェック失敗: " << SDLNet_GetError());
            break;
        }

        if (ready_count > 0)
        {
            // 新しいクライアントの接続チェック
            if (SDLNet_SocketReady(server_socket))
            {
                TCPsocket new_client_socket = network_accept_client(server_socket, players);
                if (new_client_socket)
                {
                    SDLNet_TCP_AddSocket(socket_set, new_client_socket);
                }
            }

            // 各クライアントからのデータ受信
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (players[i].connected && SDLNet_SocketReady(players[i].socket))
                {
                    Packet packet;
                    memset(&packet, 0, sizeof(Packet));
                    int size = network_receive(players[i].socket, &packet, sizeof(Packet));

                    if (size <= 0)
                    {
                        SDLNet_TCP_DelSocket(socket_set, players[i].socket);
                        network_close_client(&players[i]);
                        continue;
                    }

                    // パケットサイズ検証
                    if (packet.size > PACKET_MAX_SIZE)
                    {
                        LOG_WARN("不正なパケットサイズ: " << packet.size);
                        continue;
                    }

                    // パケットの種類をチェック
                    if (packet.type == PACKET_TYPE_PLAYER_INPUT)
                    {
                        PlayerInput input;
                        memset(&input, 0, sizeof(PlayerInput));
                        if (packet.size == sizeof(PlayerInput))
                        {
                            memcpy(&input, packet.data, sizeof(PlayerInput));
                            apply_player_input(&state, i, input, dt);
                        }
                    }
                }
            }
        }

        // 物理更新
        update_ball(&state.ball, dt);

        // バウンド処理
        if (handle_bounce(&state.ball, 0.0f, 0.7f))
        {
            state.ball.bounce_count++;
            LOG_DEBUG("ボールバウンド! 回数=" << state.ball.bounce_count);

            // ラリー中のみ判定を行う
            if (state.phase == GAME_PHASE_IN_RALLY)
            {
                bool is_in = is_in_court(state.ball.point); // コート判定
                int winner_id = -1;

                // ケース1: 1回目のバウンドでアウトだった場合 -> 打った人のミス
                if (state.ball.bounce_count == 1 && !is_in)
                {
                    // 最後に打ったのが 0 なら 1 の勝ち、1 なら 0 の勝ち
                    winner_id = (state.ball.last_hit_player_id == 0) ? 1 : 0;
                    LOG_INFO("判定: アウト! 勝者: P" << winner_id);
                }
                // ケース2: 2回目のバウンド（ツーバウンド） -> 打った人の得点
                else if (state.ball.bounce_count == 2)
                {
                    winner_id = state.ball.last_hit_player_id;
                    LOG_INFO("判定: ツーバウンド! 勝者: P" << winner_id);
                }

                // 勝者が決まったらフェーズ移行とスコア加算
                if (winner_id != -1)
                {
                    add_point(&state.score, winner_id);
                    update_game_phase(&state, GAME_PHASE_POINT_SCORED);

                    // スコア表示
                    print_score(&state.score);
                }
            }
        }

        // ボール座標の送信処理
        network_broadcast(players, PACKET_TYPE_BALL_STATE, &state.ball.point, sizeof(Point3d));

        // プレイヤー状態の送信
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            PlayerStatePacket p_packet;
            memset(&p_packet, 0, sizeof(PlayerStatePacket));
            p_packet.player_id = i;
            p_packet.position = state.players[i].point;
            strncpy(p_packet.name, state.players[i].name, sizeof(p_packet.name) - 1);

            // 全員にブロードキャスト
            network_broadcast(players,
                              PACKET_TYPE_PLAYER_STATE,
                              &p_packet,
                              sizeof(PlayerStatePacket));
        }

        // スコア送信
        if (memcmp(&state.score, &last_sent_score, sizeof(GameScore)) != 0)
        {
            LOG_DEBUG("スコア変更を検出、ブロードキャスト中...");

            ScorePacket s_packet;
            memset(&s_packet, 0, sizeof(ScorePacket));
            s_packet.current_game_p1 = state.score.current_game_p1;
            s_packet.current_game_p2 = state.score.current_game_p2;

            // 現在のセットのゲーム取得数
            int current_set = state.score.current_set;
            if (current_set >= MAX_SETS)
                current_set = MAX_SETS - 1;

            s_packet.games_p1 = state.score.games_in_set[current_set][0];
            s_packet.games_p2 = state.score.games_in_set[current_set][1];

            s_packet.sets_p1 = 0;
            s_packet.sets_p2 = 0;

            // 全員に送信
            network_broadcast(players,
                              PACKET_TYPE_SCORE_UPDATE,
                              &s_packet,
                              sizeof(ScorePacket));

            last_sent_score = state.score;
        }

        // ゲームフェーズ更新
        update_phase(&state, dt);
        if (state.phase != last_sent_phase)
        {
            LOG_INFO("フェーズ変更: " << last_sent_phase << " -> " << state.phase);

            // フェーズ変更パケットを全員に送信
            network_broadcast(players,
                              PACKET_TYPE_GAME_PHASE,
                              &state.phase,
                              sizeof(GamePhase));

            last_sent_phase = state.phase;
        }

        // デバッグログ
        LOG_DEBUG("ボール座標: (" << state.ball.point.x << ", "
                  << state.ball.point.y << ", " << state.ball.point.z << ")");

        SDL_Delay(16); // 60fps
    }

    LOG_INFO("メインループ終了");
    SDLNet_FreeSocketSet(socket_set);
    network_shutdown_server(server_socket);

    LOG_SUCCESS("サーバーを正常終了しました");
    return 0;
}
