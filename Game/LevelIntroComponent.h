#pragma once
#include "Component.h"
#include "GameObject.h"

namespace dae
{
    class LevelIntroComponent final : public Component
    {
    public:
        LevelIntroComponent(GameObject* pOwner, int levelIndex, float duration)
            : Component(pOwner)
            , m_levelIndex(levelIndex)
            , m_duration(duration)
        {
        }

        void Update(float deltaTime) override;

    private:
        int m_levelIndex;
        float m_duration;
        float m_timer{ 0.f };
        bool m_done{ false };
        bool m_soundPlayed{ false };
    };
}