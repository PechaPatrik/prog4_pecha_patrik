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

    soundSystem->RegisterSound(dae::SoundId::ChangeSelection,
        dataLocation.string() + "Sounds/Change Selection.wav");
    soundSystem->RegisterSound(dae::SoundId::CoilyEggJump,
        dataLocation.string() + "Sounds/Coily Egg Jump.wav");
    soundSystem->RegisterSound(dae::SoundId::CoilyFall,
        dataLocation.string() + "Sounds/Coily Fall.wav");
    soundSystem->RegisterSound(dae::SoundId::CoilySnakeJump,
        dataLocation.string() + "Sounds/Coily Snake Jump.wav");
    soundSystem->RegisterSound(dae::SoundId::DiskLand,
        dataLocation.string() + "Sounds/Disk Land.wav");
    soundSystem->RegisterSound(dae::SoundId::DiskLift,
        dataLocation.string() + "Sounds/Disk Lift.wav");
    soundSystem->RegisterSound(dae::SoundId::LevelScreenTune,
        dataLocation.string() + "Sounds/Level Screen Tune.wav");
    soundSystem->RegisterSound(dae::SoundId::OtherFoesJump,
        dataLocation.string() + "Sounds/Other Foes Jump.wav");
    soundSystem->RegisterSound(dae::SoundId::QbertFall,
        dataLocation.string() + "Sounds/QBert Fall.wav");
    soundSystem->RegisterSound(dae::SoundId::QbertHit,
        dataLocation.string() + "Sounds/Qbert Hit.wav");
    soundSystem->RegisterSound(dae::SoundId::QbertJump,
        dataLocation.string() + "Sounds/QBert Jump.wav");
    soundSystem->RegisterSound(dae::SoundId::RoundCompleteTune,
        dataLocation.string() + "Sounds/Round Complete Tune.wav");
    soundSystem->RegisterSound(dae::SoundId::SlickSamCaught,
        dataLocation.string() + "Sounds/SlickSam Caught.wav");
    soundSystem->RegisterSound(dae::SoundId::Swearing,
        dataLocation.string() + "Sounds/Swearing.wav");
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