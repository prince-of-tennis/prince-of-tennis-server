#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H

typedef struct 
{
    float x, y, z;
} Vec3;

typedef struct 
{
    Vec3 position;
    Vec3 velocity;
} Ball;

// ベクトルユーティリティ
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 v, float k);
Vec3 vec3_normalize(Vec3 v);

// 物理更新
void update_ball(Ball *ball, float dt);

// バウンド処理
void handle_bounce(Ball *ball, float ground_y, float restitution);

// ラケットの打球処理
void handle_racket_hit(Ball *ball, Vec3 direction, float power);

#endif
