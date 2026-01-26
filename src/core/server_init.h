#pragma once

#include "server_context.h"

// サーバー初期化
// SDL、ネットワーク、ゲーム状態を初期化する
// port: サーバーがリッスンするポート番号
// 戻り値: 成功時true、失敗時false
bool server_initialize(ServerContext *ctx, int port);

// クライアント接続待機
// 必要人数（MAX_CLIENTS）のクライアントが接続するまで待機
// 戻り値: 成功時true、中断時false
bool server_wait_for_clients(ServerContext *ctx);

// サーバークリーンアップ
// ネットワークリソースを解放し、SDLをシャットダウン
void server_cleanup(ServerContext *ctx);

// ゲームリセット（新しいゲームのために再待機）
// クライアント接続を切断し、ゲーム状態をリセット
void server_reset_for_new_game(ServerContext *ctx);

