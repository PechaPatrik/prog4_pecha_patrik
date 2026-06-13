#pragma once
#include "Component.h"
#include "GameObject.h"
#include "Renderer.h"
#include "GameScale.h"
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
    class PauseOverlayComponent final : public Component
    {
    public:
        explicit PauseOverlayComponent(GameObject* pOwner)
            : Component(pOwner)
        {
        }

        void Update(float) override;

        void Render() const override
        {
            SDL_Renderer* renderer = Renderer::GetInstance().GetSDLRenderer();
            if (!renderer) return;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 140);
            SDL_FRect rect{ 0.f, 0.f,
                static_cast<float>(GameWindowW()),
                static_cast<float>(GameWindowH()) };
            SDL_RenderFillRect(renderer, &rect);
        }

    private:
        bool m_unpauseTriggered{ false };
        bool m_initialized{ false };
        bool m_prevEsc{ false };
        std::array<bool, 2> m_prevY{ false, false };
    };
}