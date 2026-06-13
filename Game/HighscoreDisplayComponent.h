#pragma once
#include "Component.h"
#include "GameObject.h"
#include "HighscoreEntry.h"
#include "UIInputPoller.h"
#include "Scene.h"
#include <vector>
#include <array>
#include <SDL3/SDL.h>

#ifndef __EMSCRIPTEN__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#endif

namespace dae
{
    class TextComponent;

    class HighscoreDisplayComponent final : public Component
    {
    public:
        explicit HighscoreDisplayComponent(GameObject* pOwner);

        void Update(float) override;

    private:
        bool m_transitioning{ false };
        bool m_initialized{ false };
        bool m_prevEsc{ false };
        std::array<bool, 2> m_prevY{ false, false };
    };
}
