#pragma once
#include "Component.h"
#include "GameObject.h"
#include "SceneLoader.h"
#include "HighscoreEntry.h"
#include "UIInputPoller.h"
#include <vector>
#include <string>
#include <array>
#include <SDL3/SDL.h>

namespace dae
{
    class TextComponent;
    class Scene;

    struct PlayerNameCursor
    {
        static constexpr int NAME_LEN = 3;
        char letters[NAME_LEN]{ 'A', 'A', 'A' };
        int cursor{ 0 };
        bool confirmed{ false };

        std::string GetName() const { return std::string(letters, NAME_LEN); }

        void CycleUp()
        {
            letters[cursor] = static_cast<char>((letters[cursor] - 'A' + 25) % 26 + 'A');
        }
        void CycleDown()
        {
            letters[cursor] = static_cast<char>((letters[cursor] - 'A' + 1) % 26 + 'A');
        }
        void MoveLeft() { if (cursor > 0) --cursor; }
        void MoveRight() { if (cursor < NAME_LEN - 1) ++cursor; }
        void Confirm()
        {
            if (cursor < NAME_LEN - 1) ++cursor;
            else confirmed = true;
        }
    };

    class NameEntryComponent final : public Component
    {
    public:
        NameEntryComponent(GameObject* pOwner,
            const std::vector<PlayerEndData>& players,
            bool victory);

        void Update(float) override;

    private:
        void BuildDisplay();
        void UpdateDisplay(int pi);
        std::string BuildLetterString(int pi, bool unused) const;
        std::string BuildCursorIndicator(int pi) const;
        void SaveAndContinue();
        const bool* GetKeys();

        std::vector<PlayerEndData> m_players;
        bool m_victory;
        bool m_done{ false };
        bool m_initialized{ false };

        std::vector<PlayerNameCursor> m_cursors;
        std::vector<TextComponent*> m_displayTexts;
        std::vector<TextComponent*> m_cursorTexts;

        UIKeyboardEdge m_kbEdgeP1{};
        UIKeyboardEdge m_kbEdgeP2{};
        std::array<UIControllerEdge, 2> m_ctrlEdge{};
    };
}
