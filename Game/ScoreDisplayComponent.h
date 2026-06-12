#pragma once
#include "Component.h"
#include "IObserver.h"
#include "TextComponent.h"
#include <string>

namespace dae
{
    class ScoreDisplayComponent final : public Component, public IObserver
    {
    public:
        ScoreDisplayComponent(GameObject* pOwner)
            : Component(pOwner)
            , m_score(0)
        {
        }

        void Update(float) override
        {
            if (!m_dirty) return;
            if (auto* tc = GetOwner()->GetComponent<TextComponent>())
                tc->SetText(std::to_string(m_score));
            m_dirty = false;
        }

        void OnNotify(GameEvent event, int value) override
        {
            if (event == GameEvent::ScoreChanged)
            {
                m_score = value;
                m_dirty = true;
            }
        }

    private:
        int m_score;
        bool m_dirty{ false };
    };
}