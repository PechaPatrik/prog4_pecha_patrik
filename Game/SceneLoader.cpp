#include "SceneLoader.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "GameStateManager.h"
#include "Controller.h"
#include "Scene.h"
#include "GameObject.h"
#include "TextComponent.h"
#include "ImageComponent.h"
#include "SpritesheetComponent.h"
#include "FPSComponent.h"
#include "QbertPlayerComponent.h"
#include "QbertMoveCommand.h"
#include "CoilyMoveCommand.h"
#include "ScoreDisplayComponent.h"
#include "LivesDisplayComponent.h"
#include "EnemySpawnerComponent.h"
#include "GsmUpdaterComponent.h"
#include "RoundClearAnimatorComponent.h"
#include "BonusDisplayComponent.h"
#include "MenuNavigationComponent.h"
#include "InstructionScreenComponent.h"
#include "LevelIntroComponent.h"
#include "PauseOverlayComponent.h"
#include "EndScreenComponent.h"
#include "NameEntryComponent.h"
#include "HighscoreDisplayComponent.h"
#include "GameOverObserverComponent.h"
#include "CallbackCommand.h"
#include "GameScale.h"
#include <SDL3/SDL.h>
#include <cassert>
#include <cmath>

namespace dae
{
    const std::array<const char*, SceneLoader::LEVEL_COUNT> SceneLoader::LEVEL_FILES =
    {
        "Levels/level1.json",
        "Levels/level2.json",
        "Levels/level3.json"
    };

    const std::array<const char*, SceneLoader::LEVEL_COUNT> SceneLoader::LEVEL_TITLE_IMAGES =
    {
        "Level 01 Title.png",
        "Level 02 Title.png",
        "Level 03 Title.png"
    };

    void SceneLoader::Init(const std::filesystem::path& dataLocation, const GameConfig& config)
    {
        m_dataLocation = dataLocation;
        m_gameConfig = config;
    }

