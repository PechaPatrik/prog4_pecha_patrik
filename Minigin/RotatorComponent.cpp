#include "RotatorComponent.h"
#include "GameObject.h"
#include <cmath>

using namespace dae;

RotatorComponent::RotatorComponent(GameObject* pOwner, float radius, float angularSpeed, const glm::vec2& center)
    : Component(pOwner)
    , m_radius{ radius }
    , m_angularSpeed{ angularSpeed }
    , m_center{ center }
{
}

void RotatorComponent::Update(float deltaTime)
{
    m_angle += m_angularSpeed * deltaTime;

    float x = m_center.x + std::cos(m_angle) * m_radius;
    float y = m_center.y + std::sin(m_angle) * m_radius;

    GetOwner()->SetLocalPosition(x, y);
}