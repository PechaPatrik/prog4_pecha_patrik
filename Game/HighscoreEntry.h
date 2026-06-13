#pragma once
#include <string>
#include <vector>

namespace dae
{
    struct HighscoreEntry
    {
        std::string name{};
        int score{ 0 };
    };

    std::vector<HighscoreEntry> LoadHighscores(const std::string& filePath);
    void SaveHighscores(const std::string& filePath, const std::vector<HighscoreEntry>& entries);

    // Inserts a new entry and keeps the list sorted descending by score, max 10 entries.
    void AddHighscore(std::vector<HighscoreEntry>& list, const HighscoreEntry& entry);
}
