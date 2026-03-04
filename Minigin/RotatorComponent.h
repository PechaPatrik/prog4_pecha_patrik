#pragma once
#include "Component.h"
#include <glm/glm.hpp>

namespace dae
{
    class RotatorComponent final : public Component
    {
    public:
        RotatorComponent(GameObject* pOwner, float radius, float angularSpeed);
        ~RotatorComponent() override = default;

        RotatorComponent(const RotatorComponent& other) = delete;
        RotatorComponent(RotatorComponent&& other) = delete;
        RotatorComponent& operator=(const RotatorComponent& other) = delete;
        RotatorComponent& operator=(RotatorComponent&& other) = delete;

        void Update(float deltaTime) override;

    private:
        float m_radius{};
        float m_angularSpeed{};
        float m_angle{ 0.f };
    };
}