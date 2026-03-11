#pragma once
#include "Component.h"
#include "ThrashTheCache.h"
#include <imgui.h>

namespace dae
{
    class ThrashTheCacheComponent final : public Component
    {
    public:
        explicit ThrashTheCacheComponent(GameObject* pOwner)
            : Component(pOwner) {
        }

        ~ThrashTheCacheComponent() override = default;

        ThrashTheCacheComponent(const ThrashTheCacheComponent&) = delete;
        ThrashTheCacheComponent(ThrashTheCacheComponent&&) = delete;
        ThrashTheCacheComponent& operator=(const ThrashTheCacheComponent&) = delete;
        ThrashTheCacheComponent& operator=(ThrashTheCacheComponent&&) = delete;

        void Update(float) override {}

        void Render() const override
        {
            ImGui::Begin("Thrash The Cache");
            m_thrashTheCache.Render();
            ImGui::End();
        }

    private:
        mutable ThrashTheCache m_thrashTheCache{};
    };
}