#pragma once
#include "Component.h"
#include "IObserver.h"
#include "ImageComponent.h"
#include <array>

namespace dae
{
    static constexpr int MAX_LIVES = 3;

    class LivesDisplayComponent final : public Component, public IObserver
    {
    public:
        LivesDisplayComponent(GameObject* pOwner, int playerIndex, int startingLives = MAX_LIVES)
            : Component(pOwner)
            , m_playerIndex(playerIndex)
            , m_lives(startingLives)
        {
        }

        void SetHeart(int index, ImageComponent* heart)
        {
            if (index >= 0 && index < MAX_LIVES)
                m_hearts[index] = heart;
        }

        void Update(float) override
        {
            if (!m_dirty) return;
            for (int i = 0; i < MAX_LIVES; ++i)
                if (m_hearts[i])
                    m_hearts[i]->SetVisible(i < m_lives);
            m_dirty = false;
        }

        void OnNotify(GameEvent event, int value) override
        {
            if (event == GameEvent::LivesChanged)
            {
                m_lives = value;
                m_dirty = true;
            }
        }

    private:
        int m_playerIndex;
        int m_lives;
        bool m_dirty{ false };
        std::array<ImageComponent*, MAX_LIVES> m_hearts{ nullptr, nullptr, nullptr };
    };
}