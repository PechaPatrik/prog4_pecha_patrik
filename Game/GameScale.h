#pragma once

namespace dae
{
    // Base NES resolution, never changes
    static constexpr int BASE_WINDOW_W = 256;
    static constexpr int BASE_WINDOW_H = 240;

    // Set once in Main before anything else runs; all game code reads from here
    extern float PIXEL_SCALE;

    inline int GameWindowW() { return static_cast<int>(BASE_WINDOW_W * PIXEL_SCALE); }
    inline int GameWindowH() { return static_cast<int>(BASE_WINDOW_H * PIXEL_SCALE); }
}