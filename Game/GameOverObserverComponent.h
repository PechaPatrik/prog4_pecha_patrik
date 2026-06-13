#pragma once
#include "Component.h"
#include "GameObject.h"
#include "IObserver.h"

namespace dae
{
    class GameOverObserverComponent final : public Component, public IObserver
    {
    public:
        GameOverObserverComponent(GameObject* pOwner, int playerCount, bool anyDeath = false)
            : Component(pOwner)
            , m_playerCount(playerCount)
            , m_anyDeath(anyDeath)
        {
        }

        void Update(float) override {}

        void OnNotify(GameEvent event, int /*value*/) override
        {
            if (event != GameEvent::PlayerDied) return;
            ++m_deadCount;
        }

        bool AllPlayersDead() const
        {
            if (m_anyDeath)
                return m_deadCount >= 1;
            return m_deadCount >= m_playerCount;
        }

    private:
        int m_playerCount{ 1 };
        int m_deadCount{ 0 };
        bool m_anyDeath{ false };
    };
}