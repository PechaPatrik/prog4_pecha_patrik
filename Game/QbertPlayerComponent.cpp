#include "QbertPlayerComponent.h"
#include "GameStateManager.h"
#include "Scene.h"

namespace dae
{
    void QbertPlayerComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;
        if (m_dead) return;

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
                    // Hop animation finished going off the edge: trigger death now
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

        bool outOfBounds = (newRow < 0 || newRow >= PYRAMID_ROWS || newCol < 0 || newCol > newRow);
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

        // Save current position as valid respawn point (landed safely on pyramid)
        m_respawnRow = m_gridRow;
        m_respawnCol = m_gridCol;

        if (m_scene)
            GameStateManager::GetInstance().CheckPlayerLandedAt(
                this, m_gridRow, m_gridCol, m_scene, m_freezeDuration);
    }
}
