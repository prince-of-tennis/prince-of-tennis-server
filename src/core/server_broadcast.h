#pragma once

#include "server_context.h"

// ボール状態をブロードキャスト
void broadcast_ball_state(ServerContext *ctx);

// ゲームフェーズをブロードキャスト（変更時のみ）
void broadcast_phase_update(ServerContext *ctx);

// スコアをブロードキャスト（変更時のみ）
void broadcast_score_update(ServerContext *ctx);

// 初期プレイヤー状態をブロードキャスト
void broadcast_initial_player_states(ServerContext *ctx);

// 能力状態をブロードキャスト
void broadcast_ability_state(ServerContext *ctx, int player_id);

// 試合結果をブロードキャスト
void broadcast_match_result(ServerContext *ctx, int winner_id);
