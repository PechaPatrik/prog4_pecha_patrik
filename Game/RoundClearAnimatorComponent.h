#pragma once
#include "Component.h"
#include "GameStateManager.h"
#include "QbertPyramid.h"
#include "SpritesheetComponent.h"
#include "ServiceLocator.h"
#include "SoundId.h"

namespace dae
{
    // Cycles all cube sprites through rows 0->1->2->0->... while IsRoundClear() is true.
    class RoundClearAnimatorComponent final : public Component
    {
    public:
        RoundClearAnimatorComponent(GameObject* pOwner, const PyramidGrid* grid,
            int colorColumn, float frameInterval)
            : Component(pOwner)
            , m_colorColumn(colorColumn)
            , m_frameInterval(frameInterval > 0.f ? frameInterval : 0.001f)
        {
            if (!grid) return;
            int numRows = grid->NumRows();
            for (int r = 0; r < numRows; ++r)
            {
                int width = grid->rowWidths[r];
                for (int i = 0; i < width; ++i)
                {
                    GameObject* go = grid->cubes[r][i];
                    if (!go) continue;
                    auto* sheet = go->GetComponent<SpritesheetComponent>();
                    if (sheet)
                        m_sheets.push_back(sheet);
                }
            }
        }

        ~RoundClearAnimatorComponent() override = default;

        RoundClearAnimatorComponent(const RoundClearAnimatorComponent&) = delete;
        RoundClearAnimatorComponent(RoundClearAnimatorComponent&&) = delete;
        RoundClearAnimatorComponent& operator=(const RoundClearAnimatorComponent&) = delete;
        RoundClearAnimatorComponent& operator=(RoundClearAnimatorComponent&&) = delete;

        void Update(float deltaTime) override
        {
            if (!GameStateManager::GetInstance().IsRoundClear()) return;

            if (!m_soundPlayed)
            {
                m_soundPlayed = true;
                ServiceLocator::GetSoundSystem().PlaySound(SoundId::RoundCompleteTune);
            }

            m_frameTimer += deltaTime;
            if (m_frameTimer < m_frameInterval) return;
            m_frameTimer -= m_frameInterval;

            m_currentRow = (m_currentRow + 1) % 3;
            for (auto* sheet : m_sheets)
                sheet->SetFrame(m_colorColumn, m_currentRow);
        }

    private:
        std::vector<SpritesheetComponent*> m_sheets;
        int m_colorColumn;
        float m_frameInterval;
        float m_frameTimer{ 0.f };
        int m_currentRow{ 2 };
        bool m_soundPlayed{ false };
    };
}