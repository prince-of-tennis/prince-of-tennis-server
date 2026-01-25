#ifndef SCORE_LOGIC_H
#define SCORE_LOGIC_H

#include "common/GameScore.h"

// スコアを初期化 (0-0 スタート) する関数
void init_score(GameScore *score);
// ポイントを加算する関数
bool add_point(GameScore *score, int player_index);
// スコアを表示する関数
void print_score(const GameScore *score);
// マッチ終了判定
bool match_finished(const GameScore *score);
// 勝者を返す（-1: まだ決まっていない、0: P1、1: P2）
int get_match_winner(const GameScore *score);
#endif // SCORE_LOGIC_H