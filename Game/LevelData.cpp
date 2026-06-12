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

        cfg.firstSpawnDelay = j.value("firstSpawnDelay", 5.f);

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
        cfg.freezeDuration = j.value("freezeDuration", 1.5f);
        cfg.discFlightDuration = j.value("discFlightDuration", 2.0f);
        cfg.pointsPerCubeChange = j.value("pointsPerCubeChange", 25);
        cfg.pointsCoilyDisc = j.value("pointsCoilyDisc", 500);
        cfg.pointsDiscRemaining = j.value("pointsDiscRemaining", 50);
        cfg.pointsSlickSam = j.value("pointsSlickSam", 300);
        return cfg;
    }
}