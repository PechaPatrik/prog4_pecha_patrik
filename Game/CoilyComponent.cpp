#include "CoilyComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include <cmath>

namespace dae
{
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
            // Start above the screen, same X as spawn tile
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

                // Check collision on landing at spawn tile
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
            auto newState = m_state->Update(deltaTime, *this);
            if (newState)
                m_state = std::move(newState);
        }

        UpdateSprite();
    }
}