#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "InputManager.h"
#include "Controller.h"
#include "Scene.h"
#include "GameObject.h"
#include "FPSComponent.h"
#include "TextComponent.h"
#include "SpritesheetComponent.h"
#include "QbertPlayerComponent.h"
#include "QbertMoveCommand.h"
#include "QbertPyramid.h"
#include "LevelData.h"
#include "ServiceLocator.h"
#include "SDLSoundSystem.h"
#include "LoggingSoundSystem.h"
#include "SoundId.h"
#include "Command.h"
#include "ScoreDisplayComponent.h"
#include "LivesDisplayComponent.h"
#include "ImageComponent.h"
#include "EnemySpawnerComponent.h"
#include "GameStateManager.h"
#include "GsmUpdaterComponent.h"
#include "RoundClearAnimatorComponent.h"
#include "BonusDisplayComponent.h"
#include "GameScale.h"
#include <filesystem>
#include <array>
namespace fs = std::filesystem;

static constexpr int LEVEL_COUNT = 3;
static constexpr int QBERT_SRC_W = 17;
static constexpr int QBERT_SRC_H = 16;

static int g_currentLevel = 0;
static int g_currentRound = 0;
static fs::path g_dataLocation;
static dae::PyramidGrid g_pyramidGrid;
static dae::GameConfig g_gameConfig;
static int g_persistedScore = 0;
static int g_persistedLives = -1;

static const std::array<const char*, LEVEL_COUNT> LEVEL_FILES =
{
    "Levels/level1.json",
    "Levels/level2.json",
    "Levels/level3.json"
};

static void LoadLevel(int levelIndex, int round);

namespace dae
{
    class SkipLevelCommand final : public Command
    {
    public:
        void Execute() override
        {
            auto& gsm = dae::GameStateManager::GetInstance();
            const auto& players = gsm.GetPlayers();
            if (!players.empty() && players[0])
            {
                g_persistedScore = players[0]->GetScore();
                g_persistedLives = players[0]->GetLives();
            }
            g_currentLevel = (g_currentLevel + 1) % LEVEL_COUNT;
            g_currentRound = 0;
            LoadLevel(g_currentLevel, g_currentRound);
        }
    };
}

static void AdvanceRound()
{
    // Capture player state before the scene is torn down
    auto& gsm = dae::GameStateManager::GetInstance();
    const auto& players = gsm.GetPlayers();
    if (!players.empty() && players[0])
    {
        g_persistedScore = players[0]->GetScore();
        g_persistedLives = players[0]->GetLives();
    }

    dae::LevelData levelData = dae::LoadLevelData(
        (g_dataLocation / LEVEL_FILES[g_currentLevel]).string());
    int roundCount = static_cast<int>(levelData.roundColorColumns.size());

    g_currentRound++;
    if (g_currentRound >= roundCount)
    {
        g_currentRound = 0;
        g_currentLevel = (g_currentLevel + 1) % LEVEL_COUNT;
    }

    LoadLevel(g_currentLevel, g_currentRound);
}

