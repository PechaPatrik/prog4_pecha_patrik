#pragma once
#include "Component.h"
#include "GameObject.h"
#include "CubeTileState.h"
#include "SpritesheetComponent.h"
#include <memory>

namespace dae
{
    class CubeComponent final : public Component
    {
    public:
        CubeComponent(GameObject* pOwner, LevelRule rule, int colorColumn)
            : Component(pOwner)
            , m_rule(rule)
            , m_colorColumn(colorColumn)
            , m_state(std::make_unique<UntouchedState>())
        {
        }

        ~CubeComponent() override = default;

        CubeComponent(const CubeComponent&) = delete;
        CubeComponent(CubeComponent&&) = delete;
        CubeComponent& operator=(const CubeComponent&) = delete;
        CubeComponent& operator=(CubeComponent&&) = delete;

        void Update(float) override {}

        void Step()
        {
            auto newState = m_state->OnStep(m_rule);
            if (newState)
                m_state = std::move(newState);
            UpdateSprite();
        }

        // Slick/Sam: target -> intermediate (DoubleStep), target -> untouched (others), intermediate -> untouched
        void StepBack()
        {
            if (m_state->IsTarget())
            {
                if (m_rule == LevelRule::DoubleStep)
                    m_state = std::make_unique<IntermediateState>();
                else
                    m_state = std::make_unique<UntouchedState>();
            }
            else if (m_state->GetRow() == 1)
            {
                m_state = std::make_unique<UntouchedState>();
            }
            else return;
            UpdateSprite();
        }

        bool IsTarget() const { return m_state->IsTarget(); }

    private:
        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_colorColumn, m_state->GetRow());
        }

        LevelRule m_rule;
        int m_colorColumn;
        std::unique_ptr<CubeTileState> m_state;
    };
}