#pragma once

#include "server_context.h"

// クライアントからの入力を処理
void game_handle_client_input(ServerContext *ctx, float dt);

// ゲーム物理とスコアリングを更新
void game_update_physics_and_scoring(ServerContext *ctx, float dt);

