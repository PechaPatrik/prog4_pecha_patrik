#pragma once
#include "Component.h"
#include "GameObject.h"
#include "Subject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include <vector>
#include <cmath>

namespace dae
{
    static constexpr float QBERT_ARC_HEIGHT = 12.f * PIXEL_SCALE;
    static constexpr float DISC_DROP_DURATION = 0.25f;

    // direction: 0=up-right(W), 1=up-left(A), 2=down-right(D), 3=down-left(S), made to match spritesheet columns
    static const int QbertDRow[4] = { -1, -1,  1,  1 };
    static const int QbertDCol[4] = { 0, -1,  1,  0 };

    class Scene;
    class DiscComponent;

    class QbertPlayerComponent final : public Component
    {
    public:
        QbertPlayerComponent(GameObject* pOwner, int playerIndex = 0,
            int startRow = 0, int startCol = 0, int maxLives = 3)
            : Component(pOwner)
            , m_playerIndex(playerIndex)
            , m_gridRow(startRow)
            , m_gridCol(startCol)
            , m_respawnRow(startRow)
            , m_respawnCol(startCol)
            , m_lives(maxLives)
        {
        }

        ~QbertPlayerComponent() override = default;

        QbertPlayerComponent(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent(QbertPlayerComponent&&) = delete;
        QbertPlayerComponent& operator=(const QbertPlayerComponent&) = delete;
        QbertPlayerComponent& operator=(QbertPlayerComponent&&) = delete;

        void SetPyramidGrid(PyramidGrid* grid) { m_grid = grid; }
        void SetScene(Scene* scene) { m_scene = scene; }
        void SetFreezeDuration(float d) { m_freezeDuration = d; }
        void SetPointsPerCubeChange(int pts) { m_pointsPerCubeChange = pts; }
        void SetPointsSlickSam(int pts) { m_pointsSlickSam = pts; }

        void Update(float deltaTime) override;

        void RequestMove(int direction)
        {
            if (m_hopping || m_dead || m_onDisc) return;
            m_pendingDirection = direction;
            m_hasPendingMove = true;
        }

        bool IsHopping() const { return m_hopping; }
        bool IsDead() const { return m_dead; }
        bool IsOnDisc() const { return m_onDisc; }
        int GetPlayerIndex() const { return m_playerIndex; }
        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }
        int GetLives() const { return m_lives; }
        int GetScore() const { return m_score; }

        // World position to use for placing the curse image and for respawn
        glm::vec2 GetDeathWorldPos() const
        {
            glm::vec2 wp = const_cast<GameObject*>(GetOwner())->GetWorldPosition();
            return { wp.x, wp.y };
        }

        void Respawn()
        {
            m_dead = false;
            m_gridRow = m_respawnRow;
            m_gridCol = m_respawnCol;
            SnapToGrid();
        }

        void TriggerDeath()
        {
            if (m_dead) return;
            m_dead = true;
            if (m_lives > 0) --m_lives;
            m_subject.NotifyObservers(GameEvent::LivesChanged, m_lives);
            if (m_lives == 0)
                m_subject.NotifyObservers(GameEvent::PlayerDied, m_playerIndex);
        }

        void LandFromDisc()
        {
            m_onDisc = false;
            m_discRiding = false;
            m_disc = nullptr;
            m_gridRow = 0;
            m_gridCol = 0;
            m_respawnRow = 0;
            m_respawnCol = 0;
            SnapToGrid();
            if (m_grid)
            {
                auto* cube = m_grid->GetCube(0, 0);
                if (cube)
                {
                    bool wasTarget = cube->IsTarget();
                    cube->Step();
                    if (!wasTarget && cube->IsTarget())
                        AddScore(m_pointsPerCubeChange);
                }
            }
            m_subject.NotifyObservers(GameEvent::PlayerMoved, m_playerIndex);
        }

        void StartDiscRide(glm::vec2 playerOnDiscPos, glm::vec2 hoverPos,
            float flightDuration, DiscComponent* disc)
        {
            m_onDisc = true;
            m_discRiding = true;
            m_discRideTimer = 0.f;
            m_discDespawned = false;
            m_disc = disc;

            m_discPhase1Duration = flightDuration * 0.5f;
            m_discPhase2Duration = flightDuration * 0.5f;

            m_discStartPos = playerOnDiscPos;
            m_discHoverPos = hoverPos;

            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 17;
            int srcH = sheet ? sheet->GetFrameHeight() : 16;
            m_discDropPos = GridToCharacterPos(0, 0, srcW, srcH);

            GetOwner()->SetLocalPosition(playerOnDiscPos.x, playerOnDiscPos.y);
        }

        void OnCaughtSlickSam()
        {
            AddScore(m_pointsSlickSam);
        }

        void AddScore(int points)
        {
            m_score += points;
            m_subject.NotifyObservers(GameEvent::ScoreChanged, m_score);
        }

        void AddObserver(IObserver* o) { m_subject.AddObserver(o); }
        void RemoveObserver(IObserver* o) { m_subject.RemoveObserver(o); }

        void RegisterDisc(DiscComponent* disc) { m_discs.push_back(disc); }
        void ClearDiscs() { m_discs.clear(); }

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
            m_hopOffEdge = false;
        }

        void BeginHopOffEdge(int destRow, int destCol, int dir)
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
            m_hopOffEdge = true;
            m_respawnRow = m_gridRow;
            m_respawnCol = m_gridCol;
        }

        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arcY = -QBERT_ARC_HEIGHT * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void OnLanded();

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

        Scene* m_scene{ nullptr };
        PyramidGrid* m_grid{ nullptr };

        int m_playerIndex;
        int m_gridRow;
        int m_gridCol;
        int m_respawnRow;
        int m_respawnCol;
        int m_lives;
        int m_score{ 0 };
        int m_pointsPerCubeChange{ 25 };
        int m_pointsSlickSam{ 300 };
        float m_freezeDuration{ 1.5f };
        Subject m_subject;

        int m_pendingDirection{ 0 };
        bool m_hasPendingMove{ false };
        bool m_dead{ false };

        bool m_hopping{ false };
        bool m_hopOffEdge{ false };
        float m_hopPhase{ 0.f };
        float m_hopDuration{ 0.3f };
        glm::vec2 m_fromPos{ 0.f, 0.f };
        glm::vec2 m_toPos{ 0.f, 0.f };
        int m_destRow{ 0 };
        int m_destCol{ 0 };
        int m_lastDir{ 2 };

        bool m_onDisc{ false };
        bool m_discRiding{ false };
        bool m_discDespawned{ false };
        float m_discRideTimer{ 0.f };
        float m_discPhase1Duration{ 1.f };
        float m_discPhase2Duration{ 1.f };
        glm::vec2 m_discStartPos{ 0.f, 0.f };
        glm::vec2 m_discHoverPos{ 0.f, 0.f };
        glm::vec2 m_discDropPos{ 0.f, 0.f };
        DiscComponent* m_disc{ nullptr };

        std::vector<DiscComponent*> m_discs;
    };
}