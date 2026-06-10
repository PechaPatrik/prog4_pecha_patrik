#pragma once
#include "Component.h"
#include "GameObject.h"
#include "Subject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"

namespace dae
{
    // direction: 0=up-right(W), 1=up-left(A), 2=down-right(D), 3=down-left(S), made to match spritesheet columns
    static const int QbertDRow[4] = { -1, -1,  1,  1 };
    static const int QbertDCol[4] = { 0, -1,  1,  0 };

    class QbertPlayerComponent final : public Component
    {
    public:
        QbertPlayerComponent(GameObject* pOwner, int startRow, int startCol, int maxLives = 3)
            : Component(pOwner)
            , m_gridRow(startRow)
            , m_gridCol(startCol)
            , m_lives(maxLives)
            , m_score(0)
        {
        }

        ~QbertPlayerComponent() override = default;

        QbertPlayerComponent(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent(QbertPlayerComponent&&) = delete;
        QbertPlayerComponent& operator=(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent& operator=(QbertPlayerComponent&&) = delete;

        void Update(float) override {}

        void RequestMove(int direction)
        {
            m_pendingDirection = direction;
            m_hasPendingMove = true;
        }

        // Called by the level system each frame after input is processed
        void ApplyPendingMove()
        {
            if (!m_hasPendingMove) return;
            m_hasPendingMove = false;

            int dir = m_pendingDirection;
            int newRow = m_gridRow + QbertDRow[dir];
            int newCol = m_gridCol + QbertDCol[dir];

            UpdateSprite(dir);

            // Fell off pyramid
            if (newRow < 0 || newRow > 6 || newCol < 0 || newCol > newRow)
            {
                LoseLife();
                return;
            }

            m_gridRow = newRow;
            m_gridCol = newCol;
            UpdateWorldPosition();
            m_subject.NotifyObservers(GameEvent::PlayerMoved, 0);
        }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }
        int GetLives() const { return m_lives; }
        int GetScore() const { return m_score; }

        void AddScore(int points)
        {
            m_score += points;
            m_subject.NotifyObservers(GameEvent::ScoreChanged, m_score);
        }

        void LoseLife()
        {
            if (m_lives <= 0) return;
            --m_lives;
            m_subject.NotifyObservers(GameEvent::LivesChanged, m_lives);
            if (m_lives == 0)
                m_subject.NotifyObservers(GameEvent::PlayerDied, 0);
            // Reset to top
            m_gridRow = 0;
            m_gridCol = 0;
            UpdateWorldPosition();
        }

        void AddObserver(IObserver* o) { m_subject.AddObserver(o); }
        void RemoveObserver(IObserver* o) { m_subject.RemoveObserver(o); }

    private:
        void UpdateSprite(int dir)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(dir, 0);
        }

        void UpdateWorldPosition()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 17;
            int srcH = sheet ? sheet->GetFrameHeight() : 16;
            glm::vec2 pos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            GetOwner()->SetLocalPosition(pos.x, pos.y);
        }

        int m_gridRow;
        int m_gridCol;
        int m_lives;
        int m_score;
        Subject m_subject;
        int m_pendingDirection{ 0 };
        bool m_hasPendingMove{ false };
    };
}