#include "SlickSamComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include <cmath>

namespace dae
{
    void SlickSamComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;

        if (!m_initialized)
        {
            m_initialized = true;
            UpdateSprite();

            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : SLICK_SAM_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : SLICK_SAM_SRC_H;

            m_introTo = m_grid
                ? GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH)
                : glm::vec2{ 0.f, 0.f };

            // Start just above the top of the window
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
                if (!GetOwner()->IsMarkedForRemoval())
                {
                    RevertCube();
                    if (m_pendingFall)
                    {
                        m_pendingFall = false;
                        BeginFall();
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
            return;
        }

        if (m_falling)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcH = sheet ? sheet->GetFrameHeight() : SLICK_SAM_SRC_H;
            float margin = static_cast<float>(srcH) * PIXEL_SCALE * 2.f;

            m_fallSpeed += m_fallGravity * deltaTime;
            m_fallPos.y += m_fallSpeed * deltaTime;
            GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);
            if (m_fallPos.y > static_cast<float>(GameWindowH()) + margin)
                GetOwner()->MarkForRemoval();
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

                if (!IsOnPyramid(m_destRow, m_destCol))
                {
                    BeginFall();
                    return;
                }

                m_gridRow = m_destRow;
                m_gridCol = m_destCol;

                auto& gsm = GameStateManager::GetInstance();
                for (auto& entry : gsm.GetEnemies())
                {
                    if (entry.component == static_cast<void*>(this))
                    {
                        gsm.CheckEnemyLandedAt(m_gridRow, m_gridCol, entry, m_scene, m_freezeDuration);
                        break;
                    }
                }
                if (!GetOwner()->IsMarkedForRemoval())
                {
                    RevertCube();
                    if (m_pendingFall)
                    {
                        m_pendingFall = false;
                        BeginFall();
                    }
                }
            }
            else
            {
                ApplyArcPosition(m_hopPhase);
            }
            return;
        }

        m_groundTimer += deltaTime;
        if (m_groundTimer < m_hopInterval) return;
        m_groundTimer = 0.f;

        ChooseAndBeginHop();
    }
}