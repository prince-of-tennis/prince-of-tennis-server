#include "point_judge.h"
#include "physics/court_check.h"
#include "common/game_constants.h"
#include "../log.h"

using namespace GameConstants;

// enumハック: 判定結果の文字列配列
const char* point_judge_result_strings[POINT_JUDGE_COUNT] = {
    "判定なし",
    "アウト",
    "ツーバウンド",
    "ネット"
};

// 得点判定を実行
// 戻り値: 勝者のプレイヤーID（PLAYER_ID_INVALID = 判定なし）
int judge_point(GameState *state)
{
    // nullptr チェック
    if (!state)
    {
        LOG_ERROR("GameState が nullptr です");
        return PLAYER_ID_INVALID;
    }

    // ラリー中のみ判定を行う
    if (state->phase != GAME_PHASE_IN_RALLY)
    {
        return PLAYER_ID_INVALID;
    }

    Ball &ball = state->ball;
    bool is_in = is_in_court(ball.point); // コート判定
    int winner_id = PLAYER_ID_INVALID;

    // ケース1: 1回目のバウンドでアウトだった場合 -> 打った人のミス
    if (ball.bounce_count == 1 && !is_in)
    {
        // 最後に打ったのが 0 なら 1 の勝ち、1 なら 0 の勝ち
        winner_id = get_opponent_player_id(ball.last_hit_player_id);
        LOG_INFO("判定: " << point_judge_result_strings[POINT_JUDGE_OUT]
                 << "! 勝者: P" << winner_id);
    }
    // ケース2: 2回目のバウンド（ツーバウンド） -> 打った人の得点
    else if (ball.bounce_count == 2)
    {
        winner_id = ball.last_hit_player_id;
        LOG_INFO("判定: " << point_judge_result_strings[POINT_JUDGE_TWO_BOUNCE]
                 << "! 勝者: P" << winner_id);
    }

    // プレイヤーID検証
    if (winner_id != PLAYER_ID_INVALID && !is_valid_player_id(winner_id))
    {
        LOG_ERROR("無効な勝者プレイヤーID: " << winner_id);
        return PLAYER_ID_INVALID;
    }

    return winner_id;
}

