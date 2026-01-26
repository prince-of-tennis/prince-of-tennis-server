#pragma once

// フェーズタイマー
constexpr float TIME_MATCH_COMPLETE = 2.0f;
constexpr float TIME_AFTER_POINT = 3.0f;
constexpr float TIME_GAME_FINISHED = 1.0f;

// ネットワークタイムアウト
constexpr int NETWORK_RECEIVE_MAX_ATTEMPTS = 100;
constexpr int SOCKET_TIMEOUT_CLIENT_WAIT_MS = 100;
constexpr int SOCKET_TIMEOUT_INIT_WAIT_MS = 500;
constexpr int SOCKET_TIMEOUT_MAIN_LOOP_MS = 10;

// テニススコア
enum TennisPointScore {
    TENNIS_SCORE_LOVE = 0,
    TENNIS_SCORE_FIFTEEN = 15,
    TENNIS_SCORE_THIRTY = 30,
    TENNIS_SCORE_FORTY = 40,
    TENNIS_SCORE_ADVANTAGE = 50
};

constexpr int GAME_SCORE_INVALID = -1;

// プレイヤー操作
constexpr float PLAYER_SWING_RADIUS = 5.0f;
constexpr float SWING_ACCELERATION_THRESHOLD = 5.0f;

// ボール打撃
constexpr float BALL_SHOT_SPEED = 20.0f;
constexpr float BALL_SHOT_ANGLE_Y = 0.5f;

// スイング加速度の最大値
constexpr float SWING_ACC_MAX_X = 15.0f;
constexpr float SWING_ACC_MAX_Y = 15.0f;
constexpr float SWING_ACC_MAX_Z = 15.0f;

// 打球方向の角度範囲
constexpr float SWING_ANGLE_X_MIN = -0.3f;
constexpr float SWING_ANGLE_X_MAX = 0.3f;
constexpr float SWING_ANGLE_Y_MIN = 0.25f;
constexpr float SWING_ANGLE_Y_MAX = 1.0f;
constexpr float SWING_ANGLE_Z_BASE = 0.55f;

// 速度パラメータ
constexpr float SWING_SPEED_MULTIPLIER = 1.0f;
constexpr float SWING_SPEED_MIN = 12.0f;
constexpr float SWING_SPEED_MAX = 25.0f;
constexpr float BALL_SHOT_SPEED_BASE = 12.0f;
constexpr float Z_VELOCITY_DAMPING = 2.2f;

// 能力
constexpr float ABILITY_SPEED_UP_MULTIPLIER = 2.0f;

// ロブショット（Bボタン）
constexpr float LOB_SHOT_SPEED_MULTIPLIER = 1.0f;
constexpr float LOB_SHOT_Y_BOOST = 1.7f;
constexpr float LOB_SHOT_Z_BOOST = 0.5f;
constexpr float LOB_SHOT_GRAVITY_MULTIPLIER = 0.8f;
