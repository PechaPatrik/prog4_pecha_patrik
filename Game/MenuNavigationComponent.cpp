#include "MenuNavigationComponent.h"
#include "SceneLoader.h"

namespace dae
{
    void MenuNavigationComponent::Update(float)
    {
        if (m_confirmed) return;

        int numKeys{};
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        auto kb = m_kbEdge.GetEdge(keys, true, true);

        bool up = kb.up;
        bool down = kb.down;
        bool confirm = kb.confirm;

        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            auto ce = m_ctrlEdge[ci].GetEdge(ci);
            up |= ce.up;
            down |= ce.down;
            confirm |= ce.confirm;
        }

        if (up)
        {
            m_selection = (m_selection + 2) % 3;
            UpdateArrows();
        }
        else if (down)
        {
            m_selection = (m_selection + 1) % 3;
            UpdateArrows();
        }

        if (confirm)
        {
            m_confirmed = true;
            GameMode mode = static_cast<GameMode>(m_selection);
            SceneLoader::GetInstance().LoadInstructionScreen(mode);
        }
    }
}
