#pragma once
#include "Component.h"
#include "GameStateManager.h"
#include "QbertPyramid.h"
#include "Scene.h"
#include <functional>
#include <vector>

namespace dae
{
    class GsmUpdaterComponent final : public Component
    {
    public:
        GsmUpdaterComponent(GameObject* pOwner, Scene* scene,
            const PyramidGrid* grid,
            float roundClearDuration,
            int pointsDiscRemaining,
            int roundBonusBase,
            int roundBonusIncrement,
            float roundBonusDisplayDuration,
            int completedLevelIndex,
            int completedRoundIndex,
            int roundsPerLevel,
            std::function<void()> advanceCallback)
            : Component(pOwner)
            , m_scene(scene)
            , m_grid(grid)
            , m_roundClearDuration(roundClearDuration)
            , m_pointsDiscRemaining(pointsDiscRemaining)
            , m_roundBonusBase(roundBonusBase)
            , m_roundBonusIncrement(roundBonusIncrement)
            , m_roundBonusDisplayDuration(roundBonusDisplayDuration)
            , m_completedLevelIndex(completedLevelIndex)
            , m_completedRoundIndex(completedRoundIndex)
            , m_roundsPerLevel(roundsPerLevel)
            , m_advanceCallback(std::move(advanceCallback))
        {
        }

        void Update(float dt) override
        {
            auto& gsm = GameStateManager::GetInstance();
            gsm.Update(dt, m_scene);

            if (m_roundClearTriggered) return;
            if (gsm.IsFrozen() || gsm.IsDiscRiding()) return;
            if (!m_grid || !m_grid->AllTarget()) return;

            m_roundClearTriggered = true;

            // Award points for each remaining disc
            for (auto* player : gsm.GetPlayers())
            {
                if (!player) continue;
                player->AddScore(player->GetDiscCount() * m_pointsDiscRemaining);
            }

            int totalRound = m_completedLevelIndex * m_roundsPerLevel + m_completedRoundIndex;
            int bonus = m_roundBonusBase + m_roundBonusIncrement * totalRound;

            gsm.TriggerRoundClear(m_roundClearDuration, bonus, m_roundBonusDisplayDuration,
                m_advanceCallback);
        }

    private:
        Scene* m_scene;
        const PyramidGrid* m_grid;
        float m_roundClearDuration;
        int m_pointsDiscRemaining;
        int m_roundBonusBase;
        int m_roundBonusIncrement;
        float m_roundBonusDisplayDuration;
        int m_completedLevelIndex;
        int m_completedRoundIndex;
        int m_roundsPerLevel;
        std::function<void()> m_advanceCallback;
        bool m_roundClearTriggered{ false };
    };
}