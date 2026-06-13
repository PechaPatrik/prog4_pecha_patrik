#pragma once
#include "Singleton.h"
#include "QbertPyramid.h"
#include <vector>
#include <functional>

namespace dae
{
    class QbertPlayerComponent;
    class CoilyComponent;
    class Scene;
    class GameObject;

    struct EnemyEntry
    {
        enum class Type { Coily, Ugg, Wrongway, SlickSam };
        Type type{};
        void* component{};
        int collisionDRow{};
        int collisionDCol{};
        std::function<int()> getRow;
        std::function<int()> getCol;
        std::function<bool()> isHopping;
        std::function<void()> markForRemoval;
        std::function<void()> triggerFall;
        float freezeDuration{ 1.5f };
        bool markedForRemoval{ false };
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
        bool IsDiscRiding() const { return m_discRiding; }
        bool IsRoundClear() const { return m_roundClear; }
        bool IsBonusScreen() const { return m_bonusScreen; }
        int GetBonusPoints() const { return m_bonusPoints; }

        void SetCurseOffset(float x, float y) { m_curseOffsetX = x; m_curseOffsetY = y; }

        void TriggerPlayerDeath(QbertPlayerComponent* dyingPlayer, Scene* scene,
            float cursePosX, float cursePosY, float freezeDuration);

        void TriggerDiscRide(QbertPlayerComponent* rider, Scene* scene,
            int discRow, int discCol, float flightDuration,
            int pointsCoilyDisc, float freezeDuration, float discDropDuration);

        void TriggerRoundClear(float duration, int bonusPoints, float bonusDisplayDuration,
            std::function<void()> advanceCallback);

        void OnCoilyFellDuringDisc(int coilyRow, int coilyCol) const;

        void Update(float deltaTime, Scene* scene);

        void CheckEnemyLandedAt(int row, int col, EnemyEntry& entry, Scene* scene, float freezeDuration);
        void CheckPlayerLandedAt(QbertPlayerComponent* player, int row, int col,
            Scene* scene, float freezeDuration);

        bool CoilyCanReachDisc(int discRow, int discCol, float flightDuration) const;

    private:
        friend class Singleton<GameStateManager>;
        GameStateManager() = default;

        void FinishDiscRide(Scene* scene);
        void DismissNonCoilyEnemies();
        void BeginBonusPhase();

        std::vector<QbertPlayerComponent*> m_players;
        std::vector<EnemyEntry> m_enemies;

        bool m_frozen{ false };
        float m_freezeTimer{ 0.f };
        float m_freezeDuration{ 1.5f };
        float m_curseOffsetX{ 12.f };
        float m_curseOffsetY{ 24.f };
        GameObject* m_curseGameObject{ nullptr };
        QbertPlayerComponent* m_dyingPlayer{ nullptr };
        std::vector<std::function<void()>> m_respawnCallbacks;

        bool m_roundClear{ false };

        bool m_bonusScreen{ false };
        int m_bonusPoints{ 0 };
        float m_bonusDisplayDuration{ 2.f };
        std::function<void()> m_advanceCallback;

        bool m_discRiding{ false };
        float m_discRideTimer{ 0.f };
        float m_discRideDuration{ 2.f };
        QbertPlayerComponent* m_discRider{ nullptr };
        int m_pointsCoilyDisc{ 500 };
        float m_discFreezeDuration{ 1.5f };
        float m_discDropDuration{ 0.25f };
        Scene* m_discScene{ nullptr };
    };
}