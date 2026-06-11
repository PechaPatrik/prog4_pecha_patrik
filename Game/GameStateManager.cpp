#include "GameStateManager.h"
#include <algorithm>
#include "QbertPlayerComponent.h"
#include "ImageComponent.h"

#include "Scene.h"
#include "GameObject.h"
#include "GameScale.h"

namespace dae
{
    void GameStateManager::Reset()
    {
        m_players.clear();
        m_enemies.clear();
        m_frozen = false;
        m_freezeTimer = 0.f;
        m_curseGameObject = nullptr;
        m_dyingPlayer = nullptr;
        m_respawnCallbacks.clear();
    }

    void GameStateManager::RegisterPlayer(QbertPlayerComponent* player)
    {
        for (auto* p : m_players)
            if (p == player) return;
        m_players.push_back(player);
    }

    void GameStateManager::UnregisterPlayer(QbertPlayerComponent* player)
    {
        m_players.erase(
            std::remove(m_players.begin(), m_players.end(), player),
            m_players.end());
    }

    void GameStateManager::RegisterEnemy(EnemyEntry entry)
    {
        m_enemies.push_back(std::move(entry));
    }

    void GameStateManager::UnregisterAllEnemies()
    {
        m_enemies.clear();
    }

    void GameStateManager::TriggerPlayerDeath(QbertPlayerComponent* dyingPlayer, Scene* scene,
        float cursePosX, float cursePosY, float freezeDuration)
    {
        if (m_frozen) return;

        m_frozen = true;
        m_freezeTimer = 0.f;
        m_freezeDuration = freezeDuration;
        m_dyingPlayer = dyingPlayer;

        auto curseGo = std::make_unique<GameObject>();
        static constexpr float CURSE_OFFSET_Y = 24.f * PIXEL_SCALE;
        static constexpr float CURSE_OFFSET_X = 12.f * PIXEL_SCALE;
        curseGo->SetLocalPosition(cursePosX - CURSE_OFFSET_X, cursePosY - CURSE_OFFSET_Y);
        curseGo->AddComponent<ImageComponent>("Qbert Curses.png", PIXEL_SCALE);
        m_curseGameObject = curseGo.get();
        scene->Add(std::move(curseGo));
    }

    void GameStateManager::Update(float deltaTime, Scene* scene)
    {
        (void)scene;
        if (!m_frozen) return;

        m_freezeTimer += deltaTime;
        if (m_freezeTimer < m_freezeDuration) return;

        m_frozen = false;

        if (m_curseGameObject)
        {
            m_curseGameObject->MarkForRemoval();
            m_curseGameObject = nullptr;
        }

        // Remove all enemies from the scene
        for (auto& entry : m_enemies)
            entry.markForRemoval();
        m_enemies.clear();

        for (auto* player : m_players)
            player->Respawn();

        for (auto& cb : m_respawnCallbacks)
            cb();

        m_dyingPlayer = nullptr;
    }

    void GameStateManager::CheckEnemyLandedAt(int row, int col, EnemyEntry& entry,
        Scene* scene, float freezeDuration)
    {
        if (m_frozen) return;

        for (auto* player : m_players)
        {
            if (player->IsHopping()) continue;
            if (player->GetGridRow() != row || player->GetGridCol() != col) continue;

            if (entry.type == EnemyEntry::Type::SlickSam)
            {
                // Slick/Sam: player catches them, enemy is removed, player scores
                entry.markForRemoval();
                player->OnCaughtSlickSam();
            }
            else
            {
                // Coily/Ugg/Wrongway: player dies
                glm::vec2 worldPos = player->GetDeathWorldPos();
                player->TriggerDeath();
                TriggerPlayerDeath(player, scene, worldPos.x, worldPos.y, freezeDuration);
            }
            return;
        }
    }

    void GameStateManager::CheckPlayerLandedAt(QbertPlayerComponent* player, int row, int col,
        Scene* scene, float freezeDuration)
    {
        if (m_frozen) return;

        for (auto& entry : m_enemies)
        {
            if (entry.isHopping()) continue;

            int eRow = entry.getRow() + entry.collisionDRow;
            int eCol = entry.getCol() + entry.collisionDCol;

            if (eRow != row || eCol != col) continue;

            if (entry.type == EnemyEntry::Type::SlickSam)
            {
                entry.markForRemoval();
                player->OnCaughtSlickSam();
            }
            else
            {
                glm::vec2 worldPos = player->GetDeathWorldPos();
                player->TriggerDeath();
                TriggerPlayerDeath(player, scene, worldPos.x, worldPos.y, freezeDuration);
            }
            return;
        }
    }
}