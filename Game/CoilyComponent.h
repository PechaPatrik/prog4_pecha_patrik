#pragma once
#include "Component.h"
#include "GameObject.h"
#include "CoilyState.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include <memory>

namespace dae
{
    class CoilyComponent final : public Component
    {
    public:
        explicit CoilyComponent(GameObject* pOwner, float hopInterval = 0.5f)
            : Component(pOwner)
            , m_state(std::make_unique<CoilyEggState>(hopInterval))
            , m_gridRow(0)
            , m_gridCol(0)
            , m_inAir(false)
        {
        }

        ~CoilyComponent() override = default;

        CoilyComponent(const CoilyComponent&) = delete;
        CoilyComponent(CoilyComponent&&) = delete;
        CoilyComponent& operator=(const CoilyComponent&) = delete;
        CoilyComponent& operator=(CoilyComponent&&) = delete;

        void Update(float deltaTime) override
        {
            auto newState = m_state->Update(deltaTime, *this);
            if (newState)
                m_state = std::move(newState);
            UpdateSprite();
        }

        void SetGridPosition(int row, int col)
        {
            m_gridRow = row;
            m_gridCol = col;
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 16;
            int srcH = sheet ? sheet->GetFrameHeight() : 32;
            glm::vec2 pos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            GetOwner()->SetLocalPosition(pos.x, pos.y);
        }

        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }

        void SetInAir(bool inAir) { m_inAir = inAir; }
        bool IsEgg() const { return m_state->IsEgg(); }

        void SetTargetGridPosition(int row, int col)
        {
            m_targetRow = row;
            m_targetCol = col;
        }

        int GetTargetRow() const { return m_targetRow; }
        int GetTargetCol() const { return m_targetCol; }

        void SetLastMoveDirection(int dir) { m_lastMoveDir = dir; }
        int GetLastMoveDirection() const { return m_lastMoveDir; }

    private:
        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_state->GetCol(m_inAir), 0);
        }

        std::unique_ptr<CoilyBaseState> m_state;
        int m_gridRow;
        int m_gridCol;
        bool m_inAir;
        int m_targetRow{ 0 };
        int m_targetCol{ 0 };
        int m_lastMoveDir{ 2 };
    };
}