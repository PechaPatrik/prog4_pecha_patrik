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
        cfg.spawnRow = j.value("spawnRow", 0);
        cfg.spawnCol = j.value("spawnCol", 0);

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

        data.freezeDuration = j.value("freezeDuration", 1.f);

        if (j.contains("coily"))
            data.coily = ParseEnemyConfig(j["coily"], roundCount);

        if (j.contains("uggWrongway"))
            data.uggWrongway = ParseEnemyConfig(j["uggWrongway"], roundCount);

        if (j.contains("slickSam"))
            data.slickSam = ParseEnemyConfig(j["slickSam"], roundCount);

        if (j.contains("points"))
        {
            auto& pts = j["points"];
            data.pointsPerCubeChange = pts.value("cubeChange", 25);
            data.pointsCoilyDisc = pts.value("coilyDisc", 500);
            data.pointsDiscRemaining = pts.value("discRemaining", 50);
            data.pointsSlickSam = pts.value("slickSam", 300);
        }

        return data;
    }
}