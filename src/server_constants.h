#pragma once

// ============================================
// サーバー側の定数定義
// ============================================

// フェーズタイマー定数
constexpr float TIME_MATCH_COMPLETE = 2.0f;    // マッチング完了後、ゲーム開始までの時間（秒）
constexpr float TIME_AFTER_POINT = 3.0f;       // 得点後から次のサーブまでの時間（秒）
constexpr float TIME_GAME_FINISHED = 1.0f;     // ゲーム終了後、サーバー終了までの時間（秒）

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
constexpr float SWING_ACCELERATION_THRESHOLD = 5.0f;  // スイング判定の加速度閾値（m/s²）

// ボール打撃関連
constexpr float BALL_SHOT_SPEED = 20.0f;          // 打球速度（m/s）
constexpr float BALL_SHOT_ANGLE_Y = 0.5f;         // 打球の上方向成分

// スイング補正パラメータ（加速度ベース角度調整）
// Joy-Conの加速度の想定最大値
constexpr float SWING_ACC_MAX_X = 15.0f;          // X軸（左右）の加速度最大値
constexpr float SWING_ACC_MAX_Y = 15.0f;          // Y軸（上下）の加速度最大値
constexpr float SWING_ACC_MAX_Z = 15.0f;          // Z軸（前後）の加速度最大値

// 打球方向の角度範囲（正規化前の成分値）
constexpr float SWING_ANGLE_X_MIN = -0.3f;        // X軸（左右）の最小値（左）
constexpr float SWING_ANGLE_X_MAX = 0.3f;         // X軸（左右）の最大値（右）
constexpr float SWING_ANGLE_Y_MIN = 0.25f;        // Y軸（高さ）の最小値（低い弾道）
constexpr float SWING_ANGLE_Y_MAX = 1.0f;         // Y軸（高さ）の最大値（高い弾道）
constexpr float SWING_ANGLE_Z_BASE = 0.55f;       // Z軸（前方）の基本値（固定）

// 速度パラメータ
constexpr float SWING_SPEED_MULTIPLIER = 1.0f;    // 加速度から速度への変換係数
constexpr float SWING_SPEED_MIN = 12.0f;          // 打球の最低速度（m/s）
constexpr float SWING_SPEED_MAX = 25.0f;          // 打球の最高速度（m/s）
constexpr float BALL_SHOT_SPEED_BASE = 12.0f;     // 基本打球速度（m/s）

// Z軸速度抑制パラメータ
constexpr float Z_VELOCITY_DAMPING = 2.2f;

// 能力関連パラメータ
constexpr float ABILITY_SPEED_UP_MULTIPLIER = 2.0f;  // #84: スピードアップ時の速度倍率        // Z軸方向の速度減衰係数

// ロブショット用パラメータ（Bボタン）
constexpr float LOB_SHOT_SPEED_MULTIPLIER = 1.0f;    // ロブショット速度倍率
constexpr float LOB_SHOT_Y_BOOST = 1.7;             // ロブショットのY軸方向ブースト
constexpr float LOB_SHOT_Z_BOOST = 0.5f;             // ロブショットのZ軸方向ブースト
constexpr float LOB_SHOT_GRAVITY_MULTIPLIER = 0.8f;  // ロブショットの重力倍率（少しゆっくり落ちる）
