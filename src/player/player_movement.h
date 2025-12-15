#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

// 必要な共通構造体をインクルード
#include "common/player.h" // Player struct
#include "common/player_input.h" // PlayerInput struct
#include "player_manager.h" // player_move function definition is needed

/**
 * @brief プレイヤーの入力を基に位置を更新します。
 * * @param player 更新するプレイヤー構造体への参照。
 * @param input クライアントから送信されたプレイヤー入力情報。
 * @param deltaTime 前のフレームからの経過時間。
 */
void player_update_position(Player &player, const PlayerInput &input, float deltaTime);

#endif // PLAYER_MOVEMENT_H