#include "server_loop.h"
#include <SDL2/SDL.h>
#include <string.h>
#include "log.h"
#include "game/game_phase_manager.h"
#include "common/game_constants.h"
#include "common/ability.h"
#include "server_broadcast.h"
#include "game_update.h"
#include "../server_constants.h"

// 能力状態のtick処理
static void update_ability_states(ServerContext *ctx)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        AbilityState *state = &ctx->state.ability_states[i];

        if (state->active_ability == ABILITY_GIANT || state->active_ability == ABILITY_CLONE)
        {
            continue;
        }

        if (state->remaining_frames > 0)
        {
            state->remaining_frames--;

            // 能力終了時にブロードキャスト
            if (state->remaining_frames == 0)
            {
                LOG_INFO("能力終了: player=" << i << " ability=" << static_cast<int>(state->active_ability));
                state->active_ability = ABILITY_NONE;
                broadcast_ability_state(ctx, i);
            }
        }
    }
}

void server_run_main_loop(ServerContext *ctx)
{
    const float dt = GameConstants::FRAME_TIME; // 60FPS

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
        int ready_count = SDLNet_CheckSockets(ctx->socket_set, SOCKET_TIMEOUT_MAIN_LOOP_MS);

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
        update_phase_timer(&ctx->state, dt, ctx->running);

        // 物理更新とスコアリング
        game_update_physics_and_scoring(ctx, dt);

        // 能力状態の更新
        update_ability_states(ctx);

        // ボール状態の送信処理
        broadcast_ball_state(ctx);

        // ゲームフェーズ更新の送信
        broadcast_phase_update(ctx);

        // 試合終了時、結果を送信（1回だけ）
        if (ctx->state.phase == GAME_PHASE_GAME_FINISHED && !ctx->state.match_result_sent)
        {
            broadcast_match_result(ctx, ctx->state.match_winner);
            ctx->state.match_result_sent = true;
        }

        SDL_Delay(GameConstants::FRAME_DELAY_MS); // 60fps
    }

    LOG_INFO("メインループ終了");
}

