#include "EndScreenComponent.h"
#include "SceneLoader.h"
#include <SDL3/SDL.h>

namespace dae
{
    void EndScreenComponent::Update(float)
    {
        if (m_transitioning) return;

        int numKeys{};
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        bool escPressed = keys[SDL_SCANCODE_ESCAPE] && !m_prevEsc;
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

        if (escPressed || yPressed)
        {
            m_transitioning = true;
            SceneLoader::GetInstance().LoadNameEntry(m_victory);
        }
    }
}
