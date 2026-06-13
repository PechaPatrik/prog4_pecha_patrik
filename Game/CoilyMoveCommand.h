#pragma once
#include "Command.h"
#include "CoilyComponent.h"
#include <memory>

namespace dae
{
    class CoilyMoveCommand final : public Command
    {
    public:
        CoilyMoveCommand(CoilyComponent* coily, int direction)
            : m_aliveFlag(coily->GetAliveFlag())
            , m_coily(coily)
            , m_direction(direction)
        {
        }

        void Execute() override
        {
            if (!m_aliveFlag || !*m_aliveFlag) return;
            m_coily->QueueMove(m_direction);
        }

    private:
        std::shared_ptr<bool> m_aliveFlag;
        CoilyComponent* m_coily;
        int m_direction;
    };
}