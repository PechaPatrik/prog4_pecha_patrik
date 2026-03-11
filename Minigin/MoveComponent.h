#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace dae
{
	class MoveComponent final : public Component
	{
	public:
		explicit MoveComponent(GameObject* pOwner, float speed = 100.f);
		~MoveComponent() override = default;

		MoveComponent(const MoveComponent&) = delete;
		MoveComponent(MoveComponent&&) = delete;
		MoveComponent& operator=(const MoveComponent&) = delete;
		MoveComponent& operator=(MoveComponent&&) = delete;

		void Update(float deltaTime) override;

		void AddDirection(const glm::vec2& dir) { m_direction += dir; }
		float GetSpeed() const { return m_speed; }

	private:
		glm::vec2 m_direction{};
		float m_speed;
	};
}