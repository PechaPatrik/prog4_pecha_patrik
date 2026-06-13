#include "CoilyComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include "ServiceLocator.h"
#include "SoundId.h"
#include <cmath>
#include <cstdlib>

namespace dae
{
    static const int C_dRow[4] = { -1, -1, 1, 1 };
    static const int C_dCol[4] = { 0, -1, 1, 0 };

    void CoilyComponent::ForceJumpOff()
    {
        if (m_fallingOff || m_introFalling) return;
        for (int dir = 2; dir <= 3; ++dir)
        {
            int nr = m_gridRow + C_dRow[dir];
            int nc = m_gridCol + C_dCol[dir];
            if (!m_grid || !m_grid->IsValid(nr, nc))
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

        m_fromPos = m_grid
            ? GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH)
            : glm::vec2{ 0.f, 0.f };
        int offRow = m_gridRow + dRow;
        int offCol = m_gridCol + dCol;
        m_toPos = m_grid
            ? GridToCharacterPos(offRow, offCol, srcW, srcH)
            : glm::vec2{ 0.f, 0.f };
        m_destRow = offRow;
        m_destCol = offCol;
        m_hopDuration = m_hopInterval * 0.5f;
        if (m_hopDuration <= 0.f) m_hopDuration = 0.001f;
        m_hopPhase = 0.f;
        m_hopping = true;
        m_fallingOff = true;
        ServiceLocator::GetSoundSystem().PlaySound(SoundId::CoilyFall);
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

            m_introTo = m_grid
                ? GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH)
                : glm::vec2{ 0.f, 0.f };

            // Start just above the window top
            float textureH = static_cast<float>(srcH) * PIXEL_SCALE;
            m_introFrom = { m_introTo.x, -textureH };

            m_introLength = m_introTo.y - m_introFrom.y;
            if (m_introLength <= 0.f) m_introLength = 0.001f;
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

        if (m_fallingOff && !m_hopping)
        {
            m_fallSpeed += m_fallGravity * deltaTime;
            m_fallPos.y += m_fallSpeed * deltaTime;
            GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);

            if (m_fallPos.y > static_cast<float>(GameWindowH()) + 64.f)
            {
                if (m_onFellOff) m_onFellOff();
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

                // Player-controlled Coily hopped off the board: award points and fall
                if (m_playerControlled && m_grid && !m_grid->IsValid(m_gridRow, m_gridCol))
                {
                    m_isDoingDiscChase = true;
                    GameStateManager::GetInstance().OnCoilyFellDuringDisc(m_gridRow, m_gridCol);
                    if (m_scene)
                        m_scene->MoveToBack(GetOwner());
                    glm::vec2 wp = GetOwner()->GetWorldPosition();
                    m_fallPos = { wp.x, wp.y };
                    m_fallSpeed = 0.f;
                    m_fallingOff = true;
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
                    m_isDoingDiscChase = true;

                    // Award points immediately when the fall hop begins
                    GameStateManager::GetInstance().OnCoilyFellDuringDisc(m_gridRow, m_gridCol);

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