static void LoadLevel(int levelIndex, int round)
{
    auto& sceneManager = dae::SceneManager::GetInstance();
    auto& input = dae::InputManager::GetInstance();
    auto& gsm = dae::GameStateManager::GetInstance();

    gsm.Reset();
    gsm.SetCurseOffset(g_gameConfig.curseOffsetX, g_gameConfig.curseOffsetY);

    input.UnbindKeyboardCommand(SDL_SCANCODE_W, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_A, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_D, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_S, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadUp, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadLeft, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadRight, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadDown, dae::Controller::KeyState::Down);

    sceneManager.MarkAllScenesForRemoval();
    auto& scene = sceneManager.CreateScene();

    auto fpsGo = std::make_unique<dae::GameObject>();
    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
    auto* fpsText = fpsGo->AddComponent<dae::TextComponent>("0 FPS", font);
    fpsText->SetColor({ 255, 255, 255, 255 });
    fpsText->SetPosition(10.f, 10.f);
    fpsGo->AddComponent<dae::FPSComponent>();
    scene.Add(std::move(fpsGo));

    dae::LevelData levelData = dae::LoadLevelData((g_dataLocation / LEVEL_FILES[levelIndex]).string());
    int roundsPerLevel = static_cast<int>(levelData.roundColorColumns.size());

    auto mcFont = dae::ResourceManager::GetInstance().LoadFont("Minecraft.ttf", 32);
    auto mcFontSmall = dae::ResourceManager::GetInstance().LoadFont("Minecraft.ttf", 24);

    auto titleGo = std::make_unique<dae::GameObject>();
    titleGo->SetLocalPosition(10.f, 40.f);
    auto* titleSheet = titleGo->AddComponent<dae::SpritesheetComponent>("Player Titles.png", 65, 11);
    float titleH = titleSheet->GetFrameHeight() * dae::PIXEL_SCALE;
    scene.Add(std::move(titleGo));

    float scoreY = 40.f + titleH + 4.f;

    auto scoreGo = std::make_unique<dae::GameObject>();
    auto* scoreText = scoreGo->AddComponent<dae::TextComponent>("0", mcFont);
    scoreText->SetColor({ 255, 165, 0, 255 });
    scoreText->SetPosition(10.f, scoreY);
    auto* scoreObs = scoreGo->AddComponent<dae::ScoreDisplayComponent>();
    scene.Add(std::move(scoreGo));

    float changeY = scoreY + 48.f;

    auto changeLabelGo = std::make_unique<dae::GameObject>();
    auto* changeText = changeLabelGo->AddComponent<dae::TextComponent>("CHANGE TO:", mcFontSmall);
    changeText->SetColor({ 220, 30, 30, 255 });
    changeText->SetPosition(10.f, changeY);
    scene.Add(std::move(changeLabelGo));

    int colorColIdx = round % roundsPerLevel;
    int colorColumn = levelData.roundColorColumns[colorColIdx];

    auto iconGo = std::make_unique<dae::GameObject>();
    iconGo->SetLocalPosition(10.f + 172.f, changeY - 8.f);
    auto* iconSheet = iconGo->AddComponent<dae::SpritesheetComponent>("Color Icons Spritesheet.png", 14, 12);
    iconSheet->SetFrame(colorColumn, 1);
    scene.Add(std::move(iconGo));

    float rightX = static_cast<float>(dae::GameWindowW()) - 200.f;
    auto levelLabelGo = std::make_unique<dae::GameObject>();
    auto* levelLabelText = levelLabelGo->AddComponent<dae::TextComponent>("LEVEL:", mcFontSmall);
    levelLabelText->SetColor({ 30, 200, 30, 255 });
    levelLabelText->SetPosition(rightX, 80.f);
    scene.Add(std::move(levelLabelGo));

    auto levelNumGo = std::make_unique<dae::GameObject>();
    auto* levelNumText = levelNumGo->AddComponent<dae::TextComponent>(std::to_string(levelIndex + 1), mcFontSmall);
    levelNumText->SetColor({ 255, 165, 0, 255 });
    levelNumText->SetPosition(rightX + 110.f, 80.f);
    scene.Add(std::move(levelNumGo));

    auto roundLabelGo = std::make_unique<dae::GameObject>();
    auto* roundLabelText = roundLabelGo->AddComponent<dae::TextComponent>("ROUND:", mcFontSmall);
    roundLabelText->SetColor({ 255, 105, 180, 255 });
    roundLabelText->SetPosition(rightX, 80.f + 28.f);
    scene.Add(std::move(roundLabelGo));

    auto roundNumGo = std::make_unique<dae::GameObject>();
    auto* roundNumText = roundNumGo->AddComponent<dae::TextComponent>(std::to_string(round + 1), mcFontSmall);
    roundNumText->SetColor({ 255, 165, 0, 255 });
    roundNumText->SetPosition(rightX + 110.f, 80.f + 28.f);
    scene.Add(std::move(roundNumGo));

    static constexpr float HEART_SRC_SIZE = 14.f;
    float heartRenderedH = HEART_SRC_SIZE * dae::PIXEL_SCALE;
    float heartStartY = changeY + 48.f;

    auto livesGo = std::make_unique<dae::GameObject>();
    auto* livesDisplay = livesGo->AddComponent<dae::LivesDisplayComponent>(g_gameConfig.maxLives);
    scene.Add(std::move(livesGo));

    for (int i = 0; i < g_gameConfig.maxLives; ++i)
    {
        auto heartGo = std::make_unique<dae::GameObject>();
        heartGo->SetLocalPosition(10.f, heartStartY + i * (heartRenderedH + 4.f));
        heartGo->AddComponent<dae::ImageComponent>("Heart.png", dae::PIXEL_SCALE);
        livesDisplay->SetHeart(i, heartGo.get());
        scene.Add(std::move(heartGo));
    }

    g_pyramidGrid = dae::BuildPyramid(scene, levelData, round);

    int apexCol = g_pyramidGrid.rowOffsets.empty() ? 0 : g_pyramidGrid.rowOffsets[0];

    // Bonus display: pre-built in the scene, activated/deactivated by BonusDisplayComponent
    {
        float bonusY = dae::PyramidTopY()
            + static_cast<float>(g_pyramidGrid.NumRows()) * dae::TileStepY()
            + 12.f * dae::PIXEL_SCALE;
        float midX = static_cast<float>(dae::GameWindowW()) * 0.5f;
        static constexpr float GAP = 8.f;

        auto bonusFont = dae::ResourceManager::GetInstance().LoadFont("Minecraft.ttf", 32);

        auto bonusLabelGo = std::make_unique<dae::GameObject>();
        auto* bonusLabelText = bonusLabelGo->AddComponent<dae::TextComponent>("BONUS", bonusFont);
        bonusLabelText->SetColor({ 180, 0, 255, 255 });
        bonusLabelText->SetPosition(midX - 110.f - GAP, bonusY);
        dae::GameObject* bonusLabelRaw = bonusLabelGo.get();
        scene.Add(std::move(bonusLabelGo));

        auto bonusValueGo = std::make_unique<dae::GameObject>();
        auto* bonusValueText = bonusValueGo->AddComponent<dae::TextComponent>("0", bonusFont);
        bonusValueText->SetColor({ 255, 165, 0, 255 });
        bonusValueText->SetPosition(midX + GAP, bonusY);
        dae::GameObject* bonusValueRaw = bonusValueGo.get();
        scene.Add(std::move(bonusValueGo));

        auto bonusCoordGo = std::make_unique<dae::GameObject>();
        bonusCoordGo->AddComponent<dae::BonusDisplayComponent>(
            bonusLabelRaw, bonusValueRaw, bonusValueText);
        scene.Add(std::move(bonusCoordGo));
    }

    auto qbertGo = std::make_unique<dae::GameObject>();
    glm::vec2 startPos = dae::GridToCharacterPos(0, apexCol, QBERT_SRC_W, QBERT_SRC_H);
    qbertGo->SetLocalPosition(startPos.x, startPos.y);
    qbertGo->AddComponent<dae::SpritesheetComponent>("Qbert P1 Spritesheet.png", QBERT_SRC_W, QBERT_SRC_H);
    auto* qbert = qbertGo->AddComponent<dae::QbertPlayerComponent>(0, 0, apexCol, g_gameConfig.maxLives);
    qbert->SetPyramidGrid(&g_pyramidGrid);
    qbert->SetScene(&scene);
    qbert->SetFreezeDuration(g_gameConfig.freezeDuration);
    qbert->SetPointsPerCubeChange(g_gameConfig.pointsPerCubeChange);
    qbert->SetPointsSlickSam(g_gameConfig.pointsSlickSam);
    qbert->SetArcHeight(g_gameConfig.arcHeight);
    qbert->SetHopDuration(g_gameConfig.hopDurationQbert);
    qbert->SetDiscDropDuration(g_gameConfig.discDropDuration);

    qbert->SetInitialScore(g_persistedScore);
    if (g_persistedLives >= 0)
        qbert->SetInitialLives(g_persistedLives);

    qbert->AddObserver(scoreObs);
    qbert->AddObserver(livesDisplay);
    qbert->NotifyInitialValues();

    gsm.RegisterPlayer(qbert);

    // Spawner added before Q*bert so Q*bert renders on top of all enemies and discs
    auto gsmAndSpawnerGo = std::make_unique<dae::GameObject>();
    gsmAndSpawnerGo->AddComponent<dae::GsmUpdaterComponent>(
        &scene,
        &g_pyramidGrid,
        g_gameConfig.roundClearDuration,
        g_gameConfig.pointsDiscRemaining,
        g_gameConfig.roundBonusBase,
        g_gameConfig.roundBonusIncrement,
        g_gameConfig.roundBonusDisplayDuration,
        levelIndex,
        round,
        roundsPerLevel,
        AdvanceRound);
    gsmAndSpawnerGo->AddComponent<dae::EnemySpawnerComponent>(
        levelData, g_gameConfig, &g_pyramidGrid, &scene, gsm.GetPlayers(), round);
    gsmAndSpawnerGo->AddComponent<dae::RoundClearAnimatorComponent>(
        &g_pyramidGrid, colorColumn, g_gameConfig.roundClearFrameInterval);
    scene.Add(std::move(gsmAndSpawnerGo));

    scene.Add(std::move(qbertGo));

    input.BindKeyboardCommand(SDL_SCANCODE_W, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 0));
    input.BindKeyboardCommand(SDL_SCANCODE_A, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 1));
    input.BindKeyboardCommand(SDL_SCANCODE_D, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 2));
    input.BindKeyboardCommand(SDL_SCANCODE_S, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 3));

    input.BindControllerCommand(0, dae::Controller::Button::DPadUp, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 0));
    input.BindControllerCommand(0, dae::Controller::Button::DPadLeft, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 1));
    input.BindControllerCommand(0, dae::Controller::Button::DPadRight, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 2));
    input.BindControllerCommand(0, dae::Controller::Button::DPadDown, dae::Controller::KeyState::Down,
        std::make_unique<dae::QbertMoveCommand>(qbert, 3));
}

