#pragma once
#include <SDL3/SDL.h>
#include <array>

// UIInputPoller provides rising-edge detection for keyboard and XInput controller buttons
// used in UI screens (menus, name entry, pause) without going through InputManager bindings.
// XInput is only compiled on non-Emscripten targets

#ifndef __EMSCRIPTEN__
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")
#endif

namespace dae
{
    struct UIControllerState
    {
        bool up{ false };
        bool down{ false };
        bool left{ false };
        bool right{ false };
        bool confirm{ false };
        bool back{ false };
    };

    // Returns XInput button state for controller [index]. Returns all-false on Emscripten.
    inline UIControllerState QueryControllerRaw(unsigned int index)
    {
#ifndef __EMSCRIPTEN__
        XINPUT_STATE state{};
        if (XInputGetState(index, &state) != ERROR_SUCCESS)
            return {};
        WORD b = state.Gamepad.wButtons;
        UIControllerState s{};
        s.up = (b & XINPUT_GAMEPAD_DPAD_UP) != 0;
        s.down = (b & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        s.left = (b & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
        s.right = (b & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
        s.confirm = (b & XINPUT_GAMEPAD_A) != 0 || (b & XINPUT_GAMEPAD_START) != 0;
        s.back = (b & XINPUT_GAMEPAD_B) != 0 || (b & XINPUT_GAMEPAD_BACK) != 0;
        return s;
#else
        (void)index;
        return {};
#endif
    }

    // Tracks previous frame state for a single controller to produce rising edges
    struct UIControllerEdge
    {
        UIControllerState prev{};

        UIControllerState GetEdge(unsigned int index)
        {
            UIControllerState cur = QueryControllerRaw(index);
            UIControllerState edge{};
            edge.up = cur.up && !prev.up;
            edge.down = cur.down && !prev.down;
            edge.left = cur.left && !prev.left;
            edge.right = cur.right && !prev.right;
            edge.confirm = cur.confirm && !prev.confirm;
            edge.back = cur.back && !prev.back;
            prev = cur;
            return edge;
        }
    };

    // Tracks previous keyboard state for menu navigation keys
    struct UIKeyboardEdge
    {
        bool prevUp{ false };
        bool prevDown{ false };
        bool prevLeft{ false };
        bool prevRight{ false };
        bool prevConfirm{ false };
        bool prevBack{ false };
        bool prevTab{ false };

        struct KeyEdge
        {
            bool up{ false };
            bool down{ false };
            bool left{ false };
            bool right{ false };
            bool confirm{ false };
            bool back{ false };
            bool tab{ false };
        };

        KeyEdge GetEdge(const bool* keys, bool includeWasd = true, bool includeArrows = true)
        {
            bool upCur = false;
            bool downCur = false;
            bool leftCur = false;
            bool rightCur = false;
            if (includeWasd)
            {
                upCur |= keys[SDL_SCANCODE_W];
                downCur |= keys[SDL_SCANCODE_S];
                leftCur |= keys[SDL_SCANCODE_A];
                rightCur |= keys[SDL_SCANCODE_D];
            }
            if (includeArrows)
            {
                upCur |= keys[SDL_SCANCODE_UP];
                downCur |= keys[SDL_SCANCODE_DOWN];
                leftCur |= keys[SDL_SCANCODE_LEFT];
                rightCur |= keys[SDL_SCANCODE_RIGHT];
            }

            bool confirmCur = keys[SDL_SCANCODE_RETURN] || keys[SDL_SCANCODE_RETURN2];
            bool backCur = keys[SDL_SCANCODE_ESCAPE];
            bool tabCur = keys[SDL_SCANCODE_TAB];

            KeyEdge e{};
            e.up = upCur && !prevUp;
            e.down = downCur && !prevDown;
            e.left = leftCur && !prevLeft;
            e.right = rightCur && !prevRight;
            e.confirm = confirmCur && !prevConfirm;
            e.back = backCur && !prevBack;
            e.tab = tabCur && !prevTab;

            prevUp = upCur;
            prevDown = downCur;
            prevLeft = leftCur;
            prevRight = rightCur;
            prevConfirm = confirmCur;
            prevBack = backCur;
            prevTab = tabCur;
            return e;
        }
    };
}
