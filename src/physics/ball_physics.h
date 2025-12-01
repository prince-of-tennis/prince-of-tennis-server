// ball_physics.h
#ifndef BALL_PHYSICS_H
#define BALL_PHYSICS_H

#include "ball.h"
// �x�N�g�����[�e�B���e�B
Point3d point3d_add(Point3d a, Point3d b);
Point3d point3d_mul(Point3d v, float k);
Point3d point3d_normalize(Point3d v);
// �����X�V
void update_ball(Ball *ball, float dt);
// �o�E���h����
void handle_bounce(Ball *bsall, float ground_y, float restitution);
// ���P�b�g�̑ŋ�����
void handle_racket_hit(Ball *ball, Point3d direction, float power);
#endif