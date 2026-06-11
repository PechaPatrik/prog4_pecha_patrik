#pragma once
#include "Component.h"
#include "GameObject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include "QbertPlayerComponent.h"
#include <cmath>
#include <cstdlib>

namespace dae
{
    static constexpr float UGG_ARC_DIST = 10.f * PIXEL_SCALE;
    static constexpr float UGG_FALL_SPEED = 200.f;
    static constexpr float UGG_GRAVITY = 800.f;
    static constexpr int UGG_SRC_W = 16;
    static constexpr int UGG_SRC_H = 16;
    // Intro slide speed (pixels per second) from off-screen to spawn tile
    static constexpr float UGG_INTRO_SPEED = 300.f;

    class Scene;

    class UggWrongwayComponent final : public Component
    {
    public:
        UggWrongwayComponent(GameObject* pOwner, bool isLeftSide, float hopInterval = 0.5f,
            int startRow = PYRAMID_ROWS - 1, int startCol = -1)
            : Component(pOwner)
            , m_isLeftSide(isLeftSide)
            , m_spriteRow(isLeftSide ? 1 : 0)
            , m_hopInterval(hopInterval)
            , m_gridRow(startRow)
            // startCol -1 means use default per side
            , m_gridCol(startCol < 0 ? (isLeftSide ? 0 : (PYRAMID_ROWS - 1)) : startCol)
            , m_spriteCol(isLeftSide ? 0 : 2)
        {
        }

        ~UggWrongwayComponent() override = default;

        UggWrongwayComponent(const UggWrongwayComponent&) = delete;
        UggWrongwayComponent(UggWrongwayComponent&&) = delete;
        UggWrongwayComponent& operator=(const UggWrongwayComponent&) = delete;
        UggWrongwayComponent& operator=(UggWrongwayComponent&&) = delete;

        void SetQbert(QbertPlayerComponent* qbert) { m_qbert = qbert; }
        void SetScene(Scene* scene) { m_scene = scene; }
        void SetFreezeDuration(float d) { m_freezeDuration = d; }

        // Collision tile offset: left=(0,+1), right=(+1,+1)
        int GetCollisionDRow() const { return 1; }
        int GetCollisionDCol() const { return m_isLeftSide ? 0 : 1; }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }

        glm::vec2 GetInitialPos(int charSrcW, int charSrcH) const
        {
            return GetSidePos(m_gridRow, m_gridCol, charSrcW, charSrcH);
        }

        // Returns true while not yet settled on a tile (intro slide, hop arc, or death fall)
        bool IsHopping() const { return m_introFalling || m_hopping || m_falling; }

        void Update(float deltaTime) override;

    private:
        void ChooseAndBeginHop()
        {
            int r = m_gridRow;
            int c = m_gridCol;
            int newRow, newCol;

            if (rand() % 2 == 0)
            {
                // Down-right according to current 'down'
                newRow = m_isLeftSide ? r - 1 : r;
                newCol = m_isLeftSide ? c : c - 1;
                m_spriteCol = m_isLeftSide ? 1 : 2;
            }
            else
            {
                // Down-left according to current 'down'
                newRow = m_isLeftSide ? r : r - 1;
                newCol = m_isLeftSide ? c + 1 : c - 1;
                m_spriteCol = m_isLeftSide ? 0 : 3;
            }

            UpdateSprite();
            BeginHop(newRow, newCol);
        }

        void BeginHop(int destRow, int destCol)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : UGG_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : UGG_SRC_H;

            m_fromPos = GetSidePos(m_gridRow, m_gridCol, srcW, srcH);
            m_toPos = GetSidePos(destRow, destCol, srcW, srcH);
            m_destRow = destRow;
            m_destCol = destCol;
            m_hopDuration = m_hopInterval * 0.5f;
            m_hopPhase = 0.f;
            m_hopping = true;
        }

        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arc = UGG_ARC_DIST * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x + (m_isLeftSide ? -arc : arc), y + arc);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_spriteCol, m_spriteRow);
        }

        // Anchor point within the cube sprite (source pixels, scaled):
        // Wrongway (left): 8px from left edge, 12px from bottom edge
        // Ugg (right): 8px from right edge, 12px from bottom edge
        glm::vec2 GetSidePos(int row, int col, int charSrcW, int) const
        {
            glm::vec2 cubePos = GridToScreen(row, col);
            float charW = charSrcW * PIXEL_SCALE;
            float anchorX = m_isLeftSide
                ? cubePos.x + 8.f * PIXEL_SCALE
                : cubePos.x + (CUBE_SRC_W - 8) * PIXEL_SCALE;
            float anchorY = cubePos.y + (CUBE_SRC_H - 12) * PIXEL_SCALE;
            float spriteX = m_isLeftSide ? anchorX - charW : anchorX;
            return { spriteX, anchorY };
        }

        static bool IsOnPyramid(int row, int col)
        {
            return row >= 0 && row < PYRAMID_ROWS && col >= 0 && col <= row;
        }

        QbertPlayerComponent* m_qbert{ nullptr };
        Scene* m_scene{ nullptr };
        float m_freezeDuration{ 1.f };

        bool m_isLeftSide;
        int m_spriteRow;
        int m_spriteCol;
        float m_hopInterval;

        int m_gridRow;
        int m_gridCol;

        bool m_hopping{ false };
        float m_hopPhase{ 0.f };
        float m_hopDuration{ 0.25f };
        float m_groundTimer{ 0.f };
        glm::vec2 m_fromPos{ 0.f, 0.f };
        glm::vec2 m_toPos{ 0.f, 0.f };
        int m_destRow{ 0 };
        int m_destCol{ 0 };
        bool m_falling{ false };
        glm::vec2 m_fallPos{ 0.f, 0.f };
        glm::vec2 m_fallDir{ 0.f, 1.f };
        float m_fallSpeed{ UGG_FALL_SPEED };
        bool m_initialized{ false };

        bool m_introFalling{ true };
        glm::vec2 m_introFrom{ 0.f, 0.f };
        glm::vec2 m_introTo{ 0.f, 0.f };
        float m_introProgress{ 0.f };
        float m_introLength{ 1.f };
    };
}