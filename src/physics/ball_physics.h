#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H

#include "common/ball.h"
// ベクトルユーティリティ（計算用
Point3d point3d_add(Point3d a, Point3d b);
Point3d point3d_mul(Point3d v, float k);
Point3d point3d_normalize(Point3d v);
// ボールの更新
void update_ball(Ball *ball, float dt);
// バウンド処理
bool handle_bounce(Ball *ball, float ground_y, float restitution);
// ラケットでの打撃処理
void handle_racket_hit(Ball *ball, Point3d direction, float power);
#endif