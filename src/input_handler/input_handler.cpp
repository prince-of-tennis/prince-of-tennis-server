#include "input_handler.h"
#include "player/player_manneger.h"

void apply_player_input(Player &player, const PlayerInput &input, float deltaTime)
{
    float dx = 0.0f;
    float dy = 0.0f;

    if (input.right)
        dx += 1.0f;
    if (input.left)
        dx -= 1.0f;
    if (input.front)
        dy += 1.0f;
    if (input.back)
        dy -= 1.0f;

    // �ړ�����
    player_move(player, dx, dy, 0.0f, deltaTime);

    // ���P�b�g�U�菈��
    if (input.swing)
    {
        // TODO: �X�C���O�����{�[���Ƃ̏Փˏ���
    }
}
