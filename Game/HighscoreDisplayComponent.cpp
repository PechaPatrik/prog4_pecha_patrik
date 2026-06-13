#include "HighscoreDisplayComponent.h"
#include "SceneLoader.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "GameObject.h"
#include "Scene.h"
#include "SceneManager.h"
#include "GameScale.h"
#include <SDL3/SDL.h>
#include <filesystem>

namespace dae
{
    static const std::string HIGHSCORE_FILENAME = "highscores.json";

    HighscoreDisplayComponent::HighscoreDisplayComponent(GameObject* pOwner)
        : Component(pOwner)
    {
    }

    void HighscoreDisplayComponent::Update(float)
    {
        // On first update, spawn score text objects into the scene
        if (!m_initialized)
        {
            m_initialized = true;

            const std::filesystem::path& dataLoc = SceneLoader::GetInstance().GetDataLocation();
            std::string path = (dataLoc / HIGHSCORE_FILENAME).string();
            auto scores = LoadHighscores(path);

            Scene* scene = SceneManager::GetInstance().GetLastScene();
            if (scene)
            {
                auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", 22);
                float y = 130.f;
                float winW = static_cast<float>(GameWindowW());

                if (scores.empty())
                {
                    auto go = std::make_unique<GameObject>();
                    auto* t = go->AddComponent<TextComponent>("No scores yet!", font);
                    t->SetColor({ 180, 180, 180, 255 });
                    t->SetPosition(winW * 0.3f, y);
                    scene->Add(std::move(go));
                }

                for (int i = 0; i < static_cast<int>(scores.size()); ++i)
                {
                    auto go = std::make_unique<GameObject>();
                    std::string line = std::to_string(i + 1) + ".  "
                        + scores[i].name + "    "
                        + std::to_string(scores[i].score);
                    auto* t = go->AddComponent<TextComponent>(line, font);
                    t->SetColor(i == 0
                        ? SDL_Color{ 255, 200, 0, 255 }
                        : SDL_Color{ 220, 220, 220, 255 });
                    t->SetPosition(winW * 0.2f, y);
                    scene->Add(std::move(go));
                    y += 34.f;
                }
            }
        }

        if (m_transitioning) return;

        int numKeys{};
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        bool escPressed = keys[SDL_SCANCODE_ESCAPE] && !m_prevEsc;
        m_prevEsc = keys[SDL_SCANCODE_ESCAPE];

        bool yPressed = false;
#ifndef __EMSCRIPTEN__
        for (unsigned int ci = 0; ci < 2; ++ci)
        {
            XINPUT_STATE state{};
            if (XInputGetState(ci, &state) == ERROR_SUCCESS)
            {
                bool yCur = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
                if (yCur && !m_prevY[ci]) yPressed = true;
                m_prevY[ci] = yCur;
            }
        }
#endif

        if (escPressed || yPressed)
        {
            m_transitioning = true;
            SceneLoader::GetInstance().LoadMainMenu();
        }
    }
}
