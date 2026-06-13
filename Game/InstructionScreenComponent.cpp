#include "InstructionScreenComponent.h"
#include "SceneLoader.h"

namespace dae
{
    void InstructionScreenComponent::Update(float)
    {
        if (m_confirmed) return;

        int numKeys{};
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        if (!m_inputReady)
        {
            m_kbEdge.GetEdge(keys, true, true);
            for (unsigned int ci = 0; ci < 2; ++ci)
                m_ctrlEdge[ci].GetEdge(ci);

            m_prevEsc = keys[SDL_SCANCODE_ESCAPE];
#ifndef __EMSCRIPTEN__
            for (unsigned int ci = 0; ci < 2; ++ci)
            {
                XINPUT_STATE state{};
                if (XInputGetState(ci, &state) == ERROR_SUCCESS)
                    m_prevY[ci] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
            }
#endif
            m_inputReady = true;
            return;
        }

        bool backPressed = keys[SDL_SCANCODE_ESCAPE] && !m_prevEsc;
        m_prevEsc = keys[SDL_SCANCODE_ESCAPE];

        bool yPressed = false;
#ifndef __EMSCRIPTEN__
        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            XINPUT_STATE state{};
            if (XInputGetState(ci, &state) == ERROR_SUCCESS)
            {
                bool yCur = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
                if (yCur && !m_prevY[ci]) yPressed = true;
                m_prevY[ci] = yCur;
            }
        }
#endif

        if (backPressed || yPressed)
        {
            m_confirmed = true;
            SceneLoader::GetInstance().LoadMainMenu();
            return;
        }

        auto kb = m_kbEdge.GetEdge(keys, true, true);
        bool confirm = kb.confirm;

        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            auto ce = m_ctrlEdge[ci].GetEdge(ci);
            confirm |= ce.confirm;
        }

        if (confirm)
        {
            m_confirmed = true;
            SceneLoader::GetInstance().LoadLevelIntro(0);
        }
    }
}