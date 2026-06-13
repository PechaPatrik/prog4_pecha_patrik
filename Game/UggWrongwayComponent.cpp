#include "UggWrongwayComponent.h"
#include "GameStateManager.h"
#include "Scene.h"
#include <cmath>
#include <algorithm>

namespace dae
{
    void UggWrongwayComponent::Update(float deltaTime)
    {
        if (GameStateManager::GetInstance().IsFrozen()) return;

        if (!m_initialized)
        {
            m_initialized = true;

            ResolveStartPosition();
            UpdateSprite();

            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : UGG_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : UGG_SRC_H;

            m_introTo = GetSidePos(m_gridRow, m_gridCol, srcW, srcH);

            static constexpr float SIN120 = 0.8660254f;
            static constexpr float COS120 = 0.5f;
            float dirX = m_isLeftSide ? SIN120 : -SIN120;
            float dirY = -COS120;

            // Walk backward along the approach direction until the start point is off-screen
            float margin = static_cast<float>(std::max(srcW, srcH)) * PIXEL_SCALE * 2.f;
            float maxDist = static_cast<float>(std::max(GameWindowW(), GameWindowH())) + margin;
            m_introFrom.x = m_introTo.x - dirX * maxDist;
            m_introFrom.y = m_introTo.y - dirY * maxDist;

            float dx = m_introTo.x - m_introFrom.x;
            float dy = m_introTo.y - m_introFrom.y;
            m_introLength = std::sqrt(dx * dx + dy * dy);
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
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : UGG_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : UGG_SRC_H;
            float margin = static_cast<float>(std::max(srcW, srcH)) * PIXEL_SCALE * 2.f;

            m_fallSpeed += m_fallGravity * deltaTime;
            m_fallPos += m_fallDir * m_fallSpeed * deltaTime;
            GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);

            float ww = static_cast<float>(GameWindowW());
            float wh = static_cast<float>(GameWindowH());
            if (m_fallPos.x < -margin || m_fallPos.x > ww + margin ||
                m_fallPos.y > wh + margin || m_fallPos.y < -margin)
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