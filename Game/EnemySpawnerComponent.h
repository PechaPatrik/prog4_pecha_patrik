#pragma once
#include "Component.h"
#include "GameObject.h"
#include "LevelData.h"
#include "QbertPyramid.h"
#include "GameStateManager.h"
#include "CoilyComponent.h"
#include "UggWrongwayComponent.h"
#include "SlickSamComponent.h"
#include "DiscComponent.h"
#include "SpritesheetComponent.h"
#include "Scene.h"
#include <cstdlib>
#include <cmath>
#include <vector>
#include <utility>

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
            const GameConfig& gameConfig,
            PyramidGrid* grid, Scene* scene,
            const std::vector<QbertPlayerComponent*>& players,
            int round = 0)
            : Component(pOwner)
            , m_levelData(levelData)
            , m_gameConfig(gameConfig)
            , m_grid(grid)
            , m_scene(scene)
            , m_players(players)
            , m_round(round)
        {
            ResetTimers();
            SpawnDiscs();
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
            if (GameStateManager::GetInstance().IsDiscRiding()) return;

            TickCoily(deltaTime);
            TickUgg(deltaTime);
            TickWrongway(deltaTime);
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
            int r = RoundIndex();

            if (IsEnabledForRound(m_levelData.coily))
                m_coilyTimer = FirstDelay(m_levelData.coily, r);
            else
                m_coilyTimer = -1.f;

            m_coilyAlive = false;

            if (IsEnabledForRound(m_levelData.ugg))
                m_uggTimer = FirstDelay(m_levelData.ugg, r);
            else
                m_uggTimer = -1.f;

            if (IsEnabledForRound(m_levelData.wrongway))
                m_wrongwayTimer = FirstDelay(m_levelData.wrongway, r);
            else
                m_wrongwayTimer = -1.f;

            if (IsEnabledForRound(m_levelData.slickSam))
                m_ssTimer = FirstDelay(m_levelData.slickSam, r);
            else
                m_ssTimer = -1.f;
        }

        float FirstDelay(const EnemySpawnConfig& cfg, int r) const
        {
            if (r < static_cast<int>(cfg.firstSpawnDelayPerRound.size()))
                return cfg.firstSpawnDelayPerRound[r];
            return 5.f;
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

        float WaitAtBottom(const EnemySpawnConfig& cfg) const
        {
            int r = RoundIndex();
            if (r < static_cast<int>(cfg.waitAtBottomPerRound.size()))
                return cfg.waitAtBottomPerRound[r];
            return 0.f;
        }

        std::pair<int, int> RandomSpawnLocation(const EnemySpawnConfig& cfg) const
        {
            if (cfg.spawnLocations.empty())
            {
                int lastRow = m_grid ? m_grid->NumRows() - 1 : 6;
                return { lastRow, 0 };
            }
            int idx = rand() % static_cast<int>(cfg.spawnLocations.size());
            return cfg.spawnLocations[idx];
        }

        QbertPlayerComponent* GetFirstPlayer() const
        {
            return m_players.empty() ? nullptr : m_players[0];
        }

        void ApplyCommonEnemyConfig(CoilyComponent* c) const
        {
            c->SetArcHeight(m_gameConfig.arcHeight);
            c->SetFallGravity(m_gameConfig.enemyFallGravity);
            c->SetIntroSpeed(m_levelData.enemyIntroSpeed);
            c->SetPyramidGrid(m_grid);
        }

        void ApplyCommonEnemyConfig(UggWrongwayComponent* u) const
        {
            u->SetArcHeight(m_gameConfig.arcHeight);
            u->SetFallGravity(m_gameConfig.enemyFallGravity);
            u->SetIntroSpeed(m_levelData.enemyIntroSpeed);
            u->SetPyramidGrid(m_grid);
        }

        void ApplyCommonEnemyConfig(SlickSamComponent* s) const
        {
            s->SetArcHeight(m_gameConfig.arcHeight);
            s->SetFallGravity(m_gameConfig.enemyFallGravity);
            s->SetIntroSpeed(m_levelData.enemyIntroSpeed);
            s->SetPyramidGrid(m_grid);
        }

        void SpawnDiscs()
        {
            int r = RoundIndex();
            int discCount = (r < static_cast<int>(m_levelData.discCountsPerRound.size()))
                ? m_levelData.discCountsPerRound[r] : 2;

            int colorGroup = (r < static_cast<int>(m_levelData.roundColorColumns.size()))
                ? m_levelData.roundColorColumns[r] : 0;

            std::vector<std::pair<int, int>> candidates;
            for (int row = -1; row <= 5; ++row)
            {
                candidates.emplace_back(row, -1);
                candidates.emplace_back(row, row + 1);
            }

            for (int i = static_cast<int>(candidates.size()) - 1; i > 0; --i)
            {
                int j = rand() % (i + 1);
                std::swap(candidates[i], candidates[j]);
            }

            int placed = 0;
            for (auto& [dRow, dCol] : candidates)
            {
                if (placed >= discCount) break;

                auto go = std::make_unique<GameObject>();
                glm::vec2 pos = m_grid
                    ? DiscWorldPos(dRow, dCol)
                    : glm::vec2{ 0.f, 0.f };
                go->SetLocalPosition(pos.x, pos.y);
                go->AddComponent<SpritesheetComponent>("Disk Spritesheet.png", DISC_SRC_W, DISC_SRC_H);

                auto* disc = go->AddComponent<DiscComponent>(
                    dRow, dCol, colorGroup,
                    m_gameConfig.discFlightDuration,
                    m_gameConfig.pointsCoilyDisc,
                    m_gameConfig.freezeDuration,
                    m_gameConfig.discFrameDuration,
                    m_gameConfig.discDropDuration);
                disc->SetScene(m_scene);
                disc->SetPyramidGrid(m_grid);

                for (auto* player : m_players)
                    if (player) player->RegisterDisc(disc);

                m_scene->Add(std::move(go));
                ++placed;
            }
        }

        void TickCoily(float deltaTime)
        {
            if (m_coilyTimer < 0.f || m_coilyAlive) return;

            m_coilyTimer -= deltaTime;
            if (m_coilyTimer > 0.f) return;

            m_coilyAlive = true;
            m_coilyTimer = 0.f;

            float hopInterval = HopInterval(m_levelData.coily.hopIntervals);
            float waitAtBottom = WaitAtBottom(m_levelData.coily);
            auto [spawnRow, spawnCol] = RandomSpawnLocation(m_levelData.coily);

            auto go = std::make_unique<GameObject>();
            go->AddComponent<SpritesheetComponent>("Coily Spritesheet.png", COILY_SRC_W_SPAWN, COILY_SRC_H_SPAWN);
            auto* coily = go->AddComponent<CoilyComponent>(hopInterval, spawnRow, spawnCol, waitAtBottom);
            coily->SetQbert(GetFirstPlayer());
            coily->SetScene(m_scene);
            coily->SetFreezeDuration(m_gameConfig.freezeDuration);
            ApplyCommonEnemyConfig(coily);

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
            entry.freezeDuration = m_gameConfig.freezeDuration;

            GameStateManager::GetInstance().RegisterEnemy(std::move(entry));
            m_scene->Add(std::move(go));
        }

        void TickUgg(float deltaTime)
        {
            if (m_uggTimer < 0.f) return;

            m_uggTimer -= deltaTime;
            if (m_uggTimer > 0.f) return;

            m_uggTimer = RandomInterval(
                m_levelData.ugg.spawnIntervalMin,
                m_levelData.ugg.spawnIntervalMax);

            float hopInterval = HopInterval(m_levelData.ugg.hopIntervals);
            auto [spawnRow, spawnCol] = RandomSpawnLocation(m_levelData.ugg);
            SpawnUggWrongway(false, spawnRow, spawnCol, hopInterval);
        }

        void TickWrongway(float deltaTime)
        {
            if (m_wrongwayTimer < 0.f) return;

            m_wrongwayTimer -= deltaTime;
            if (m_wrongwayTimer > 0.f) return;

            m_wrongwayTimer = RandomInterval(
                m_levelData.wrongway.spawnIntervalMin,
                m_levelData.wrongway.spawnIntervalMax);

            float hopInterval = HopInterval(m_levelData.wrongway.hopIntervals);
            auto [spawnRow, spawnCol] = RandomSpawnLocation(m_levelData.wrongway);
            SpawnUggWrongway(true, spawnRow, spawnCol, hopInterval);
        }

        void SpawnUggWrongway(bool isLeft, int row, int col, float hopInterval)
        {
            auto go = std::make_unique<GameObject>();
            auto* ugg = go->AddComponent<UggWrongwayComponent>(isLeft, hopInterval, row, col);
            go->AddComponent<SpritesheetComponent>("Ugg Wrongway Spritesheet.png", UGG_SRC_W_SPAWN, UGG_SRC_H_SPAWN);
            ugg->SetQbert(GetFirstPlayer());
            ugg->SetScene(m_scene);
            ugg->SetFreezeDuration(m_gameConfig.freezeDuration);
            ApplyCommonEnemyConfig(ugg);

            EnemyEntry entry;
            entry.type = isLeft ? EnemyEntry::Type::Wrongway : EnemyEntry::Type::Ugg;
            entry.component = ugg;
            entry.collisionDRow = ugg->GetCollisionDRow();
            entry.collisionDCol = ugg->GetCollisionDCol();
            entry.getRow = [ugg]() { return ugg->GetGridRow(); };
            entry.getCol = [ugg]() { return ugg->GetGridCol(); };
            entry.isHopping = [ugg]() { return ugg->IsHopping(); };
            GameObject* uggGo = go.get();
            entry.markForRemoval = [uggGo]() { uggGo->MarkForRemoval(); };
            entry.triggerFall = [ugg]() { ugg->TriggerFall(); };
            entry.freezeDuration = m_gameConfig.freezeDuration;

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
            auto [spawnRow, spawnCol] = RandomSpawnLocation(m_levelData.slickSam);
            bool isSlick = (rand() % 2) == 0;
            SpawnSlickSam(isSlick, spawnRow, spawnCol, hopInterval);
        }

        void SpawnSlickSam(bool isSlick, int row, int col, float hopInterval)
        {
            auto go = std::make_unique<GameObject>();
            go->AddComponent<SpritesheetComponent>("Slick Sam Spritesheet.png", SS_SRC_W_SPAWN, SS_SRC_H_SPAWN);
            auto* ss = go->AddComponent<SlickSamComponent>(isSlick, hopInterval, row, col);
            ss->SetPyramidGrid(m_grid);
            ss->SetQbert(GetFirstPlayer());
            ss->SetScene(m_scene);
            ss->SetFreezeDuration(m_gameConfig.freezeDuration);
            ApplyCommonEnemyConfig(ss);

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
            entry.triggerFall = [ss]() { ss->TriggerFall(); };
            entry.freezeDuration = m_gameConfig.freezeDuration;

            GameStateManager::GetInstance().RegisterEnemy(std::move(entry));
            m_scene->Add(std::move(go));
        }

        LevelData m_levelData;
        GameConfig m_gameConfig;
        PyramidGrid* m_grid;
        Scene* m_scene;
        std::vector<QbertPlayerComponent*> m_players;
        int m_round;

        float m_coilyTimer{ 0.f };
        bool m_coilyAlive{ false };
        float m_uggTimer{ 0.f };
        float m_wrongwayTimer{ 0.f };
        float m_ssTimer{ 0.f };
    };
}