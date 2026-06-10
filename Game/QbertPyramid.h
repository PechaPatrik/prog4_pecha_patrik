#pragma once
#include "GameScale.h"
#include "LevelData.h"
#include "CubeComponent.h"
#include "GameObject.h"
#include <vector>

namespace dae
{
    class Scene;

    // At 3x scale (original 256x240 -> 768x720)
    // can't just make constexpr in minigin and pass because of SIOF
    static constexpr int WINDOW_W = 768;
    static constexpr int WINDOW_H = 720;
    static constexpr int PYRAMID_ROWS = 7;
    // Source cube frame size in the spritesheet
    static constexpr int CUBE_SRC_W = 32;
    static constexpr int CUBE_SRC_H = 32;
    // Rendered cube size
    static constexpr float CUBE_W = CUBE_SRC_W * PIXEL_SCALE;
    static constexpr float CUBE_H = CUBE_SRC_H * PIXEL_SCALE;
    // Tile step: half a cube width/height in screen space (isometric)
    static constexpr float TILE_STEP_X = CUBE_W * 0.5f;
    static constexpr float TILE_STEP_Y = CUBE_H * 0.75f;
    // Top cube left edge: center window on the top face center of the apex cube
    static constexpr float PYRAMID_TOP_X = WINDOW_W * 0.5f - CUBE_W * 0.5f;
    // Top cube top edge: one top-face height from the top of the screen
    static constexpr float PYRAMID_TOP_Y = CUBE_H * 1.25f;

    struct PyramidGrid
    {
        // cubes[row][col] raw pointer, no shared pointer because owned by scene
        std::vector<std::vector<GameObject*>> cubes;

        bool IsValid(int row, int col) const
        {
            if (row < 0 || row >= PYRAMID_ROWS) return false;
            if (col < 0 || col > row) return false;
            return true;
        }

        CubeComponent* GetCube(int row, int col) const
        {
            if (!IsValid(row, col)) return nullptr;
            return cubes[row][col]->GetComponent<CubeComponent>();
        }

        bool AllTarget() const
        {
            for (auto& rowVec : cubes)
                for (auto* go : rowVec)
                    if (go && !go->GetComponent<CubeComponent>()->IsTarget())
                        return false;
            return true;
        }
    };

    // Returns the screen position (top-left of the cube sprite) for a given grid cell
    inline glm::vec2 GridToScreen(int row, int col)
    {
        float x = PYRAMID_TOP_X + (col - row * 0.5f) * TILE_STEP_X * 2.f;
        float y = PYRAMID_TOP_Y + row * TILE_STEP_Y;
        return { x, y };
    }

    // Returns where a character should stand on top of a cube (top-face center, accounting for sprite offset)
    inline glm::vec2 GridToCharacterPos(int row, int col, int charSrcW, int charSrcH)
    {
        glm::vec2 cubePos = GridToScreen(row, col);
        float cubeCenterX = CUBE_SRC_W * PIXEL_SCALE * 0.5f;
        float cubeTopFaceY = CUBE_SRC_H * PIXEL_SCALE * 0.25f;
        // Place character so its bottom-center aligns with the cube top-face center
        float charW = charSrcW * PIXEL_SCALE;
        float charH = charSrcH * PIXEL_SCALE;
        return {
            cubePos.x + cubeCenterX - charW * 0.5f,
            cubePos.y + cubeTopFaceY - charH
        };
    }

    PyramidGrid BuildPyramid(Scene& scene, const LevelData& levelData, int round);
}