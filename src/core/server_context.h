#pragma once

#include <SDL2/SDL_net.h>
#include "game/game_state.h"
#include "network/network.h"

// サーバー全体のコンテキスト構造体
// グローバル変数を集約し、関数間でのデータ受け渡しを明確化
struct ServerContext
{
    // ゲーム状態
    GameState state;

    // プレイヤー管理
    Player players[MAX_CLIENTS];
    ClientConnection connections[MAX_CLIENTS];

    // ネットワーク
    TCPsocket server_socket;
    SDLNet_SocketSet socket_set;

    // フェーズ・スコア変更検知用
    GamePhase last_sent_phase;
    GameScore last_sent_score;

    // 実行制御（シグナルハンドラーから参照）
    volatile int *running;
};

