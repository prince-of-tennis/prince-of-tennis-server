#include "ball_physics.h"
#include "common/game_constants.h"
#include <math.h>

// ベクトル計算
Point3d point3d_add(Point3d a, Point3d b)
{
    Point3d r = {a.x + b.x, a.y + b.y, a.z + b.z};
    return r;
}

Point3d point3d_mul(Point3d v, float k)
{
    Point3d r = {v.x * k, v.y * k, v.z * k};
    return r;
}

Point3d point3d_normalize(Point3d v)
{
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len == 0)
        return v;
    Point3d r = {v.x / len, v.y / len, v.z / len};
    return r;
}

// ボールの更新
void update_ball(Ball *ball, float dt)
{
    // 前フレームのZ座標を保存（ネット判定用）
    ball->previous_z = ball->point.z;

    // 重力倍率（未設定なら1.0）
    float gravity_mult = (ball->gravity_multiplier > 0.0f) ? ball->gravity_multiplier : 1.0f;

    // 重力 (Y軸マイナス方向へ)
    ball->velocity.y -= GameConstants::GRAVITY * gravity_mult * dt;

    // 位置の更新
    ball->point = point3d_add(ball->point, point3d_mul(ball->velocity, dt));
}

// バウンド処理
bool handle_bounce(Ball *ball, float ground_y, float restitution)
{
    bool bounced = false;
    // ボールが地面以下にあり、かつ下向きに移動している場合のみバウンド
    if (ball->point.y <= ground_y && ball->velocity.y < 0)
    {
        ball->point.y = ground_y;
        ball->velocity.y = -ball->velocity.y * restitution;
        bounced = true;
    }
    return bounced;
}

// ラケットでの打撃処理
void handle_racket_hit(Ball *ball, Point3d direction, float power)
{
    Point3d dir = point3d_normalize(direction);
    ball->velocity = point3d_mul(dir, power);
}

// ボール初期化（得点後のリセット用）
void reset_ball(Ball *ball, int server_player_id)
{
    // サーバーのプレイヤーIDに応じて初期位置を設定
    // Player0 (ID=0): 手前側 (Z > 0)
    // Player1 (ID=1): 奥側 (Z < 0)

    constexpr float SERVE_POSITION_Z = GameConstants::PLAYER_BASELINE_DISTANCE - GameConstants::BALL_SERVE_OFFSET_FROM_BASELINE;

    if (server_player_id == 0)
        ball->point = (Point3d){0.0f, GameConstants::BALL_SERVE_HEIGHT, SERVE_POSITION_Z};
    else
        ball->point = (Point3d){0.0f, GameConstants::BALL_SERVE_HEIGHT, -SERVE_POSITION_Z};

    ball->velocity = (Point3d){0.0f, 0.0f, 0.0f};
    ball->angle = 0;
    ball->last_hit_player_id = server_player_id;
    ball->bounce_count = 0;
    ball->hit_count = 0;
    ball->previous_z = ball->point.z;
    ball->gravity_multiplier = 1.0f;
}

