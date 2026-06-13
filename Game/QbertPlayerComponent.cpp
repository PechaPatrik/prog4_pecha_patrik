#include "QbertPlayerComponent.h"
#include "DiscComponent.h"
#include "GameStateManager.h"
#include "Scene.h"

namespace dae
{
    void QbertPlayerComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;
        if (m_dead) return;

        if (m_onDisc)
        {
            m_discRideTimer += deltaTime;

            if (m_discRideTimer <= m_discPhase1Duration)
            {
                // Phase 1: disc follows Q*bert upward (disc handles its own position in Update)
                float t1 = m_discRideTimer / m_discPhase1Duration;
                float x = m_discStartPos.x + (m_discHoverPos.x - m_discStartPos.x) * t1;
                float y = m_discStartPos.y + (m_discHoverPos.y - m_discStartPos.y) * t1;
                GetOwner()->SetLocalPosition(x, y);
            }
            else if (m_discRideTimer <= m_discPhase1Duration + m_discPhase2Duration)
            {
                // Phase 2: hover above 0,0, waiting
                GetOwner()->SetLocalPosition(m_discHoverPos.x, m_discHoverPos.y);
            }
            else
            {
                // Phase 3: drop onto tile 0,0; disc despawns at the start of this phase
                if (!m_discDespawned)
                {
                    m_discDespawned = true;
                    if (m_disc)
                    {
                        m_disc->Despawn();
                        m_disc = nullptr;
                    }
                }
                float t3 = (m_discRideTimer - m_discPhase1Duration - m_discPhase2Duration)
                    / m_discDropDuration;
                if (t3 > 1.f) t3 = 1.f;
                float x = m_discHoverPos.x + (m_discDropPos.x - m_discHoverPos.x) * t3;
                float y = m_discHoverPos.y + (m_discDropPos.y - m_discHoverPos.y) * t3;
                GetOwner()->SetLocalPosition(x, y);
            }

            return;
        }

        if (m_hopping)
        {
            m_hopPhase += deltaTime / m_hopDuration;
            if (m_hopPhase >= 1.f)
            {
                m_hopPhase = 1.f;
                m_hopping = false;
                ApplyArcPosition(1.f);

                if (m_hopOffEdge)
                {
                    for (auto* disc : m_discs)
                    {
                        if (disc && disc->CheckLanded(this, m_destRow, m_destCol))
                        {
                            m_hopOffEdge = false;
                            return;
                        }
                    }
                    glm::vec2 worldPos = GetDeathWorldPos();
                    TriggerDeath();
                    GameStateManager::GetInstance().TriggerPlayerDeath(
                        this, m_scene, worldPos.x, worldPos.y, m_freezeDuration);
                }
                else
                {
                    m_gridRow = m_destRow;
                    m_gridCol = m_destCol;
                    ApplyArcPosition(1.f);
                    OnLanded();
                }
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

        bool outOfBounds = m_grid ? !m_grid->IsValid(newRow, newCol) : true;
        if (outOfBounds)
        {
            BeginHopOffEdge(newRow, newCol, dir);
            return;
        }

        BeginHop(newRow, newCol, dir);
    }

    void QbertPlayerComponent::OnLanded()
    {
        if (m_grid)
        {
            auto* cube = m_grid->GetCube(m_gridRow, m_gridCol);
            if (cube)
            {
                bool wasTarget = cube->IsTarget();
                cube->Step();
                if (!wasTarget && cube->IsTarget())
                    AddScore(m_pointsPerCubeChange);
            }
        }
        m_subject.NotifyObservers(GameEvent::PlayerMoved, m_playerIndex);

        m_respawnRow = m_gridRow;
        m_respawnCol = m_gridCol;

        if (m_scene)
            GameStateManager::GetInstance().CheckPlayerLandedAt(
                this, m_gridRow, m_gridCol, m_scene, m_freezeDuration);
    }
}