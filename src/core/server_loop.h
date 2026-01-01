#pragma once

#include "server_context.h"

// メインゲームループを実行
// クライアント入力の処理、物理更新、ブロードキャストを担当
void server_run_main_loop(ServerContext *ctx);

