#include "NameEntryComponent.h"
#include "HighscoreEntry.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "SceneManager.h"
#include "GameScale.h"
#include "ServiceLocator.h"
#include "SoundId.h"
#include <filesystem>
#include <string>

namespace dae
{
    static const std::string HIGHSCORE_FILENAME = "highscores.json";

    NameEntryComponent::NameEntryComponent(GameObject* pOwner,
        const std::vector<PlayerEndData>& players,
        bool victory)
        : Component(pOwner)
        , m_players(players)
        , m_victory(victory)
    {
        for (size_t i = 0; i < players.size(); ++i)
            m_cursors.push_back(PlayerNameCursor{});
    }

    void NameEntryComponent::Update(float)
    {
        if (m_done) return;

        if (!m_initialized)
        {
            m_initialized = true;
            BuildDisplay();
        }

        const bool* keys = GetKeys();

        bool singlePlayer = (m_players.size() == 1);
        int playerCount = static_cast<int>(m_players.size());

        for (int pi = 0; pi < playerCount; ++pi)
        {
            if (m_cursors[pi].confirmed) continue;

            bool up = false, down = false, left = false, right = false, confirm = false;

            if (pi == 0)
            {
                // P0 always gets WASD; also gets arrows and both controllers in single-player
                auto kb = m_kbEdgeP1.GetEdge(keys, true, singlePlayer);
                up = kb.up; down = kb.down; left = kb.left;
                right = kb.right; confirm = kb.confirm;

                auto ce0 = m_ctrlEdge[0].GetEdge(0);
                up |= ce0.up; down |= ce0.down; left |= ce0.left;
                right |= ce0.right; confirm |= ce0.confirm;

                if (singlePlayer)
                {
                    auto ce1 = m_ctrlEdge[1].GetEdge(1);
                    up |= ce1.up; down |= ce1.down; left |= ce1.left;
                    right |= ce1.right; confirm |= ce1.confirm;
                }
            }
            else if (pi == 1)
            {
                auto kb = m_kbEdgeP2.GetEdge(keys, false, true);
                up = kb.up; down = kb.down; left = kb.left;
                right = kb.right; confirm = kb.confirm;

                auto ce = m_ctrlEdge[1].GetEdge(1);
                up |= ce.up; down |= ce.down; left |= ce.left;
                right |= ce.right; confirm |= ce.confirm;
            }

            if (up) m_cursors[pi].CycleUp();
            if (down) m_cursors[pi].CycleDown();
            if (left) m_cursors[pi].MoveLeft();
            if (right) m_cursors[pi].MoveRight();
            if (confirm) m_cursors[pi].Confirm();

            if ((up || down || left || right) && pi < static_cast<int>(m_displayTexts.size()))
            {
                ServiceLocator::GetSoundSystem().PlaySound(SoundId::ChangeSelection);
                UpdateDisplay(pi);
            }
            else if (confirm && pi < static_cast<int>(m_displayTexts.size()))
            {
                ServiceLocator::GetSoundSystem().PlaySound(SoundId::QbertHit);
                UpdateDisplay(pi);
            }
        }

        bool allDone = true;
        for (int pi = 0; pi < playerCount; ++pi)
            if (!m_cursors[pi].confirmed) { allDone = false; break; }

        if (allDone)
        {
            m_done = true;
            SaveAndContinue();
        }
    }

    void NameEntryComponent::BuildDisplay()
    {
        Scene* scene = SceneManager::GetInstance().GetLastScene();
        if (!scene) return;

        float winW = static_cast<float>(GameWindowW());
        float winH = static_cast<float>(GameWindowH());

        uint8_t fontSize = static_cast<uint8_t>(std::round(12.f * PIXEL_SCALE));
        uint8_t fontSizeSmall = static_cast<uint8_t>(std::round(6.f * PIXEL_SCALE));

        auto font = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSize);
        auto fontSmall = ResourceManager::GetInstance().LoadFont("Minecraft.ttf", fontSizeSmall);

