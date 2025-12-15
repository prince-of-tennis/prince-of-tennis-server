#include "score_logic.h"
#include <stdio.h>

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

    // ポイント加算ロジック
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
        if (*lose_pt < 40)
        {
            // ゲーム獲得 (相手が40未満なら勝ち)
            goto WIN_GAME;
        }
        else if (*lose_pt == 40)
        {
            // デュースからアドバンテージへ
            *win_pt = 50; // アドバンテージを50で表現
        }
        else if (*lose_pt == 50)
        {
            // 相手のアドバンテージを相殺、デュースに戻る
            *lose_pt = 40;
            *win_pt = 40;
        }
    }

    else if (*win_pt == 50)
    {
        // アドバンテージ状態でポイント取ったのでゲーム獲得
        goto WIN_GAME;
    }
    return true;

WIN_GAME:
    // ゲーム獲得処理
    score->games_in_set[score->current_set][winner]++;
    score->current_game_p1 = 0;
    score->current_game_p2 = 0;

    printf("[SCORE] Player %d wins the GAME!\n", winner + 1);

    // --- セット終了判定 (簡易版: 6ゲーム先取) ---
    if (score->games_in_set[score->current_set][winner] >= 6)
    {
        printf("[SCORE] Player %d wins the SET %d!\n", winner + 1, score->current_set + 1);

        // 次のセットへ
        score->current_set++;
        if (match_finished(score))
        {
            printf("[SCORE] MATCH FINISHED! Player %d WINS!\n", winner + 1);
            return false;
        }
    }

    return true;
}

void print_score(const GameScore *score)
{
    if (score->current_set < MAX_SETS)
    {
        printf("Set: %d | Games: %d - %d | Points: %d - %d\n",
               score->current_set + 1,
               score->games_in_set[score->current_set][0],
               score->games_in_set[score->current_set][1],
               score->current_game_p1,
               score->current_game_p2);
    }
    else
    {
        printf("Match Finished.\n");
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