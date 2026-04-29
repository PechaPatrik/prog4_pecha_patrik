#pragma once
#include "IObserver.h"
#ifdef USE_STEAMWORKS
#include <steam_api.h>
#endif
namespace dae
{
    class SteamAchievementObserver final : public IObserver
    {
    public:
        void OnNotify(GameEvent event, int value) override
        {
            if (event != GameEvent::ScoreChanged) return;
            if (m_unlocked) return;
            if (value < 500) return;

#ifdef USE_STEAMWORKS
            ISteamUserStats* stats = SteamUserStats();
            if (!stats) return;

            // Check if already unlocked on Steam side before setting it
            bool alreadyUnlocked = false;
            stats->GetAchievement("ACH_WIN_ONE_GAME", &alreadyUnlocked);
            if (alreadyUnlocked)
            {
                m_unlocked = true;
                return;
            }

            stats->SetAchievement("ACH_WIN_ONE_GAME");
            stats->StoreStats();
#endif
            m_unlocked = true;
        }
    private:
        bool m_unlocked{ false };
    };
}