int main(int, char* [])
{
#if __EMSCRIPTEN__
    g_dataLocation = "";
#else
    g_dataLocation = "./Data/";
    if (!fs::exists(g_dataLocation))
        g_dataLocation = "../Data/";
#endif

    g_gameConfig = dae::LoadGameConfig((g_dataLocation / "game_config.json").string());
    dae::PIXEL_SCALE = g_gameConfig.pixelScale;

    int windowW = dae::GameWindowW();
    int windowH = dae::GameWindowH();

    dae::Minigin engine(g_dataLocation, windowW, windowH);

#if _DEBUG
    auto soundSystem = std::make_unique<dae::LoggingSoundSystem>(
        std::make_unique<dae::SDLSoundSystem>());
#else
    auto soundSystem = std::make_unique<dae::SDLSoundSystem>();
#endif

    soundSystem->RegisterSound(dae::SoundId::QbertHit, g_dataLocation.string() + "Sounds/Qbert Hit.wav");
    dae::ServiceLocator::RegisterSoundSystem(std::move(soundSystem));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    engine.Run([]()
        {
            dae::InputManager::GetInstance().BindKeyboardCommand(
                SDL_SCANCODE_F1, dae::Controller::KeyState::Down,
                std::make_unique<dae::SkipLevelCommand>());

            LoadLevel(g_currentLevel, g_currentRound);
        });

    dae::ServiceLocator::RegisterSoundSystem(nullptr);
    return 0;
}