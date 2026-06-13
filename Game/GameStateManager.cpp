#include "GameStateManager.h"
#include <algorithm>
#include <cmath>
#include <climits>
#include "QbertPlayerComponent.h"
#include "CoilyComponent.h"
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
        m_discRiding = false;
        m_discRideTimer = 0.f;
        m_discRider = nullptr;
        m_discScene = nullptr;
        m_roundClear = false;
        m_bonusScreen = false;
        m_bonusPoints = 0;
        m_advanceCallback = nullptr;
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

        // If a disc ride is in progress and it's a different player dying, finish the ride
        // immediately so the rider lands cleanly before the death freeze begins.
        if (m_discRiding && m_discRider != dyingPlayer)
            FinishDiscRide(scene);

        m_frozen = true;
        m_freezeTimer = 0.f;
        m_freezeDuration = freezeDuration;
        m_dyingPlayer = dyingPlayer;

        auto curseGo = std::make_unique<GameObject>();
        float offsetX = m_curseOffsetX * PIXEL_SCALE;
        float offsetY = m_curseOffsetY * PIXEL_SCALE;
        curseGo->SetLocalPosition(cursePosX - offsetX, cursePosY - offsetY);
        curseGo->AddComponent<ImageComponent>("Qbert Curses.png", PIXEL_SCALE);
        m_curseGameObject = curseGo.get();
        scene->Add(std::move(curseGo));
    }

    void GameStateManager::TriggerRoundClear(float duration, int bonusPoints,
        float bonusDisplayDuration, std::function<void()> advanceCallback)
    {
        if (m_frozen || m_discRiding) return;

        m_frozen = true;
        m_freezeTimer = 0.f;
        m_freezeDuration = duration;
        m_roundClear = true;
        m_bonusPoints = bonusPoints;
        m_bonusDisplayDuration = bonusDisplayDuration;
        m_advanceCallback = std::move(advanceCallback);
    }

    void GameStateManager::BeginBonusPhase()
    {
        for (auto& entry : m_enemies)
            entry.markForRemoval();
        m_enemies.clear();

        for (auto* player : m_players)
            player->GetGameObject()->SetActive(false);

        for (auto* player : m_players)
            player->AddScore(m_bonusPoints);

        m_frozen = true;
        m_freezeTimer = 0.f;
        m_freezeDuration = m_bonusDisplayDuration;
        m_bonusScreen = true;
    }

    bool GameStateManager::CoilyCanReachDisc(int discRow, int discCol, float flightDuration) const
    {
        for (auto& entry : m_enemies)
        {
            if (entry.type != EnemyEntry::Type::Coily) continue;

            auto* coily = static_cast<CoilyComponent*>(entry.component);
            if (coily->IsEgg()) return false;

            int row = entry.getRow();
            int col = entry.getCol();
            int dist = std::abs(row - discRow) + std::abs(col - discCol) - 1;
            float hopInterval = coily->GetHopInterval();
            float timeNeeded = static_cast<float>(dist) * hopInterval;
            return timeNeeded <= flightDuration;
        }
        return false;
    }

    void GameStateManager::DismissNonCoilyEnemies()
    {
        for (auto& entry : m_enemies)
        {
            if (entry.type == EnemyEntry::Type::Coily) continue;
            if (entry.triggerFall)
                entry.triggerFall();
            else
                entry.markForRemoval();
        }
        m_enemies.erase(
            std::remove_if(m_enemies.begin(), m_enemies.end(),
                [](const EnemyEntry& e) { return e.type != EnemyEntry::Type::Coily; }),
            m_enemies.end());
    }

    void GameStateManager::TriggerDiscRide(QbertPlayerComponent* rider, Scene* scene,
        int discRow, int discCol, float flightDuration,
        int pointsCoilyDisc, float freezeDuration, float discDropDuration)
    {
        if (m_discRiding || m_frozen) return;

        m_discRiding = true;
        m_discRideTimer = 0.f;
        float safeDropDuration = discDropDuration > 0.f ? discDropDuration : 0.001f;
        m_discRideDuration = flightDuration + safeDropDuration;
        m_discRider = rider;
        m_pointsCoilyDisc = pointsCoilyDisc;
        m_discFreezeDuration = freezeDuration;
        m_discDropDuration = safeDropDuration;
        m_discScene = scene;

        DismissNonCoilyEnemies();

        bool coilyWillPursue = CoilyCanReachDisc(discRow, discCol, flightDuration);
        if (!coilyWillPursue)
        {
            for (auto& entry : m_enemies)
            {
                if (entry.type != EnemyEntry::Type::Coily) continue;
                auto* coily = static_cast<CoilyComponent*>(entry.component);
                coily->ForceJumpOff();
            }
        }
        else
        {
            for (auto& entry : m_enemies)
            {
                if (entry.type != EnemyEntry::Type::Coily) continue;
                auto* coily = static_cast<CoilyComponent*>(entry.component);
                coily->SetDiscTarget(discRow, discCol);
            }
        }
    }

    void GameStateManager::OnCoilyFellDuringDisc(int coilyRow, int coilyCol) const
    {
        QbertPlayerComponent* nearest = nullptr;
        int bestDist = INT_MAX;
        for (auto* player : m_players)
        {
            int dist = std::abs(player->GetGridRow() - coilyRow) + std::abs(player->GetGridCol() - coilyCol);
            if (dist < bestDist)
            {
                bestDist = dist;
                nearest = player;
            }
        }
        if (nearest)
            nearest->AddScore(m_pointsCoilyDisc);
    }

    void GameStateManager::FinishDiscRide(Scene* scene)
    {
        m_discRiding = false;

        for (auto& entry : m_enemies)
        {
            if (entry.type == EnemyEntry::Type::Coily)
            {
                auto* coily = static_cast<CoilyComponent*>(entry.component);
                if (coily->IsDoingDiscChase()) continue;
            }
            entry.markForRemoval();
        }
        m_enemies.clear();

        if (m_discRider)
            m_discRider->LandFromDisc();

        auto callbacks = m_respawnCallbacks;
        for (auto& cb : callbacks)
            if (cb) cb();

        m_discRider = nullptr;
        m_discScene = nullptr;
        (void)scene;
    }

    void GameStateManager::Update(float deltaTime, Scene* scene)
    {
        if (m_discRiding)
        {
            m_discRideTimer += deltaTime;
            if (m_discRideTimer >= m_discRideDuration)
                FinishDiscRide(scene);
            return;
        }

        if (m_frozen)
        {
            m_freezeTimer += deltaTime;
            if (m_freezeTimer < m_freezeDuration) return;

            m_frozen = false;

            if (m_bonusScreen)
            {
                m_bonusScreen = false;
                auto cb = std::move(m_advanceCallback);
                m_advanceCallback = nullptr;
                if (cb) cb();
                return;
            }

            if (m_roundClear)
            {
                m_roundClear = false;
                BeginBonusPhase();
                return;
            }

            if (m_curseGameObject)
            {
                m_curseGameObject->MarkForRemoval();
                m_curseGameObject = nullptr;
            }

            for (auto& entry : m_enemies)
                entry.markForRemoval();
            m_enemies.clear();

            for (auto* player : m_players)
                player->Respawn();

            auto callbacks = m_respawnCallbacks;
            for (auto& cb : callbacks)
                if (cb) cb();

            m_dyingPlayer = nullptr;
            return;
        }

        m_enemies.erase(
            std::remove_if(m_enemies.begin(), m_enemies.end(),
                [](const EnemyEntry& e) { return e.markedForRemoval; }),
            m_enemies.end());

        for (auto* player : m_players)
        {
            if (player->IsHopping() || player->IsDead() || player->IsOnDisc()) continue;
            int pRow = player->GetGridRow();
            int pCol = player->GetGridCol();

            for (auto& entry : m_enemies)
            {
                if (entry.markedForRemoval || entry.isHopping()) continue;

                int eRow = entry.getRow() + entry.collisionDRow;
                int eCol = entry.getCol() + entry.collisionDCol;

                if (eRow != pRow || eCol != pCol) continue;

                if (entry.type == EnemyEntry::Type::SlickSam)
                {
                    entry.markedForRemoval = true;
                    entry.markForRemoval();
                    player->OnCaughtSlickSam();
                }
                else
                {
                    entry.markedForRemoval = true;
                    glm::vec2 worldPos = player->GetDeathWorldPos();
                    player->TriggerDeath();
                    TriggerPlayerDeath(player, scene, worldPos.x, worldPos.y, entry.freezeDuration);
                }
                return;
            }
        }
    }

    void GameStateManager::CheckEnemyLandedAt(int row, int col, EnemyEntry& entry,
        Scene* scene, float freezeDuration)
    {
        if (m_frozen || m_discRiding) return;
        if (entry.markedForRemoval) return;

        for (auto* player : m_players)
        {
            if (player->IsHopping()) continue;
            if (player->GetGridRow() != row || player->GetGridCol() != col) continue;

            if (entry.type == EnemyEntry::Type::SlickSam)
            {
                entry.markedForRemoval = true;
                entry.markForRemoval();
                player->OnCaughtSlickSam();
            }
            else
            {
                entry.markedForRemoval = true;
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
        if (m_frozen || m_discRiding) return;

        for (auto& entry : m_enemies)
        {
            if (entry.markedForRemoval || entry.isHopping()) continue;

            int eRow = entry.getRow() + entry.collisionDRow;
            int eCol = entry.getCol() + entry.collisionDCol;

            if (eRow != row || eCol != col) continue;

            if (entry.type == EnemyEntry::Type::SlickSam)
            {
                entry.markedForRemoval = true;
                entry.markForRemoval();
                player->OnCaughtSlickSam();
            }
            else
            {
                entry.markedForRemoval = true;
                glm::vec2 worldPos = player->GetDeathWorldPos();
                player->TriggerDeath();
                TriggerPlayerDeath(player, scene, worldPos.x, worldPos.y, freezeDuration);
            }
            return;
        }
    }
}