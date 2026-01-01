#ifndef GAME_PHASE_MANAGER_H
#define GAME_PHASE_MANAGER_H

#include "game_state.h"

// フェーズ管理の初期化
void init_phase_manager(GameState *state);

// ゲームフェーズを設定
void set_game_phase(GameState *state, GamePhase new_phase);

// フェーズタイマーを更新（自動遷移処理）
void update_phase_timer(GameState *state, float dt);

// 物理更新が有効なフェーズかチェック
bool is_physics_active_phase(GamePhase phase);

// スイング可能なフェーズかチェック
bool is_swing_allowed_phase(GamePhase phase);

#endif