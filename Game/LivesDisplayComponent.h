#pragma once
#include "Component.h"
#include "IObserver.h"
#include "TextComponent.h"
#include <string>

namespace dae
{
    class LivesDisplayComponent final : public Component, public IObserver
    {
    public:
        LivesDisplayComponent(GameObject* pOwner, int playerIndex, int startingLives = 3)
            : Component(pOwner)
            , m_playerIndex(playerIndex)
            , m_lives(startingLives)
        {
        }

        void Update(float) override
        {
            if (!m_dirty) return;
            if (auto* tc = GetOwner()->GetComponent<TextComponent>())
                tc->SetText("P" + std::to_string(m_playerIndex + 1) + " Lives: " + std::to_string(m_lives));
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
    };
}