#pragma once
#include "Component.h"
#include "GameObject.h"
#include "QbertPyramid.h"
#include "QbertPlayerComponent.h"
#include "GameStateManager.h"
#include "SpritesheetComponent.h"
#include <vector>

namespace dae
{
    static constexpr int DISC_SRC_W = 16;
    static constexpr int DISC_SRC_H = 10;
    // Each color group occupies 5 columns but only the first 4 are the animation frames
    static constexpr int DISC_ANIM_FRAMES = 4;
    static constexpr int DISC_GROUP_STRIDE = 5;
    static constexpr float DISC_FRAME_DURATION = 0.12f;

    inline glm::vec2 DiscWorldPos(int row, int col)
    {
        float discW = static_cast<float>(DISC_SRC_W) * PIXEL_SCALE;
        float discH = static_cast<float>(DISC_SRC_H) * PIXEL_SCALE;
        glm::vec2 cubePos = GridToScreen(row, col);
        float cubeCenterX = static_cast<float>(CUBE_SRC_W) * PIXEL_SCALE * 0.5f;
        float cubeTopFaceY = static_cast<float>(CUBE_SRC_H) * PIXEL_SCALE * 0.25f;
        float centerX = cubePos.x + cubeCenterX;
        float centerY = cubePos.y + cubeTopFaceY;
        return { centerX - discW * 0.5f, centerY - discH * 0.5f };
    }

    class Scene;

    class DiscComponent final : public Component
    {
    public:
        DiscComponent(GameObject* pOwner, int row, int col, int colorGroupIndex,
            float flightDuration, int pointsCoilyDisc, float freezeDuration)
            : Component(pOwner)
            , m_row(row)
            , m_col(col)
            , m_firstFrame(colorGroupIndex* DISC_GROUP_STRIDE)
            , m_flightDuration(flightDuration)
            , m_pointsCoilyDisc(pointsCoilyDisc)
            , m_freezeDuration(freezeDuration)
        {
        }

        ~DiscComponent() override = default;

        DiscComponent(const DiscComponent&) = delete;
        DiscComponent(DiscComponent&&) = delete;
        DiscComponent& operator=(const DiscComponent&) = delete;
        DiscComponent& operator=(DiscComponent&&) = delete;

        void SetScene(Scene* scene) { m_scene = scene; }

        void Update(float deltaTime) override
        {
            if (m_done) return;

            // Always animate regardless of whether we are riding or idle
            m_frameTimer += deltaTime;
            if (m_frameTimer >= DISC_FRAME_DURATION)
            {
                m_frameTimer -= DISC_FRAME_DURATION;
                m_currentFrame = (m_currentFrame + 1) % DISC_ANIM_FRAMES;
                auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
                if (sheet)
                    sheet->SetFrame(m_firstFrame + m_currentFrame, 0);
            }

            // During phase 1 of the ride, follow Q*bert's position
            if (m_riding && m_rider)
            {
                glm::vec2 riderPos = m_rider->GetDeathWorldPos();
                float discW = static_cast<float>(DISC_SRC_W) * PIXEL_SCALE;
                float discH = static_cast<float>(DISC_SRC_H) * PIXEL_SCALE;
                // Center the disc under Q*bert: disc center-x matches rider center-x,
                // disc top sits just below rider bottom
                int riderSrcW = 17;
                int riderSrcH = 16;
                float riderW = static_cast<float>(riderSrcW) * PIXEL_SCALE;
                float riderH = static_cast<float>(riderSrcH) * PIXEL_SCALE;
                float discX = riderPos.x + riderW * 0.5f - discW * 0.5f;
                float discY = riderPos.y + riderH - discH * 0.5f;
                GetOwner()->SetLocalPosition(discX, discY);
            }
        }

        // Called after the hop animation completes and Q*bert lands on this disc's tile.
        bool CheckLanded(QbertPlayerComponent* player, int landedRow, int landedCol)
        {
            if (m_riding || m_done) return false;
            if (landedRow != m_row || landedCol != m_col) return false;

            m_riding = true;
            m_rider = player;

            glm::vec2 playerStart = player->GetDeathWorldPos();

            glm::vec2 apex = GridToScreen(0, 0);
            glm::vec2 hoverPos = {
                GridToCharacterPos(0, 0, 17, 16).x,
                apex.y - static_cast<float>(CUBE_SRC_H) * PIXEL_SCALE
            };

            player->StartDiscRide(playerStart, hoverPos, m_flightDuration, this);

            GameStateManager::GetInstance().TriggerDiscRide(
                player, m_scene,
                m_row, m_col,
                m_flightDuration,
                m_pointsCoilyDisc,
                m_freezeDuration);

            return true;
        }

        // Called by QbertPlayerComponent at the start of the hover phase to stop following
        void StopRiding() { m_riding = false; m_rider = nullptr; }

        // Called by QbertPlayerComponent to despawn the disc (marks for removal)
        void Despawn()
        {
            m_done = true;
            m_riding = false;
            m_rider = nullptr;
            GetOwner()->MarkForRemoval();
        }

        int GetRow() const { return m_row; }
        int GetCol() const { return m_col; }

    private:
        int m_row;
        int m_col;
        int m_firstFrame;
        float m_flightDuration;
        int m_pointsCoilyDisc;
        float m_freezeDuration;
        Scene* m_scene{ nullptr };

        int m_currentFrame{ 0 };
        float m_frameTimer{ 0.f };
        bool m_riding{ false };
        bool m_done{ false };
        QbertPlayerComponent* m_rider{ nullptr };
    };
}