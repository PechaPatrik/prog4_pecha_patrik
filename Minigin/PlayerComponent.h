#pragma once
#include "Component.h"
#include "Subject.h"

namespace dae
{
    class PlayerComponent final : public Component
    {
    public:
        explicit PlayerComponent(GameObject* pOwner, int maxLives = 3)
            : Component(pOwner)
            , m_lives(maxLives)
            , m_score(0)
        {
        }

        void Update(float) override {}

        void AddObserver(IObserver* o) { m_subject.AddObserver(o); }
        void RemoveObserver(IObserver* o) { m_subject.RemoveObserver(o); }

        void LoseLife()
        {
            if (m_lives <= 0) return;
            --m_lives;
            m_subject.NotifyObservers(GameEvent::LivesChanged, m_lives);
            if (m_lives == 0)
            {
                m_subject.NotifyObservers(GameEvent::PlayerDied, 0);
                GetOwner()->MarkForRemoval();
            }
        }

        void AddScore(int points)
        {
            m_score += points;
            m_subject.NotifyObservers(GameEvent::ScoreChanged, m_score);
        }

        int GetLives() const { return m_lives; }
        int GetScore() const { return m_score; }

    private:
        int m_lives;
        int m_score;
        Subject m_subject;
    };
}