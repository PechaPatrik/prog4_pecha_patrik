#pragma once
#include "Component.h"
#include "GameObject.h"
#include "LevelData.h"
#include "QbertPyramid.h"
#include "GameStateManager.h"
#include "CoilyComponent.h"
#include "UggWrongwayComponent.h"
#include "SlickSamComponent.h"
#include "SpritesheetComponent.h"
#include "Scene.h"
#include <cstdlib>
#include <cmath>

namespace dae
{
    static constexpr int COILY_SRC_W_SPAWN = 16;
    static constexpr int COILY_SRC_H_SPAWN = 32;
    static constexpr int UGG_SRC_W_SPAWN = UGG_SRC_W;
    static constexpr int UGG_SRC_H_SPAWN = UGG_SRC_H;
    static constexpr int SS_SRC_W_SPAWN = SLICK_SAM_SRC_W;
    static constexpr int SS_SRC_H_SPAWN = SLICK_SAM_SRC_H;

    class EnemySpawnerComponent final : public Component
    {
    public:
        EnemySpawnerComponent(GameObject* pOwner, const LevelData& levelData,
            PyramidGrid* grid, Scene* scene,
            const std::vector<QbertPlayerComponent*>& players,
            int round = 0)
            : Component(pOwner)
            , m_levelData(levelData)
            , m_grid(grid)
            , m_scene(scene)
            , m_players(players)
            , m_round(round)
        {
            ResetTimers();
            GameStateManager::GetInstance().RegisterRespawnCallback([this]() { ResetTimers(); });
        }

        ~EnemySpawnerComponent() override = default;

        EnemySpawnerComponent(const EnemySpawnerComponent&) = delete;
        EnemySpawnerComponent(EnemySpawnerComponent&&) = delete;
        EnemySpawnerComponent& operator=(const EnemySpawnerComponent&) = delete;
        EnemySpawnerComponent& operator=(EnemySpawnerComponent&&) = delete;

        void Update(float deltaTime) override
        {
            if (GameStateManager::GetInstance().IsFrozen()) return;

            TickCoily(deltaTime);
            TickUggWrongway(deltaTime);
            TickSlickSam(deltaTime);
        }

    private:
        bool IsEnabledForRound(const EnemySpawnConfig& cfg) const
        {
            int r = RoundIndex();
            if (r < static_cast<int>(cfg.enabledPerRound.size()))
                return cfg.enabledPerRound[r];
            return false;
        }

        void ResetTimers()
        {
            if (IsEnabledForRound(m_levelData.coily))
                m_coilyTimer = m_levelData.coily.firstSpawnDelay;
            else
                m_coilyTimer = -1.f;

            m_coilyAlive = false;

            if (IsEnabledForRound(m_levelData.uggWrongway))
                m_uggTimer = m_levelData.uggWrongway.firstSpawnDelay;
            else
                m_uggTimer = -1.f;

            if (IsEnabledForRound(m_levelData.slickSam))
                m_ssTimer = m_levelData.slickSam.firstSpawnDelay;
            else
                m_ssTimer = -1.f;
        }

        int RoundIndex() const
        {
            int count = static_cast<int>(m_levelData.roundColorColumns.size());
            return m_round % (count > 0 ? count : 1);
        }

        float RandomInterval(const std::vector<float>& mins, const std::vector<float>& maxs) const
        {
            int r = RoundIndex();
            float mn = (r < static_cast<int>(mins.size())) ? mins[r] : 3.f;
            float mx = (r < static_cast<int>(maxs.size())) ? maxs[r] : 6.f;
            if (mx <= mn) return mn;
            float t = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            return mn + t * (mx - mn);
        }

        float HopInterval(const std::vector<float>& hops) const
        {
            int r = RoundIndex();
            return (r < static_cast<int>(hops.size())) ? hops[r] : 1.f;
        }

        QbertPlayerComponent* GetFirstPlayer() const
        {
            return m_players.empty() ? nullptr : m_players[0];
        }

        void TickCoily(float deltaTime)
        {
            if (m_coilyTimer < 0.f || m_coilyAlive) return;

            m_coilyTimer -= deltaTime;
            if (m_coilyTimer > 0.f) return;

            m_coilyAlive = true;
            m_coilyTimer = 0.f;

            float hopInterval = HopInterval(m_levelData.coily.hopIntervals);
            int spawnRow = m_levelData.coily.spawnRow;
            // Random spawn column: either the json spawnCol or spawnCol+1
            int spawnCol = m_levelData.coily.spawnCol + (rand() % 2);

            auto go = std::make_unique<GameObject>();
            // Position is set by the component on first Update (intro fall from above)
            go->AddComponent<SpritesheetComponent>("Coily Spritesheet.png", COILY_SRC_W_SPAWN, COILY_SRC_H_SPAWN);
            auto* coily = go->AddComponent<CoilyComponent>(hopInterval, spawnRow, spawnCol);
            coily->SetQbert(GetFirstPlayer());
            coily->SetScene(m_scene);
            coily->SetFreezeDuration(m_levelData.freezeDuration);

            GameObject* coilyGo = go.get();

            EnemyEntry entry;
            entry.type = EnemyEntry::Type::Coily;
            entry.component = coily;
            entry.collisionDRow = 0;
            entry.collisionDCol = 0;
            entry.getRow = [coily]() { return coily->GetGridRow(); };
            entry.getCol = [coily]() { return coily->GetGridCol(); };
            entry.isHopping = [coily]() { return coily->IsHopping(); };
            entry.markForRemoval = [coilyGo, this]()
                {
                    coilyGo->MarkForRemoval();
                    m_coilyAlive = false;
                    m_coilyTimer = RandomInterval(
                        m_levelData.coily.spawnIntervalMin,
                        m_levelData.coily.spawnIntervalMax);
                };

            GameStateManager::GetInstance().RegisterEnemy(std::move(entry));
            m_scene->Add(std::move(go));
        }

