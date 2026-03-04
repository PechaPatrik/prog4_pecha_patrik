#include "RotatorComponent.h"
#include "GameObject.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

using namespace dae;

RotatorComponent::RotatorComponent(GameObject* pOwner, float radius, float angularSpeed)
    : Component(pOwner)
    , m_radius{ radius }
    , m_angularSpeed{ angularSpeed }
{
}

void RotatorComponent::Update(float deltaTime)
{
    m_angle += m_angularSpeed * deltaTime;
    m_angle = std::fmod(m_angle, 2.f * glm::pi<float>());

    float x = std::cos(m_angle) * m_radius;
    float y = std::sin(m_angle) * m_radius;

    GetOwner()->SetLocalPosition(x, y);
}