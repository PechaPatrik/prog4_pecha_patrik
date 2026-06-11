#pragma once
#include "CubeTileState.h"
#include <vector>
#include <string>

namespace dae
{
    struct EnemySpawnConfig
    {
        std::vector<bool> enabledPerRound;
        float firstSpawnDelay{ 5.f };
        std::vector<float> spawnIntervalMin;
        std::vector<float> spawnIntervalMax;
        std::vector<float> hopIntervals;
        int spawnRow{ 0 };
        int spawnCol{ 0 };
    };

    struct LevelData
    {
        std::vector<int> roundColorColumns;
        LevelRule rule{};

        EnemySpawnConfig coily;
        EnemySpawnConfig uggWrongway;
        EnemySpawnConfig slickSam;

        float freezeDuration{ 1.f };

        int pointsPerCubeChange{ 25 };
        int pointsCoilyDisc{ 500 };
        int pointsDiscRemaining{ 50 };
        int pointsSlickSam{ 300 };
    };

    LevelData LoadLevelData(const std::string& filePath);
}