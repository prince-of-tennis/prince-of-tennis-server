#include "ball_physics.h"
#include <math.h>


// ベクトル計算
Vec3 vec3_add(Vec3 a, Vec3 b) 
{
    Vec3 r = { a.x + b.x, a.y + b.y, a.z + b.z };
    return r;
}

Vec3 vec3_mul(Vec3 v, float k) 
{
    Vec3 r = { v.x * k, v.y * k, v.z * k };
    return r;
}

Vec3 vec3_normalize(Vec3 v) 
{
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len == 0) return v;
    Vec3 r = { v.x/len, v.y/len, v.z/len };
    return r;
}

// 物理更新
void update_ball(Ball *ball, float dt) 
{
    // 重力（y方向を下に）
    ball->velocity.y -= 9.8f * dt;

    // 位置更新
    ball->position = vec3_add(ball->position, vec3_mul(ball->velocity, dt));
}


// バウンド処理
void handle_bounce(Ball *ball, float ground_y, float restitution) 
{
    if (ball->position.y <= ground_y) {
        ball->position.y = ground_y;
        ball->velocity.y = -ball->velocity.y * restitution;
    }
}

// ラケット打球処理
void handle_racket_hit(Ball *ball, Vec3 direction, float power)
{
    Vec3 dir = vec3_normalize(direction);
    ball->velocity = vec3_mul(dir, power);
}
