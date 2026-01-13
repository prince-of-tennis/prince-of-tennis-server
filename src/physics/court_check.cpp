#include "court_check.h"
#include "common/game_constants.h"
#include "../log.h"

// コート範囲内かチェック
bool is_in_court(Point3d p)
{
    // デバッグ: コート範囲設定を出力（初回のみ）
    static bool first_call = true;
    if (first_call)
    {
        LOG_INFO("【コート範囲設定】");
        LOG_INFO("  COURT_HALF_WIDTH: " << GameConstants::COURT_HALF_WIDTH << "m (範囲: "
                 << -GameConstants::COURT_HALF_WIDTH << " ～ " << GameConstants::COURT_HALF_WIDTH << ")");
        LOG_INFO("  COURT_HALF_LENGTH: " << GameConstants::COURT_HALF_LENGTH << "m (範囲: "
                 << -GameConstants::COURT_HALF_LENGTH << " ～ " << GameConstants::COURT_HALF_LENGTH << ")");
        LOG_INFO("  COURT_MAX_HEIGHT: " << GameConstants::COURT_MAX_HEIGHT << "m");
        first_call = false;
    }

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