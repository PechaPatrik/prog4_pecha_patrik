#pragma once
#include "Singleton.h"
#include "QbertPyramid.h"
#include <vector>
#include <functional>

namespace dae
{
    class QbertPlayerComponent;
    class CoilyComponent;
    class UggWrongwayComponent;
    class SlickSamComponent;
    class Scene;
    class GameObject;

    struct EnemyEntry
    {
        enum class Type { Coily, UggWrongway, SlickSam };
        Type type;
        void* component;
        // collision tile offset relative to the enemy's own grid position
        // for Coily/Slick/Sam this is (0,0), for Ugg/Wrongway it is set at registration
        int collisionDRow;
        int collisionDCol;
        // callback to get current grid position
        std::function<int()> getRow;
        std::function<int()> getCol;
        // is the enemy currently hopping (not grounded)
        std::function<bool()> isHopping;
        // mark it for removal (Slick/Sam on contact)
        std::function<void()> markForRemoval;
    };

    class GameStateManager final : public Singleton<GameStateManager>
    {
    public:
        void Reset();

        void RegisterRespawnCallback(std::function<void()> cb) { m_respawnCallbacks.push_back(std::move(cb)); }

        void RegisterPlayer(QbertPlayerComponent* player);
        void UnregisterPlayer(QbertPlayerComponent* player);
        const std::vector<QbertPlayerComponent*>& GetPlayers() const { return m_players; }

        void RegisterEnemy(EnemyEntry entry);
        void UnregisterAllEnemies();
        std::vector<EnemyEntry>& GetEnemies() { return m_enemies; }

        bool IsFrozen() const { return m_frozen; }
        // called by a player component when it lands and should trigger death sequence
        // respawnWorldPos is where to put the player after the freeze
        void TriggerPlayerDeath(QbertPlayerComponent* dyingPlayer, Scene* scene,
            float cursePosX, float cursePosY, float freezeDuration);

        void Update(float deltaTime, Scene* scene);

        // collision check called by each entity after landing
        void CheckEnemyLandedAt(int row, int col, EnemyEntry& entry, Scene* scene, float freezeDuration);
        void CheckPlayerLandedAt(QbertPlayerComponent* player, int row, int col,
            Scene* scene, float freezeDuration);

    private:
        friend class Singleton<GameStateManager>;
        GameStateManager() = default;

        std::vector<QbertPlayerComponent*> m_players;
        std::vector<EnemyEntry> m_enemies;

        bool m_frozen{ false };
        float m_freezeTimer{ 0.f };
        float m_freezeDuration{ 1.f };

        // curse image game object raw pointer, owned by scene
        // we keep a pointer only to remove it after the freeze
        GameObject* m_curseGameObject{ nullptr };
        QbertPlayerComponent* m_dyingPlayer{ nullptr };
        std::vector<std::function<void()>> m_respawnCallbacks;
    };
}