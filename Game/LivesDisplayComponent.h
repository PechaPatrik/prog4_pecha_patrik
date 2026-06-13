#pragma once
#include "Component.h"
#include "IObserver.h"
#include "ImageComponent.h"
#include <vector>

namespace dae
{
    class LivesDisplayComponent final : public Component, public IObserver
    {
    public:
        LivesDisplayComponent(GameObject* pOwner, int maxLives = 3)
            : Component(pOwner)
            , m_maxLives(maxLives)
            , m_lives(maxLives)
            , m_hearts(maxLives, nullptr)
        {
        }

        void SetHeart(int index, ImageComponent* heart)
        {
            if (index >= 0 && index < m_maxLives)
                m_hearts[index] = heart;
        }

        void Update(float) override
        {
            if (!m_dirty) return;
            for (int i = 0; i < m_maxLives; ++i)
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
        int m_maxLives;
        int m_lives;
        bool m_dirty{ false };
        std::vector<ImageComponent*> m_hearts;
    };
}