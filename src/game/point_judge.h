#ifndef POINT_JUDGE_H
#define POINT_JUDGE_H

#include "game_state.h"

// 得点判定の結果（enumハック対応）
enum PointJudgeResult {
    POINT_JUDGE_NONE = 0,          // 判定なし（ラリー継続）
    POINT_JUDGE_OUT = 1,           // アウト（打った人の負け）
    POINT_JUDGE_TWO_BOUNCE = 2,    // ツーバウンド（打った人の勝ち）
    POINT_JUDGE_NET = 3,           // ネット（将来実装用）
    POINT_JUDGE_COUNT
};

// enumハック: 判定結果の文字列配列
extern const char* point_judge_result_strings[POINT_JUDGE_COUNT];

// 得点判定を実行
// 戻り値: 勝者のプレイヤーID（PLAYER_ID_INVALID = 判定なし）
int judge_point(GameState *state);

#endif // POINT_JUDGE_H

