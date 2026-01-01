#include "score_logic.h"
#include "../log.h"
#include "../server_constants.h"
#include <stdio.h>

// ゲーム獲得処理のヘルパー関数
static bool award_game(GameScore *score, int winner)
{
    score->games_in_set[score->current_set][winner]++;
    score->current_game_p1 = 0;
    score->current_game_p2 = 0;

    printf("[SCORE] Player %d wins the GAME!\n", winner + 1);

    // --- セット終了判定 (簡易版: 6ゲーム先取) ---
    if (score->games_in_set[score->current_set][winner] >= 6)
    {
        LOG_SUCCESS("プレイヤー " << (winner + 1) << " がセット " << (score->current_set + 1) << " を獲得！");

        // 次のセットへ
        score->current_set++;
        if (match_finished(score))
        {
            LOG_SUCCESS("試合終了！プレイヤー " << (winner + 1) << " の勝利！");
            return false;
        }
    }

    return true;
}

// 40点状態での得点処理（デュース、アドバンテージ）
static bool handle_forty_point(int *win_pt, int *lose_pt, GameScore *score, int winner)
{
    if (*lose_pt < TENNIS_SCORE_FORTY)
    {
        // ゲーム獲得 (相手が40未満なら勝ち)
        return award_game(score, winner);
    }
    else if (*lose_pt == TENNIS_SCORE_FORTY)
    {
        // デュースからアドバンテージへ
        *win_pt = TENNIS_SCORE_ADVANTAGE;
    }
    else if (*lose_pt == TENNIS_SCORE_ADVANTAGE)
    {
        // 相手のアドバンテージを相殺、デュースに戻る
        *lose_pt = TENNIS_SCORE_FORTY;
        *win_pt = TENNIS_SCORE_FORTY;
    }

    return true;
}

void init_score(GameScore *score)
{
    score->current_set = 0;
    score->current_game_p1 = 0;
    score->current_game_p2 = 0;

    for (int i = 0; i < MAX_SETS; i++)
    {
        score->games_in_set[i][0] = 0;
        score->games_in_set[i][1] = 0;
    }
}

bool add_point(GameScore *score, int winner)
{
    // 勝者と敗者の現在のポイントへのポインタを取得
    int *win_pt = (winner == 0) ? &score->current_game_p1 : &score->current_game_p2;
    int *lose_pt = (winner == 0) ? &score->current_game_p2 : &score->current_game_p1;

    // ポイント加算ロジック（テニスのスコア遷移）
    if (*win_pt == TENNIS_SCORE_LOVE)
    {
        *win_pt = TENNIS_SCORE_FIFTEEN;
    }
    else if (*win_pt == TENNIS_SCORE_FIFTEEN)
    {
        *win_pt = TENNIS_SCORE_THIRTY;
    }
    else if (*win_pt == TENNIS_SCORE_THIRTY)
    {
        *win_pt = TENNIS_SCORE_FORTY;
    }
    else if (*win_pt == TENNIS_SCORE_FORTY)
    {
        return handle_forty_point(win_pt, lose_pt, score, winner);
    }
    else if (*win_pt == TENNIS_SCORE_ADVANTAGE)
    {
        // アドバンテージ状態でポイント取ったのでゲーム獲得
        return award_game(score, winner);
    }

    return true;
}

void print_score(const GameScore *score)
{
    if (score->current_set < MAX_SETS)
    {
        LOG_INFO("セット: " << (score->current_set + 1)
                 << " | ゲーム: " << score->games_in_set[score->current_set][0]
                 << " - " << score->games_in_set[score->current_set][1]
                 << " | ポイント: " << score->current_game_p1
                 << " - " << score->current_game_p2);
    }
    else
    {
        LOG_INFO("試合終了");
    }
}

// ========================================================
// マッチ終了判定の実装
// ========================================================
bool match_finished(const GameScore *score)
{
    // 現在のセット番号が MAX_SETS (例: 1または3) に達していたら終了とみなす
    return score->current_set >= MAX_SETS;
}