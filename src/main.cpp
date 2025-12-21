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
#include "common/game_constants.h"
#include "input_handler/input_handler.h"
#include "game/game_phase_manager.h"
#include "physics/court_check.h"
#include "game/score_logic.h"
#include "common/player_id.h"

#define DEBUG

using namespace GameConstants;

// グローバル変数: Ctrl+C対応
volatile int g_running = 1;

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
ClientConnection connections[MAX_CLIENTS] = {0};

int main(int argc, char *argv[])
{
    // シグナルハンドラーを設定
    signal(SIGINT, signal_handler);

    // デバッグ情報: 構造体サイズの確認
    LOG_DEBUG("=== 構造体サイズ情報 ===");
    LOG_DEBUG("sizeof(Packet) = " << sizeof(Packet) << " バイト");
    LOG_DEBUG("sizeof(Player) = " << sizeof(Player) << " バイト");
    LOG_DEBUG("sizeof(Point3d) = " << sizeof(Point3d) << " バイト");
    LOG_DEBUG("sizeof(PlayerInput) = " << sizeof(PlayerInput) << " バイト");
    LOG_DEBUG("PACKET_MAX_SIZE = " << PACKET_MAX_SIZE << " バイト");

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

    const float dt = FRAME_TIME; // 60FPS

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

        int player_id = 0;
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
                TCPsocket new_client_socket = network_accept_client(server_socket, players, connections);
                if (new_client_socket)
                {
                    // 新しく接続されたクライアントは network_accept_client 内で players に設定される想定
                    LOG_INFO("新しいクライアントを受け付けました");
                    PlayerId id = { player_id };
                    Packet packet;
                    packet.type = PACKET_TYPE_SET_PLAYER_ID;
                    memcpy(packet.data, &id, sizeof(PlayerId));
                    packet.size = sizeof(PlayerId);

                    network_send_packet(new_client_socket, &packet);

                    player_id++;
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

    // Playerのデータを送信
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (players[i].connected && connections[i].socket)
        {
            Packet player_packet;
            memset(&player_packet, 0, sizeof(Packet));
            player_packet.type = PACKET_TYPE_PLAYER_STATE;
            player_packet.size = sizeof(Player);
            memcpy(player_packet.data, &state.players[i], sizeof(Player));

            network_broadcast(players, connections, &player_packet);
        }
    }

    // Ctrl+Cが押された場合は終了
    if (!g_running)
    {
        LOG_INFO("クライアント待機中に中断されました");
        network_shutdown_server(server_socket);
        return 0;
    }

    update_game_phase(&state, GAME_PHASE_START_GAME);

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
        if (players[i].connected && connections[i].socket)
        {
            SDLNet_TCP_AddSocket(socket_set, connections[i].socket);
        }
    }

    LOG_SUCCESS("ゲーム開始！");

    // 前回のフェーズを記憶する変数（変更検知用）
    GamePhase last_sent_phase = (GamePhase)-1;
    // 前回のスコアを記憶する変数（変更検知用）
    GameScore last_sent_score;
    memset(&last_sent_score, 0xFF, sizeof(GameScore)); // 初期値を-1で埋めて、最初の送信を確実に行う

    // 初期スコア（0-0）を送信
    {
        LOG_INFO("初期スコアを送信: 0-0");
        Packet score_packet;
        memset(&score_packet, 0, sizeof(Packet));
        score_packet.type = PACKET_TYPE_SCORE_UPDATE;
        score_packet.size = sizeof(GameScore);
        memcpy(score_packet.data, &state.score, sizeof(GameScore));
        network_broadcast(players, connections, &score_packet);
        last_sent_score = state.score;
    }

    while (g_running != 0)
    {
        int ready_count = SDLNet_CheckSockets(socket_set, 10);  // 10msタイムアウト

        if (ready_count < 0)
        {
            LOG_ERROR("ソケットチェック失敗: " << SDLNet_GetError());
            break;
        }

        // ソケットが準備できている場合のみ通信処理
        if (ready_count > 0)
        {
            // 新しいクライアントの接続チェック
            if (SDLNet_SocketReady(server_socket))
            {
                TCPsocket new_client_socket = network_accept_client(server_socket, players, connections);
                if (new_client_socket)
                {
                    SDLNet_TCP_AddSocket(socket_set, new_client_socket);
                }
            }

            // 各クライアントからのデータ受信
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (players[i].connected && connections[i].socket && SDLNet_SocketReady(connections[i].socket))
                {
                    Packet packet;
                    int size = network_receive_packet(connections[i].socket, &packet);

                    if (size <= 0)
                    {
                        SDLNet_TCP_DelSocket(socket_set, connections[i].socket);
                        network_close_client(&players[i], &connections[i]);
                        continue;
                    }

                    // パケットの種類をチェック
                    PacketType pkt_type = (PacketType)packet.type;
                    if (pkt_type == PACKET_TYPE_PLAYER_INPUT)
                    {
                        PlayerInput input;
                        memset(&input, 0, sizeof(PlayerInput));
                        if (packet.size == sizeof(PlayerInput))
                        {
                            memcpy(&input, packet.data, sizeof(PlayerInput));
                            apply_player_input(&state, i, input, dt);

                            // いずれかの入力がtrueの場合、そのプレイヤーの状態を送信
                            if (input.right || input.left || input.front || input.back || input.swing)
                            {
                                Packet player_packet;
                                memset(&player_packet, 0, sizeof(Packet));
                                player_packet.type = PACKET_TYPE_PLAYER_STATE;
                                player_packet.size = sizeof(Player);
                                memcpy(player_packet.data, &state.players[i], sizeof(Player));

                                network_broadcast(players, connections, &player_packet);
                            }
                        }
                        else
                        {
                            LOG_WARN("PlayerInputのサイズが不一致: 受信=" << packet.size
                                    << ", 期待=" << sizeof(PlayerInput));
                        }
                    }
                }
            }
        }

        // フェーズ管理の更新（タイマーベースのフェーズ遷移）
        update_phase(&state, dt);

        // 物理更新（サーブ待機中と得点後は更新しない）
        if (state.phase != GAME_PHASE_START_GAME && state.phase != GAME_PHASE_POINT_SCORED)
        {
            update_ball(&state.ball, dt);
        }

        // バウンド処理（サーブ待機中と得点後はスキップ）
        if (state.phase != GAME_PHASE_START_GAME
            && state.phase != GAME_PHASE_POINT_SCORED
            && handle_bounce(&state.ball, GROUND_Y, BOUNCE_RESTITUTION))
        {
            state.ball.bounce_count++;

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
                    LOG_INFO("得点加算: Player" << winner_id << " が得点");

                    // スコア加算
                    add_point(&state.score, winner_id);

                    // スコア送信
                    Packet score_packet;
                    score_packet.type = PACKET_TYPE_SCORE_UPDATE;
                    score_packet.size = sizeof(GameScore);
                    memcpy(score_packet.data, &state.score, sizeof(GameScore));
                    network_broadcast(players, connections, &score_packet);

                    // スコア表示（現在のスコアを確認）
                    print_score(&state.score);

                    // フェーズ移行（POINT_SCOREDへ）
                    update_game_phase(&state, GAME_PHASE_START_GAME);

                    // 得点後にボールを初期化（次のサーブ準備）
                    // サーバーは得点者に基づいて決定（テニスのルール: 得点者の相手がサーブ）
                    int next_server = (winner_id == 0) ? 1 : 0;
                    reset_ball(&state.ball, next_server);
                    state.server_player_id = next_server;
                    LOG_INFO("次のサーブ: Player" << next_server);
                }
            }
        }

        // ボール状態の送信処理（位置だけでなく全情報を送信）
        Packet ball_packet;
        memset(&ball_packet, 0, sizeof(Packet));
        ball_packet.type = PACKET_TYPE_BALL_STATE;
        ball_packet.size = sizeof(Ball);
        memcpy(ball_packet.data, &state.ball, sizeof(Ball));
        network_broadcast(players, connections, &ball_packet);

        // ゲームフェーズ更新
        update_phase(&state, dt);
        if (state.phase != last_sent_phase)
        {
            LOG_INFO("フェーズ変更: " << last_sent_phase << " -> " << state.phase);

            // フェーズ変更パケットを全員に送信
            Packet phase_packet;
            memset(&phase_packet, 0, sizeof(Packet));
            phase_packet.type = PACKET_TYPE_GAME_PHASE;
            phase_packet.size = sizeof(GamePhase);
            memcpy(phase_packet.data, &state.phase, sizeof(GamePhase));
            network_broadcast(players, connections, &phase_packet);

            last_sent_phase = state.phase;
        }

        SDL_Delay(FRAME_DELAY_MS); // 60fps
    }

    LOG_INFO("メインループ終了");
    SDLNet_FreeSocketSet(socket_set);
    network_shutdown_server(server_socket);

    LOG_SUCCESS("サーバーを正常終了しました");
    return 0;
}
