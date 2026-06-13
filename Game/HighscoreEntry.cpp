#include "HighscoreEntry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <algorithm>

namespace dae
{
    std::vector<HighscoreEntry> LoadHighscores(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return {};

        nlohmann::json j;
        file >> j;

        std::vector<HighscoreEntry> entries;
        for (auto& item : j)
        {
            HighscoreEntry e;
            e.name = item.value("name", "???");
            e.score = item.value("score", 0);
            entries.push_back(e);
        }
        return entries;
    }

    void SaveHighscores(const std::string& filePath, const std::vector<HighscoreEntry>& entries)
    {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& e : entries)
        {
            j.push_back({ {"name", e.name}, {"score", e.score} });
        }
        std::ofstream file(filePath);
        if (file.is_open())
            file << j.dump(2);
    }

    void AddHighscore(std::vector<HighscoreEntry>& list, const HighscoreEntry& entry)
    {
        list.push_back(entry);
        std::sort(list.begin(), list.end(), [](const HighscoreEntry& a, const HighscoreEntry& b) {
            return a.score > b.score;
            });
        if (list.size() > 10)
            list.resize(10);
    }
}
