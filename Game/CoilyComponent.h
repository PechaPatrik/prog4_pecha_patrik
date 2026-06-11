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

        // Called by state. Duration is the air-phase duration (half the total hop interval).
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

        int GetTargetRow() const { return m_qbert ? m_qbert->GetGridRow() : 0; }
        int GetTargetCol() const { return m_qbert ? m_qbert->GetGridCol() : 0; }

    private:
        void ApplyArcPosition(float t)
        {
            float x = m_fromPos.x + (m_toPos.x - m_fromPos.x) * t;
            float y = m_fromPos.y + (m_toPos.y - m_fromPos.y) * t;
            // sin arc: 0 at t=0, peak at t=0.5, 0 at t=1
            float arcY = -COILY_ARC_HEIGHT * std::sin(t * 3.14159265f);
            GetOwner()->SetLocalPosition(x, y + arcY);
        }

        void UpdateSprite()
        {
            auto* sheet = GetOwner()->GetComponent<SpritesheetComponent>();
            if (sheet)
                sheet->SetFrame(m_state->GetCol(m_hopping), 0);
        }

        QbertPlayerComponent* m_qbert{ nullptr };
        Scene* m_scene{ nullptr };
        float m_freezeDuration{ 1.f };

        std::unique_ptr<CoilyBaseState> m_state;
        int m_gridRow;
        int m_gridCol;

        bool m_hopping{ false };
        float m_hopPhase{ 0.f };
        float m_hopDuration{ 0.25f };
        glm::vec2 m_fromPos{ 0.f, 0.f };
        glm::vec2 m_toPos{ 0.f, 0.f };
        int m_destRow{ 0 };
        int m_destCol{ 0 };

        // Intro fall from above to spawn tile
        bool m_introFalling{ true };
        bool m_introInitialized{ false };
        glm::vec2 m_introFrom{ 0.f, 0.f };
        glm::vec2 m_introTo{ 0.f, 0.f };
        float m_introSpeed{ 400.f };
        float m_introProgress{ 0.f };
        float m_introLength{ 1.f };
    };
}