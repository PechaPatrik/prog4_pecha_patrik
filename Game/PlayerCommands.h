#pragma once
#include "Command.h"
#include "PlayerComponent.h"
#include "ServiceLocator.h"
#include "SoundId.h"

namespace dae
{
    class LoseLifeCommand final : public Command
    {
    public:
        explicit LoseLifeCommand(PlayerComponent* player) : m_player(player) {}
        void Execute() override
        {
            if (m_player && m_player->GetLives() > 0)
            {
                m_player->LoseLife();
                ServiceLocator::GetSoundSystem().PlaySound(SoundId::PlayerDamage, 128);
            }
        }
    private:
        PlayerComponent* m_player;
    };

    class AddScoreCommand final : public Command
    {
    public:
        AddScoreCommand(PlayerComponent* player, int points)
            : m_player(player), m_points(points) {
        }
        void Execute() override
        {
            if (m_player && m_player->GetLives() > 0)
                m_player->AddScore(m_points);
        }
    private:
        PlayerComponent* m_player;
        int m_points;
    };
}