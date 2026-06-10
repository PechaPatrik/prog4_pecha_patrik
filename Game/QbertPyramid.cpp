#include "QbertPyramid.h"
#include "Scene.h"
#include "GameObject.h"
#include "SpritesheetComponent.h"
#include "CubeComponent.h"

namespace dae
{
    PyramidGrid BuildPyramid(Scene& scene, const LevelData& levelData, int round)
    {
        int colorColumn = levelData.roundColorColumns[round % static_cast<int>(levelData.roundColorColumns.size())];

        PyramidGrid grid;
        grid.cubes.resize(PYRAMID_ROWS);

        for (int row = 0; row < PYRAMID_ROWS; ++row)
        {
            int cols = row + 1;
            grid.cubes[row].resize(cols, nullptr);

            for (int col = 0; col < cols; ++col)
            {
                auto go = std::make_unique<GameObject>();
                glm::vec2 pos = GridToScreen(row, col);
                go->SetLocalPosition(pos.x, pos.y);

                go->AddComponent<SpritesheetComponent>("Qbert Cubes.png", CUBE_SRC_W, CUBE_SRC_H, PIXEL_SCALE);
                auto* cube = go->AddComponent<CubeComponent>(levelData.rule, colorColumn);
                // Initialize sprite to untouched state (row 0)
                go->GetComponent<SpritesheetComponent>()->SetFrame(colorColumn, 0);
                (void)cube;

                grid.cubes[row][col] = go.get();
                scene.Add(std::move(go));
            }
        }

        return grid;
    }
}