#include "LevelData.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

namespace dae
{
    LevelData LoadLevelData(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            throw std::runtime_error("Could not open level file: " + filePath);

        nlohmann::json j;
        file >> j;

        LevelData data;
        data.roundColorColumns = j["roundColorColumns"].get<std::vector<int>>();

        std::string ruleStr = j["rule"].get<std::string>();
        if (ruleStr == "SingleStep")
            data.rule = LevelRule::SingleStep;
        else if (ruleStr == "DoubleStep")
            data.rule = LevelRule::DoubleStep;
        else if (ruleStr == "RevertOnRetouch")
            data.rule = LevelRule::RevertOnRetouch;
        else
            data.rule = LevelRule::SingleStep;

        data.enemySpawnInterval = j.value("enemySpawnInterval", 5.0f);
        data.enemyMoveSpeed = j.value("enemyMoveSpeed", 2.0f);

        return data;
    }
}
