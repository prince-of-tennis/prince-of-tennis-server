// ball_physics.h
#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H

#include "ball.h"
// ベクトルユーティリティ
Point3d point3d_add(Point3d a, Point3d b);
Point3d point3d_mul(Point3d v, float k);
Point3d point3d_normalize(Point3d v);
// 物理更新
void update_ball(Ball *ball, float dt);
// バウンド処理
void handle_bounce(Ball *bsall, float ground_y, float restitution);
// ラケットの打球処理
void handle_racket_hit(Ball *ball, Point3d direction, float power);
#endif