#include "PauseOverlayComponent.h"
#include "SceneLoader.h"

namespace dae
{
    void PauseOverlayComponent::Update(float)
    {
        if (m_unpauseTriggered) return;

        int numKeys{};
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        if (!m_initialized)
        {
            m_initialized = true;
            m_prevEsc = keys[SDL_SCANCODE_ESCAPE];
#ifndef __EMSCRIPTEN__
            for (unsigned int ci = 0; ci < 2; ++ci)
            {
                XINPUT_STATE state{};
                if (XInputGetState(ci, &state) == ERROR_SUCCESS)
                    m_prevY[ci] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
            }
#endif
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
            m_unpauseTriggered = true;
            SceneLoader::GetInstance().Unpause();
        }
    }
}