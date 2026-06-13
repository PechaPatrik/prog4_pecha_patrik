#pragma once
#include "Component.h"
#include "GameObject.h"
#include "UIInputPoller.h"
#include <SDL3/SDL.h>
#include <array>

#ifndef __EMSCRIPTEN__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#endif

namespace dae
{
    class InstructionScreenComponent final : public Component
    {
    public:
        explicit InstructionScreenComponent(GameObject* pOwner)
            : Component(pOwner)
        {
        }

        void Update(float) override;

    private:
        bool m_confirmed{ false };
        bool m_inputReady{ false };

        // Edge-based confirm (Enter / A button)
        UIKeyboardEdge m_kbEdge{};
        std::array<UIControllerEdge, 2> m_ctrlEdge{};

        // Raw-state back (ESC / Y button)
        bool m_prevEsc{ false };
        std::array<bool, 2> m_prevY{ false, false };
    };
}