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
        std::vector<float> firstSpawnDelayPerRound;
        std::vector<std::pair<int, int>> spawnLocations;
        std::vector<float> spawnIntervalMin;
        std::vector<float> spawnIntervalMax;
        std::vector<float> hopIntervals;
        // Only used by Coily: time at bottom of pyramid before transforming
        std::vector<float> waitAtBottomPerRound;
    };

    struct LevelData
    {
        std::vector<int> roundColorColumns;
        LevelRule rule{};

        // Pyramid shape: rowWidths[r] = number of tiles in row r
        // rowOffsets[r] = horizontal shift of that row in tile-width units (can be negative)
        std::vector<int> rowWidths;
        std::vector<int> rowOffsets;

        EnemySpawnConfig coily;
        EnemySpawnConfig ugg;
        EnemySpawnConfig wrongway;
        EnemySpawnConfig slickSam;

        std::vector<int> discCountsPerRound;

        // Pixels per second for enemy intro slide animation (level-specific)
        float enemyIntroSpeed{ 400.f };
    };

    struct GameConfig
    {
        float pixelScale{ 3.f };
        float freezeDuration{ 1.5f };
        float discFlightDuration{ 2.0f };
        float discDropDuration{ 0.25f };
        float discFrameDuration{ 0.12f };
        float arcHeight{ 12.f };
        float enemyFallGravity{ 800.f };
        float hopDurationQbert{ 0.3f };
        int pointsPerCubeChange{ 25 };
        int pointsCoilyDisc{ 500 };
        int pointsDiscRemaining{ 50 };
        int pointsSlickSam{ 300 };
        int maxLives{ 3 };
        float curseOffsetX{ 12.f };
        float curseOffsetY{ 24.f };
        float roundClearDuration{ 3.f };
        float roundClearFrameInterval{ 0.1f };
        int roundBonusBase{ 1000 };
        int roundBonusIncrement{ 250 };
        float roundBonusDisplayDuration{ 2.f };
    };

    LevelData LoadLevelData(const std::string& filePath);
    GameConfig LoadGameConfig(const std::string& filePath);
}