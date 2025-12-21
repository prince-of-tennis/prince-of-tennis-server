#ifndef COURT_CHECK_H
#define COURT_CHECK_H

#include "common/util/point_3d.h"

// コートの寸法（メートル単位）
#define COURT_HALF_WIDTH 4.115f
#define COURT_HALF_LENGTH 11.89f

// プレイヤー初期配置（ベースライン位置）
// Blender座標: Player1=(0, -24.9674, 0), Player2=(0, 24.9674, 0)
// ゲーム座標系では Y→Z、符号反転で Z軸に変換
#define PLAYER_BASELINE_DISTANCE 24.9674f

bool is_in_court(Point3d p);

#endif // COURT_CHECK_H