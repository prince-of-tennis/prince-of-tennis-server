#ifndef SCORE_LOGIC_H
#define SCORE_LOGIC_H

#include "common/GameScore.h"

// スコアを初期化（0-0 スタート）にする関数
void init_score(GameScore *score);
// ポイントを加算する関数
bool add_point(GameScore *score, int player_index);

void print_score(const GameScore *score);

#endif // SCORE_LOGIC_H