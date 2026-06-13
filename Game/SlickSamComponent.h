#pragma once
#include "Component.h"
#include "GameObject.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include "QbertPlayerComponent.h"
#include "CubeComponent.h"
#include "ServiceLocator.h"
#include "SoundId.h"
#include <cmath>
#include <cstdlib>

namespace dae
{
    static constexpr int SLICK_SAM_SRC_W = 12;
    static constexpr int SLICK_SAM_SRC_H = 16;

    static const int SS_dRow[4] = { -1, -1, 1, 1 };
    static const int SS_dCol[4] = { 0, -1, 1, 0 };

    class Scene;

    class SlickSamComponent final : public Component
    {
    public:
        SlickSamComponent(GameObject* pOwner, bool isSlick, float hopInterval = 0.5f,
            int spawnRow = 1, int spawnCol = 0)
            : Component(pOwner)
            , m_spriteRow(isSlick ? 0 : 1)
            , m_hopInterval(hopInterval)
            , m_gridRow(spawnRow)
            , m_gridCol(spawnCol)
        {
        }

        ~SlickSamComponent() override = default;

        SlickSamComponent(const SlickSamComponent&) = delete;
        SlickSamComponent(SlickSamComponent&&) = delete;
        SlickSamComponent& operator=(const SlickSamComponent&) = delete;
        SlickSamComponent& operator=(SlickSamComponent&&) = delete;

        void SetPyramidGrid(PyramidGrid* grid) { m_grid = grid; }
        void SetQbert(QbertPlayerComponent* qbert) { m_qbert = qbert; }
        void SetScene(Scene* scene) { m_scene = scene; }
        void SetFreezeDuration(float d) { m_freezeDuration = d; }
        void SetArcHeight(float h) { m_arcHeight = h; }
        void SetFallGravity(float g) { m_fallGravity = g; }
        void SetIntroSpeed(float s) { m_introSpeed = s; }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }
        bool IsHopping() const { return m_introFalling || m_hopping; }

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

        void Update(float deltaTime) override;

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

            m_fromPos = m_grid
                ? GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH)
                : glm::vec2{ 0.f, 0.f };
            m_toPos = m_grid
                ? GridToCharacterPos(destRow, destCol, srcW, srcH)
                : glm::vec2{ 0.f, 0.f };
            m_destRow = destRow;
            m_destCol = destCol;
            m_hopDuration = m_hopInterval * 0.5f;
            if (m_hopDuration <= 0.f) m_hopDuration = 0.001f;
            m_hopPhase = 0.f;
            m_hopping = true;
            ServiceLocator::GetSoundSystem().PlaySound(SoundId::OtherFoesJump);
        }

        void BeginFall()
        {
            m_falling = true;
            glm::vec2 wp = GetOwner()->GetWorldPosition();
            m_fallPos = { wp.x, wp.y };
            m_fallSpeed = 0.f;
        }

        void ApplyArcPosition(float t)
        {
            float arcH = m_arcHeight * PIXEL_SCALE;
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arcY = -arcH * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_spriteCol, m_spriteRow);
        }

        bool IsOnPyramid(int row, int col) const
        {
            return m_grid && m_grid->IsValid(row, col);
        }

        PyramidGrid* m_grid{ nullptr };
        QbertPlayerComponent* m_qbert{ nullptr };
        Scene* m_scene{ nullptr };
        float m_freezeDuration{ 1.f };
        float m_arcHeight{ 12.f };
        float m_fallGravity{ 800.f };
        float m_introSpeed{ 400.f };

        int m_spriteRow;
        int m_spriteCol{ 0 };
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