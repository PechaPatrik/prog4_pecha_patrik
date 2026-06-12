#include "UggWrongwayComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include <cmath>

namespace dae
{
    void UggWrongwayComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;

        if (!m_initialized)
        {
            m_initialized = true;
            UpdateSprite();

            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : UGG_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : UGG_SRC_H;

            m_introTo = GetSidePos(m_gridRow, m_gridCol, srcW, srcH);

            // 120 degrees CW from screen-up for right side, 120 degrees CCW for left side
            static constexpr float SIN120 = 0.8660254f;
            static constexpr float COS120 = 0.5f;
            float dirX = m_isLeftSide ? SIN120 : -SIN120;
            float dirY = -COS120;

            // Place start far enough off-screen along that direction
            static constexpr float INTRO_OFFSCREEN_DIST = 400.f;
            m_introFrom.x = m_introTo.x - dirX * INTRO_OFFSCREEN_DIST;
            m_introFrom.y = m_introTo.y - dirY * INTRO_OFFSCREEN_DIST;

            float dx = m_introTo.x - m_introFrom.x;
            float dy = m_introTo.y - m_introFrom.y;
            m_introLength = std::sqrt(dx * dx + dy * dy);
            m_introProgress = 0.f;

            GetOwner()->SetLocalPosition(m_introFrom.x, m_introFrom.y);
        }

        if (m_introFalling)
        {
            m_introProgress += UGG_INTRO_SPEED * deltaTime;
            if (m_introProgress >= m_introLength)
            {
                m_introFalling = false;
                GetOwner()->SetLocalPosition(m_introTo.x, m_introTo.y);

                auto& gsm = GameStateManager::GetInstance();
                for (auto& entry : gsm.GetEnemies())
                {
                    if (entry.component == static_cast<void*>(this))
                    {
                        int checkRow = m_gridRow + entry.collisionDRow;
                        int checkCol = m_gridCol + entry.collisionDCol;
                        gsm.CheckEnemyLandedAt(checkRow, checkCol, entry, m_scene, m_freezeDuration);
                        break;
                    }
                }

                if (m_pendingFall)
                {
                    m_pendingFall = false;
                    BeginFall();
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
            m_fallSpeed += UGG_GRAVITY * deltaTime;
            m_fallPos += m_fallDir * m_fallSpeed * deltaTime;
            GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);
            if (m_fallPos.x < -UGG_SRC_W * PIXEL_SCALE * 2.f ||
                m_fallPos.x > WINDOW_W + UGG_SRC_W * PIXEL_SCALE * 2.f ||
                m_fallPos.y > WINDOW_H + UGG_SRC_H * PIXEL_SCALE * 2.f ||
                m_fallPos.y < -UGG_SRC_H * PIXEL_SCALE * 2.f)
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
                        int checkRow = m_gridRow + entry.collisionDRow;
                        int checkCol = m_gridCol + entry.collisionDCol;
                        gsm.CheckEnemyLandedAt(checkRow, checkCol, entry, m_scene, m_freezeDuration);
                        break;
                    }
                }

                if (m_pendingFall)
                {
                    m_pendingFall = false;
                    BeginFall();
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