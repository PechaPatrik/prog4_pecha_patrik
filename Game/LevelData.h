#pragma once
#include "CubeTileState.h"
#include <vector>
#include <string>

namespace dae
{
    struct LevelData
    {
        // Color column index per round (4 rounds per level)
        std::vector<int> roundColorColumns;
        LevelRule rule;
        float enemySpawnInterval;
        float enemyMoveSpeed;
    };

    LevelData LoadLevelData(const std::string& filePath);
}
