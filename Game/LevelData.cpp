#include "LevelData.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace dae
{
    static EnemySpawnConfig ParseEnemyConfig(const nlohmann::json& j, int roundCount)
    {
        EnemySpawnConfig cfg;

        if (j.contains("enabledPerRound"))
            cfg.enabledPerRound = j["enabledPerRound"].get<std::vector<bool>>();
        else if (j.value("enabled", false))
            cfg.enabledPerRound.assign(roundCount, true);

        if (j.contains("firstSpawnDelayPerRound"))
            cfg.firstSpawnDelayPerRound = j["firstSpawnDelayPerRound"].get<std::vector<float>>();
        else
        {
            float singleDelay = j.value("firstSpawnDelay", 5.f);
            cfg.firstSpawnDelayPerRound.assign(roundCount, singleDelay);
        }

        if (j.contains("spawnLocations"))
        {
            for (auto& loc : j["spawnLocations"])
                cfg.spawnLocations.emplace_back(loc[0].get<int>(), loc[1].get<int>());
        }

        if (j.contains("spawnIntervalMin"))
            cfg.spawnIntervalMin = j["spawnIntervalMin"].get<std::vector<float>>();
        else
            cfg.spawnIntervalMin.assign(roundCount, 3.f);

        if (j.contains("spawnIntervalMax"))
            cfg.spawnIntervalMax = j["spawnIntervalMax"].get<std::vector<float>>();
        else
            cfg.spawnIntervalMax.assign(roundCount, 6.f);

        if (j.contains("hopIntervals"))
            cfg.hopIntervals = j["hopIntervals"].get<std::vector<float>>();
        else
            cfg.hopIntervals.assign(roundCount, 1.f);

        if (j.contains("waitAtBottomPerRound"))
            cfg.waitAtBottomPerRound = j["waitAtBottomPerRound"].get<std::vector<float>>();
        else
        {
            float single = j.value("waitAtBottom", 0.f);
            cfg.waitAtBottomPerRound.assign(roundCount, single);
        }

        return cfg;
    }

    LevelData LoadLevelData(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("Could not open level file: " + filePath);

        nlohmann::json j;
        file >> j;

        LevelData data;
        data.roundColorColumns = j["roundColorColumns"].get<std::vector<int>>();
        int roundCount = static_cast<int>(data.roundColorColumns.size());

        std::string ruleStr = j["rule"].get<std::string>();
        if (ruleStr == "SingleStep")
            data.rule = LevelRule::SingleStep;
        else if (ruleStr == "DoubleStep")
            data.rule = LevelRule::DoubleStep;
        else if (ruleStr == "RevertOnRetouch")
            data.rule = LevelRule::RevertOnRetouch;
        else
            data.rule = LevelRule::SingleStep;

        if (j.contains("rowWidths"))
            data.rowWidths = j["rowWidths"].get<std::vector<int>>();
        else
            data.rowWidths = { 1, 2, 3, 4, 5, 6, 7 };

        if (j.contains("rowOffsets"))
            data.rowOffsets = j["rowOffsets"].get<std::vector<int>>();
        else
            data.rowOffsets.assign(data.rowWidths.size(), 0);

        // Ensure offsets vector matches rowWidths length
        if (data.rowOffsets.size() < data.rowWidths.size())
            data.rowOffsets.resize(data.rowWidths.size(), 0);

        if (j.contains("coily"))
            data.coily = ParseEnemyConfig(j["coily"], roundCount);

        if (j.contains("ugg"))
            data.ugg = ParseEnemyConfig(j["ugg"], roundCount);

        if (j.contains("wrongway"))
            data.wrongway = ParseEnemyConfig(j["wrongway"], roundCount);

        if (j.contains("slickSam"))
            data.slickSam = ParseEnemyConfig(j["slickSam"], roundCount);

        if (j.contains("discCountsPerRound"))
            data.discCountsPerRound = j["discCountsPerRound"].get<std::vector<int>>();
        else
            data.discCountsPerRound.assign(roundCount, 2);

        data.enemyIntroSpeed = j.value("enemyIntroSpeed", 400.f);

        return data;
    }

    GameConfig LoadGameConfig(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("Could not open game config: " + filePath);

        nlohmann::json j;
        file >> j;

        GameConfig cfg;
        cfg.pixelScale = j.value("pixelScale", 3.f);
        cfg.freezeDuration = j.value("freezeDuration", 1.5f);
        cfg.discFlightDuration = j.value("discFlightDuration", 2.0f);
        cfg.discDropDuration = j.value("discDropDuration", 0.25f);
        cfg.discFrameDuration = j.value("discFrameDuration", 0.12f);
        cfg.arcHeight = j.value("arcHeight", 12.f);
        cfg.enemyFallGravity = j.value("enemyFallGravity", 800.f);
        cfg.hopDurationQbert = j.value("hopDurationQbert", 0.3f);
        cfg.pointsPerCubeChange = j.value("pointsPerCubeChange", 25);
        cfg.pointsCoilyDisc = j.value("pointsCoilyDisc", 500);
        cfg.pointsDiscRemaining = j.value("pointsDiscRemaining", 50);
        cfg.pointsSlickSam = j.value("pointsSlickSam", 300);
        cfg.maxLives = j.value("maxLives", 3);
        cfg.curseOffsetX = j.value("curseOffsetX", 12.f);
        cfg.curseOffsetY = j.value("curseOffsetY", 24.f);
        return cfg;
    }
}