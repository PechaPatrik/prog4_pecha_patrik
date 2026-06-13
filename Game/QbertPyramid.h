#pragma once
#include "GameScale.h"
#include "LevelData.h"
#include "CubeComponent.h"
#include "GameObject.h"
#include <vector>

namespace dae
{
    class Scene;

    // Source cube frame size in the spritesheet
    static constexpr int CUBE_SRC_W = 32;
    static constexpr int CUBE_SRC_H = 32;

    inline float CubeW() { return CUBE_SRC_W * PIXEL_SCALE; }
    inline float CubeH() { return CUBE_SRC_H * PIXEL_SCALE; }
    inline float TileStepX() { return CubeW() * 0.5f; }
    inline float TileStepY() { return CubeH() * 0.75f; }
    inline float PyramidTopX() { return GameWindowW() * 0.5f - CubeW() * 0.5f; }
    inline float PyramidTopY() { return CubeH() * 1.25f; }

    struct PyramidGrid
    {
        std::vector<std::vector<GameObject*>> cubes;
        std::vector<int> rowWidths;
        std::vector<int> rowOffsets;

        int NumRows() const { return static_cast<int>(rowWidths.size()); }

        bool IsValid(int row, int col) const
        {
            if (row < 0 || row >= NumRows()) return false;
            int offset = rowOffsets[row];
            if (col < offset || col >= offset + rowWidths[row]) return false;
            return true;
        }

        CubeComponent* GetCube(int row, int col) const
        {
            if (!IsValid(row, col)) return nullptr;
            return cubes[row][col - rowOffsets[row]]->GetComponent<CubeComponent>();
        }

        bool AllTarget() const
        {
            for (int r = 0; r < NumRows(); ++r)
            {
                int offset = rowOffsets[r];
                for (int c = offset; c < offset + rowWidths[r]; ++c)
                {
                    int idx = c - offset;
                    if (cubes[r][idx] && !cubes[r][idx]->GetComponent<CubeComponent>()->IsTarget())
                        return false;
                }
            }
            return true;
        }

        bool IsAtBottomRow(int row) const
        {
            return row == NumRows() - 1;
        }
    };

    // Screen position of a grid cell, accounting for per-row offset.
    // Safe for out-of-bounds rows (hop-off-edge destinations): offset clamps to 0.
    inline glm::vec2 GridToScreen(int row, int col)
    {
        float x = PyramidTopX() + (col - row * 0.5f) * TileStepX() * 2.f;
        float y = PyramidTopY() + row * TileStepY();
        return { x, y };
    }

    inline glm::vec2 GridToCharacterPos(int row, int col, int charSrcW, int charSrcH)
    {
        glm::vec2 cubePos = GridToScreen(row, col);
        float cubeCenterX = CUBE_SRC_W * PIXEL_SCALE * 0.5f;
        float cubeTopFaceY = CUBE_SRC_H * PIXEL_SCALE * 0.25f;
        float charW = charSrcW * PIXEL_SCALE;
        float charH = charSrcH * PIXEL_SCALE;
        return {
            cubePos.x + cubeCenterX - charW * 0.5f,
            cubePos.y + cubeTopFaceY - charH
        };
    }

    PyramidGrid BuildPyramid(Scene& scene, const LevelData& levelData, int round);
}