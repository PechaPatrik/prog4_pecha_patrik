#include "CoilyComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include <cmath>
#include <cstdlib>

namespace dae
{
    static constexpr float COILY_FALL_GRAVITY = 800.f;

    static bool CoilyInBounds(int row, int col)
    {
        return row >= 0 && row <= 6 && col >= 0 && col <= row;
    }

    // hop direction table: 0=up-right, 1=up-left, 2=down-right, 3=down-left
    static const int C_dRow[4] = { -1, -1,  1,  1 };
    static const int C_dCol[4] = { 0, -1,  1,  0 };

    void CoilyComponent::ForceJumpOff()
    {
        if (m_fallingOff || m_introFalling) return;
        for (int dir = 2; dir <= 3; ++dir)
        {
            int nr = m_gridRow + C_dRow[dir];
            int nc = m_gridCol + C_dCol[dir];
            if (!CoilyInBounds(nr, nc))
            {
                BeginFallOff(C_dRow[dir], C_dCol[dir]);
                return;
            }
        }
        BeginFallOff(C_dRow[2], C_dCol[2]);
    }

    void CoilyComponent::BeginFallOff(int dRow, int dCol)
    {
        auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
        int srcW = sheet ? sheet->GetFrameWidth() : 16;
        int srcH = sheet ? sheet->GetFrameHeight() : 32;

        m_fromPos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
        int offRow = m_gridRow + dRow;
        int offCol = m_gridCol + dCol;
        m_toPos = GridToCharacterPos(offRow, offCol, srcW, srcH);
        m_destRow = offRow;
        m_destCol = offCol;
        m_hopDuration = m_hopInterval * 0.5f;
        m_hopPhase = 0.f;
        m_hopping = true;
        m_fallingOff = true;
    }

    void CoilyComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;

        if (!m_introInitialized)
        {
            m_introInitialized = true;

            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 16;
            int srcH = sheet ? sheet->GetFrameHeight() : 32;

            m_introTo = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            static constexpr float INTRO_ABOVE_DIST = 200.f;
            m_introFrom = { m_introTo.x, m_introTo.y - INTRO_ABOVE_DIST };

            float dy = m_introTo.y - m_introFrom.y;
            m_introLength = std::abs(dy);
            m_introProgress = 0.f;

            GetOwner()->SetLocalPosition(m_introFrom.x, m_introFrom.y);
        }

        if (m_introFalling)
        {
            m_introProgress += m_introSpeed * deltaTime;
            if (m_introProgress >= m_introLength)
            {
                m_introFalling = false;
                GetOwner()->SetLocalPosition(m_introTo.x, m_introTo.y);

                auto& gsm = GameStateManager::GetInstance();
                for (auto& entry : gsm.GetEnemies())
                {
                    if (entry.component == static_cast<void*>(this))
                    {
                        gsm.CheckEnemyLandedAt(m_gridRow, m_gridCol, entry, m_scene, m_freezeDuration);
                        break;
                    }
                }
            }
            else
            {
                float t = m_introProgress / m_introLength;
                float x = m_introFrom.x + (m_introTo.x - m_introFrom.x) * t;
                float y = m_introFrom.y + (m_introTo.y - m_introFrom.y) * t;
                GetOwner()->SetLocalPosition(x, y);
            }
            UpdateSprite();
            return;
        }

        // Gravity fall straight down after the off-edge hop completes
        if (m_fallingOff && !m_hopping)
        {
            m_fallSpeed += COILY_FALL_GRAVITY * deltaTime;
            m_fallPos.y += m_fallSpeed * deltaTime;
            GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);

            if (m_fallPos.y > WINDOW_H + 64.f)
            {
                if (m_awardDiscPointsOnFall)
                {
                    auto& gsm = GameStateManager::GetInstance();
                    if (gsm.IsDiscRiding())
                        gsm.OnCoilyFellDuringDisc(m_qbert);
                }
                GetOwner()->MarkForRemoval();
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
                m_gridRow = m_destRow;
                m_gridCol = m_destCol;
                ApplyArcPosition(1.f);

                if (m_fallingOff)
                {
                    glm::vec2 wp = GetOwner()->GetWorldPosition();
                    m_fallPos = { wp.x, wp.y };
                    m_fallSpeed = 0.f;
                    return;
                }

                auto newState = m_state->OnLanded(*this);
                if (newState)
                    m_state = std::move(newState);

                auto& gsm = GameStateManager::GetInstance();
                for (auto& entry : gsm.GetEnemies())
                {
                    if (entry.component == static_cast<void*>(this))
                    {
                        gsm.CheckEnemyLandedAt(m_gridRow, m_gridCol, entry, m_scene, m_freezeDuration);
                        break;
                    }
                }
            }
            else
            {
                ApplyArcPosition(m_hopPhase);
            }
        }
        else
        {
            if (m_hasDiscTarget && !m_state->IsEgg())
            {
                if (m_gridRow == m_discNeighbourRow && m_gridCol == m_discNeighbourCol)
                {
                    m_hasDiscTarget = false;
                    m_awardDiscPointsOnFall = true;
                    if (m_scene)
                        m_scene->MoveToBack(GetOwner());
                    BeginFallOff(m_discFinalDRow, m_discFinalDCol);
                }
                else
                {
                    auto newState = m_state->Update(deltaTime, *this);
                    if (newState)
                        m_state = std::move(newState);
                }
            }
            else
            {
                auto newState = m_state->Update(deltaTime, *this);
                if (newState)
                    m_state = std::move(newState);
            }
        }

        UpdateSprite();
    }
}