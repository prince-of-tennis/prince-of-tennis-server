#include "ball_physics.h"
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

// 物理更新
void update_ball(Ball *ball, float dt)
{
    // 重力（y方向を下に）
    ball->velocity.y -= 9.8f * dt;

    // 位置更新
    ball->point = point3d_add(ball->point, point3d_mul(ball->velocity, dt));
}

// バウンド処理
void handle_bounce(Ball *ball, float ground_y, float restitution)
{
    if (ball->point.y <= ground_y)
    {
        ball->point.y = ground_y;
        ball->velocity.y = -ball->velocity.y * restitution;
    }
}

// ラケット打球処理
void handle_racket_hit(Ball *ball, Point3d direction, float power)
{
    Point3d dir = point3d_normalize(direction);
    ball->velocity = point3d_mul(dir, power);
}
