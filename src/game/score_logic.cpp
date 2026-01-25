#include "score_logic.h"
#include "../log.h"
#include "../server_constants.h"

void init_score(GameScore *score)
{
    score->point_p1 = 0;
    score->point_p2 = 0;
    score->sets_p1 = 0;
    score->sets_p2 = 0;
}

// セット獲得処理
static bool award_set(GameScore *score, int winner)
{
    if (winner == 0)
    {
        score->sets_p1++;
        LOG_SUCCESS("プレイヤー 1 がセットを獲得！ (" << score->sets_p1 << " - " << score->sets_p2 << ")");
    }
    else
    {
        score->sets_p2++;
        LOG_SUCCESS("プレイヤー 2 がセットを獲得！ (" << score->sets_p1 << " - " << score->sets_p2 << ")");
    }

    // ポイントをリセット
    score->point_p1 = 0;
    score->point_p2 = 0;

    // 試合終了判定
    if (match_finished(score))
    {
        LOG_SUCCESS("試合終了！プレイヤー " << (winner + 1) << " の勝利！");
        return false;  // 試合終了
    }

    return true;  // 試合継続
}

bool add_point(GameScore *score, int winner)
{
    int *win_pt = (winner == 0) ? &score->point_p1 : &score->point_p2;

    // ポイント加算 (0 → 15 → 30 → 40 → 50)
    if (*win_pt == 0)
    {
        *win_pt = 15;
    }
    else if (*win_pt == 15)
    {
        *win_pt = 30;
    }
    else if (*win_pt == 30)
    {
        *win_pt = 40;
    }
    else if (*win_pt == 40)
    {
        *win_pt = 50;
        // 50点でセット獲得
        return award_set(score, winner);
    }

    return true;
}

void print_score(const GameScore *score)
{
    LOG_INFO("セット: " << score->sets_p1 << " - " << score->sets_p2
             << " | ポイント: " << score->point_p1 << " - " << score->point_p2);
}

bool match_finished(const GameScore *score)
{
    return (score->sets_p1 >= SETS_TO_WIN || score->sets_p2 >= SETS_TO_WIN);
}

int get_match_winner(const GameScore *score)
{
    if (score->sets_p1 >= SETS_TO_WIN) return 0;
    if (score->sets_p2 >= SETS_TO_WIN) return 1;
    return -1;
}
