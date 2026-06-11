#pragma once
#include "Component.h"
#include "GameObject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include "QbertPlayerComponent.h"
#include "CubeComponent.h"
#include <cmath>
#include <cstdlib>

namespace dae
{
    static constexpr int SLICK_SAM_SRC_W = 12;
    static constexpr int SLICK_SAM_SRC_H = 16;
    static constexpr float SLICK_SAM_ARC_HEIGHT = 10.f * PIXEL_SCALE;
    static constexpr float SLICK_SAM_FALL_SPEED_INIT = 200.f;
    static constexpr float SLICK_SAM_GRAVITY = 800.f;

    static const int SS_dRow[4] = { -1, -1, 1, 1 };
    static const int SS_dCol[4] = { 0, -1, 1, 0 };

    class SlickSamComponent final : public Component
    {
    public:
        // isSlick: true = Slick (spritesheet row 0), false = Sam (spritesheet row 1)
        // isLeftSide: true spawns at row 1 col 0, false spawns at row 1 col 1
        SlickSamComponent(GameObject* pOwner, bool isSlick, bool isLeftSide, float hopInterval = 0.5f)
            : Component(pOwner)
            , m_spriteRow(isSlick ? 0 : 1)
            , m_isLeftSide(isLeftSide)
            , m_hopInterval(hopInterval)
            , m_gridRow(1)
            , m_gridCol(isLeftSide ? 0 : 1)
        {
        }

        ~SlickSamComponent() override = default;

        SlickSamComponent(const SlickSamComponent&) = delete;
        SlickSamComponent(SlickSamComponent&&) = delete;
        SlickSamComponent& operator=(const SlickSamComponent&) = delete;
        SlickSamComponent& operator=(SlickSamComponent&&) = delete;

        void SetPyramidGrid(PyramidGrid* grid) { m_grid = grid; }
        void SetQbert(QbertPlayerComponent* qbert) { m_qbert = qbert; }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }

        glm::vec2 GetInitialPos(int charSrcW, int charSrcH) const
        {
            return GridToCharacterPos(m_gridRow, m_gridCol, charSrcW, charSrcH);
        }

        void Update(float deltaTime) override
        {
            if (!m_initialized)
            {
                m_initialized = true;
                UpdateSprite();
                RevertCube();
            }

            if (m_falling)
            {
                m_fallSpeed += SLICK_SAM_GRAVITY * deltaTime;
                m_fallPos.y += m_fallSpeed * deltaTime;
                GetOwner()->SetLocalPosition(m_fallPos.x, m_fallPos.y);
                if (m_fallPos.y > WINDOW_H + SLICK_SAM_SRC_H * PIXEL_SCALE * 2.f)
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
                    RevertCube();
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

    private:
        void RevertCube()
        {
            if (!m_grid) return;
            auto* cube = m_grid->GetCube(m_gridRow, m_gridCol);
            if (cube)
                cube->StepBack();
        }

        void ChooseAndBeginHop()
        {
            int r = m_gridRow;
            int c = m_gridCol;

            int dir = (rand() % 2) + 2;
            int newRow = r + SS_dRow[dir];
            int newCol = c + SS_dCol[dir];

            if (!IsOnPyramid(newRow, newCol))
            {
                dir = (dir == 2) ? 3 : 2;
                newRow = r + SS_dRow[dir];
                newCol = c + SS_dCol[dir];
            }

            m_spriteCol = (dir == 2) ? 1 : 0;
            UpdateSprite();
            BeginHop(newRow, newCol);
        }

        void BeginHop(int destRow, int destCol)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : SLICK_SAM_SRC_W;
            int srcH = sheet ? sheet->GetFrameHeight() : SLICK_SAM_SRC_H;

            m_fromPos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            m_toPos = GridToCharacterPos(destRow, destCol, srcW, srcH);
            m_destRow = destRow;
            m_destCol = destCol;
            m_hopDuration = m_hopInterval * 0.5f;
            m_hopPhase = 0.f;
            m_hopping = true;
        }

        void BeginFall()
        {
            m_falling = true;
            glm::vec2 wp = GetOwner()->GetWorldPosition();
            m_fallPos = { wp.x, wp.y };
            m_fallSpeed = SLICK_SAM_FALL_SPEED_INIT;
        }

        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arcY = -SLICK_SAM_ARC_HEIGHT * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_spriteCol, m_spriteRow);
        }

        static bool IsOnPyramid(int row, int col)
        {
            return row >= 0 && row < PYRAMID_ROWS && col >= 0 && col <= row;
        }

        PyramidGrid* m_grid{ nullptr };
        QbertPlayerComponent* m_qbert{ nullptr };

        int m_spriteRow;
        int m_spriteCol{ 0 };
        bool m_isLeftSide;
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
        float m_fallSpeed{ SLICK_SAM_FALL_SPEED_INIT };

        bool m_initialized{ false };
    };
}