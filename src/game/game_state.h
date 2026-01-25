#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "physics/ball_physics.h"
#include "common/GameScore.h"
#include "common/GamePhase.h"
#include "common/ability.h"
#include "common/player.h"
#include "network/network.h"

typedef struct
{
    Ball ball;
    Player players[MAX_CLIENTS];
    GameScore score;
    float state_timer;

    GamePhase phase;

    int server_player_id;

    // 能力状態（プレイヤーごと）
    AbilityState ability_states[MAX_CLIENTS];

    // 試合結果（-1: 未確定、0: P1勝利、1: P2勝利）
    int match_winner;
    bool match_result_sent;  // 試合結果送信済みフラグ
} GameState;

void init_game(GameState *state);
void update_game(GameState *state, float dt);
#endif
