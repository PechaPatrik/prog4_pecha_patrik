#pragma once
#include "Component.h"
#include "GameObject.h"
#include "GameMode.h"
#include "UIInputPoller.h"
#include <array>

namespace dae
{
    class MenuNavigationComponent final : public Component
    {
    public:
        MenuNavigationComponent(GameObject* pOwner, std::array<GameObject*, 3> arrows)
            : Component(pOwner)
            , m_arrows(arrows)
        {
        }

        void Update(float) override;

    private:
        void UpdateArrows()
        {
            for (int i = 0; i < 3; ++i)
                if (m_arrows[i])
                    m_arrows[i]->SetActive(i == m_selection);
        }

        std::array<GameObject*, 3> m_arrows{};
        int m_selection{ 0 };
        bool m_confirmed{ false };

        UIKeyboardEdge m_kbEdge{};
        std::array<UIControllerEdge, 2> m_ctrlEdge{};
    };
}
