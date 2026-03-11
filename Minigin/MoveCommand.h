#pragma once
#include "Command.h"
#include "MoveComponent.h"
#include <glm/glm.hpp>

namespace dae
{
	class MoveCommand final : public Command
	{
	public:
		MoveCommand(MoveComponent* pMoveComp, glm::vec2 direction)
			: m_pMoveComp(pMoveComp)
			, m_direction(direction)
		{
		}

		void Execute() override
		{
			if (m_pMoveComp)
				m_pMoveComp->AddDirection(m_direction);
		}

	private:
		MoveComponent* m_pMoveComp;
		glm::vec2 m_direction;
	};
}