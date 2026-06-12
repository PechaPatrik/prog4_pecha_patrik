#pragma once
#include "CubeTileState.h"
#include <vector>
#include <string>
#include <utility>

namespace dae
{
    struct EnemySpawnConfig
    {
        std::vector<bool> enabledPerRound;
        float firstSpawnDelay{ 5.f };
        // Each entry is {row, col}
        std::vector<std::pair<int, int>> spawnLocations;
        std::vector<float> spawnIntervalMin;
        std::vector<float> spawnIntervalMax;
        std::vector<float> hopIntervals;
    };

    struct LevelData
    {
        std::vector<int> roundColorColumns;
        LevelRule rule{};

        EnemySpawnConfig coily;
        // Ugg and Wrongway are separate so their spawn points can be configured independently
        EnemySpawnConfig ugg;
        EnemySpawnConfig wrongway;
        EnemySpawnConfig slickSam;

        // Number of discs to spawn per round index
        std::vector<int> discCountsPerRound;
    };

    struct GameConfig
    {
        float freezeDuration{ 1.5f };
        float discFlightDuration{ 2.0f };
        int pointsPerCubeChange{ 25 };
        int pointsCoilyDisc{ 500 };
        int pointsDiscRemaining{ 50 };
        int pointsSlickSam{ 300 };
    };

    LevelData LoadLevelData(const std::string& filePath);
    GameConfig LoadGameConfig(const std::string& filePath);
}