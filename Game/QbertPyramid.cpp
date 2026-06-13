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
        grid.rowWidths = levelData.rowWidths;
        grid.rowOffsets = levelData.rowOffsets;

        int numRows = grid.NumRows();
        grid.cubes.resize(numRows);

        for (int row = 0; row < numRows; ++row)
        {
            int cols = grid.rowWidths[row];
            int offset = grid.rowOffsets[row];
            grid.cubes[row].resize(cols, nullptr);

            for (int i = 0; i < cols; ++i)
            {
                int logicalCol = offset + i;
                auto go = std::make_unique<GameObject>();
                glm::vec2 pos = GridToScreen(row, logicalCol);
                go->SetLocalPosition(pos.x, pos.y);

                go->AddComponent<SpritesheetComponent>("Qbert Cubes.png", CUBE_SRC_W, CUBE_SRC_H);
                go->AddComponent<CubeComponent>(levelData.rule, colorColumn);
                go->GetComponent<SpritesheetComponent>()->SetFrame(colorColumn);

                grid.cubes[row][i] = go.get();
                scene.Add(std::move(go));
            }
        }

        return grid;
    }
}