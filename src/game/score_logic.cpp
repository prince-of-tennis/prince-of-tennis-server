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

static bool award_set(GameScore *score, int winner)
{
    if (winner == 0)
        score->sets_p1++;
    else
        score->sets_p2++;

    score->point_p1 = 0;
    score->point_p2 = 0;

    if (match_finished(score))
    {
        LOG_SUCCESS("試合終了！Player " << (winner + 1) << " の勝利");
        return false;
    }

    LOG_SUCCESS("セット獲得: " << score->sets_p1 << " - " << score->sets_p2);
    return true;
}

bool add_point(GameScore *score, int winner)
{
    int *win_pt = (winner == 0) ? &score->point_p1 : &score->point_p2;

    switch (*win_pt)
    {
        case 0:  *win_pt = 15; break;
        case 15: *win_pt = 30; break;
        case 30: *win_pt = 40; break;
        case 40:
            *win_pt = 50;
            return award_set(score, winner);
    }
    return true;
}

void print_score(const GameScore *score)
{
    // ログ削減のため空実装
    (void)score;
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
