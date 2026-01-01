#pragma once

// ============================================
// サーバー側の定数定義
// ============================================

// フェーズタイマー定数
constexpr float TIME_MATCH_COMPLETE = 2.0f;    // マッチング完了後、ゲーム開始までの時間（秒）
constexpr float TIME_AFTER_POINT = 3.0f;       // 得点後から次のサーブまでの時間（秒）
constexpr float TIME_GAME_FINISHED = 5.0f;     // ゲーム終了後、サーバー終了までの時間（秒）

// ネットワークタイムアウト定数
constexpr int NETWORK_RECEIVE_MAX_ATTEMPTS = 100;      // パケット受信の最大試行回数
constexpr int SOCKET_TIMEOUT_CLIENT_WAIT_MS = 100;     // クライアント待機時のタイムアウト（ミリ秒）
constexpr int SOCKET_TIMEOUT_INIT_WAIT_MS = 500;       // 初期化待機時のタイムアウト（ミリ秒）
constexpr int SOCKET_TIMEOUT_MAIN_LOOP_MS = 10;        // メインループのタイムアウト（ミリ秒）

// テニススコア定数
enum TennisPointScore {
    TENNIS_SCORE_LOVE = 0,
    TENNIS_SCORE_FIFTEEN = 15,
    TENNIS_SCORE_THIRTY = 30,
    TENNIS_SCORE_FORTY = 40,
    TENNIS_SCORE_ADVANTAGE = 50  // アドバンテージを50で表現
};

// ゲームスコア無効値
constexpr int GAME_SCORE_INVALID = -1;

// プレイヤー操作関連
constexpr float PLAYER_SWING_RADIUS = 5.0f;       // スイング可能範囲（メートル）

// ボール打撃関連
constexpr float BALL_SHOT_SPEED = 25.0f;          // 打球速度（m/s）
constexpr float BALL_SHOT_ANGLE_Y = 0.5f;         // 打球の上方向成分
