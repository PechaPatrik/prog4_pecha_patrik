#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "InputManager.h"
#include "Controller.h"
#include "ServiceLocator.h"
#include "SDLSoundSystem.h"
#include "LoggingSoundSystem.h"
#include "SoundId.h"
#include "CallbackCommand.h"
#include "SceneLoader.h"
#include "LevelData.h"
#include "GameScale.h"
#include <filesystem>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

static bool g_muted = false;

int main(int, char* [])
{
    fs::path dataLocation;
#if __EMSCRIPTEN__
    dataLocation = "";
#else
    dataLocation = "./Data/";
    if (!fs::exists(dataLocation))
        dataLocation = "../Data/";
#endif

    dae::GameConfig config = dae::LoadGameConfig(
        (dataLocation / "game_config.json").string());
    dae::PIXEL_SCALE = config.pixelScale;

    int windowW = dae::GameWindowW();
    int windowH = dae::GameWindowH();

    dae::Minigin engine(dataLocation, windowW, windowH);

#if _DEBUG
    auto soundSystem = std::make_unique<dae::LoggingSoundSystem>(
        std::make_unique<dae::SDLSoundSystem>());
#else
    auto soundSystem = std::make_unique<dae::SDLSoundSystem>();
#endif

    soundSystem->RegisterSound(
        dae::SoundId::QbertHit,
        dataLocation.string() + "Sounds/Qbert Hit.wav");
    dae::ServiceLocator::RegisterSoundSystem(std::move(soundSystem));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    dae::SceneLoader::GetInstance().Init(dataLocation, config);

    engine.Run([]()
        {
            auto& input = dae::InputManager::GetInstance();

            input.BindKeyboardCommand(SDL_SCANCODE_F2, dae::Controller::KeyState::Down,
                std::make_unique<dae::CallbackCommand>([]()
                    {
                        g_muted = !g_muted;
                        dae::ServiceLocator::GetSoundSystem().SetMuted(g_muted);
                    }));

            dae::SceneLoader::GetInstance().LoadMainMenu();
        });

    dae::ServiceLocator::RegisterSoundSystem(nullptr);
    return 0;
}