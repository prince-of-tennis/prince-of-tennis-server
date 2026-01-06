#include "court_check.h"
#include "common/game_constants.h"

// コート範囲内かチェック
bool is_in_court(Point3d p)
{
    if (p.x < -GameConstants::COURT_HALF_WIDTH || p.x > GameConstants::COURT_HALF_WIDTH)
    {
        return false;
    }

    if (p.z < -GameConstants::COURT_HALF_LENGTH || p.z > GameConstants::COURT_HALF_LENGTH)
    {
        return false;
    }

    // 高さチェック（コートの最大高さを超えたらアウト）
    if (p.y > GameConstants::COURT_MAX_HEIGHT)
    {
        return false;
    }

    return true;
}