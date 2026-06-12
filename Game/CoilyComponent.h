#pragma once
#include "Component.h"
#include "GameObject.h"
#include "CoilyState.h"
#include "SpritesheetComponent.h"
#include "QbertPyramid.h"
#include "QbertPlayerComponent.h"
#include <memory>
#include <cmath>

namespace dae
{
    static constexpr float COILY_ARC_HEIGHT = 12.f * PIXEL_SCALE;

    class Scene;

    class CoilyComponent final : public Component
    {
    public:
        CoilyComponent(GameObject* pOwner, float hopInterval = 0.5f, int spawnRow = 0, int spawnCol = 0)
            : Component(pOwner)
            , m_state(std::make_unique<CoilyEggState>(hopInterval))
            , m_hopInterval(hopInterval)
            , m_gridRow(spawnRow)
            , m_gridCol(spawnCol)
        {
        }

        ~CoilyComponent() override = default;

        CoilyComponent(const CoilyComponent&) = delete;
        CoilyComponent(CoilyComponent&&) = delete;
        CoilyComponent& operator=(const CoilyComponent&) = delete;
        CoilyComponent& operator=(CoilyComponent&&) = delete;

        void SetQbert(QbertPlayerComponent* qbert) { m_qbert = qbert; }
        void SetScene(Scene* scene) { m_scene = scene; }
        void SetFreezeDuration(float d) { m_freezeDuration = d; }

        void Update(float deltaTime) override;

        void BeginHop(int destRow, int destCol, float hopInterval)
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            int srcW = sheet ? sheet->GetFrameWidth() : 16;
            int srcH = sheet ? sheet->GetFrameHeight() : 32;
            m_fromPos = GridToCharacterPos(m_gridRow, m_gridCol, srcW, srcH);
            m_toPos = GridToCharacterPos(destRow, destCol, srcW, srcH);
            m_destRow = destRow;
            m_destCol = destCol;
            m_hopDuration = hopInterval * 0.5f;
            m_hopPhase = 0.f;
            m_hopping = true;
        }

        bool IsHopping() const { return m_introFalling || m_hopping; }
        int GetGridRow() const { return m_gridRow; }
        int GetGridCol() const { return m_gridCol; }
        bool IsEgg() const { return m_state->IsEgg(); }
        float GetHopInterval() const { return m_hopInterval; }

        int GetTargetRow() const
        {
            if (m_hasDiscTarget) return m_discNeighbourRow;
            return m_qbert ? m_qbert->GetGridRow() : 0;
        }
        int GetTargetCol() const
        {
            if (m_hasDiscTarget) return m_discNeighbourCol;
            return m_qbert ? m_qbert->GetGridCol() : 0;
        }

        void ForceJumpOff();

        void SetDiscTarget(int discRow, int discCol)
        {
            m_hasDiscTarget = true;
            m_discTileRow = discRow;
            m_discTileCol = discCol;

            if (discCol < 0)
            {
                // Left-side disc at [discRow][-1]
                m_discNeighbourRow = discRow + 1;
                m_discNeighbourCol = 0;
                m_discFinalDRow = (discRow >= 0) ? 0 : -1;
                m_discFinalDCol = -1;
            }
            else
            {
                // Right-side disc at [discRow][discRow+1]
                m_discNeighbourRow = discRow + 1;
                m_discNeighbourCol = discCol;
                m_discFinalDRow = (discRow >= 0) ? 0 : -1;
                m_discFinalDCol = 1;
            }
        }

        // True while coily is doing the disc-chase fall; must not be removed by FinishDiscRide
        bool IsDoingDiscChase() const { return m_awardDiscPointsOnFall; }

    private:
        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            float arcY = -COILY_ARC_HEIGHT * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_state->GetCol(m_hopping), 0);
        }

        void BeginFallOff(int dRow, int dCol);

        QbertPlayerComponent* m_qbert{ nullptr };
        Scene* m_scene{ nullptr };
        float m_freezeDuration{ 1.5f };

        std::unique_ptr<CoilyBaseState> m_state;
        float m_hopInterval;
        int m_gridRow;
        int m_gridCol;

        bool m_hopping{ false };
        float m_hopPhase{ 0.f };
        float m_hopDuration{ 0.25f };
        glm::vec2 m_fromPos{ 0.f, 0.f };
        glm::vec2 m_toPos{ 0.f, 0.f };
        int m_destRow{ 0 };
        int m_destCol{ 0 };

        bool m_introFalling{ true };
        bool m_introInitialized{ false };
        glm::vec2 m_introFrom{ 0.f, 0.f };
        glm::vec2 m_introTo{ 0.f, 0.f };
        float m_introSpeed{ 400.f };
        float m_introProgress{ 0.f };
        float m_introLength{ 1.f };

        bool m_fallingOff{ false };
        bool m_awardDiscPointsOnFall{ false };
        glm::vec2 m_fallPos{ 0.f, 0.f };
        float m_fallSpeed{ 0.f };

        bool m_hasDiscTarget{ false };
        int m_discTileRow{ 0 };
        int m_discTileCol{ 0 };
        int m_discNeighbourRow{ 0 };
        int m_discNeighbourCol{ 0 };
        int m_discFinalDRow{ 0 };
        int m_discFinalDCol{ 0 };
    };
}