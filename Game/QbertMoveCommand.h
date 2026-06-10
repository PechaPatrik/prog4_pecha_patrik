#pragma once
#include "Command.h"
#include "QbertPlayerComponent.h"

namespace dae
{
    class QbertMoveCommand final : public Command
    {
    public:
        QbertMoveCommand(QbertPlayerComponent* player, int direction)
            : m_player(player)
            , m_direction(direction)
        {
        }

        void Execute() override
        {
            if (m_player)
                m_player->RequestMove(m_direction);
        }

    private:
        QbertPlayerComponent* m_player;
        int m_direction;
    };
}
