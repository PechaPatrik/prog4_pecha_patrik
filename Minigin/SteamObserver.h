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

            m_unlocked = true;

#ifdef USE_STEAMWORKS
            SteamUserStats()->SetAchievement("ACH_WIN_ONE_GAME");
            SteamUserStats()->StoreStats();
#endif
        }

    private:
        bool m_unlocked{ false };
    };
}