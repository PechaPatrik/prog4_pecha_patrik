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
#include "CoilyComponent.h"
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

#include <filesystem>
#include <array>
namespace fs = std::filesystem;

static constexpr int LEVEL_COUNT = 3;
static constexpr int QBERT_SRC_W = 17;
static constexpr int QBERT_SRC_H = 16;
static constexpr int COILY_SRC_W = 16;
static constexpr int COILY_SRC_H = 32;

static int g_currentLevel = 0;
static fs::path g_dataLocation;
static dae::PyramidGrid g_pyramidGrid;

static const std::array<const char*, LEVEL_COUNT> LEVEL_FILES =
{
    "Levels/level1.json",
    "Levels/level2.json",
    "Levels/level3.json"
};

static void LoadLevel(int levelIndex);

namespace dae
{
    class SkipLevelCommand final : public Command
    {
    public:
        void Execute() override
        {
            g_currentLevel = (g_currentLevel + 1) % LEVEL_COUNT;
            LoadLevel(g_currentLevel);
        }
    };
}

static void LoadLevel(int levelIndex)
{
    auto& sceneManager = dae::SceneManager::GetInstance();
    auto& input = dae::InputManager::GetInstance();

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

    auto iconGo = std::make_unique<dae::GameObject>();
    iconGo->SetLocalPosition(10.f + 172.f, changeY - 8.f);
    auto* iconSheet = iconGo->AddComponent<dae::SpritesheetComponent>("Color Icons Spritesheet.png", 14, 12);
    iconSheet->SetFrame(levelData.roundColorColumns[0], 1);
    scene.Add(std::move(iconGo));

    float rightX = dae::WINDOW_W - 200.f;
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
    auto* roundNumText = roundNumGo->AddComponent<dae::TextComponent>("1", mcFontSmall);
    roundNumText->SetColor({ 255, 165, 0, 255 });
    roundNumText->SetPosition(rightX + 110.f, 80.f + 28.f);
    scene.Add(std::move(roundNumGo));

    // Heart.png is 28x28 source; rendered at PIXEL_SCALE
    static constexpr float HEART_SRC_SIZE = 14.f;
    float heartRenderedH = HEART_SRC_SIZE * dae::PIXEL_SCALE;
    float heartStartY = changeY + 48.f;

    auto livesGo = std::make_unique<dae::GameObject>();
    auto* livesDisplay = livesGo->AddComponent<dae::LivesDisplayComponent>(3);
    scene.Add(std::move(livesGo));

    for (int i = 0; i < dae::MAX_LIVES; ++i)
    {
        auto heartGo = std::make_unique<dae::GameObject>();
        heartGo->SetLocalPosition(10.f, heartStartY + i * (heartRenderedH + 4.f));
        auto* heart = heartGo->AddComponent<dae::ImageComponent>("Heart.png", dae::PIXEL_SCALE);
        livesDisplay->SetHeart(i, heart);
        scene.Add(std::move(heartGo));
    }

    g_pyramidGrid = dae::BuildPyramid(scene, levelData, 0);

    auto qbertGo = std::make_unique<dae::GameObject>();
    glm::vec2 startPos = dae::GridToCharacterPos(0, 0, QBERT_SRC_W, QBERT_SRC_H);
    qbertGo->SetLocalPosition(startPos.x, startPos.y);
    qbertGo->AddComponent<dae::SpritesheetComponent>("Qbert P1 Spritesheet.png", QBERT_SRC_W, QBERT_SRC_H);
    auto* qbert = qbertGo->AddComponent<dae::QbertPlayerComponent>();
    qbert->SetPyramidGrid(&g_pyramidGrid);
    qbert->AddObserver(scoreObs);
    qbert->AddObserver(livesDisplay);
    scene.Add(std::move(qbertGo));

    auto coilyGo = std::make_unique<dae::GameObject>();
    glm::vec2 coilyPos = dae::GridToCharacterPos(0, 0, COILY_SRC_W, COILY_SRC_H);
    coilyGo->SetLocalPosition(coilyPos.x, coilyPos.y);
    coilyGo->AddComponent<dae::SpritesheetComponent>("Coily Spritesheet.png", COILY_SRC_W, COILY_SRC_H);
    auto* coily = coilyGo->AddComponent<dae::CoilyComponent>(1.f / levelData.enemyMoveSpeed);
    coily->SetQbert(qbert);
    scene.Add(std::move(coilyGo));

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
    dae::Minigin engine(g_dataLocation);

#if _DEBUG
    auto soundSystem = std::make_unique<dae::LoggingSoundSystem>(
        std::make_unique<dae::SDLSoundSystem>());
#else
    auto soundSystem = std::make_unique<dae::SDLSoundSystem>();
#endif

    soundSystem->RegisterSound(dae::SoundId::QbertHit, g_dataLocation.string() + "Sounds/Qbert Hit.wav");
    dae::ServiceLocator::RegisterSoundSystem(std::move(soundSystem));

    engine.Run([]()
        {
            // F1 binding only needs to be set once, it survives level loads
            dae::InputManager::GetInstance().BindKeyboardCommand(
                SDL_SCANCODE_F1, dae::Controller::KeyState::Down,
                std::make_unique<dae::SkipLevelCommand>());

            LoadLevel(g_currentLevel);
        });

    dae::ServiceLocator::RegisterSoundSystem(nullptr);
    return 0;
}