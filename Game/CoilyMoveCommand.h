#pragma once
#include "Command.h"
#include "CoilyComponent.h"

namespace dae
{
    class CoilyMoveCommand final : public Command
    {
    public:
        CoilyMoveCommand(CoilyComponent* coily, int direction)
            : m_coily(coily)
            , m_direction(direction)
        {
        }

        void Execute() override
        {
            if (m_coily)
                m_coily->QueueMove(m_direction);
        }

    private:
        CoilyComponent* m_coily;
        int m_direction;
    };
}
