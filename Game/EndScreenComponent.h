#pragma once
#include "Component.h"
#include "GameObject.h"
#include <array>

#ifndef __EMSCRIPTEN__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#endif

namespace dae
{
    class EndScreenComponent final : public Component
    {
    public:
        EndScreenComponent(GameObject* pOwner, bool victory)
            : Component(pOwner)
            , m_victory(victory)
        {
        }

        void Update(float) override;

    private:
        bool m_victory;
        bool m_transitioning{ false };
        bool m_prevEsc{ false };
        std::array<bool, 2> m_prevY{ false, false };
    };
}