        int playerCount = static_cast<int>(m_players.size());
        float startY = winH * 0.22f;
        float stepY = winH * 0.36f;

        m_displayTexts.resize(playerCount, nullptr);
        m_cursorTexts.resize(playerCount, nullptr);

        for (int pi = 0; pi < playerCount; ++pi)
        {
            float y = startY + pi * stepY;

            auto labelGo = std::make_unique<GameObject>();
            std::string label = "PLAYER " + std::to_string(m_players[pi].playerIndex + 1)
                + "  score: " + std::to_string(m_players[pi].score);
            auto* lt = labelGo->AddComponent<TextComponent>(label, fontSmall);
            lt->SetColor({ static_cast<uint8_t>(200), static_cast<uint8_t>(200), static_cast<uint8_t>(200), static_cast<uint8_t>(255) });
            lt->SetPosition(winW * 0.18f, y - static_cast<float>(fontSizeSmall) * 1.6f);
            scene->Add(std::move(labelGo));

            auto letterGo = std::make_unique<GameObject>();
            auto* dt = letterGo->AddComponent<TextComponent>(
                BuildLetterString(pi, false), font);
            dt->SetColor({ static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(100), static_cast<uint8_t>(255) });
            dt->SetPosition(winW * 0.28f, y);
            m_displayTexts[pi] = dt;
            scene->Add(std::move(letterGo));

            auto cursorGo = std::make_unique<GameObject>();
            auto* ct = cursorGo->AddComponent<TextComponent>(
                BuildCursorIndicator(pi), fontSmall);
            ct->SetColor({ static_cast<uint8_t>(100), static_cast<uint8_t>(255), static_cast<uint8_t>(100), static_cast<uint8_t>(255) });
            ct->SetPosition(winW * 0.28f, y + static_cast<float>(fontSize) * 1.5f);
            m_cursorTexts[pi] = ct;
            scene->Add(std::move(cursorGo));
        }
    }

    std::string NameEntryComponent::BuildLetterString(int pi, bool /*unused*/) const
    {
        const PlayerNameCursor& c = m_cursors[pi];
        std::string s;
        for (int i = 0; i < PlayerNameCursor::NAME_LEN; ++i)
        {
            s += c.letters[i];
            if (i < PlayerNameCursor::NAME_LEN - 1) s += "  ";
        }
        return s;
    }

    std::string NameEntryComponent::BuildCursorIndicator(int pi) const
    {
        const PlayerNameCursor& c = m_cursors[pi];
        if (c.confirmed) return "confirmed!";
        std::string s;
        for (int i = 0; i < PlayerNameCursor::NAME_LEN; ++i)
        {
            s += (i == c.cursor) ? "^" : " ";
            if (i < PlayerNameCursor::NAME_LEN - 1) s += "       ";
        }
        return s;
    }

    void NameEntryComponent::UpdateDisplay(int pi)
    {
        if (pi < static_cast<int>(m_displayTexts.size()) && m_displayTexts[pi])
            m_displayTexts[pi]->SetText(BuildLetterString(pi, false));
        if (pi < static_cast<int>(m_cursorTexts.size()) && m_cursorTexts[pi])
            m_cursorTexts[pi]->SetText(BuildCursorIndicator(pi));
    }

    void NameEntryComponent::SaveAndContinue()
    {
        const std::filesystem::path& dataLoc = SceneLoader::GetInstance().GetDataLocation();
        std::string path = (dataLoc / HIGHSCORE_FILENAME).string();

        auto scores = LoadHighscores(path);
        for (int pi = 0; pi < static_cast<int>(m_players.size()); ++pi)
        {
            HighscoreEntry entry;
            entry.name = m_cursors[pi].GetName();
            entry.score = m_players[pi].score;
            AddHighscore(scores, entry);
        }
        SaveHighscores(path, scores);

        SceneLoader::GetInstance().LoadHighscoreDisplay();
    }

    const bool* NameEntryComponent::GetKeys()
    {
        int n{};
        return SDL_GetKeyboardState(&n);
    }
}