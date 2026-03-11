#include "MoveComponent.h"
#include "GameObject.h"

namespace dae
{
	MoveComponent::MoveComponent(GameObject* pOwner, float speed)
		: Component(pOwner)
		, m_speed(speed)
	{
	}

	void MoveComponent::Update(float deltaTime)
	{
		if (m_direction.x == 0.f && m_direction.y == 0.f)
			return;

		auto* owner = GetOwner();
		const glm::vec3 currentPos = owner->GetWorldPosition();

		glm::vec2 dir = m_direction;
		float len = glm::length(dir);
		if (len > 0.f)
			dir /= len;

		owner->SetLocalPosition(
			currentPos.x + dir.x * m_speed * deltaTime,
			currentPos.y + dir.y * m_speed * deltaTime
		);

		m_direction = {};
	}
}