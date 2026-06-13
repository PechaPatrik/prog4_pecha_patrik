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
    static constexpr int UGG_SRC_W = 16;
    static constexpr int UGG_SRC_H = 16;

    class Scene;

    class UggWrongwayComponent final : public Component
    {
    public:
        UggWrongwayComponent(GameObject* pOwner, bool isLeftSide, float hopInterval = 0.5f,
            int startRow = -1, int startCol = -1)
            : Component(pOwner)
            , m_isLeftSide(isLeftSide)
            , m_spriteRow(isLeftSide ? 1 : 0)
            , m_spriteCol(isLeftSide ? 0 : 2)
            , m_hopInterval(hopInterval)
            , m_pendingStartRow(startRow)
            , m_pendingStartCol(startCol)
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
        void SetPyramidGrid(const PyramidGrid* grid) { m_grid = grid; }
        void SetArcHeight(float h) { m_arcHeight = h; }
        void SetFallGravity(float g) { m_fallGravity = g; }
        void SetIntroSpeed(float s) { m_introSpeed = s; }

        int GetCollisionDRow() const { return 1; }
        int GetCollisionDCol() const { return m_isLeftSide ? 0 : 1; }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }

        void TriggerFall()
        {
            if (m_falling) return;
            if (m_hopping || m_introFalling)
            {
                m_pendingFall = true;
                return;
            }
            BeginFall();
        }

        bool IsHopping() const { return m_introFalling || m_hopping || m_falling; }

        void Update(float deltaTime) override;

    private:
        void ResolveStartPosition()
        {
            int lastRow = m_grid ? m_grid->NumRows() - 1 : 6;
            if (m_pendingStartRow >= 0)
            {
                m_gridRow = m_pendingStartRow;
                m_gridCol = (m_pendingStartCol >= 0)
                    ? m_pendingStartCol
                    : (m_isLeftSide ? 0 : lastRow);
            }
            else
            {
                m_gridRow = lastRow;
                m_gridCol = m_isLeftSide ? 0 : lastRow;
            }
        }

        void ChooseAndBeginHop()
        {
            int r = m_gridRow;
            int c = m_gridCol;
            int newRow, newCol;

            if (rand() % 2 == 0)
            {
                newRow = m_isLeftSide ? r - 1 : r;
                newCol = m_isLeftSide ? c : c - 1;
                m_spriteCol = m_isLeftSide ? 1 : 2;
            }
            else
            {
                newRow = m_isLeftSide ? r : r - 1;
                newCol = m_isLeftSide ? c + 1 : c - 1;
                m_spriteCol = m_isLeftSide ? 0 : 3;
            }

            UpdateSprite();
            BeginHop(newRow, newCol);
        }

        void BeginFall()
        {
            m_falling = true;
            glm::vec2 wp = GetOwner()->GetWorldPosition();
            m_fallPos = { wp.x, wp.y };
            static constexpr float SIN120 = 0.8660254f;
            static constexpr float COS120 = 0.5f;
            m_fallDir = { m_isLeftSide ? SIN120 : -SIN120, -COS120 };
            m_fallSpeed = 0.f;
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
            if (m_hopDuration <= 0.f) m_hopDuration = 0.001f;
            m_hopPhase = 0.f;
            m_hopping = true;
        }

        void ApplyArcPosition(float t)
        {
            float arcDist = m_arcHeight * PIXEL_SCALE;
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arc = arcDist * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x + (m_isLeftSide ? -arc : arc), y + arc);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_spriteCol, m_spriteRow);
        }

        glm::vec2 GetSidePos(int row, int col, int charSrcW, int) const
        {
            glm::vec2 cubePos = m_grid
                ? GridToScreen(row, col)
                : glm::vec2{ 0.f, 0.f };
            float charW = static_cast<float>(charSrcW) * PIXEL_SCALE;
            float anchorX = m_isLeftSide
                ? cubePos.x + 8.f * PIXEL_SCALE
                : cubePos.x + static_cast<float>(CUBE_SRC_W - 8) * PIXEL_SCALE;
            float anchorY = cubePos.y + static_cast<float>(CUBE_SRC_H - 12) * PIXEL_SCALE;
            float spriteX = m_isLeftSide ? anchorX - charW : anchorX;
            return { spriteX, anchorY };
        }

        bool IsOnPyramid(int row, int col) const
        {
            return m_grid && m_grid->IsValid(row, col);
        }

        QbertPlayerComponent* m_qbert{ nullptr };
        Scene* m_scene{ nullptr };
        const PyramidGrid* m_grid{ nullptr };
        float m_freezeDuration{ 1.f };
        float m_arcHeight{ 12.f };
        float m_fallGravity{ 800.f };
        float m_introSpeed{ 400.f };

        bool m_isLeftSide;
        int m_spriteRow;
        int m_spriteCol;
        float m_hopInterval;

        int m_pendingStartRow{ -1 };
        int m_pendingStartCol{ -1 };
        int m_gridRow{ 0 };
        int m_gridCol{ 0 };

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
        float m_fallSpeed{ 0.f };
        bool m_pendingFall{ false };
        bool m_initialized{ false };

        bool m_introFalling{ true };
        glm::vec2 m_introFrom{ 0.f, 0.f };
        glm::vec2 m_introTo{ 0.f, 0.f };
        float m_introProgress{ 0.f };
        float m_introLength{ 1.f };
    };
}