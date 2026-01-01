#include "server_loop.h"
#include <SDL2/SDL.h>
#include <cstring>
#include "log.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "server_broadcast.h"
#include "game_update.h"

using namespace GameConstants;

void server_run_main_loop(ServerContext *ctx)
{
    const float dt = FRAME_TIME; // 60FPS

    // 初期プレイヤー状態を送信
    broadcast_initial_player_states(ctx);

    // ゲームフェーズを開始状態に設定
    set_game_phase(&ctx->state, GAME_PHASE_START_GAME);

    // 初期スコア（0-0）を送信
    LOG_INFO("初期スコアを送信: 0-0");
    broadcast_score_update(ctx);

    LOG_SUCCESS("ゲーム開始！");

    while (*(ctx->running) != 0)
    {
        int ready_count = SDLNet_CheckSockets(ctx->socket_set, 10);  // 10msタイムアウト

        if (ready_count < 0)
        {
            LOG_ERROR("ソケットチェック失敗: " << SDLNet_GetError());
            break;
        }

        // ソケットが準備できている場合のみ通信処理
        if (ready_count > 0)
        {
            // 新しいクライアントの接続チェック
            if (SDLNet_SocketReady(ctx->server_socket))
            {
                TCPsocket new_client_socket = network_accept_client(ctx->server_socket, ctx->players, ctx->connections);
                if (new_client_socket)
                {
                    SDLNet_TCP_AddSocket(ctx->socket_set, new_client_socket);
                }
            }

            // 各クライアントからのデータ受信
            game_handle_client_input(ctx, dt);
        }

        // フェーズ管理の更新（タイマーベースのフェーズ遷移）
        update_phase_timer(&ctx->state, dt);

        // 物理更新とスコアリング
        game_update_physics_and_scoring(ctx, dt);

        // ボール状態の送信処理
        broadcast_ball_state(ctx);

        // ゲームフェーズ更新の送信
        broadcast_phase_update(ctx);

        SDL_Delay(FRAME_DELAY_MS); // 60fps
    }

    LOG_INFO("メインループ終了");
}

