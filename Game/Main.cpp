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

    // Tear down existing scene and gameplay bindings
    // The scene manager removes scenes marked for removal on next Update,
    // so mark all current scenes and create a fresh one immediately.
    // Gameplay bindings reference old component pointers so unbind them first.
    input.UnbindKeyboardCommand(SDL_SCANCODE_W, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_A, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_D, dae::Controller::KeyState::Down);
    input.UnbindKeyboardCommand(SDL_SCANCODE_S, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadUp, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadLeft, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadRight, dae::Controller::KeyState::Down);
    input.UnbindControllerCommand(0, dae::Controller::Button::DPadDown, dae::Controller::KeyState::Down);

    // Mark old scenes for removal (cleaned up at start of next Update)
    // We create the new scene immediately so it is ready this frame.
    // SceneManager iterates all scenes each frame, so old ones are gone next tick.
    sceneManager.MarkAllScenesForRemoval();
    auto& scene = sceneManager.CreateScene();

    // FPS counter
    auto fpsGo = std::make_unique<dae::GameObject>();
    auto font = dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 18);
    auto* fpsText = fpsGo->AddComponent<dae::TextComponent>("0 FPS", font);
    fpsText->SetColor({ 255, 255, 255, 255 });
    fpsText->SetPosition(10.f, 10.f);
    fpsGo->AddComponent<dae::FPSComponent>();
    scene.Add(std::move(fpsGo));

    dae::LevelData levelData = dae::LoadLevelData((g_dataLocation / LEVEL_FILES[levelIndex]).string());

    dae::PyramidGrid grid = dae::BuildPyramid(scene, levelData, 0);
    (void)grid;

    auto qbertGo = std::make_unique<dae::GameObject>();
    glm::vec2 startPos = dae::GridToCharacterPos(0, 0, QBERT_SRC_W, QBERT_SRC_H);
    qbertGo->SetLocalPosition(startPos.x, startPos.y);
    qbertGo->AddComponent<dae::SpritesheetComponent>("Qbert P1 Spritesheet.png", QBERT_SRC_W, QBERT_SRC_H, dae::PIXEL_SCALE);
    auto* qbert = qbertGo->AddComponent<dae::QbertPlayerComponent>(0, 0, 3);
    scene.Add(std::move(qbertGo));

    auto coilyGo = std::make_unique<dae::GameObject>();
    glm::vec2 coilyPos = dae::GridToCharacterPos(0, 0, COILY_SRC_W, COILY_SRC_H);
    coilyGo->SetLocalPosition(coilyPos.x, coilyPos.y);
    coilyGo->AddComponent<dae::SpritesheetComponent>("Coily Spritesheet.png", COILY_SRC_W, COILY_SRC_H, dae::PIXEL_SCALE);
    coilyGo->AddComponent<dae::CoilyComponent>(1.f / levelData.enemyMoveSpeed);
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