#pragma once
#include "Component.h"
#include "GameObject.h"
#include "GameStateManager.h"
#include "TextComponent.h"
#include <string>

namespace dae
{
    class BonusDisplayComponent final : public Component
    {
    public:
        BonusDisplayComponent(GameObject* pOwner,
            GameObject* labelGo, GameObject* valueGo, TextComponent* valueText)
            : Component(pOwner)
            , m_labelGo(labelGo)
            , m_valueGo(valueGo)
            , m_valueText(valueText)
        {
            SetVisible(false);
        }

        ~BonusDisplayComponent() override = default;

        BonusDisplayComponent(const BonusDisplayComponent&) = delete;
        BonusDisplayComponent(BonusDisplayComponent&&) = delete;
        BonusDisplayComponent& operator=(const BonusDisplayComponent&) = delete;
        BonusDisplayComponent& operator=(BonusDisplayComponent&&) = delete;

        void Update(float) override
        {
            bool active = GameStateManager::GetInstance().IsBonusScreen();

            if (active && !m_wasActive)
            {
                if (m_valueText)
                    m_valueText->SetText(std::to_string(
                        GameStateManager::GetInstance().GetBonusPoints()));
                SetVisible(true);
            }
            else if (!active && m_wasActive)
            {
                SetVisible(false);
            }

            m_wasActive = active;
        }

    private:
        void SetVisible(bool visible)
        {
            if (m_labelGo) m_labelGo->SetActive(visible);
            if (m_valueGo) m_valueGo->SetActive(visible);
        }

        GameObject* m_labelGo{ nullptr };
        GameObject* m_valueGo{ nullptr };
        TextComponent* m_valueText{ nullptr };
        bool m_wasActive{ false };
    };
}
