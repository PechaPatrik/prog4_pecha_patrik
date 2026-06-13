#pragma once
#include "Singleton.h"
#include "GameMode.h"
#include "LevelData.h"
#include "QbertPyramid.h"
#include <filesystem>
#include <array>
#include <string>
#include <vector>

namespace dae
{
    class Scene;
    class QbertPlayerComponent;
    class CoilyComponent;
    class ScoreDisplayComponent;
    class LivesDisplayComponent;

    struct PlayerEndData
    {
        int score{ 0 };
        int playerIndex{ 0 };
    };

    class SceneLoader final : public Singleton<SceneLoader>
    {
    public:
        static constexpr int LEVEL_COUNT = 3;
        static constexpr int QBERT_SRC_W = 17;
        static constexpr int QBERT_SRC_H = 16;

        static const std::array<const char*, LEVEL_COUNT> LEVEL_FILES;
        static const std::array<const char*, LEVEL_COUNT> LEVEL_TITLE_IMAGES;

        void Init(const std::filesystem::path& dataLocation, const GameConfig& config);

        void LoadMainMenu();
        void LoadInstructionScreen(GameMode mode);
        void LoadLevelIntro(int levelIndex);
        void LoadGameplay(int levelIndex, int round);
        void LoadPauseOverlay();
        void Unpause();
        void LoadEndScreen(bool victory);
        void LoadNameEntry(bool victory);
        void LoadHighscoreDisplay();

        void AdvanceRound();
        void SkipLevel();

        GameMode GetCurrentMode() const { return m_gameMode; }
        const GameConfig& GetConfig() const { return m_gameConfig; }
        const std::filesystem::path& GetDataLocation() const { return m_dataLocation; }

        int GetCurrentLevel() const { return m_currentLevel; }
        int GetCurrentRound() const { return m_currentRound; }

        void SetPlayerEndData(const std::vector<PlayerEndData>& data) { m_playerEndData = data; }
        const std::vector<PlayerEndData>& GetPlayerEndData() const { return m_playerEndData; }

        bool IsVictory() const { return m_lastEndWasVictory; }

    private:
        friend class Singleton<SceneLoader>;
        SceneLoader() = default;

        void UnbindGameplayInputs();
        void BindGameplayInputsForPlayer(QbertPlayerComponent* player, int playerIndex) const;
        void BindVersusCoilyInputs(CoilyComponent* coily);

        std::filesystem::path m_dataLocation{};
        GameConfig m_gameConfig{};
        GameMode m_gameMode{ GameMode::Solo };

        int m_currentLevel{ 0 };
        int m_currentRound{ 0 };

        PyramidGrid m_pyramidGrid{};

        std::array<int, 2> m_persistedScores{ 0, 0 };
        std::array<int, 2> m_persistedLives{ -1, -1 };

        std::vector<PlayerEndData> m_playerEndData{};
        bool m_lastEndWasVictory{ false };

        Scene* m_gameplayScene{ nullptr };
    };
}