        void TickUggWrongway(float deltaTime)
        {
            if (m_uggTimer < 0.f) return;

            m_uggTimer -= deltaTime;
            if (m_uggTimer > 0.f) return;

            m_uggTimer = RandomInterval(
                m_levelData.uggWrongway.spawnIntervalMin,
                m_levelData.uggWrongway.spawnIntervalMax);

            float hopInterval = HopInterval(m_levelData.uggWrongway.hopIntervals);
            int spawnRow = m_levelData.uggWrongway.spawnRow;

            // Randomly spawn either Wrongway (left) or Ugg (right)
            bool spawnLeft = (rand() % 2) == 0;
            int spawnCol = spawnLeft ? 0 : PYRAMID_ROWS - 1;
            SpawnUggWrongway(spawnLeft, spawnRow, spawnCol, hopInterval);
        }

        void SpawnUggWrongway(bool isLeft, int row, int col, float hopInterval)
        {
            auto go = std::make_unique<GameObject>();
            auto* ugg = go->AddComponent<UggWrongwayComponent>(isLeft, hopInterval, row, col);
            go->AddComponent<SpritesheetComponent>("Ugg Wrongway Spritesheet.png", UGG_SRC_W_SPAWN, UGG_SRC_H_SPAWN);
            // Position is set by the component on first Update (intro fall from off-screen)
            ugg->SetQbert(GetFirstPlayer());
            ugg->SetScene(m_scene);
            ugg->SetFreezeDuration(m_levelData.freezeDuration);

            EnemyEntry entry;
            entry.type = EnemyEntry::Type::UggWrongway;
            entry.component = ugg;
            entry.collisionDRow = ugg->GetCollisionDRow();
            entry.collisionDCol = ugg->GetCollisionDCol();
            entry.getRow = [ugg]() { return ugg->GetGridRow(); };
            entry.getCol = [ugg]() { return ugg->GetGridCol(); };
            entry.isHopping = [ugg]() { return ugg->IsHopping(); };
            GameObject* uggGo = go.get();
            entry.markForRemoval = [uggGo]() { uggGo->MarkForRemoval(); };

            GameStateManager::GetInstance().RegisterEnemy(std::move(entry));
            m_scene->Add(std::move(go));
        }

        void TickSlickSam(float deltaTime)
        {
            if (m_ssTimer < 0.f) return;

            m_ssTimer -= deltaTime;
            if (m_ssTimer > 0.f) return;

            m_ssTimer = RandomInterval(
                m_levelData.slickSam.spawnIntervalMin,
                m_levelData.slickSam.spawnIntervalMax);

            float hopInterval = HopInterval(m_levelData.slickSam.hopIntervals);
            int spawnRow = m_levelData.slickSam.spawnRow;

            // Randomly spawn either Slick (left, col 0) or Sam (right, col 1)
            bool spawnLeft = (rand() % 2) == 0;
            bool isSlick = spawnLeft;
            int spawnCol = spawnLeft ? 0 : 1;
            SpawnSlickSam(isSlick, spawnLeft, spawnRow, spawnCol, hopInterval);
        }

        void SpawnSlickSam(bool isSlick, bool isLeft, int row, int col, float hopInterval)
        {
            auto go = std::make_unique<GameObject>();
            go->AddComponent<SpritesheetComponent>("Slick Sam Spritesheet.png", SS_SRC_W_SPAWN, SS_SRC_H_SPAWN);
            auto* ss = go->AddComponent<SlickSamComponent>(isSlick, isLeft, hopInterval, row, col);
            ss->SetPyramidGrid(m_grid);
            ss->SetQbert(GetFirstPlayer());
            ss->SetScene(m_scene);
            ss->SetFreezeDuration(m_levelData.freezeDuration);
            // Position is set by the component on first Update (intro fall from above)

            EnemyEntry entry;
            entry.type = EnemyEntry::Type::SlickSam;
            entry.component = ss;
            entry.collisionDRow = 0;
            entry.collisionDCol = 0;
            entry.getRow = [ss]() { return ss->GetGridRow(); };
            entry.getCol = [ss]() { return ss->GetGridCol(); };
            entry.isHopping = [ss]() { return ss->IsHopping(); };
            GameObject* ssGo = go.get();
            entry.markForRemoval = [ssGo]() { ssGo->MarkForRemoval(); };

            GameStateManager::GetInstance().RegisterEnemy(std::move(entry));
            m_scene->Add(std::move(go));
        }

        LevelData m_levelData;
        PyramidGrid* m_grid;
        Scene* m_scene;
        std::vector<QbertPlayerComponent*> m_players;
        int m_round;

        float m_coilyTimer{ 0.f };
        bool m_coilyAlive{ false };
        float m_uggTimer{ 0.f };
        float m_ssTimer{ 0.f };
    };
}