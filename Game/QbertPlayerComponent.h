#pragma once
#include "Component.h"
#include "GameObject.h"
#include "Subject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include <cmath>

namespace dae
{
    static constexpr float QBERT_ARC_HEIGHT = 12.f * PIXEL_SCALE;

    // direction: 0=up-right(W), 1=up-left(A), 2=down-right(D), 3=down-left(S), made to match spritesheet columns
    static const int QbertDRow[4] = { -1, -1,  1,  1 };
    static const int QbertDCol[4] = { 0, -1,  1,  0 };

    class QbertPlayerComponent final : public Component
    {
    public:
        QbertPlayerComponent(GameObject* pOwner, int startRow = 0, int startCol = 0, int maxLives = 3)
            : Component(pOwner)
            , m_gridRow(startRow)
            , m_gridCol(startCol)
            , m_lives(maxLives)
        {
        }

        ~QbertPlayerComponent() override = default;

        QbertPlayerComponent(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent(QbertPlayerComponent&&) = delete;
        QbertPlayerComponent& operator=(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent& operator=(QbertPlayerComponent&&) = delete;

        void SetPyramidGrid(PyramidGrid* grid) { m_grid = grid; }

        void Update(float deltaTime) override
        {
            if (m_hopping)
            {
                m_hopPhase += deltaTime / m_hopDuration;
                if (m_hopPhase >= 1.f)
                {
                    m_hopPhase = 1.f;
                    m_hopping = false;
                    m_gridRow = m_destRow;
                    m_gridCol = m_destCol;
                    ApplyArcPosition(1.f);
                    OnLanded();
                }
                else
                {
                    ApplyArcPosition(m_hopPhase);
                }
                UpdateSprite(m_lastDir);
                return;
            }

            if (!m_hasPendingMove) return;
            m_hasPendingMove = false;

            int dir = m_pendingDirection;
            int newRow = m_gridRow + QbertDRow[dir];
            int newCol = m_gridCol + QbertDCol[dir];

            m_lastDir = dir;
            UpdateSprite(dir);

            if (newRow < 0 || newRow > 6 || newCol < 0 || newCol > newRow)
            {
                LoseLife();
                return;
            }

            BeginHop(newRow, newCol, dir);
        }

        void RequestMove(int direction)
        {
            if (m_hopping) return;
            m_pendingDirection = direction;
            m_hasPendingMove = true;
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
            m_gridRow = 0;
            m_gridCol = 0;
            SnapToGrid();
        }

        void AddObserver(IObserver* o) { m_subject.AddObserver(o); }
        void RemoveObserver(IObserver* o) { m_subject.RemoveObserver(o); }

    private:
        void BeginHop(int destRow, int destCol, int dir)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 17;
            int srcH = sheet ? sheet->GetFrameHeight() : 16;
            m_fromPos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            m_toPos = GridToCharacterPos(destRow, destCol, srcW, srcH);
            m_destRow = destRow;
            m_destCol = destCol;
            m_lastDir = dir;
            m_hopDuration = 0.3f;
            m_hopPhase = 0.f;
            m_hopping = true;
        }

        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arcY = -QBERT_ARC_HEIGHT * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void OnLanded()
        {
            if (m_grid)
            {
                auto* cube = m_grid->GetCube(m_gridRow, m_gridCol);
                if (cube)
                {
                    bool wasTarget = cube->IsTarget();
                    cube->Step();
                    if (!wasTarget && cube->IsTarget())
                        AddScore(25);
                }
            }
            m_subject.NotifyObservers(GameEvent::PlayerMoved, 0);
        }

        void UpdateSprite(int dir)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(dir, 0);
        }

        void SnapToGrid()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 17;
            int srcH = sheet ? sheet->GetFrameHeight() : 16;
            glm::vec2 pos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            GetOwner()->SetLocalPosition(pos.x, pos.y);
        }

        PyramidGrid* m_grid{ nullptr };

        int m_gridRow;
        int m_gridCol;
        int m_lives;
        int m_score{ 0 };
        Subject m_subject;

        int m_pendingDirection{ 0 };
        bool m_hasPendingMove{ false };

        bool m_hopping{ false };
        float m_hopPhase{ 0.f };
        float m_hopDuration{ 0.3f };
        glm::vec2 m_fromPos{ 0.f, 0.f };
        glm::vec2 m_toPos{ 0.f, 0.f };
        int m_destRow{ 0 };
        int m_destCol{ 0 };
        int m_lastDir{ 2 };
    };
}