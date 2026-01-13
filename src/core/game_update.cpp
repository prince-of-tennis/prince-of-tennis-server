#include "game_update.h"
#include <cstring>
#include "log.h"
#include "input_handler/input_handler.h"
#include "physics/ball_physics.h"
#include "game/score_logic.h"
#include "game/game_phase_manager.h"
#include "game/point_judge.h"
#include "common/player_input.h"
#include "common/player_swing.h"
#include "common/game_constants.h"
#include "server_broadcast.h"

// クライアント入力を処理
void game_handle_client_input(ServerContext *ctx, float dt)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (ctx->players[i].connected && ctx->connections[i].socket && SDLNet_SocketReady(ctx->connections[i].socket))
        {
            Packet packet;
            int size = network_receive_packet(ctx->connections[i].socket, &packet);

            if (size <= 0)
            {
                LOG_WARN("クライアント " << i << " から切断されました");
                SDLNet_TCP_DelSocket(ctx->socket_set, ctx->connections[i].socket);
                network_close_client(&ctx->players[i], &ctx->connections[i]);
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
                    apply_player_input(&ctx->state, i, &input, dt);

                    // いずれかの入力がある場合、そのプレイヤーの状態を送信
                    bool has_input = input.right || input.left || input.front || input.back;
                    if (has_input)
                    {
                        Packet player_packet = create_packet_player_state(&ctx->state.players[i]);
                        network_broadcast(ctx->players, ctx->connections, &player_packet);
                    }
                }
                else
                {
                    LOG_WARN("PlayerInputのサイズが不一致: 受信=" << packet.size
                            << ", 期待=" << sizeof(PlayerInput));
                }
            }
            else if (pkt_type == PACKET_TYPE_PLAYER_SWING)
            {
                PlayerSwing swing;
                memset(&swing, 0, sizeof(PlayerSwing));
                if (packet.size == sizeof(PlayerSwing))
                {
                    memcpy(&swing, packet.data, sizeof(PlayerSwing));
                    apply_player_swing(&ctx->state, i, &swing);
                }
                else
                {
                    LOG_WARN("PlayerSwingのサイズが不一致: 受信=" << packet.size
                            << ", 期待=" << sizeof(PlayerSwing));
                }
            }
        }
    }
}

// ゲーム物理とスコアリングを更新
void game_update_physics_and_scoring(ServerContext *ctx, float dt)
{
    // 物理更新（フェーズチェックヘルパーを使用）
    if (is_physics_active_phase(ctx->state.phase))
    {
        update_ball(&ctx->state.ball, dt);
    }

    // バウンド処理
    if (is_physics_active_phase(ctx->state.phase)
        && handle_bounce(&ctx->state.ball, GameConstants::GROUND_Y, GameConstants::BOUNCE_RESTITUTION))
    {
        ctx->state.ball.bounce_count++;

        // 得点判定を実行
        int winner_id = judge_point(&ctx->state);

        // 勝者が決まったらスコア加算と次のサーブ準備
        if (winner_id != GameConstants::PLAYER_ID_INVALID)
        {
            LOG_INFO("得点加算: Player" << winner_id << " が得点");

            // スコア加算
            add_point(&ctx->state.score, winner_id);

            // スコア送信
            broadcast_score_update(ctx);

            // スコア表示
            print_score(&ctx->state.score);

            // フェーズ移行（次のサーブ準備へ）
            set_game_phase(&ctx->state, GAME_PHASE_START_GAME);

            // 得点後にボールを初期化
            // 次のサーバーは得点者の相手（テニスのルール）
            int next_server = GameConstants::get_opponent_player_id(winner_id);
            reset_ball(&ctx->state.ball, next_server);
            ctx->state.server_player_id = next_server;
            LOG_INFO("次のサーブ: Player" << next_server);
        }
    }
}