    void SceneLoader::UnbindGameplayInputs()
    {
        auto& input = InputManager::GetInstance();
        using KS = Controller::KeyState;

        input.UnbindKeyboardCommand(SDL_SCANCODE_W, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_A, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_D, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_S, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_UP, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_LEFT, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_RIGHT, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_DOWN, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_ESCAPE, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_F1, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_TAB, KS::Down);

        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            input.UnbindControllerCommand(ci, Controller::Button::DPadUp, KS::Down);
            input.UnbindControllerCommand(ci, Controller::Button::DPadLeft, KS::Down);
            input.UnbindControllerCommand(ci, Controller::Button::DPadRight, KS::Down);
            input.UnbindControllerCommand(ci, Controller::Button::DPadDown, KS::Down);
            input.UnbindControllerCommand(ci, Controller::Button::Y, KS::Down);
        }
    }

    void SceneLoader::BindGameplayInputsForPlayer(QbertPlayerComponent* player, int playerIndex) const
    {
        auto& input = InputManager::GetInstance();
        using KS = Controller::KeyState;

        if (playerIndex == 0)
        {
            input.BindKeyboardCommand(SDL_SCANCODE_W, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 0));
            input.BindKeyboardCommand(SDL_SCANCODE_A, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 1));
            input.BindKeyboardCommand(SDL_SCANCODE_D, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 2));
            input.BindKeyboardCommand(SDL_SCANCODE_S, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 3));

            bool p2UsesKeyboard = (m_gameMode == GameMode::Coop || m_gameMode == GameMode::Versus);
            if (!p2UsesKeyboard)
            {
                input.BindKeyboardCommand(SDL_SCANCODE_UP, KS::Down,
                    std::make_unique<QbertMoveCommand>(player, 0));
                input.BindKeyboardCommand(SDL_SCANCODE_LEFT, KS::Down,
                    std::make_unique<QbertMoveCommand>(player, 1));
                input.BindKeyboardCommand(SDL_SCANCODE_RIGHT, KS::Down,
                    std::make_unique<QbertMoveCommand>(player, 2));
                input.BindKeyboardCommand(SDL_SCANCODE_DOWN, KS::Down,
                    std::make_unique<QbertMoveCommand>(player, 3));
            }

            input.BindControllerCommand(0, Controller::Button::DPadUp, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 0));
            input.BindControllerCommand(0, Controller::Button::DPadLeft, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 1));
            input.BindControllerCommand(0, Controller::Button::DPadRight, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 2));
            input.BindControllerCommand(0, Controller::Button::DPadDown, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 3));
        }
        else if (playerIndex == 1)
        {
            input.BindKeyboardCommand(SDL_SCANCODE_UP, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 0));
            input.BindKeyboardCommand(SDL_SCANCODE_LEFT, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 1));
            input.BindKeyboardCommand(SDL_SCANCODE_RIGHT, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 2));
            input.BindKeyboardCommand(SDL_SCANCODE_DOWN, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 3));

            input.BindControllerCommand(1, Controller::Button::DPadUp, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 0));
            input.BindControllerCommand(1, Controller::Button::DPadLeft, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 1));
            input.BindControllerCommand(1, Controller::Button::DPadRight, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 2));
            input.BindControllerCommand(1, Controller::Button::DPadDown, KS::Down,
                std::make_unique<QbertMoveCommand>(player, 3));
        }
    }

    void SceneLoader::BindVersusCoilyInputs(CoilyComponent* coily)
    {
        auto& input = InputManager::GetInstance();
        using KS = Controller::KeyState;

        // Remove any previous Coily bindings on these keys before adding new ones
        input.UnbindKeyboardCommand(SDL_SCANCODE_UP, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_LEFT, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_RIGHT, KS::Down);
        input.UnbindKeyboardCommand(SDL_SCANCODE_DOWN, KS::Down);
        input.UnbindControllerCommand(1, Controller::Button::DPadUp, KS::Down);
        input.UnbindControllerCommand(1, Controller::Button::DPadLeft, KS::Down);
        input.UnbindControllerCommand(1, Controller::Button::DPadRight, KS::Down);
        input.UnbindControllerCommand(1, Controller::Button::DPadDown, KS::Down);

        input.BindKeyboardCommand(SDL_SCANCODE_UP, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 0));
        input.BindKeyboardCommand(SDL_SCANCODE_LEFT, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 1));
        input.BindKeyboardCommand(SDL_SCANCODE_RIGHT, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 2));
        input.BindKeyboardCommand(SDL_SCANCODE_DOWN, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 3));

        input.BindControllerCommand(1, Controller::Button::DPadUp, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 0));
        input.BindControllerCommand(1, Controller::Button::DPadLeft, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 1));
        input.BindControllerCommand(1, Controller::Button::DPadRight, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 2));
        input.BindControllerCommand(1, Controller::Button::DPadDown, KS::Down,
            std::make_unique<CoilyMoveCommand>(coily, 3));
    }

    void SceneLoader::LoadMainMenu()
    {
        auto& sceneManager = SceneManager::GetInstance();
        auto& input = InputManager::GetInstance();
        auto& gsm = GameStateManager::GetInstance();

        gsm.Reset();
        UnbindGameplayInputs();
        sceneManager.MarkAllScenesForRemoval();
        m_gameplayScene = nullptr;

        m_currentLevel = 0;
        m_currentRound = 0;
        m_persistedScores = { 0, 0 };
        m_persistedLives = { -1, -1 };

        auto& scene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());
        uint8_t fontSize = static_cast<uint8_t>(std::round(9.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto titleGo = std::make_unique<GameObject>();
        titleGo->SetLocalPosition(winW * 0.18f, winH * 0.1f);
        titleGo->AddComponent<ImageComponent>("Game Title.png", PIXEL_SCALE / 3.f);
        scene.Add(std::move(titleGo));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSize);

        float labelStartY = winH * 0.45f;
        float labelStep = winH * 0.09f;
        float labelX = winW * 0.42f;
        float arrowX = labelX - 17.f * PIXEL_SCALE;

        std::array<GameObject*, 3> arrows{};

        const char* modeLabels[3] = { "SOLO", "CO-OP", "VERSUS" };
        for (int i = 0; i < 3; ++i)
        {
            auto labelGo = std::make_unique<GameObject>();
            auto* text = labelGo->AddComponent<TextComponent>(modeLabels[i], font);
            text->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255) });
            text->SetPosition(labelX, labelStartY + i * labelStep);
            scene.Add(std::move(labelGo));

            auto arrowGo = std::make_unique<GameObject>();
            arrowGo->SetLocalPosition(arrowX, labelStartY + i * labelStep - PIXEL_SCALE);
            arrowGo->AddComponent<ImageComponent>("Selection Arrow.png", PIXEL_SCALE);
            arrowGo->SetActive(i == 0);
            arrows[i] = arrowGo.get();
            scene.Add(std::move(arrowGo));
        }

        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto selectHintGo = std::make_unique<GameObject>();
        auto* selectHintText = selectHintGo->AddComponent<TextComponent>("Enter / A: select", fontSmall);
        selectHintText->SetColor({ static_cast<uint8_t>(160), static_cast<uint8_t>(160), static_cast<uint8_t>(160), static_cast<uint8_t>(255) });
        selectHintText->SetPosition(winW * 0.38f, winH * 0.82f);
        scene.Add(std::move(selectHintGo));

        auto hintGo = std::make_unique<GameObject>();
        auto* hintText = hintGo->AddComponent<TextComponent>("Tab / Start: view highscores", fontSmall);
        hintText->SetColor({ static_cast<uint8_t>(160), static_cast<uint8_t>(160), static_cast<uint8_t>(160), static_cast<uint8_t>(255) });
        hintText->SetPosition(winW * 0.30f, winH * 0.88f);
        scene.Add(std::move(hintGo));

        auto navGo = std::make_unique<GameObject>();
        navGo->AddComponent<MenuNavigationComponent>(arrows);
        scene.Add(std::move(navGo));

        input.BindKeyboardCommand(SDL_SCANCODE_TAB, Controller::KeyState::Down,
            std::make_unique<CallbackCommand>([]() {
                SceneLoader::GetInstance().LoadHighscoreDisplay();
                }));
        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            input.BindControllerCommand(ci, Controller::Button::Start, Controller::KeyState::Down,
                std::make_unique<CallbackCommand>([]() {
                    SceneLoader::GetInstance().LoadHighscoreDisplay();
                    }));
        }
    }

    void SceneLoader::LoadInstructionScreen(GameMode mode)
    {
        m_gameMode = mode;

        auto& sceneManager = SceneManager::GetInstance();
        auto& input = InputManager::GetInstance();

        input.UnbindKeyboardCommand(SDL_SCANCODE_TAB, Controller::KeyState::Down);
        for (unsigned int ci = 0; ci < 2; ++ci)
            input.UnbindControllerCommand(ci, Controller::Button::Start, Controller::KeyState::Down);
        sceneManager.MarkAllScenesForRemoval();

        auto& scene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());
        uint8_t fontSize = static_cast<uint8_t>(std::round(7.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(5.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSize);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto titleGo = std::make_unique<GameObject>();
        auto* titleText = titleGo->AddComponent<TextComponent>("HOW TO PLAY", font);
        titleText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(200), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
        titleText->SetPosition(winW * 0.4f, winH * 0.04f);
        scene.Add(std::move(titleGo));

        const char* lines[] = {
            "Hop on every cube to change its color.",
            "Change all cubes to the target color to advance.",
            "Avoid purple enemies, they are deadly!",
            "Catch green enemies for bonus points.",
            "Use floating discs to escape to the top.",
            "",
            "P1: WASD or controller 0 D-pad",
            "P2: Arrow keys or controller 1 D-pad",
            "",
            "ESC or Y button: Pause"
        };

        float textY = winH * 0.12f;
        float lineH = winH * 0.025f;
        for (auto* line : lines)
        {
            if (line[0] != '\0')
            {
                auto go = std::make_unique<GameObject>();
                auto* t = go->AddComponent<TextComponent>(line, fontSmall);
                t->SetColor({ static_cast<uint8_t>(220), static_cast<uint8_t>(220), static_cast<uint8_t>(220), static_cast<uint8_t>(255) });
                t->SetPosition(winW * 0.3f, textY);
                scene.Add(std::move(go));
            }
            textY += lineH;
        }

        float controlsY = winH * 0.60f;

        auto p1CtrlGo = std::make_unique<GameObject>();
        p1CtrlGo->SetLocalPosition(winW * 0.04f, controlsY);
        p1CtrlGo->AddComponent<ImageComponent>("P1 Controls.png");
        scene.Add(std::move(p1CtrlGo));

        if (mode == GameMode::Coop)
        {
            auto p2CtrlGo = std::make_unique<GameObject>();
            p2CtrlGo->SetLocalPosition(winW * 0.58f, controlsY);
            p2CtrlGo->AddComponent<ImageComponent>("P2 QBert Controls.png");
            scene.Add(std::move(p2CtrlGo));
        }
        else if (mode == GameMode::Versus)
        {
            auto p2CtrlGo = std::make_unique<GameObject>();
            p2CtrlGo->SetLocalPosition(winW * 0.58f, controlsY);
            p2CtrlGo->AddComponent<ImageComponent>("P2 Coily Controls.png");
            scene.Add(std::move(p2CtrlGo));
        }

        auto promptGo = std::make_unique<GameObject>();
        auto* promptText = promptGo->AddComponent<TextComponent>(
            "ENTER or START to begin, ESC or Y button to go back to main menu", fontSmall);
        promptText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255) });
        promptText->SetPosition(winW * 0.1f, winH - fontSizeSmall * 1.5f);
        scene.Add(std::move(promptGo));

        auto screenGo = std::make_unique<GameObject>();
        screenGo->AddComponent<InstructionScreenComponent>();
        scene.Add(std::move(screenGo));
    }

    void SceneLoader::LoadLevelIntro(int levelIndex)
    {
        assert(levelIndex >= 0 && levelIndex < LEVEL_COUNT);

        auto& sceneManager = SceneManager::GetInstance();
        sceneManager.MarkAllScenesForRemoval();

        auto& scene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());

        auto imgGo = std::make_unique<GameObject>();
        imgGo->SetLocalPosition(winW * 0.18f, winH * 0.35f);
        imgGo->AddComponent<ImageComponent>(LEVEL_TITLE_IMAGES[levelIndex], PIXEL_SCALE / 3.f);
        scene.Add(std::move(imgGo));

        auto timerGo = std::make_unique<GameObject>();
        timerGo->AddComponent<LevelIntroComponent>(levelIndex, m_gameConfig.levelIntroDuration);
        scene.Add(std::move(timerGo));
    }

    void SceneLoader::LoadGameplay(int levelIndex, int round)
    {
        auto& sceneManager = SceneManager::GetInstance();
        auto& input = InputManager::GetInstance();
        auto& gsm = GameStateManager::GetInstance();

        gsm.Reset();
        gsm.SetCurseOffset(m_gameConfig.curseOffsetX, m_gameConfig.curseOffsetY);

        UnbindGameplayInputs();
        sceneManager.MarkAllScenesForRemoval();
        auto& scene = sceneManager.CreateScene();
        m_gameplayScene = &scene;

        LevelData levelData = LoadLevelData(
            (m_dataLocation / LEVEL_FILES[levelIndex]).string());
        int roundsPerLevel = static_cast<int>(levelData.roundColorColumns.size());

        // Font sizes scaled from PIXEL_SCALE so they resize with the window
        uint8_t fontSizeLarge = static_cast<uint8_t>(std::round(10.f * PIXEL_SCALE));
        uint8_t fontSizeMed = static_cast<uint8_t>(std::round(8.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto mcFont = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeLarge);
        auto mcFontMed = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeMed);
        auto mcFontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        float winW = static_cast<float>(GameWindowW());

        auto fpsGo = std::make_unique<GameObject>();
        auto fontFps = ResourceManager::GetInstance().LoadFont("Lingua.otf", fontSizeSmall);
        auto* fpsText = fpsGo->AddComponent<TextComponent>("0 FPS", fontFps);
        fpsText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255) });
        fpsText->SetPosition(3.f * PIXEL_SCALE, 3.f * PIXEL_SCALE);
        fpsGo->AddComponent<FPSComponent>();
        scene.Add(std::move(fpsGo));

        // Left HUD column: P1 title, score, change-to label+icon, hearts
        float leftX = 3.f * PIXEL_SCALE;

        auto p1TitleGo = std::make_unique<GameObject>();
        p1TitleGo->SetLocalPosition(leftX, 13.f * PIXEL_SCALE);
        auto* p1TitleSheet = p1TitleGo->AddComponent<SpritesheetComponent>("Player Titles.png", 65, 11);
        p1TitleSheet->SetFrame(0, 0);
        float titleH = p1TitleSheet->GetFrameHeight() * PIXEL_SCALE;
        scene.Add(std::move(p1TitleGo));

        float p1ScoreY = 13.f * PIXEL_SCALE + titleH + PIXEL_SCALE;

        auto p1ScoreGo = std::make_unique<GameObject>();
        auto* p1ScoreText = p1ScoreGo->AddComponent<TextComponent>("0", mcFont);
        p1ScoreText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
        p1ScoreText->SetPosition(leftX, p1ScoreY);
        auto* p1ScoreObs = p1ScoreGo->AddComponent<ScoreDisplayComponent>();
        scene.Add(std::move(p1ScoreGo));

        float changeY = p1ScoreY + fontSizeLarge + 2.f * PIXEL_SCALE;

        auto changeLabelGo = std::make_unique<GameObject>();
        auto* changeText = changeLabelGo->AddComponent<TextComponent>("CHANGE TO:", mcFontMed);
        changeText->SetColor({ static_cast<uint8_t>(220), static_cast<uint8_t>(30), static_cast<uint8_t>(30), static_cast<uint8_t>(255) });
        changeText->SetPosition(leftX, changeY);
        scene.Add(std::move(changeLabelGo));

        int colorColIdx = round % roundsPerLevel;
        int colorColumn = levelData.roundColorColumns[colorColIdx];

        auto iconGo = std::make_unique<GameObject>();
        // Approximate width of "CHANGE TO:" text at fontSizeMed
        iconGo->SetLocalPosition(leftX + static_cast<float>(fontSizeMed) * 7.f, changeY - 2.f * PIXEL_SCALE);
        auto* iconSheet = iconGo->AddComponent<SpritesheetComponent>(
            "Color Icons Spritesheet.png", 14, 12);
        iconSheet->SetFrame(colorColumn, 1);
        scene.Add(std::move(iconGo));

        static constexpr float HEART_SRC_SIZE = 14.f;
        float heartRenderedH = HEART_SRC_SIZE * PIXEL_SCALE;
        float heartsY = changeY + fontSizeMed + 2.f * PIXEL_SCALE;

        auto p1LivesGo = std::make_unique<GameObject>();
        auto* p1LivesDisplay = p1LivesGo->AddComponent<LivesDisplayComponent>(m_gameConfig.maxLives);
        scene.Add(std::move(p1LivesGo));

        for (int i = 0; i < m_gameConfig.maxLives; ++i)
        {
            auto heartGo = std::make_unique<GameObject>();
            heartGo->SetLocalPosition(leftX, heartsY + i * (heartRenderedH + PIXEL_SCALE));
            heartGo->AddComponent<ImageComponent>("Heart.png", PIXEL_SCALE);
            p1LivesDisplay->SetHeart(i, heartGo.get());
            scene.Add(std::move(heartGo));
        }

        // Level and round labels: top center in all modes
        float centerX = winW * 0.5f;
        float topInfoY = 3.f * PIXEL_SCALE;

        auto levelLabelGo = std::make_unique<GameObject>();
        auto* levelLabelText = levelLabelGo->AddComponent<TextComponent>("LEVEL:", mcFontSmall);
        levelLabelText->SetColor({ static_cast<uint8_t>(30), static_cast<uint8_t>(200), static_cast<uint8_t>(30), static_cast<uint8_t>(255) });
        levelLabelText->SetPosition(centerX - 30.f * PIXEL_SCALE, topInfoY);
        scene.Add(std::move(levelLabelGo));

        auto levelNumGo = std::make_unique<GameObject>();
        auto* levelNumText = levelNumGo->AddComponent<TextComponent>(
            std::to_string(levelIndex + 1), mcFontSmall);
        levelNumText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
        levelNumText->SetPosition(centerX - 30.f * PIXEL_SCALE + fontSizeSmall * 4.5f, topInfoY);
        scene.Add(std::move(levelNumGo));

        auto roundLabelGo = std::make_unique<GameObject>();
        auto* roundLabelText = roundLabelGo->AddComponent<TextComponent>("ROUND:", mcFontSmall);
        roundLabelText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(105), static_cast<uint8_t>(180), static_cast<uint8_t>(255) });
        roundLabelText->SetPosition(centerX - 30.f * PIXEL_SCALE, topInfoY + fontSizeSmall + PIXEL_SCALE);
        scene.Add(std::move(roundLabelGo));

        auto roundNumGo = std::make_unique<GameObject>();
        auto* roundNumText = roundNumGo->AddComponent<TextComponent>(
            std::to_string(round + 1), mcFontSmall);
        roundNumText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
        roundNumText->SetPosition(centerX - 30.f * PIXEL_SCALE + fontSizeSmall * 4.5f, topInfoY + fontSizeSmall + PIXEL_SCALE);
        scene.Add(std::move(roundNumGo));

        // Coop: second player HUD on the right side
        ScoreDisplayComponent* p2ScoreObs = nullptr;
        LivesDisplayComponent* p2LivesDisplay = nullptr;
        if (m_gameMode == GameMode::Coop)
        {
            float rightX = winW - 70.f * PIXEL_SCALE;

            auto p2TitleGo = std::make_unique<GameObject>();
            p2TitleGo->SetLocalPosition(rightX, 13.f * PIXEL_SCALE);
            auto* p2TitleSheet = p2TitleGo->AddComponent<SpritesheetComponent>("Player Titles.png", 65, 11);
            p2TitleSheet->SetFrame(0, 1);
            scene.Add(std::move(p2TitleGo));

            float p2ScoreY = 13.f * PIXEL_SCALE + titleH + PIXEL_SCALE;

            auto p2ScoreGo = std::make_unique<GameObject>();
            auto* p2ScoreText = p2ScoreGo->AddComponent<TextComponent>("0", mcFont);
            p2ScoreText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
            p2ScoreText->SetPosition(rightX, p2ScoreY);
            p2ScoreObs = p2ScoreGo->AddComponent<ScoreDisplayComponent>();
            scene.Add(std::move(p2ScoreGo));

            auto p2LivesGo = std::make_unique<GameObject>();
            p2LivesDisplay = p2LivesGo->AddComponent<LivesDisplayComponent>(m_gameConfig.maxLives);
            scene.Add(std::move(p2LivesGo));

            for (int i = 0; i < m_gameConfig.maxLives; ++i)
            {
                auto heartGo = std::make_unique<GameObject>();
                heartGo->SetLocalPosition(winW - leftX - heartRenderedH, heartsY + i * (heartRenderedH + PIXEL_SCALE));
                heartGo->AddComponent<ImageComponent>("Heart.png", PIXEL_SCALE);
                p2LivesDisplay->SetHeart(i, heartGo.get());
                scene.Add(std::move(heartGo));
            }
        }

        // Bonus display below pyramid
        m_pyramidGrid = BuildPyramid(scene, levelData, round);
        int apexCol = m_pyramidGrid.rowOffsets.empty() ? 0 : m_pyramidGrid.rowOffsets[0];

        {
            float bonusY = PyramidTopY()
                + static_cast<float>(m_pyramidGrid.NumRows()) * TileStepY()
                + 12.f * PIXEL_SCALE;
            float midX = winW * 0.5f;
            float gap = 2.f * PIXEL_SCALE;

            auto bonusLabelGo = std::make_unique<GameObject>();
            auto* bonusLabelText = bonusLabelGo->AddComponent<TextComponent>("BONUS", mcFont);
            bonusLabelText->SetColor({ static_cast<uint8_t>(180), static_cast<uint8_t>(0), static_cast<uint8_t>(255), static_cast<uint8_t>(255) });
            bonusLabelText->SetPosition(midX - 37.f * PIXEL_SCALE - gap, bonusY);
            GameObject* bonusLabelRaw = bonusLabelGo.get();
            scene.Add(std::move(bonusLabelGo));

            auto bonusValueGo = std::make_unique<GameObject>();
            auto* bonusValueText = bonusValueGo->AddComponent<TextComponent>("0", mcFont);
            bonusValueText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
            bonusValueText->SetPosition(midX + gap, bonusY);
            GameObject* bonusValueRaw = bonusValueGo.get();
            scene.Add(std::move(bonusValueGo));

            auto bonusCoordGo = std::make_unique<GameObject>();
            bonusCoordGo->AddComponent<BonusDisplayComponent>(
                bonusLabelRaw, bonusValueRaw, bonusValueText);
            scene.Add(std::move(bonusCoordGo));
        }

        // In coop: 2 qberts, anyDeath = true; in versus: 1 qbert, standard
        int qbertCount = (m_gameMode == GameMode::Coop) ? 2 : 1;
        bool anyDeath = (m_gameMode == GameMode::Coop);

        auto gameOverGo = std::make_unique<GameObject>();
        auto* gameOverObs = gameOverGo->AddComponent<GameOverObserverComponent>(qbertCount, anyDeath);
        scene.Add(std::move(gameOverGo));

        gsm.RegisterRespawnCallback([this, gameOverObs]()
            {
                if (gameOverObs->AllPlayersDead())
                    LoadEndScreen(false);
            });

        auto gsmGo = std::make_unique<GameObject>();
        gsmGo->AddComponent<GsmUpdaterComponent>(
            &scene, &m_pyramidGrid,
            m_gameConfig.roundClearDuration,
            m_gameConfig.pointsDiscRemaining,
            m_gameConfig.roundBonusBase,
            m_gameConfig.roundBonusIncrement,
            m_gameConfig.roundBonusDisplayDuration,
            levelIndex, round, roundsPerLevel,
            [this]() { AdvanceRound(); });
        gsmGo->AddComponent<RoundClearAnimatorComponent>(
            &m_pyramidGrid, colorColumn, m_gameConfig.roundClearFrameInterval);
        scene.Add(std::move(gsmGo));

        // Build player GO list without adding to scene yet.
        // Coop: P0 at bottom-left corner, P1 at bottom-right corner of bottom row.
        // Solo/Versus: P0 at apex.
        std::vector<QbertPlayerComponent*> qbertPlayers;
        std::vector<std::unique_ptr<GameObject>> pendingPlayerGOs;

        for (int pi = 0; pi < qbertCount; ++pi)
        {
            int startRow, startCol;

            if (m_gameMode == GameMode::Coop)
            {
                int lastRow = m_pyramidGrid.NumRows() - 1;
                int offset = m_pyramidGrid.rowOffsets[lastRow];
                int width = m_pyramidGrid.rowWidths[lastRow];
                if (pi == 0)
                {
                    startRow = lastRow;
                    startCol = offset;
                }
                else
                {
                    startRow = lastRow;
                    startCol = offset + width - 1;
                }
            }
            else
            {
                startRow = 0;
                startCol = apexCol;
            }

            glm::vec2 startPos = GridToCharacterPos(
                startRow, startCol, QBERT_SRC_W, QBERT_SRC_H);

            auto qbertGo = std::make_unique<GameObject>();
            qbertGo->SetLocalPosition(startPos.x, startPos.y);

            const char* spriteFile = (pi == 0)
                ? "Qbert P1 Spritesheet.png"
                : "Qbert P2 Spritesheet.png";
            qbertGo->AddComponent<SpritesheetComponent>(spriteFile, QBERT_SRC_W, QBERT_SRC_H);

            int livesToUse = m_gameConfig.maxLives;
            int scoreToUse = 0;
            if (pi < 2 && m_persistedLives[pi] >= 0)
                livesToUse = m_persistedLives[pi];
            if (pi < 2)
                scoreToUse = m_persistedScores[pi];

            auto* qbert = qbertGo->AddComponent<QbertPlayerComponent>(
                pi, startRow, startCol, livesToUse);
            qbert->SetPyramidGrid(&m_pyramidGrid);
            qbert->SetScene(&scene);
            qbert->SetFreezeDuration(m_gameConfig.freezeDuration);
            qbert->SetPointsPerCubeChange(m_gameConfig.pointsPerCubeChange);
            qbert->SetPointsSlickSam(m_gameConfig.pointsSlickSam);
            qbert->SetArcHeight(m_gameConfig.arcHeight);
            qbert->SetHopDuration(m_gameConfig.hopDurationQbert);
            qbert->SetDiscDropDuration(m_gameConfig.discDropDuration);
            qbert->SetInitialScore(scoreToUse);

            if (pi == 0)
            {
                qbert->AddObserver(p1ScoreObs);
                qbert->AddObserver(p1LivesDisplay);
            }
            else
            {
                if (p2ScoreObs) qbert->AddObserver(p2ScoreObs);
                if (p2LivesDisplay) qbert->AddObserver(p2LivesDisplay);
            }
            qbert->AddObserver(gameOverObs);
            qbert->NotifyInitialValues();

            gsm.RegisterPlayer(qbert);
            BindGameplayInputsForPlayer(qbert, pi);
            qbertPlayers.push_back(qbert);
            pendingPlayerGOs.push_back(std::move(qbertGo));
        }

        // Spawner GO is created here; SpawnDiscs inserts disc GOs into the scene now,
        // before player GOs are added, so discs render behind players.
        // The spawner GO itself is added after players so Coily pathfinds to a valid player.
        auto spawnerGo = std::make_unique<GameObject>();
        auto* spawner = spawnerGo->AddComponent<EnemySpawnerComponent>(
            levelData, m_gameConfig, &m_pyramidGrid, &scene, qbertPlayers, round,
            m_gameMode == GameMode::Versus);

        spawner->SpawnDiscs(qbertPlayers);

        // Now add player GOs (after discs, so players render on top of discs)
        for (auto& pGo : pendingPlayerGOs)
            scene.Add(std::move(pGo));

        if (m_gameMode == GameMode::Versus)
        {
            spawner->SetOnCoilyBecameSnake([this](CoilyComponent* coily)
                {
                    BindVersusCoilyInputs(coily);
                });
        }

        scene.Add(std::move(spawnerGo));

        input.BindKeyboardCommand(SDL_SCANCODE_ESCAPE, Controller::KeyState::Down,
            std::make_unique<CallbackCommand>([]() {
                SceneLoader::GetInstance().LoadPauseOverlay();
                }));
        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            input.BindControllerCommand(ci, Controller::Button::Y, Controller::KeyState::Down,
                std::make_unique<CallbackCommand>([]() {
                    SceneLoader::GetInstance().LoadPauseOverlay();
                    }));
        }

        input.BindKeyboardCommand(SDL_SCANCODE_F1, Controller::KeyState::Down,
            std::make_unique<CallbackCommand>([this]() { SkipLevel(); }));
    }

    void SceneLoader::LoadPauseOverlay()
    {
        auto& sceneManager = SceneManager::GetInstance();

        Scene* gameplay = sceneManager.GetFirstScene();
        if (gameplay)
            gameplay->SetPaused(true);

        auto& input = InputManager::GetInstance();
        input.UnbindKeyboardCommand(SDL_SCANCODE_ESCAPE, Controller::KeyState::Down);
        for (unsigned int ci = 0; ci < 2; ++ci)
            input.UnbindControllerCommand(ci, Controller::Button::Y, Controller::KeyState::Down);

        auto& overlayScene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());
        uint8_t fontSizeLarge = static_cast<uint8_t>(std::round(12.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeLarge);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto overlayLogicGo = std::make_unique<GameObject>();
        overlayLogicGo->AddComponent<PauseOverlayComponent>();
        overlayScene.Add(std::move(overlayLogicGo));

        auto pauseGo = std::make_unique<GameObject>();
        auto* pauseText = pauseGo->AddComponent<TextComponent>("GAME PAUSED", font);
        pauseText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(100), static_cast<uint8_t>(255) });
        pauseText->SetPosition(winW * 0.34f, winH * 0.38f);
        overlayScene.Add(std::move(pauseGo));

        auto hintGo = std::make_unique<GameObject>();
        auto* hintText = hintGo->AddComponent<TextComponent>(
            "ESC or Y to resume", fontSmall);
        hintText->SetColor({ static_cast<uint8_t>(200), static_cast<uint8_t>(200), static_cast<uint8_t>(200), static_cast<uint8_t>(255) });
        hintText->SetPosition(winW * 0.38f, winH * 0.54f);
        overlayScene.Add(std::move(hintGo));
    }

    void SceneLoader::Unpause()
    {
        auto& sceneManager = SceneManager::GetInstance();
        auto& input = InputManager::GetInstance();

        Scene* overlay = sceneManager.GetLastScene();
        if (overlay)
            overlay->MarkForRemoval();

        Scene* gameplay = sceneManager.GetFirstScene();
        if (gameplay && gameplay != overlay)
            gameplay->SetPaused(false);

        input.UnbindKeyboardCommand(SDL_SCANCODE_ESCAPE, Controller::KeyState::Down);
        for (unsigned int ci = 0; ci < 2; ++ci)
            input.UnbindControllerCommand(ci, Controller::Button::Y, Controller::KeyState::Down);

        input.BindKeyboardCommand(SDL_SCANCODE_ESCAPE, Controller::KeyState::Down,
            std::make_unique<CallbackCommand>([]() {
                SceneLoader::GetInstance().LoadPauseOverlay();
                }));
        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            input.BindControllerCommand(ci, Controller::Button::Y, Controller::KeyState::Down,
                std::make_unique<CallbackCommand>([]() {
                    SceneLoader::GetInstance().LoadPauseOverlay();
                    }));
        }
    }

    void SceneLoader::LoadEndScreen(bool victory)
    {
        m_lastEndWasVictory = victory;

        auto& gsm = GameStateManager::GetInstance();
        m_playerEndData.clear();
        for (auto* player : gsm.GetPlayers())
        {
            if (player)
                m_playerEndData.push_back({ player->GetScore(), player->GetPlayerIndex() });
        }

        UnbindGameplayInputs();
        gsm.Reset();
        SceneManager::GetInstance().MarkAllScenesForRemoval();
        m_gameplayScene = nullptr;

        auto& scene = SceneManager::GetInstance().CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());
        uint8_t fontSizeLarge = static_cast<uint8_t>(std::round(10.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeLarge);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto imgGo = std::make_unique<GameObject>();
        imgGo->SetLocalPosition(winW * 0.18f, winH * 0.06f);
        imgGo->AddComponent<ImageComponent>(
            victory ? "Victory Title.png" : "Game Over Title.png", 1.f);
        scene.Add(std::move(imgGo));

        float scoreY = winH * 0.36f;
        for (const auto& pd : m_playerEndData)
        {
            auto labelGo = std::make_unique<GameObject>();
            std::string label = "PLAYER " + std::to_string(pd.playerIndex + 1)
                + ":   " + std::to_string(pd.score);
            auto* t = labelGo->AddComponent<TextComponent>(label, font);
            t->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(165), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
            t->SetPosition(winW * 0.22f, scoreY);
            scene.Add(std::move(labelGo));
            scoreY += static_cast<float>(fontSizeLarge) * 1.6f;
        }

        auto hintGo = std::make_unique<GameObject>();
        auto* hintText = hintGo->AddComponent<TextComponent>(
            "ESC or Y   enter your name", fontSmall);
        hintText->SetColor({ static_cast<uint8_t>(180), static_cast<uint8_t>(180), static_cast<uint8_t>(180), static_cast<uint8_t>(255) });
        hintText->SetPosition(winW * 0.2f, winH * 0.84f);
        scene.Add(std::move(hintGo));

        auto logicGo = std::make_unique<GameObject>();
        logicGo->AddComponent<EndScreenComponent>(victory);
        scene.Add(std::move(logicGo));
    }

    void SceneLoader::LoadNameEntry(bool victory)
    {
        auto& sceneManager = SceneManager::GetInstance();
        sceneManager.MarkAllScenesForRemoval();

        auto& scene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());
        uint8_t fontSize = static_cast<uint8_t>(std::round(9.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSize);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto titleGo = std::make_unique<GameObject>();
        auto* titleText = titleGo->AddComponent<TextComponent>("ENTER YOUR NAME", font);
        titleText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(100), static_cast<uint8_t>(255) });
        titleText->SetPosition(winW * 0.22f, winH * 0.04f);
        scene.Add(std::move(titleGo));

        auto hintGo = std::make_unique<GameObject>();
        auto* hintText = hintGo->AddComponent<TextComponent>(
            "Up/Down: letter    Left/Right: cursor    Enter/A: next/confirm", fontSmall);
        hintText->SetColor({ static_cast<uint8_t>(140), static_cast<uint8_t>(140), static_cast<uint8_t>(140), static_cast<uint8_t>(255) });
        hintText->SetPosition(winW * 0.15f, winH * 0.9f);
        scene.Add(std::move(hintGo));

        auto logicGo = std::make_unique<GameObject>();
        logicGo->AddComponent<NameEntryComponent>(m_playerEndData, victory);
        scene.Add(std::move(logicGo));
    }

    void SceneLoader::LoadHighscoreDisplay()
    {
        auto& sceneManager = SceneManager::GetInstance();
        sceneManager.MarkAllScenesForRemoval();

        auto& input = InputManager::GetInstance();
        input.UnbindKeyboardCommand(SDL_SCANCODE_TAB, Controller::KeyState::Down);
        for (unsigned int ci = 0; ci < 2; ++ci)
            input.UnbindControllerCommand(ci, Controller::Button::Start, Controller::KeyState::Down);

        auto& scene = sceneManager.CreateScene();

        float winW = static_cast<float>(GameWindowW());
        uint8_t fontSize = static_cast<uint8_t>(std::round(10.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSize);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        auto titleGo = std::make_unique<GameObject>();
        auto* titleText = titleGo->AddComponent<TextComponent>("HIGH SCORES", font);
        titleText->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(200), static_cast<uint8_t>(0), static_cast<uint8_t>(255) });
        titleText->SetPosition(winW * 0.3f, 3.f * PIXEL_SCALE);
        scene.Add(std::move(titleGo));

        auto hintGo = std::make_unique<GameObject>();
        auto* hintText = hintGo->AddComponent<TextComponent>(
            "ESC or Y   main menu", fontSmall);
        hintText->SetColor({ static_cast<uint8_t>(140), static_cast<uint8_t>(140), static_cast<uint8_t>(140), static_cast<uint8_t>(255) });
        hintText->SetPosition(winW * 0.3f, 3.f * PIXEL_SCALE + fontSize + PIXEL_SCALE);
        scene.Add(std::move(hintGo));

        auto logicGo = std::make_unique<GameObject>();
        logicGo->AddComponent<HighscoreDisplayComponent>();
        scene.Add(std::move(logicGo));
    }

    void SceneLoader::AdvanceRound()
    {
        auto& gsm = GameStateManager::GetInstance();
        const auto& players = gsm.GetPlayers();

        for (auto* player : players)
        {
            if (!player) continue;
            int pi = player->GetPlayerIndex();
            if (pi >= 0 && pi < 2)
            {
                m_persistedScores[pi] = player->GetScore();
                m_persistedLives[pi] = player->GetLives();
            }
        }

        LevelData levelData = LoadLevelData(
            (m_dataLocation / LEVEL_FILES[m_currentLevel]).string());
        int roundCount = static_cast<int>(levelData.roundColorColumns.size());

        bool wasLastRoundOfLastLevel =
            (m_currentRound == roundCount - 1) &&
            (m_currentLevel == LEVEL_COUNT - 1);

        m_currentRound++;
        if (m_currentRound >= roundCount)
        {
            m_currentRound = 0;
            m_currentLevel++;
        }

        if (wasLastRoundOfLastLevel)
        {
            LoadEndScreen(true);
            return;
        }

        bool newLevel = (m_currentRound == 0);
        if (newLevel)
            LoadLevelIntro(m_currentLevel);
        else
            LoadGameplay(m_currentLevel, m_currentRound);
    }

    void SceneLoader::SkipLevel()
    {
        auto& gsm = GameStateManager::GetInstance();
        const auto& players = gsm.GetPlayers();

        for (auto* player : players)
        {
            if (!player) continue;
            int pi = player->GetPlayerIndex();
            if (pi >= 0 && pi < 2)
            {
                m_persistedScores[pi] = player->GetScore();
                m_persistedLives[pi] = player->GetLives();
            }
        }

        m_currentRound = 0;
        m_currentLevel = (m_currentLevel + 1) % LEVEL_COUNT;

        LoadLevelIntro(m_currentLevel);
    }
}